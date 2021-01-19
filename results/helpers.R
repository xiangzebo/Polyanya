# Useful functions for parsing data
#
# @author: dharabor
# @created: 2015-01-09
#

loadCSV <- function(filenames, ...)
{
    retval <- data.frame()
    for(i in filenames) 
    {
        print(i)
        tmp <- read.csv(i, ...)
        if(nrow(retval) == 0)
        {
            tmp <- cbind(tmp, "csv"=rep(i, nrow(tmp)))
            retval <- tmp
        }
        else
        {
            tmp <- cbind(tmp, "csv"=rep(i, nrow(tmp)))
            retval <- rbind(retval, tmp)
        }
    }
    retval
}

# extract the basename from a filename string
basename <- function(x)
{
	tmp <- gsub("[a-z|A-Z|0-9|-|.]*[/]+", "", x)
	tmp
}

# Average My Series; 
# takes a series and a numeric factor; returns an average
# for each level of the factor. occasionally useful.
ams <- function(series, dist, stepsize=50)
{        
	sval <- c() 
	dval <- c()
	nval <- c()
    sdval <- c()

	for(i in seq(1, max(dist), by=stepsize))         
	{
		targetrows <- (dist >= i & dist < i+stepsize)
		selectedrows <- na.omit(series[targetrows])
        selectedrows <- selectedrows[!is.nan(selectedrows)]
		if(length(selectedrows) > 0)
		{
            sval <- c(sval, mean(selectedrows))
            dval <- c(dval, mean(seq(i, i+stepsize, by=1)))
            nval <- c(nval, length(selectedrows))
            sdval <- c(sdval, sd(selectedrows))
		}
	}

	results <- as.data.frame(dval)
	results <- cbind(results, sval, nval, sdval)
	names(results) <- c("avgdist", "avgseries", "bucketsize", "sd")
	results
}

plot.ams <- function(x, title="ams", xlabel="avgdist", ylabel="avgseries")
{
	stopifnot(c("avgdist", "avgseries") %in% names(x))
	plot(x$avgseries ~ x$avgdist, xlab=xlabel, ylab=ylabel, main=title, type="l")
}

# Generalised Average-My-Series
# sort a vector of values according to some factor and compute summary statistics 
# for each level of the factor (n, min, q1, med, mean, q3, max, sdev).
# @param series: the target series (numeric)
# @param sfac: a factor for the target series
# @param omitOutliers: before averaging, filter outliers from every level 
# (using is.outlier)
# @param cutoff: discard any levels with less elements than the specified cutoff
gams <- function(series, sfac, cutoff=1, omitOutliers=FALSE)
{
    result <- tapply(series, INDEX=sfac, FUN=function(x)
    {
        ret <- c()
        filter <- !is.nan(x) & !is.na(x)
        x <- x[filter]
        if(omitOutliers)
        {
            x <- outliers.omit(x)
        }
        ret <- c("n"=length(x), summary(x, na.rm=TRUE), "sdev"=sd(x))
    })

    #print(result)
    rnames <- names(result)
    cnames <- names(result[[1]])
    result <- matrix(as.numeric(unlist(result)), byrow=TRUE, ncol = length(cnames),
        dimnames = list(c(), cnames))
    result <- as.data.frame(result)
    result <- cbind("fac"=rnames, result)
    if(cutoff)
    {
        result <- subset(result, n >= cutoff)
    }
    result
}

# gussied up equivalent to tapply; puts the results nicely into
# a data frame instead of returning a crappy list
myapply <- function(myx, myfactors, FUN)
{
	mylevels <- levels(as.factor(myfactors))
	for(i in seq(1, length(mylevels)))
	{
		tmp <- subset(myx, myfactors == mylevels[i])
		val <- FUN(tmp) #c(round(mean(tmp), digits=2), round(sd(tmp), digits=2))
		if(i == 1)
		{
			retval <- t(as.data.frame(val))
			#colnames(retval) <- c("mean", "sd")
		}
		else
		{
			retval <- rbind(retval, val)
		}
	}
	rownames(retval) <- mylevels
	retval
}


## prints minor log ticks on plots
## ax is the target axis
## ticks_ration is the size ratio between minor and major ticks

axis.log10 <- function(ax, ticks_ratio=0.5, power_labels=TRUE, cex.axis=1,...)
{
  lims <- par("usr")
  print(lims)
  if(ax %in%c(1,3)) lims <- lims[1:2] else lims <- lims[3:4]
  print(lims)
  mn <- floor(min(lims))
  mx <- ceiling(max(lims))

  major.ticks <- seq(mn, mx, by=1)
  major.labels = 10^(major.ticks);
  if(power_labels)
  {
      major.labels <- sapply(major.ticks, function(x) { as.expression(substitute(10^A, list(A = x))) } )
  }
  #print(major.ticks)
  axis(ax,at=major.ticks,labels=major.labels,cex.axis=cex.axis, ...)

  minor.ticks <- log10(seq(1, 10, by=1))
  minor.ticks <- c(outer(minor.ticks, major.ticks,`+`)) 
  #print(minor.ticks)
  axis(ax,at=minor.ticks,tcl=par("tcl")*ticks_ratio,labels=FALSE)
  if(ax == 2)
  {
      abline(h=minor.ticks, col="grey91")
  }
  if(ax == 1)
  {
      abline(v=minor.ticks, col="grey91")
  }
}

axis.draw <- function(ax, ticks_ratio=0.5, power_labels=TRUE, logaxis=FALSE, cex.axis=1,...)
{
  lims <- par("usr")
  print(lims)
  if(ax %in%c(1,3)) lims <- lims[1:2] else lims <- lims[3:4]
  print(lims)
  mn <- floor(min(lims))
  mx <- ceiling(max(lims))

  major.ticks <- seq(mn, mx, by=1)
  major.labels = 10^(major.ticks);
  if(power_labels)
  {
      major.labels <- sapply(major.ticks, function(x) { as.expression(substitute(10^A, list(A = x))) } )
  }
  #print(major.ticks)
  axis(ax,at=major.ticks,labels=major.labels,cex.axis=cex.axis, ...)

  minor.ticks <- seq(1, 10, by=1)
  if(logaxis == TRUE)
  {
      minor.ticks <- log10(minor.ticks)
  }
  minor.ticks <- c(outer(minor.ticks, major.ticks,`+`)) 
  #print(minor.ticks)
  axis(ax,at=minor.ticks,tcl=par("tcl")*ticks_ratio,labels=FALSE)
  if(ax == 2)
  {
      abline(h=minor.ticks, col="grey91")
  }
  if(ax == 1)
  {
      abline(v=minor.ticks, col="grey91")
  }
}

logplot <- function(x, y, 
                    xlim=c(min(x), max(x)), 
                    ylim=c(min(floor(log10(y))), max(ceiling(log10(y)))),
                    x_power_labels=FALSE, y_power_labels=FALSE, cex=1, cex.axis=1,
                    las=1, type="b", col="black",...)
{
    print(ylim)
    y <- log10(y)
    plot(NULL, xlim=xlim, yaxt="n", ylim=ylim, ...)
    axis.draw(2, power_labels=y_power_labels,las=las, logaxis=TRUE, cex.axis=cex.axis)
    axis.draw(1, power_labels=x_power_labels,las=las)
    if(type == "l" || type=="b")
    {
        lines(y ~ x, cex=cex, col=col)
    }

    if(type == "p" || type=="b")
    {
        points(y ~ x, cex=cex, col=col)
    }
}

loglogplot <- function(x, y, 
                    xlim=c(min(floor(log10(x))), max(ceiling(log10(x)))),
                    ylim=c(min(floor(log10(y))), max(ceiling(log10(y)))),
                    x_power_labels=FALSE, y_power_labels=FALSE, cex=1, cex.axis=1, 
                    las=1, type="b", col="black", ...)
{
    print(ylim)
    print(xlim)
    y <- log10(y)
    x <- log10(x)
    plot(NULL, xlim=xlim, yaxt="n", xaxt="n", ylim=ylim, ...)
    axis.draw(2, power_labels=y_power_labels,las=las, logaxis=TRUE, cex.axis=cex.axis)
    axis.draw(1, power_labels=x_power_labels,las=las, logaxis=TRUE, cex.axis=cex.axis)
    if(type == "l" || type=="b")
    {
        lines(y ~ x, cex=cex, col=col)
    }

    if(type == "p" || type=="b")
    {
        points(y ~ x, cex=cex, col=col)
    }

}

# omit any points outsidde the range [Q1 - 1.5*IQR, Q3 + 1.5IQR]
outliers.omit <- function(x, ...)
{
	x[!is.outlier(x, ...)]
}

is.outlier <- function(x, ...)
{
	mid50 <- IQR(x, ...)
	Q1 <- fivenum(x, ...)[2]
	Q3 <- fivenum(x, ...)[4] 
	(is.na(x) | (x < (Q1 - 1.5*mid50) | x > (Q3+1.5*mid50)))
}


# test for normal 
# i.e. compute % of data points within 1, 2 and 3 sd of the mean.
empirical_rule <- function(x) 
{
	x <- na.omit(x)
	xm <- mean(x)
	xsd <- sd(x)
	xlen <- length(x)
	tmp <- x[x >= (xm - xsd)]
	tmp <- tmp[tmp <= (xm + xsd)]
	pct1sd <- length(tmp) / xlen
	tmp <- x[x >= (xm - 2*xsd)]
	tmp <- tmp[tmp <= (xm + 2*xsd)]
	pct2sd <- length(tmp) / xlen
	tmp <- x[x >= (xm - 3*xsd)]
	tmp <- tmp[tmp <= (xm + 3*xsd)]
	pct3sd <- length(tmp) / xlen
	c("n"=xlen, "mean"=xm, "sd"=xsd, "pct1sd"=pct1sd, "pct2sd"=pct2sd, "pct3sd"=pct3sd)
}

# does the same thing as hist; but also rounds and computes probabilities
empirical_distribution <- function(x, bucketsize)
{
	stopifnot(class(x) %in% c("numeric", "integer"))
	stopifnot(class(bucketsize) == "numeric" & bucketsize >=1)
	x <- na.omit(round_to_nearest(x, bucketsize))
	tmp <- as.data.frame(table(x))
	tmp <- cbind(tmp, tmp[,2] / sum(tmp[,2]))
	names(tmp) <- c("Value", "Freq", "Pr")
	tmp$Value <- as.numeric(as.character(tmp$Value))
	tmp
}

# round to the nearest numerical factor (default=1)
round_to_nearest <- function(x, value=1)
{
	stopifnot(class(x) %in% c("numeric", "integer"))
	stopifnot(class(value) == "numeric" & value >= 1)
	value <- as.integer(value)
	tmp <- x
	for(i in seq(1, length(tmp), by=1))
	{
		if(!is.na(tmp[i]))
		{
			if(tmp[i] < 0)
			{
				tmp[i] <- tmp[i] - (value/2)
			}
			else
			{
				tmp[i] <- tmp[i] + (value/2)
			}
		}
	}	
	tmp <- as.integer(tmp / value) * value
	tmp
}

# Identify pairs of elements where end_time < start_time
# Adjust end_time using the following formula:
# end_time = start_time + 
# 	[time left, from start_time until midnight] + 
# 	[time elapsed, from midnight until end_time]
# Still has some bugs; for example we do not detect cases where
# a train leaves more than 24h after it arrives
fix_24h_wraparound__ <- function(start_time, end_time)
{
	stopifnot(class(start_time) == "POSIXct" && class(end_time) == "POSIXct")
	tdelta <- difftime(end_time, start_time, units="mins")
	rows_to_fix <- which(!is.na(tdelta) & tdelta < 0)
	time_adjustment_secs <- 
		(86400 - (as.numeric(start_time[rows_to_fix]) %% 86400)) + # time left until midnight
		(as.numeric(end_time[rows_to_fix]) %% 86400) # plus elapsed time from next day
	end_time[rows_to_fix] <- start_time[rows_to_fix] + time_adjustment_secs
	end_time
}
