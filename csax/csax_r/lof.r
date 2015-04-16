#!/usr/bin/env Rscript

Synopsis <<- "

Run LOF on a microarray anomaly detection task.  See LOF code in lof/.

Input is (i) a training set matrix (genes x samples), and (ii) a test set matrix (same genes x samples).

Ouput LOF scores for each test set instance

"


# the following boilerplate includes modules in the necessary directories
for (src in list.files( paste(dirname(sub('--file=(.*)','\\1',commandArgs()[grep('^--file=',commandArgs())])),c('modules'),sep='/'), full.names=TRUE )) { source(src) }


# select default parameters
main <- function() {

	Usage <- sprintf("%s [-L <distance norm>={1,2}, default 2] <training set file> <test set> <output.file>", rscript.name())
	
	args <- rscript.optparse(rscript.args())
	quitif( !any(length(args)==3:3) || any(grep('^-h',rscript.args())) || any(grep('^--help$',rscript.args())), "\n%s\n\nUsage:  %s\n", Synopsis, Usage )

	# Parse args/option(s)
	norm <- parse.option(args, "L", 2, as.integer)
	if (!any(norm==c(1,2))) { stop(sprintf("norm (%d) must be 1 or 2", norm)) }

	# assume exe is in <script directory>/lof 
	lof.exe <- sprintf("%s/lof/lof.l%d" , dirname(sub('--file=(.*)','\\1',commandArgs()[grep('^--file=',commandArgs())])), norm ) 

	# read data 
	training.set <- read.table(args[1], header=TRUE, row.names=1, sep='\t')
	test.set     <- read.table(args[2], header=TRUE, row.names=1, sep='\t')

	# run LOF
	g <- lof( training.set, test.set, lof.exe )

	write.table(g, quote=FALSE, row.names=FALSE, col.names=FALSE, file=args[3])

} # main


main()


