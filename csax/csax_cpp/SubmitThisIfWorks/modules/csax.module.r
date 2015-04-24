# helper functions for CSAX algorithm.




# Create FRaC and GSEA output files
# (see parameters for 'csax')
#
# training.set: matrix
# test.set: matrix
# gsdb: gene set database (with $file, $name, ...)
#
# cache.dir:  create output here as:
#   (for the full training set):        ns.gz, <gsdb.name>/ns.gz, 
#   (for each bag):                     bag.*/test.*.xls[.gz], bag.*/<gsdb.name>/test.*.xls
#
# bags:  which bags, e.g. 1:40
# gamma: discount parameter
# frac.exe: path to executable file
# gsea.jar: path to Java JAR file
# run.full.training.set:  Should we run on the full training set?  This option exists for parallel jobs which may need to skip it (because it needs to be run only once)
#
run.csax <- function(training.set, test.set, gsdb, cache.dir, bags, Q, frac.exe, gsea.jar, tmp.dir, run.full.training.set) {

	if (!file.exists(gsea.jar)) { stop(sprintf("Missing %s",gsea.jar)) }
	if (!file.exists(frac.exe)) { stop(sprintf("Missing %s",frac.exe)) }

	chkdir(cache.dir)

	# First, we have to process the full training data
	if (run.full.training.set) { 

		ns <- frac( training.set, test.set, save.dir=cache.dir, frac.exe=frac.exe, frac.stderr='/dev/null' ) # set to stderr or file for progress updates or debugging 
		#ns <- frac( training.set, test.set, save.dir=cache.dir, frac.exe=frac.exe, frac.stderr='/dev/stderr' ) 
	
		# Run GSEA as well
		gsea.dir <- chkdir(sprintf("%s/%s", cache.dir, gsdb$name))
		hash <- sprintf("%s_%d_%f_%s_%f",gsdb$name,0,sum(training.set)/sum(test.set),format(Sys.time(),'%s'), runif(1, 100, 999))
		gsea( ns, Q, gsdb$file, gsea.dir, gsea.jar, tmp.dir=tmp.dir, hash=hash) # writes <cache>/full/test.<q>.xls.gz

	}
	
	# For each bag run FRaC and GSEA
	for (bag in bags) { 
		if (bag >=1) { 

			bag.dir <- chkdir(sprintf("%s/bag.%d", cache.dir, bag))
			gsea.dir <- chkdir(sprintf("%s/%s", bag.dir, gsdb$name))
			set.seed(bag) # ensure all bags are different from each other, but the same from run to run
			ns <- frac( training.set[,sample(names(training.set), ncol(training.set)/2)], test.set, save.dir=bag.dir, frac.exe=frac.exe, frac.stderr='/dev/null' ) # bagging sample size is 50% of training set
			hash <- sprintf("%s_%d_%f_%s_%f",gsdb$name,bag,sum(training.set)/sum(test.set),format(Sys.time(),'%s'), runif(1, 100, 999))
			gsea( ns, Q, gsdb$file, gsea.dir, gsea.jar, tmp.dir=tmp.dir, hash=hash)

		}

	}

}  # run.csax



# Compute anomaly scores
# (see parameters for 'run.csax')
#
# gamma: discount parameter
#
eval.csax <- function( Q, cache.dir, gsdb, bags, gamma ) { 

	g <- real(0) # anomaly scores

	# for each test set instance, 
	for (q in Q) { 
	
		required.files <- gsea.output.files(q,cache.dir,gsdb$name, bags)
		missing.files <- required.files[ !sapply(required.files,file.exists) ]

		if (length(missing.files)) { 
			stop(sprintf("Missing %d file(s) for test set instance #%d (first is %s)", length(missing.files), q, missing.files[1]))

		} 
	
		R <- compute.ranks(q,cache.dir,gsdb,bags)
		R.stats <- compute.ranks.stats(R, names(gsdb$genes))
		anomaly.score <- csax.anomaly.score( q, cache.dir, gsdb$name, R.stats, gamma )
		
		g <- c(g, anomaly.score) 
	
	}

	return(g)
}


compute.ranks <- function( q, cache.dir, gsdb, bags ) {
	
	# Compute median ranks for each gene set

	# what are the GSEA output tables?
	bag.files <- gsea.bagging.files(q,cache.dir,gsdb$name,bags)

	# Let R be a list, one ordering of some gene sets per bag
	R <- sapply(bag.files, function(filename) { row.names(read.table(filename, sep="\t", header=TRUE, row.names=1)) }) # now we have to compute a median ranking for each gene set

	# Convert to named list including all gene sets
	named.ranks.of.gsdb <- function(ordered.gene.sets) { return(named.ranks( names(gsdb$genes), ordered.gene.sets)) }
	R <- sapply(R, named.ranks.of.gsdb)

	# ...And to a data frame...
	R <- data.frame(t(R))
	
	# R is now a great big table, rows are GSEA output filenames,
	# columns are gene sets, cells are ranks

	#				   FILENAME                            D24729 D24730 D24731 D24732 ...
	#	/r/mad/results/nec5/1/bag.1/DFLAT2/test.1.xls.gz    2870   2870    103   1282  ...
	#	/r/mad/results/nec5/1/bag.2/DFLAT2/test.1.xls.gz    2848   2848    219   1386  ...
	#					  ...                               ...                  ...   ... 

	return(R)

}

# Create a bundle of ranking statistics, used by csax.anomaly.score
compute.ranks.stats <- function(R, gene.sets) { 
	
	# Compute statistics
	median.ranks <- sapply( R , median )
	mean.ranks   <- sapply( R , mean )
	
	# put all gene sets in order
	meta.ranking <- gene.sets[ order( median.ranks, mean.ranks ) ]

	return( list(median.ranks=median.ranks, mean.ranks=mean.ranks, meta.ranking=meta.ranking ) )
}

	
# stats = output of compute.ranks.stats
csax.anomaly.score <- function( q, cache.dir, gsdb.name, stats, gamma ) {

	# We need the enrichment (using full training set) for each gene set
	ES.table <- read.table(gzfile(sprintf("%s/%s/test.%d.xls.gz",cache.dir,gsdb.name,q)), 
	                       header=TRUE, sep="\t", row.names=1)
	ES <- function(gene.set) { 
		e <- ES.table[toupper(gene.set),"ES"]
		return(if(is.na(e)) { 0 } else { e })
	}

	# Now compute the anomaly score
	anomaly.score.contribution <- function(metarank) { 
		gene.set <- stats$meta.ranking[ metarank ]
		return( gamma**(metarank-1) * ES(gene.set) )
	}
	anomaly.score <- sum(sapply(1:length(stats$meta.ranking), anomaly.score.contribution))

	return(anomaly.score)

} # csax.anomaly.score 


# return list of gsea output files (for a specific test set instance) required to compute anomaly score
gsea.output.files <- function(q, cache.dir, gsdb.name, bags) { 
	return( c( sprintf("%s/%s/test.%d.xls.gz", cache.dir, gsdb.name, q) , gsea.bagging.files(q, cache.dir, gsdb.name, bags)))
}

gsea.bagging.files <- function(q,cache.dir,gsdb.name, bags) { 
	if (is.na(bags)) {
		return(sprintf("%s/%s/test.%d.xls.gz", cache.dir, gsdb.name, q))
	} else { 
		# normal case
		return( paste(sprintf("%s/bag.", cache.dir), bags, sprintf("/%s/test.%d.xls.gz", gsdb.name, q), sep="") )
	}
}

# Convert an ordered list of gene set names to a named vector of rank values
# that includes all gene sets.
#
named.ranks <- function(all.gene.sets, ordered.gene.sets) { 
	missing.gene.sets <- setdiff(all.gene.sets, ordered.gene.sets)
	
	# start with a named list of ranks
	ranks <- 1:length(ordered.gene.sets)
	names(ranks) <- ordered.gene.sets
	
	# keep the ranks, but put them in the order of all.gene.sets
	ranks <- ranks[all.gene.sets] 
	names(ranks) <- all.gene.sets

	# now replace missing gene sets with one rank, all tied for last place
	ranks[ is.na(ranks) ] <- length(ordered.gene.sets) + 1

	return(ranks)
}

# Run GSEA on a named vector of gene (anomaly) scores
#
# ns:  FRaC output table (rows are genes, columns are test set instances)
# Q:  Which test set instances to run on? ( e.g., 1:ncol(ns) )
# gsdb.file: path/filename of .gmt file
# output.dir:  where to put test.<q>.xls.gz files
# gsea.jar: path/filename of .jar file
# tmp.dir:  where to store GSEA input files (must not have '-' in path???)
#
# hash: is a unique string for this call.  It's hard to make sure each jobs
# 	puts it's ouput in a unique directory, which is necessary because we have to
# 	count exactly one gsea*pos*xls file to make sure the java process terminated
# 	correctly.
#
gsea <- function( ns, Q, gsdb.file, output.dir, gsea.jar, tmp.dir, hash ) {

	if (!file.nonzero(gsea.jar)) { stop(sprintf("Missing %s",gsea.jar)) }

	for (q in Q) { 

		results.file <- sprintf("%s/test.%d.xls.gz", output.dir, q)
		
		if (file.nonzero(results.file)) { 

			message(sprintf("Recovered GSEA output %s", results.file))

		} else { 
			
			message(sprintf("Run GSEA -> %s", results.file))

			gsea.tmp <- chkdir(sprintf("%s.%s.%d.gsea.tmp", tempfile(tmpdir=tmp.dir), hash, q)) # create a special temp directory for this 

			rank.file <- sprintf("%s/csax.rnk", gsea.tmp)
			rank.table <- data.frame( ns[,q] )
			row.names(rank.table) <- row.names(ns)

			rank.table <- data.frame(na.omit(rank.table)) # this keeps names, but removes all NA (NaN) values (can happen if frac. leaves a NaN value)

			write.table( rank.table,  row.names=row.names(rank.table), col.names=FALSE, sep="\t", quote=FALSE, file=rank.file )

			# Create GSEA command (note it's probably best to redirect stdout to /dev/null, that output isn't very informative, especially when running GSEA many times)
			cmd <- sprintf("java -cp %s -Xmx1g xtools.gsea.GseaPreranked -gmx %s -rnk %s -out %s -rnd_seed 9141976 -rpt_label csax -collapse false -mode Max_probe -norm meandiv -nperm 1000 -scoring_scheme weighted -include_only_symbols true -make_sets false -plot_top_x 0 -set_max 500 -set_min 7 -zip_report false -gui false 1>/dev/null ", gsea.jar, gsdb.file, rank.file, gsea.tmp)
			#debug#message(sprintf("GSEA command:  %s", cmd))
			
			system(cmd)

			# clean up and reorg [GSEA's stupid output file scheme]
			file.remove(rank.file)
			s1 <- list.files(gsea.tmp) # should be one subdir
			s2 <- list.files(sprintf("%s/%s", gsea.tmp, s1)) # should be about a dozen files or so
			s3 <- grep("^gsea_report_for_na_pos_[0-9]*\\.xls", s2) # should be one index

			if(length(s3)!=1) { stop(sprintf("GSEA Failed to create report (gsea.tmp=%s)", gsea.tmp)) }
			if(!file.nonzero( sprintf("%s/%s/%s", gsea.tmp, s1, s2[s3]) )) { stop(sprintf("GSEA created zero-size report (gsea.tmp=%s)", gsea.tmp)) }

			# Copy relevant columns to bzipped results file
			EA <- read.table( file=sprintf("%s/%s/%s", gsea.tmp, s1, s2[s3]), sep="\t", header=TRUE, row.names=1 )
			write.table( EA[, c('ES','NES','NOM.p.val','FDR.q.val','FWER.p.val','RANK.AT.MAX')], sep="\t", row.names=TRUE, col.names=TRUE, quote=FALSE, file=gzfile(results.file) )

			# and clean up
			unlink(gsea.tmp, recursive=TRUE)

		} # if results.file.nonzero

	} # next test set instance

} # main
