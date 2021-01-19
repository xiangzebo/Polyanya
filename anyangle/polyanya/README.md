# Polyanya

This repository containts C++ codes for Polyanya: a compromise-free 
pathfinding algorithm for navigation meshes. If you find this code 
useful, please cite our paper as follows:

@inproceedings{DBLP:conf/ijcai/CuiHG17,
  author    = {Michael Cui and
               Daniel Damir Harabor and
               Alban Grastien},
  title     = {Compromise-free Pathfinding on a Navigation Mesh},
  booktitle = {Proceedings of the Twenty-Sixth International Joint Conference on
               Artificial Intelligence, {IJCAI} 2017, Melbourne, Australia, August
               19-25, 2017},
  pages     = {496--502},
  year      = {2017},
  crossref  = {DBLP:conf/ijcai/2017},
  url       = {https://doi.org/10.24963/ijcai.2017/70},
  doi       = {10.24963/ijcai.2017/70},
  timestamp = {Wed, 27 Jun 2018 12:24:11 +0200},
  biburl    = {https://dblp.org/rec/bib/conf/ijcai/CuiHG17},
  bibsource = {dblp computer science bibliography, https://dblp.org}
}

# Compiling

Run `make all` in the root directory (or `make fast` to compile with
optimisations). This has been tested with recent versions of g++ (5+) on
recent versions of Linux (Ubuntu 16.04+, Arch Linux). This should
build `bin/scenariorunner` and `bin/test`.


# Example usage

After compiling, you can run a small example (Arena from Dragon Age: Origins)
with the command below:

```bash
./bin/scenariorunner ./meshes/arena.mesh ./scenarios/arena.scen
```


# Generating meshes from grids

CDT, M-CDT and Rect are defined in section 5 of our paper.

These rely on utilities in the `utils` directory. Grid to CDT requires the use
of the Fade2D library and GMP, which has only been tested on Linux.
Commercial use of Fade2D requires a valid commercial license.

Ensure that GMP is installed (`sudo apt-get install libgmp3-dev` on Ubuntu),
change directory into `utils`, and run `make fast`.

## Grid to CDT

In the `utils` folder,

```bash
./scripts/gridmap2mesh.sh < ./maps/arena.map
```

will convert a grid map of Arena from Dragon Age: Origins to a CDT mesh, output
to standard output.

## CDT to M-CDT

In the `utils` folder,

```bash
./bin/meshmerger < ../meshes/arena.mesh
```

will convert a CDT mesh of Arena from Dragon Age: Origins to a M-CDT mesh,
output to standard output. The `meshmerger` utility works for any kind of mesh.

## Grid to Rect

In the `utils` folder,

```bash
./bin/gridmap2rects < ./maps/arena.map
```

will convert a grid map of Arena from Dragon Age: Origins to a Rect mesh, output
to standard output.


# Mesh file format

We use a bespoke mesh file format. A summary of the format is shown below -
see `utils/spec/mesh/2.txt` for the complete specification.

```
mesh
2
[V: int = # of vertices in mesh] [P: int = # of polygons in mesh]
(for each vertex)
    [x: float] [y: float]
    [n: int = number of vertices this vertex is connected to]
    [p: int[n] = array of 0-indexed indices of the mesh's polygons connected to
                 this vertex in counterclockwise order. -1 if obstacle]
(for each polygon)
    [n: int = # of vertices in polygon]
    [v: int[n] = array of 0-indexed indices of the mesh's vertices defining
                 this polygon in counterclockwise order]
    [p: int[n] = array of 0-indexed indices of the mesh's polygons adjacent to
                 this polygon in counterclockwise order, such that the p[1]
                 shares the vertices v[0] and v[1]. -1 if obstacle]
```

The format is whitespace insensitive - tokens can be separated by any kind of
whitespace.


# Folder structure

| Path | Description |
| :--- | :---------- |
| `notes.md` | Assorted documentation for this implementation. |
| helpers | C++ helper functions / classes for use in the implementation. |
| meshes | Example meshes. |
| ├ `arena-merged.mesh` | M-CDT of Arena from Dragon Age: Origins. |
| ├ `arena.mesh` | CDT of Arena from Dragon Age: Origins. |
| ├ `aurora-merged.mesh` | M-CDT of Aurora from StarCraft. |
| └ `aurora.mesh` | CDT of Aurora from StarCraft. |
| scenarios | Example scenarios. |
| ├ `arena.mesh` | Scenario file for Arena from Dragon Age: Origins. |
| └ `aurora.mesh` | Scenario file for Aurora from StarCraft. |
| search | Search-related implementation: expansion, evaluation, pruning, etc. |
| structs | Structs and classes, mainly geometry-related. |
| utils | Utility tools. |
| ├ `README.md` | Detailed README of all tools. |
| ├ hardmap | Example grid maps. |
| ├ maps | Example grid maps. |
| └ spec | Specifications for bespoke formats used. |
|   ├ mesh | Specifications for the mesh file format. |
|   └ poly | Specifications for the poly file format (an intermediate representation we used). |


# License

This implementation of Polyanya is licensed under MIT. Several source files from
Daniel Harabor's [Warthog project] were used this project - these files are also
licensed under MIT.
These files are:
`helpers/cfg.cpp`, `helpers/cfg.h`, `helpers/cpool.h`, `helpers/timer.cpp` and
`helpers/timer.h`.

Fade2D is used to generate triangulations for use with this
with this implementation. Please note that commercial use of Fade2D requires
a valid commercial license.


[paper]: http://www.ijcai.org/proceedings/2017/0070.pdf
[Warthog project]: https://bitbucket.org/dharabor/pathfinding
