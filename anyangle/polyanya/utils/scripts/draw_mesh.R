load_mesh <- function(filename)
{
    rawmesh <- read.csv(filename, sep="\n", header=TRUE, stringsAsFactors=FALSE)
    mesh_maxcols <- max(count.fields(filename))
    rawmesh <- as.data.frame( 
                    read.table(filename, header=FALSE, sep=" ", 
                          col.names=paste("X", seq_len(mesh_maxcols)), 
                          colClasses=rep("numeric", mesh_maxcols), fill=TRUE, skip=2))

    # read vertices
    nverts <- rawmesh[1,1]
    npolys <- rawmesh[1,2]
    points <- rawmesh[seq(2, nverts+1), c(1,2)]

    # R is 1-indexed; increment vertex ids for all polygons
    verts_per_poly <- unlist(rawmesh[seq(nverts+2, nrow(rawmesh)), 1])
    polys <- as.matrix(rawmesh[seq(nverts+2, nrow(rawmesh)), seq(2, mesh_maxcols)]+1)

    retval <- list("header"=rawmesh[1,1:2], "points"=points, "polys"=polys, "verts_per_poly"=verts_per_poly, "filename"=filename)
    retval
}

draw_grid_from_mesh <- function(rawmesh, labels=FALSE, poly_border_col="gray", xlab="", ylab="")
{
    draw_mesh(rawmesh, poly_border_col="white", xlab=xlab, ylab=ylab)

    paste("drawing grid")
    yrange <- range(rawmesh$points[,2]) + c(-1, 1)
    xrange <- range(rawmesh$points[,1]) + c(-1, 1)
    print(yrange)
    print(xrange)
    for(i in seq(min(yrange), max(yrange)))
    {
        lines(xrange, rep(i, 2), col=poly_border_col)
    }
    for(i in seq(min(xrange), max(xrange)))
    {
        lines(rep(i, 2), yrange, col=poly_border_col)
    }
}

draw_mesh <- function(rawmesh, labels=FALSE, poly_colour="white", poly_border_col="gray", canvas_col="black", xlab="", ylab="")
{
    yrange <- range(rawmesh$points[,2]) + c(-1, 1)
    xrange <- range(rawmesh$points[,1]) + c(-1, 1)

    if(labels)
    {
        plot(NA, xlim=xrange, ylim=yrange, main=rawmesh$filename, yaxt="n", xaxt="n", xlab=xlab, ylab=ylab)
        axis(1, at=pretty(xrange), tick=TRUE, labels=TRUE)
        axis(2, at=pretty(yrange), tick=TRUE, labels=TRUE)
    }
    else
    {
        print(xlab)
        plot(NA, xlim=xrange, ylim=yrange, yaxt="n", xaxt="n", xlab=xlab, ylab=ylab)
    }

    paste("drawing mesh")
    polygon(c(rep(min(xrange), 2), rep(max(xrange), 2)), c(min(yrange), max(yrange), max(yrange), min(yrange)), col=canvas_col)
    pb <- txtProgressBar(0, nrow(rawmesh$polys), style=3)
    for(i in seq(1, nrow(rawmesh$polys)))
    {
        setTxtProgressBar(pb, i)
        poly_points <- rawmesh$points[rawmesh$polys[i, seq_len(rawmesh$verts_per_poly[i])],]
        polygon(poly_points, col=poly_colour, border=poly_border_col)
    }
}

# load 
load_trace <- function(filename)
{
    trace <- read.csv(filename, sep=";", header=FALSE, col.names=c("root", "left", "right", "priority"), stringsAsFactors=FALSE)
    trace$expanded <- grepl("popped off", trace$root)
    trace$start <- grepl("generating", trace$root)
    trace$intermediate <- grepl("intermediate", trace$root)
    trace$root <- gsub("[\tA-Za-z: =()]+", "", trace$root)
    trace$left <- gsub("[\tA-Za-z: =()]+", "", trace$left)
    trace$right <- gsub("[\tA-Za-z: =()]+", "", trace$right)

    repeating_node <- which(trace$start)+1
    trace <- trace[-repeating_node,]
    trace$expanded <- trace$expanded | trace$start
    trace
}

load_trace2 <- function(filename)
{
    rawout <- read.csv(filename, sep="\n", header=FALSE, stringsAsFactors=FALSE)
    pathrows <- grepl("path", rawout[,1])
    exprows <- grepl("root", rawout[,1])

    # parse the lines for nodes expanded
    tmp <- rawout[exprows,]
    tmp <- unlist(strsplit(tmp, ";"))
    tmp_roots <- tmp[seq(1, length(tmp), by=4)]
    tmp_lefts <- tmp[seq(2, length(tmp), by=4)]
    tmp_rights <- tmp[seq(3, length(tmp), by=4)]
    tmp_priority <- tmp[seq(4, length(tmp), by=4)]
    trace <- data.frame(list("root"=tmp_roots, "left"=tmp_lefts, "right"=tmp_rights, "priority"=tmp_priority))
    trace$expanded <- grepl("popped off", trace$root)
    trace$start <- grepl("generating", trace$root)
    trace$intermediate <- grepl("intermediate", trace$root)
    trace$root <- gsub("[\tA-Za-z: =()]+", "", trace$root)
    trace$left <- gsub("[\tA-Za-z: =()]+", "", trace$left)
    trace$right <- gsub("[\tA-Za-z: =()]+", "", trace$right)
    repeating_node <- which(trace$start)+1
    trace <- trace[-repeating_node,]
    trace$expanded <- trace$expanded | trace$start

    trace <- list("trace"=trace, "path"=rawout[pathrows,])
    trace$path <- gsub("^path [0-9]+;[ ]+", "", trace$path)
    trace
}

# visualses polyanya search instances from a trace file
# used in combination with load_trace
# e.g. draw_trace(load_trace(search_output_file))
draw_trace <- function(search_trace, draw_intermediate=TRUE)
{
    begin <- which(search_trace$trace$start)
    end <- c(begin[-1]-1, nrow(search_trace$trace))
    for(i in seq(1, length(begin)))
    {

        #if(i > 1)
        #{
        #    #print(paste(begin[i-1], end[i-1]))
        #    draw_path(search_trace$path[i-1], TRUE)
        #    draw_expansion(search_trace$trace[begin[i-1] : end[i-1], ], TRUE, draw_intermediate)
        #}
        #print(paste(begin[i], end[i]))

        draw_expansion(search_trace$trace[begin[i] : end[i], ], FALSE, draw_intermediate)
        #print(search_trace$path[i])
        if(i <= length(search_trace$path))
        {
            draw_path(search_trace$path[i])
        }
        readline(prompt="Press [enter] to show next instance")
    }
}

draw_expansion <- function(exp_trace, prev=FALSE, draw_intermediate=TRUE)
{
    # start node colour and symbol
    s_col = "orange"
    s_pch = 15

    # parent node colour and symbol
    p_col = "red"
    p_col2 = "brown"
    p_pch = 20

    # current successor node colour and symbol
    c_col = "green"
    c_col2 = "green3"
    c_pch = 20

    begin <- which(exp_trace$expanded)[1]
    current_root <- c()
    for(i in seq(begin, nrow(exp_trace)))
    {
        #print(exp_trace[i,])
        root_xy <- as.numeric(unlist(strsplit(exp_trace$root[i], ",")))
        left_xy <- as.numeric(unlist(strsplit(exp_trace$left[i], ",")))
        right_xy <- as.numeric(unlist(strsplit(exp_trace$right[i], ",")))

        if(exp_trace$intermediate[i] & !draw_intermediate) { next; }

        if(exp_trace$start[i])
        {
            mycol <- s_col
            mypch <- s_pch
        }
        else if(exp_trace$expanded[i])
        {
            mycol <- p_col
            mypch <- p_pch
        }
        else
        {
            mypch <- c_pch
            mycol <- c_col;
        }
        if(prev)
        { 
            mycol <- "black"
        }
        if(exp_trace$expanded[i] || exp_trace$start[i])
        {
            current_root <- root_xy
            points(root_xy[1], root_xy[2], pch=mypch, col=mycol, bg=mycol)
        }


        if(!all(current_root == root_xy))
        {
            points(root_xy[1], root_xy[2], pch=mypch, col=mycol, bg=mycol)
        }
        lines(c(left_xy[1], right_xy[1]), c(left_xy[2], right_xy[2]) , col=mycol, lwd=2)

        if(!prev)
        {
            readline(prompt="Press [enter] for next search step")
            if(mycol == c_col)
            {
                lines(c(left_xy[1], right_xy[1]), c(left_xy[2], right_xy[2]) , col=c_col2, lwd=2)
            }
            else if(mycol == p_col)
            {
                lines(c(left_xy[1], right_xy[1]), c(left_xy[2], right_xy[2]) , col=p_col2, lwd=2)
            }
        }
    }
}


load_paths <- function(filename, path_colour="red")
{
    rawpaths <- read.csv(filename, sep="\n", header=FALSE, stringsAsFactors=FALSE)
    unlist(rawpaths)
}

# txt_path should be a vector of strings each having the form
# (x1, y1) (x2, y2) ... (xn, yn)
draw_path <- function(rawpaths, prev=FALSE)
{
    symbol_sz = 0.6
    line_width = 1
    path_col="blue"

    for(i in seq_len(length(rawpaths)))
    {
        tmp <- gsub("^path [0-9]+;", "", rawpaths[i])
        tmp <- as.numeric(unlist(strsplit(gsub("\\(|\\)|,", "", tmp), " ")))
        path_xy <- list(
                        "points"=tmp,
                        "x"=seq(1, length(tmp), by=2),
                        "y"=seq(2, length(tmp), by=2))
        #print(as.vector(rawpaths[i]))
        if(prev)
        {
            path_col = "lightgray"
            lines(path_xy$points[path_xy$x], path_xy$points[path_xy$y], col=path_col, lwd=line_width)
            points(path_xy$points[path_xy$x], path_xy$points[path_xy$y], col=path_col, lwd=line_width, cex=symbol_sz)
        }
        else
        {
            lines(path_xy$points[path_xy$x], path_xy$points[path_xy$y], col=path_col, lwd=line_width)
            points(path_xy$points[path_xy$x], path_xy$points[path_xy$y], col=path_col, lwd=line_width, cex=symbol_sz)
        }
    }
}

