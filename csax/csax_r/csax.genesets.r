#!/usr/bin/env Rscript

# the following boilerplate includes modules in the necessary directories
BASE <<- dirname(sub('--file=(.*)','\\1',commandArgs()[grep('^--file=',commandArgs())])) # directory where this script is
for (src in list.files( paste( BASE ,c('modules'),sep='/'), full.names=TRUE )) { source(src) }
            
Synopsis <<- "

Print statistics about how CSAX ranks gene sets in a test set.  We assume that
FRaC and GSEA output files have been created, i.e., by running csax.r.
Output (written to user-specified file) is a tab-delimited table, with header
row, that lists the aggregate ranking for each test set instance and gene set.

"

Usage <<- sprintf("

Usage:  %s [-B <bagging iterations=40>] <gene sets database file> <output files cache directory> <output file>

",rscript.name())


main <- function() {

	args <- rscript.optparse(rscript.args())
	quitif(!any(length(args)==3:3) || any(grep('^--help$',rscript.args())) || any(grep('^-h',rscript.args())), "%s\n%s\n", Synopsis, Usage)

	# GSDB
	gsdb.file <- args[1]
	if (!file.exists(gsdb.file)) { stop(sprintf("Missing %s", gsdb.file)) }
    gsdb <- read.gmt(gsdb.file)
	gsdb.name <- sub('.gmt$', '', basename(gsdb.file))

	# cache.dir
	cache.dir <- args[2]

	# output file
	header <- output <- args[3]

	bags <- 1:parse.option(args, 'B', 40, as.integer)
	bags <- parse.option(args, 'b', bags, parse.integer.list)

	q = 0
	while (TRUE) { 

		q <- q + 1 

		required.files <- gsea.output.files(q,cache.dir,gsdb.name,bags)
		missing.files <- required.files[ !sapply( required.files,file.nonzero)]

		if (length(missing.files)==length(required.files)) { 
			
			break # we assume that q now exceeds the number of test examples

		} else if (length(missing.files)) { 
			
			stop(sprintf("Missing %d file(s) for test instance #%d (first is %s)", length(missing.files), q, missing.files[1]))

		} else { 

			R <- compute.ranks( q, cache.dir, gsdb, bags )
	    	median.ranks <- sapply( R , median )
		    mean.ranks   <- sapply( R , mean )
			meta.ranking <- names(gsdb$genes)[ order( median.ranks, mean.ranks ) ]

			for (gene.set in names(gsdb$genes)) { 

				write( paste("Test Set Index", "Gene Set", "Description", "Size", "Median GSEA Rank", "Mean GSEA Rank", sep="\t"), 
				       file=header, append=TRUE )
				
				header <- "/dev/null"

				write( paste(q, gene.set, gsdb$desc[gene.set], length(unlist(gsdb$genes[gene.set])), median.ranks[gene.set], mean.ranks[gene.set], sep="\t"), 
				       file=output, append=TRUE )

			}
		}
	}
}

main()
