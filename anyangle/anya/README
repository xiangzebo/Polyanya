------------------------------------------------------------------------------
About
------------------------------------------------------------------------------
2016-04-27

This file describes how to configure and run an implementation of the Anya 
pathfinding algorithm. The program is written in Java by Daniel Harabor and 
Dindar Oz. It is provided in the interest of scientific research under the
terms of the MIT License and without any guarantees.

If you find this code useful, please cite our paper:

@article{DBLP:journals/jair/HaraborGOA16,
  author    = {Daniel Damir Harabor and
               Alban Grastien and
               Dindar {\"{O}}z and
               Vural Aksakalli},
  title     = {Optimal Any-Angle Pathfinding In Practice},
  journal   = {J. Artif. Intell. Res.},
  volume    = {56},
  pages     = {89--118},
  year      = {2016},
  url       = {https://doi.org/10.1613/jair.5007},
  doi       = {10.1613/jair.5007},
  timestamp = {Sun, 02 Jun 2019 21:09:40 +0200},
  biburl    = {https://dblp.org/rec/bib/journals/jair/HaraborGOA16},
  bibsource = {dblp computer science bibliography, https://dblp.org}
}

------------------------------------------------------------------------------
Compilation pre-requisites: 
------------------------------------------------------------------------------
JVM 1.8, bash and make.

------------------------------------------------------------------------------
Compilation instructions: 
------------------------------------------------------------------------------
type "make" in the directory with all the sources (usually this is the same 
directory containing this file)

------------------------------------------------------------------------------
Running the program
------------------------------------------------------------------------------
Anya is written to accept scenario files in the format used at the 2012
Grid-based Path Planning Competition. You will need both scenario files
(which specify start/target pairs) and the associated map files from which
these locations are drawn.

The competition website is here:
http://movingai.com/GPPC/

Benchmarks used at the competition (scenario files, map files) are here:
http://www.movingai.com/benchmarks/

Each scenario file makes reference to map file. The program will use the
map file location specified in the scenario file. Usually this location
is relative to the current working directory. One simple way to 
organise everything is as follows:

-- [anya sources directory]
 |- scenarios
 |- maps
  |- [benchmark directory; e.g. dao] 

After compilation, execute the program as follows:
> java -cp ./ ScenarioRunner -ANYA [scenario file]

Also available is an implementation of the A* algorithm which computes
grid-optiomal paths for the same set of instances. To invole that algorithm:

> java -cp ./ ScenarioRunner -ASTAR [scenario file]


------------------------------------------------------------------------------
Program output
------------------------------------------------------------------------------
Output from the program is in the form of semi-colon-separated statistics 
which are printed for every problem instance. Descriptions of the columns:

exp - [integer] the experiment id 
alg - [string]  the name of the algorithm
wallt_micro - [integer] wall-clock time in microseconds
runt_micro - [integer] actual running time (i.e. cpu-time) in microseconds
expanded - [integer] nodes expanded
generated - [integer] nodes generated 
heapops - [integer] number of heap operations (insert, delete, relax)
start - [xy-coordinate] id of the start node
target - [xy-coordinate] id of the target node
gridcost - [double] the cost of the grid-optimal path between start and target
realcost - [double] the cost of the optimal path between start and target
map - [string] the name of the map on which the experiment was run

------------------------------------------------------------------------------
Examples
------------------------------------------------------------------------------

> java -cp ./ ScenarioRunner -ANYA AcrossTheCape.map.scen

exp;alg;wallt_micro;runt_micro;expanded;generated;heapops;start;target; gridcost;realcost;map
0;AnyaSearch;2453;2392;2;6;7;(283,492);(286,497); 6.24264;5.830951894845301; AcrosstheCape.map
... 
etc


