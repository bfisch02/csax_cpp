# Functions to help with R scripts
# Version 1.1

# name of the called script
rscript.name <- function() { return(substring( commandArgs()[which(substring(commandArgs(),1,7)=='--file=')], 8 )); }

# Get user arguments. (Rscript adds an artificial argument '--args' to the
# commandArgs() to indicate the rest are user-supplied.)
# param:  optional range (e.g. "rscript.args(3:5)"), will return NA for out-of-bounds indices;
rscript.args <- function(...) { 
	commandArgs() -> CA;
	if (!any(CA=='--args')) { traceback(); stop("rscript.args:  No '--args' commandArgs() arg; this does not appear to be an Rscript.") }
	which(CA=='--args') -> a; 
	if (a == length(CA)) { args <- character(0); } else { args <- CA[(a+1):length(CA)]; }
	return(args[...]); 
}

# short for number of args
rscript.argc <- function() { return(length(rscript.args())); }

# parse simple command line options of the form "-option value" "-option=value"
# (every option requires a value).
rscript.optparse <- function(args) { 
	options = character(0); #this will have an attribute for each option
	rest = character(0); #this will be the remaining args
	assign_next <- FALSE;	
	for (arg in args) { 
		if (assign_next) { 
			attr(options, assign_var) <- arg;
			assign_next <- FALSE;	
		} else if (substr(arg,1,1)=='-') { 
			while (substr(arg,1,1)=='-') { substr(arg,2,nchar(arg)) -> arg; }
			unlist(strsplit(arg,'=')) -> var;
			if (length(var)==1) {  #no '=' 
				assign_var <- var;
				assign_next <- TRUE;	
			} else {
				attr(options, var[1]) <- paste(var[2:length(var)],sep="",collapse="");
			}
		} else {
			rest <- c(rest,arg);
		}
	}
	if (assign_next) { attr(options,assign_var) <- 1; }
	attributes(rest) <- attributes(options);
	return(rest);
}

# function to parse regular options
parse.option <- function(args, flag, default, convert.function=NULL) { 
	if (is.null(attr(args,flag))) { 
		value <- default 
	} else {
		value <- attr(args,flag)
		if (!is.null(convert.function)) { value <- convert.function(value) }
	}
	return(value)
}

