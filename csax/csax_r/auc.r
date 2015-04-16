#!/usr/bin/env Rscript

Synopsis <<- "

Compute the area under the ROC curve (AUC) from (i) a labels file (containing
1's and 0's, indicating anomalous and normal test set instances) and (ii) an
anomaly score file (containing an anomaly score for each test set instance, in
the same order).  Each file should contain only numbers.  
"

# The following boilerplate includes modules in the necessary directories
for (src in list.files( paste(dirname(sub('--file=(.*)','\\1',commandArgs()[grep('^--file=',commandArgs())])),c('modules'),sep='/'), full.names=TRUE )) { source(src) }

main <- function() {

	Usage <- sprintf("%s <labels file> <anomaly scores file>", rscript.name())
	
	args <- rscript.optparse(rscript.args())
	quitif( !any(length(args)==2:2) || any(grep('^-h',rscript.args())) || any(grep('^--help$',rscript.args())), "\n%s\n\nUsage:  %s\n", Synopsis, Usage )

	# Parse args/option(s)
	y <- as.logical(scan(args[1], what=numeric(0)))
	g <- scan(args[2], what=numeric(0))

	cat(sprintf("%f\n", auroc(y,g)))
	

} # main

main()
