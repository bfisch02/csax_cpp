

# Read gene set file (".gmt"format) into a list mapping gene set identifier
# (e.g, "BP00301") to a list of gene identifiers (e.g.,
# ["ACTA2","ACTG2","AGER",...])
read.gmt <- function(file) {
	
	X <- scan(file, what=character(0), sep="\n", quiet=TRUE)  # read data into lines of ASCII
	
	gene.sets <- unlist(lapply( X, function(line) { return(unlist(strsplit(line,'\t'))[1])} )) # add use the first element for each gene set name
	gene.sets <- toupper(gene.sets) # b/c GSEA will translate names to upper case	

	parse.genes <- function(line) {
		d <- unlist(strsplit(line,'\t')) # one record:  name, description, gene1, gene2, ...
		if (length(d) <= 2) { return(list()) } else { return(d[3:(length(d))]) }
	}

	parse.description <- function(line) { 
		d <- unlist(strsplit(line,'\t')) # one record:  name, description, gene1, gene2, ...
		return(d[2])
	}

	G <- lapply( X, parse.genes ) # list of genes for each set
	names(G) <- gene.sets
	
	D <- lapply( X, parse.description)
	names(D) <- gene.sets

	return(list(file=file,name=sub('.gmt$','',basename(file)),sets=gene.sets,genes=G,desc=D))

}


