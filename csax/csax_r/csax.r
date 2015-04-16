#!/usr/bin/env Rscript
BASE <<- dirname(sub('--file=(.*)','\\1',commandArgs()[grep('^--file=',commandArgs())])) # directory where this script is
for (src in list.files(paste(BASE,c('modules'),sep='/'), full.names=TRUE)) { source(src) } # this includes modules in the necessary directories


Synopsis <<- sprintf("

Run CSAX [Noto et al., RECOMB 20014 (submitted)].

Output file contains anomaly scores, one per test set sample.

Side-effects:  FRaC and GSEA output tables are written to a user-specified
location.  Temporary files are written to a temporary directory and cleaned up.
GSEA writes to ~/gsea_home and a local directory named for the date ('mar05' on
March 5th).  These directories are not automatically deleted.

You may choose to ignore the output and run multiple instances of this script
in parallel to create the FRaC and GSEA output tables, then run it once to
compute the anomaly scores.

")

Usage <<- sprintf("

Usage: %s [options] <training set> <test set> <gene set database> <cache directory> [<output file>]

The training set is a tab-delimited matrix file; rows are gene symbols, columns
are samples.

The test set is a tab-delimited matrix file; rows are the exact same genes as
the training set, columns are other samples.

The gene set database is a file containing a collection of gene sets, where
genes are indicated with the same symbols as in the training/test sets.  This
must have a .gmt extension (or else GSEA, a subroutine of CSAX, will fail).
The file format is described here:
http://www.broadinstitute.org/cancer/software/gsea/wiki/index.php/Data_formats#GMT:_Gene_Matrix_Transposed_file_format_.28.2A.gmt.29

The output cache directory is where all FRaC and GSEA output files will be
stored, potentially for reuse.

Output file will contain the anomaly scores, one per test set instance.

Options:

  -B <integer>   Number of bagging iterations
  -Y <real>      Gamma discount parameter

(See source code for advanced options)
", rscript.name())



GSEA.JAR <- sprintf("%s/gsea/gsea.jar",BASE)
FRaC.EXE <<- sprintf("%s/frac/frac",BASE)
TMP.DIR <<- "./tmp"   # WARNING: GSEA will fail if TMP.DIR has any '-' in it.  For this reason, I don't set TMP.DIR <- tempdir().



main <- function() {

	args <- rscript.optparse(rscript.args())
	quitif(!any(length(args)==4:5) || any(grep('^--help$',rscript.args())) || any(grep('^-h',rscript.args())), "%s\n%s", Synopsis, Usage)

	# training set
	training.set <- read.table(args[1], header=TRUE, row.names=1, sep='\t')
	message(sprintf("%d genes, %d samples in %s", nrow(training.set),ncol(training.set),args[1]))
	
	# test set
	test.set <- read.table(args[2], header=TRUE, row.names=1, sep='\t')
	message(sprintf("%d genes, %d samples in %s", nrow(test.set),ncol(test.set),args[2]))

	# GSDB
	gsdb <- read.gmt(args[3])

	# output tables cache directory
	cache.dir <- chkdir(args[4])

	# ADVANCED OPTIONS can be invoked with a "-<flag> <argument>" on the command line

	# -Y <real> 
	# sets gamma, the discount parameter
	gamma <- parse.option(args, "Y", 0.95, as.real)

	# -B <integer>
	# sets the number of bagging iterations.
	# Actually, bagging iterations are indicated by a number, 
	# so this option sets bags <- 1,2,3,...,B
	bags <- 1:parse.option(args, "B", 40, as.integer)
	
	# -b <integer, integers separated by commas, or integer range> 
	# specifies exactly which bagging iterations to do (and overrides the '-B' option).
	# This option is useful for parallelizing CSAX.
	bags <- parse.option(args, "b", bags, parse.integer.list) 

	# -F <0 or 1>
	# sets whether or not to run on the FULL training set.
	# Useful for parallelizing CSAX, since this need be done exactly once
	# (as opposed to the interations of bagging).
	run.full.training.set <- parse.option(args, 'F', TRUE, as.logical)

	# -Q <integer(s)> 
	# sets which test set instances to run on.  The values should all be integers between
	# 1 and the number of test set instances.  Useful for parallelizing CSAX.
	Q <- parse.option(args, 'Q', 1:ncol(test.set), parse.integer.list)

	# run CSAX
    run.csax(training.set, test.set, gsdb, cache.dir, bags, Q,  FRaC.EXE,GSEA.JAR,TMP.DIR, run.full.training.set) # create output files

	# output file
	if (length(args)==5) { 

		message("Compute anomaly scores...")
	    g <- eval.csax( Q, cache.dir, gsdb, bags, gamma ) # compute anomaly scores (takes time, so only do it if user gave a filename)

		write(g, ncolumns=1, file=args[5])
	}

}

main()
