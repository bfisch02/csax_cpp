
# R functions for running the compiled C/C++ version of FRaC in this directory

FRaC.DEBUG <<- "/dev/null"

# Run FRaC
# 
# training.set: rows are genes, columns are samples
# test.set:     rows are the same genes, columns are different samples
# save.dir:     if a path is given, write ns (compressed) to it
#
frac <- function(training.set, test.set, save.dir=NULL, frac.exe='frac/frac', frac.stderr='/dev/stderr') { 

	if (!file.exists(frac.exe)) { stop(sprintf("Missing %s (did you run make?)", frac.exe)) } 

	# First, check if FRaC output file has a location and already exists (with the correct names and dimensions)
	if (!is.null(save.dir)) { 

		ns.gzfile <- sprintf("%s/ns.gz", chkdir(save.dir))
		if (file.exists(ns.gzfile)) {
			
			ns <- read.table(gzfile(ns.gzfile), sep="\t") 
			
			if (!(nrow(ns)==nrow(test.set) && ncol(ns)==ncol(test.set))) {  # dimensions fail to check out
				warning(sprintf("dimensions (%s) %s do not match those of test set (%s)", paste(dim(ns),collapse='x'), ns.gzfile, paste(dim(test.set),collapse='x')))
				rm(ns)

			} else { 
	
				# Now ensure that the names are reset (some FRaC output files were created without row or column names)
				if (all(row.names(ns) == 1:nrow(ns))  &&  all(colnames(ns) == paste("V",1:ncol(ns),sep=""))) { 
					# "default" row and column names
					names(ns) <- colnames(ns) <- names(test.set)
					row.names(ns) <- row.names(test.set)
				}

				if (!(all(row.names(ns)==row.names(test.set)) && all(colnames(ns)==colnames(test.set)))) { # names fail to check out
					warning(sprintf("row/column names of %s do not match those of test set", ns.gzfile))
					rm(ns)
				}
			}
		}
	}


	if (exists('ns') && all(dim(ns)==dim(test.set))) {

		message(sprintf("Recovered FRaC output %s", ns.gzfile))

	} else { 

		if (!is.null(save.dir)) { message(sprintf("Run FRaC -> %s",ns.gzfile)); } 

		frac.libsvr.flags <- "-t 0 -c 1 -p 0 " 

		d <- nrow(training.set) # number of features

		X.file <- tempfile()
		Q.file <- tempfile()
		output.table.file <- tempfile()
		
		write.table(t(training.set), row.names=FALSE, col.names=FALSE, quote=FALSE, sep='\t', file=X.file)
		write.table(t(test.set), row.names=FALSE, col.names=FALSE, quote=FALSE, sep='\t', file=Q.file)

		# (stderr prints progress)
		cmd <- sprintf("%s -X %s -Q %s %s 1>%s 2>%s", frac.exe, X.file, Q.file, frac.libsvr.flags, output.table.file, frac.stderr)

		# DEBUG # message(sprintf("Run FRaC:  %s", cmd))
		system(cmd)
		
		write(sprintf("Done.  Output file (anomaly scores for genes\\test instances) is %s", output.table.file), file=FRaC.DEBUG)
		unlink(X.file)
		unlink(Q.file)

		ns <- read.table(output.table.file)
		names(ns) <- colnames(ns) <- colnames(test.set)
		row.names(ns) <- row.names(test.set) 
		unlink(output.table.file) 

		if (!all(dim(ns)==dim(test.set))) { stop(sprintf("FRaC failed (%s!=%s).", paste(dim(ns),collapse='x'), paste(dim(test.set),collapse='x'))) }

		if (!is.null(save.dir)) { 
			write.table(ns, row.names=TRUE, col.names=TRUE, quote=FALSE, sep='\t',  file=gzfile(ns.gzfile))
		}

	}

	return(ns)

}

