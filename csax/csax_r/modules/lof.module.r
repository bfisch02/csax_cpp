#!/usr/bin/env Rscript

# Run LOF.  Output scores correspond to training set samples if test set is NOT
# given, otherwise they refer to test set samples.
#
# training.set:  data frame where rows are features, columns are samples
# test.set:   data frame where rows are features, columns are samples (may be NULL, if test.out is NULL)
# test.out:   filename to write test set LOF (may be NULL)
# lof.exe:    path to executable (there are multiple versions of LOF, like lof.l1norm)
#
lof <- function( training.set, test.set, lof.exe ) { 

	if (!file.exists(lof.exe)) { stop(sprintf("Missing %s (did you run make?)", lof.exe)) }

	MinPtsLB = 10;
	MinPtsUB = min(100, ncol(training.set)-1)

	input.file <- sprintf("%s/lof.input", tempdir())

	write.table( t(training.set), col.names=FALSE, row.names=FALSE, na="0", quote=FALSE, file=input.file, sep=',', append=FALSE )
	if (!is.null(test.set)) { 
		write.table( t(test.set),  col.names=FALSE, row.names=FALSE, na="0", quote=FALSE, file=input.file, sep=',', append=TRUE  )
	}

	# Create shell command
	# Usage:  .../lof <MinPtsLB> <MinPtsUB> <No.Features> <training.set.size> [<Test.set.size>]  <  <comma-separated-input>

	test.set.size <- if (is.null(test.set)) { "" } else { sprintf("%s", ncol(test.set)) }
	
	test.out <- tempfile()
	cmd <- sprintf("%s  %s %s  %s  %s %s  <  %s  1> %s 2>/dev/null", lof.exe, MinPtsLB, MinPtsUB, nrow(training.set), ncol(training.set), ncol(test.set), input.file, test.out)
	system(cmd)
	
	g <- as.numeric(scan(test.out))	 # LOF scores
	names(g) <- colnames(test.set)
	unlink(test.out)
	
	return(g)	

}


