#!/bin/sh

# -DUSE_PARALLELISM causes the preprocessing to use as many cores as available.
g++ -std=c++0x -O3 -fopenmp -DNDEBUG -DUSE_PARALLELISM *.cpp -o detailed_dfs_src
g++ -std=c++0x -O3 -fopenmp -DNDEBUG -DUSE_PARALLELISM -DUSE_CUT_ORDER *.cpp -lmetis -o detailed_cut_src
g++ -std=c++0x -O3 -fopenmp -DNDEBUG -DUSE_PARALLELISM -DEXTRACT_ALL_AT_ONCE *.cpp -o dfs_src
g++ -std=c++0x -O3 -fopenmp -DNDEBUG -DUSE_PARALLELISM -DUSE_CUT_ORDER -DEXTRACT_ALL_AT_ONCE *.cpp -lmetis -o cut_src

