#!/usr/bin/env Rscript

# the following boilerplate includes modules in the necessary directories
BASE <<- dirname(sub('--file=(.*)','\\1',commandArgs()[grep('^--file=',commandArgs())]))
for (src in list.files(paste(BASE,c('modules'),sep='/'), full.names=TRUE )) { source(src) }
libsvm.dir <<- sprintf("%s/svm", BASE)



Synopsis <<- sprintf("

Run a one-class SVM on a microarray anomaly detection task.  Uses a modified version of LIBSVM with default settings.  See %s.

Input is (i) a training set matrix (genes x samples), and (ii) a test set matrix (same genes x samples).

Write anomaly scores for each test instance to standard output.

", libsvm.dir)


# select default parameters
main <- function() {

	Usage <- sprintf("%s <training set> <test set> <output.file>", rscript.name())

	args <- rscript.optparse(rscript.args())
	quitif( !any(length(args)==3:3) || any(grep('^-h',rscript.args())) || any(grep('^--help$',rscript.args())), "%s\nUsage:  %s\n", Synopsis, Usage )

    training.set <- read.table(args[1], header=TRUE, row.names=1, sep='\t')
	test.set     <- read.table(args[2], header=TRUE, row.names=1, sep='\t')

	g <- one.class.svm( training.set, test.set, libsvm.dir )
		
	write.table(g, quote=FALSE, row.names=FALSE, col.names=FALSE, file=args[3])

}


main()
