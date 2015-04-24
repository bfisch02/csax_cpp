#!/usr/bin/env Rscript

BASE <<- dirname(sub('--file=(.*)','\\1',commandArgs()[grep('^--file=',commandArgs())]))
for (src in list.files(paste(BASE,c('modules'),sep='/'), full.names=TRUE )) { source(src) }
frac.exe <- sprintf("%s/frac/frac" ,BASE) # assume exe is in <script directory>/frac



Synopsis <<- sprintf("

Run FRaC on a microarray anomaly detection task.

Input is (i) a training set matrix (genes x samples), and (ii) a test set matrix (same genes x samples).

Ouput FRaC anomaly scores for each test set instance.

Required:  %s

Side-effect:  Write table of FRaC anomaly scores for each gene (to user-specified location).

", frac.exe)


main <- function() {

	Usage <- sprintf("%s <training set file> <test set> <output file>", rscript.name())

	args <- rscript.optparse(rscript.args())
	quitif( !any(length(args)==3:3) || any(grep('^-h',rscript.args())) || any(grep('^--help$',rscript.args())), "\n%s\n\nUsage:  %s\n", Synopsis, Usage )

	training.set <- read.table(args[1], header=TRUE, row.names=1, sep='\t')
	test.set     <- read.table(args[2], header=TRUE, row.names=1, sep='\t')

	if (!file.nonzero(frac.exe)) { stop(sprintf("Missing %s", frac.exe)) }

	ns <- frac( training.set, test.set, save.dir="args[3]", frac.exe=frac.exe )
	g <- sapply( ns, sum )
	write.table( g, quote=FALSE, row.names=FALSE, col.names=FALSE, file=args[3] )

} # main

main()


