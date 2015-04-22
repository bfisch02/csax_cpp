

# compute AUC (area under the ROC curve) from
# y = a set of TRUE/FALSE values 
# g = a parallel set of real-valued scores

auroc <- function(y, g) { 

	P <- length(which(y)) # number of positives
	N <- length(y) - P    # num. negatives

	T <- 0  # number of NEGATIVES with g less than g of current POSITIVE (hopefully, there are lots of them)
	NBP <- 0 # total number of *N*egatives *B*elow *P*ositives

	i <- 1 # index of next-highest prediction...
	PI <- order(g) # ...relative to ASCENDING sorted order PI

	while (i <= length(g)) {

		p <- length( which( y[ which(g==g[PI[i]]) ] ))	# number of POSITIVES with this value of g (i.e. g[i])
		n <- length(which(g==g[PI[i]])) - p             # number of NEGATIVES with this value of g (i.e. g[i])
		NBP <- NBP + p * (T + (n/2.0))
		T <- T + n

		while (i < length(g) && g[PI[i+1]]==g[PI[i]]) { i <- i + 1 }
		i <- i + 1
		
	}

	return(as.real(NBP) / (as.real(N) * as.real(P)))
}

# Compute an AUC for each test example individually
auroc.individuals <- function(y, g) {

	AUC <- real(0)

	positive <- which( y ) 
	negative <- which (!y)

	for (i in positive) { 
		S <- c(i, negative)

		auc <- (which(y[names(sort(g[S]))])-1) / (length(S)-1)

		if (auc != auroc( y[S], g[S] )) { stop(sprintf("ASSERTION FAILED (auroc.individuals): %f != %f", auc,auroc( y[S], g[S] ))) } # notox

		AUC <- c(AUC,auc)
	}

	names(AUC) <- names(positive)

	return(AUC)

}

# compute % negative instances ranked higher (so 0% is best) than each negative example
percentile.rank <- function(y,g) { 
	
	PR <- real(0) # collect one for each positive instance
	
	positive <- which( y ) 
	negative <- which (!y)

	for (i in positive) { 
		S <- c(i,negative)
		ranking <- which(y[names(sort(g[S]))]) 
		pr <- 100.0 * (length(S) - ranking) / length(negative)
		PR <- c(PR,pr)
	}
	names(PR) <- names(positive)
	return(PR)
}

