# Functions and definitions occasionally useful for pathfinding
# @author: dharabor
# @created: 2015-01-09
#

source("helpers.R")
loadResults <- function(filename, tnames, tclasses, sepchar=",")
{
	results <- read.table(filename, col.names=tnames, colClasses=tclasses,
	header = FALSE, sep=sepchar)
	results$map <- gsub("([a-zA-Z0-9-]+/)+", "", results$map)
	results$name <- gsub("\t", "", results$name)
	results$map <- gsub("\t", "", results$map)
	results	
} 

loadTable <- function(filename, tnames, tclasses, sepchar="\t")
{
        results <- read.table(filename, col.names=tnames, colClasses=tclasses,
        header = FALSE, sep=sepchar)
        results 
}

averageSeries <- function(series, runs)
{
	seriesLength <- length(series)
	numRows <- c(seriesLength/runs)
	first <- c(1)
	last <- c(numRows)
	tmp <- rep(0, numRows)

	for(i in seq(1, numRows, by=1))
	{
		targetIndices <- seq(i, seriesLength, by=numRows)
		tmp[i] <- mean(series[targetIndices])
	}

	tmp
}

averageResults <- function(results, runs)
{
	numRows <- length(results[,1])
	targetRows <- c(numRows/runs)
	first <- c(1)
	last <- c(targetRows)
	tmp <- results[1:targetRows,] 

	cols <- names(results)

	for(i in seq(1, length(results), by=1))
	{
		if(typeof(results[,i]) == "integer" | 
			typeof(results[,i]) == "double" | 
			typeof(results[,i]) == "numeric")

		{
			tmp[,i] <- averageSeries(results[,i], runs)
		}
	}
	tmp
}

speedup <- function(target, baseline, stepsize=25)
{
	results <- as.data.frame(ams(baseline$ne/target$ne, baseline$opl, stepsize))
	results <- cbind (results[,1:2], 
		ams(baseline$st/target$st, baseline$opl,stepsize)[,2:3])
	names(results) <- c("opl", "ne_speedup", "st_speedup", "num_exp")
	results
}

# Trend My Series; very old and pretty useless function; split a vector into equal size subsets
# and take the mean of each. Use tapply instead
tms <- function(series, bucketsize=50) {
	tmp <- c()
	for(i in seq(1, length(series), by=bucketsize)) tmp <- c(tmp, mean(series[i:(i+bucketsize-1)]))
	tmp
}

mysummary <- function(series, dist, runs=1, stepsize=25)
{
	runrows = length(series)/runs	
	first <- 1
	last <- runrows	
	retval <- ams(series[first:last], dist[first:last], 
			stepsize)[,1:2]

	if(runs > 1)
	{
		for(i in seq(2, runs, by=1))
		{
			first <- first + runrows;
			last <- last + runrows;
			retval <- cbind(retval, ams(series[first:last],
				dist[first:last], stepsize)$avgseries)
		}
	}
	names(retval) <- c("dist", seq(1, runs, by=1))
	retval
}

plotsummary <- function(series, dist, runs=1, stepsize=25)
{
	tmp <- mysummary(series, dist, runs, stepsize)
	minVal <- min(tmp[,2:(runs+1)])
	maxVal <- max(tmp[,2:(runs+1)])
	plot(tmp[,2] ~ tmp$dist, type="l", lty=1, main="n-Runs Summary",
	ylim=c(0,maxVal), xlab="Distance", ylab="Series")
	ltyVal <- c(1)
	namesVal <- c(toString(1))
	if(runs > 1)
	{
		for(i in seq(2, runs, by=1))
		{
			lines(tmp[,i+1] ~ tmp$dist, type="l", lty=i)
			ltyVal <- c(ltyVal, i)
			namesVal <- c(namesVal, toString(i))
		}
	}
	legend("topleft", namesVal, lty=ltyVal, cex=1.25)
}

plotresults <- function(series, dist, mainlabel, xlabel, ylabel, snames)
{
	maxy <- max(series);
	numSeries <- length(series[1,])
	plot(series[,1] ~ dist, lty=1, main=mainlabel, type="l", xlab=xlabel, 
	ylab=ylabel, ylim=c(0, maxy))
	if(numSeries > 1)
	{
		for(i in seq(2, numSeries, by=1))
		{
			lines(series[,i+1] ~ dist, type="l", lty=i)
		}
	}
	legend("topleft", snames, lty=seq(1, numSeries, by=1), cex=1.25)
}

runav <- function(raw, runs)
{
# averages the SEARCH TIME RESULTS from several runs on the 
# same experiment set.
# NB: this function IS ONLY USEFUL IF in the results file
# the same set of experiments are repeated immediately one
# after the other: i.e. execute all of scenario a, then
# execute it again a second time, without anything in between
	out <- raw[1,]
	
	firstindex = 1
	lastindex = 1
	while(firstindex < length(raw$map))
	{
		index = firstindex
		mapname <- raw$map[index]
		stepsize = 1000;
		while(stepsize >= 1)
		{
			while(mapname == raw$map[index])
			{
				lastindex = index;
				index = index + stepsize
				if(index > length(raw$map))
				{
					index = length(raw$map)
					break
				}
			}
			stepsize = as.integer(stepsize / 2)
			index = lastindex;
		}
		lastindex = index
		rawmap <- raw[firstindex:lastindex,]
		print(c(raw$map[firstindex], "range: ", firstindex, lastindex))
		flush.console()

		uniqueExperiments = as.integer(length(rawmap$map)/runs)
		for(i in seq(1, uniqueExperiments, by=1))
		{
			searchTime = rawmap$st[i]
			expIndex = i + uniqueExperiments
			while(expIndex < length(rawmap$map))
			{
				searchTime = searchTime + rawmap$st[expIndex]
				if(rawmap$id[expIndex] != rawmap$id[i])
				{	
					print("warning: averaging results for different experiments. Rows:", i, expIndex)
					flush.console()
				}
				expIndex = expIndex + uniqueExperiments
			}
			rawmap$st[i] = mean(searchTime);
		}
		out <- rbind(out, rawmap[1:uniqueExperiments,])
		firstindex = lastindex+1
	}
	out <- out[-1,]
	out	
}

# label the benchmarks explicitly
gppc12_benchmark_labels <- function(x)
{
	stopifnot(class(x) == "character")
	benchmark <- rep("", length(x))
	benchmark[grepl("^[A-Z]", x)] <- "sc"
	benchmark[grepl("^[a-z]+[0-9]+[a-z]*.map", x)] <- "dao"
	benchmark[grepl("^[a-z]+_", x)] <- "da2"
	benchmark[grepl("maze", x)] <- "maze"
	benchmark[grepl("random", x)] <- "random"
	benchmark[grepl("room", x)] <- "room"
	benchmark
}

gppc_names <- c("scenario", "foo1", "total_time", "foo2", "max_time_step", "foo3", "time_20_moves", "foo4", "total_len", "foo5", "subopt", "valid")
gppc_classes <- c("character", "character", "numeric", "character", "numeric", "character", "numeric", "character", "numeric", "character", "numeric", "character")      
tablenames <- c("id", "name", "ne", "nt", "ng", "st", "opl", "map")
tableclasses <- c("integer", "character", "integer", "integer", "integer", "double", "double", "character")
