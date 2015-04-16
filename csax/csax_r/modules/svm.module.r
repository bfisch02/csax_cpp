
one.class.svm <- function( training.set, test.set, libsvm.dir ) {

	# assume exe is in <script directory>/svm
	t.exe <<- sprintf("%s/svm-train",libsvm.dir)    ; if (!file.exists(t.exe)) { stop(sprintf("Missing %s (did you run make?)", t.exe)) }
	p.exe <<- sprintf("%s/svm-predict",libsvm.dir)  ; if (!file.exists(p.exe)) { stop(sprintf("Missing %s (did you run make?)", p.exe)) }
	svm.options <<- "-s 2"

	train.file <- tempfile()
	test.file <- tempfile()
	model.file <- tempfile()
	svm.output.file <- tempfile()

	svm.format( na.omit(training.set), -1, file=train.file )
	system(sprintf("%s %s %s %s >&/dev/null" , t.exe, svm.options, train.file, model.file))
	unlink(train.file)

	svm.format( na.omit(test.set), 0, file=test.file )
	system(sprintf("%s %s %s %s >&/dev/null" , p.exe, test.file, model.file, svm.output.file))

	unlink(test.file)
	unlink(model.file)

	g <- scan(svm.output.file) # numerical predictions
	g <- sapply(g, function(x){-x}) # higher anomaly scores => more anomalous looking
	unlink(svm.output.file)

	return(g)
	
} 


# Write table in svm-format, assume classes are Boolean.
# @param M rows are features and columns are examples (assume NO NA values; i.e., pass in na.omit(M) instead)
# @param C class labels (column vector, could be return value of svm.targets)
# @param file write vectors here, could be stdout() or stderr()
svm.format <- function( M, target, file ) { 

	X <- rbind(target,M)

	write.v <- function(v) { 
		cat(sprintf("%s\t%s\n", v[1], paste(1:(length(v)-1), v[2:length(v)], collapse=" ", sep=":")), 
		    file=file, append=TRUE)
	}

    return(sapply( X, write.v ))

}

