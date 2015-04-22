
# halt if condition is met (not necessarily an error--just write the message and halt.)
quitif <- function(condition,...) { if (condition) { message(sprintf(...)); q('no'); } }

# File exists and is nonzero size
# NOTE:  Changed from ... args passed to printf, to handle filenames with '%' in them.
file.nonzero <- function(filename) { 
	return(file.exists(filename) && as.integer((file.info(filename))['size']) > 0); 
}

# return path name, create directory if necessary 
chkdir <- mkdir <- function(path) { 
	if (!file.exists(path)) {
		# message(sprintf("Creating directory %s", path)) # nice to say, but so often used for temp. directories
		dir.create(path,recursive=TRUE) 
	}
	return(path);
}

# Translate a string description (e.g., "1, 5, 8:11") to a integer type,
# e.g., c(1,5,8,9,10,11)
parse.integer.range <- function(msg) {  # parse an integer range
	range <- as.integer(unlist(strsplit(unlist(strsplit(msg,':')),'-')))
	return(if (length(range)==1) { range } else { range[1]:range[2] })
}
parse.integer.list <- function(msg) {  # parse a list of integers and integer ranges
	return(unlist(lapply( unlist(strsplit(msg,',')) , parse.integer.range )))
}
