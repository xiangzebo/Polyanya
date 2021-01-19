# Utilities for polyanya

Useful utilities for parsing gridmaps, polymaps and meshes.

The spec for polymaps and meshes are contained within `spec`, and some example
maps have been included in `maps`.


# Tools

`gridmap2poly`: Converts grid maps into the polymap format.
Suitable for loading into Fade2D.
Takes a grid map from stdin, and prints the polymap to stdout.

`poly2mesh`: Converts polymaps into the mesh format. Suitable for search
algorithms.
Takes a polymap from stdin, and prints the mesh to stdout.
Note that `poly2mesh` outputs the Fade2D license when the program runs.

`visualiser`: Writes a PostScript file representing an input polymap.
Takes a polymap file **as the first argument**.

`meshpacker`: Compresses a mesh into a "packed" mesh. As a mesh comprises only
numbers, and is whitespace agnostic (you can parse a mesh, even with all the
new lines replaced with spaces) even though the spec does not explicitly allow
it. This "packed" file format comprises 3-byte integers such that the decimal
number 20 is encoded as 0x000014. It also has a header of "pack".
Takes a mesh file **as the first argument**, and outputs a packed mesh with a
`.packed` extension.

`meshunpacker`: Uncompresses a packed mesh. Takes a packed mesh
**as the first argument**, and outputs the original mesh without the `.packed`
extension.

`meshmerger`: Greedily merges polygons of a mesh together. This prioritises
merging polygons together to get the biggest polygon together, while also
ensuring that any "dead end" polygons are not removed by this merging. You can
also supply the `--pretty` flag to make the output easier to read (while being
slightly non-conforming to the spec). Takes a mesh from stdin, outputs to
stdout.

`gridmap2rects`: Greedily constructs rectangles from a gridmap into a mesh.
Constructs the best rectangle based on the heursitic
`min(width, height) * area`. This is to weight square-like rectangles more than
very wide or long rectangles.
Takes a gridmap from stdin, and outputs a mesh to stdout.

Included is a basic `gridmap2mesh` script which converts a gridmap to a mesh,
and also strips the Fade2D license from `poly2mesh`.


# Compiling

This has been tested on:

- Arch Linux, kernel release 4.13.3-1-ARCH and g++ 7.2.0
- Arch Linux, kernel release 4.8.13 and g++ 6.3.1
- Ubuntu 16.04 (via Windows Subsystem for Linux) and g++ 5.4.0

Ensure you have [GMP](https://gmplib.org/) installed
(`sudo apt-get install libgmp3-dev` on Ubuntu),
and run `make all`.
All the utilities will be compiled.

If you do not use Linux and still wish to compile all the tools which do not
use Fade2D and GMP, running `make nofade` will compile all the tools except
for `visualiser` and `poly2mesh`.


# Usage examples

Converting a grid map, `./maps/arena.map` into a triangulation and saving that
as `arena.mesh` in the current directory (requires Fade2D compilation):
```bash
$ ./scripts/gridmap2mesh.sh < maps/arena.map > arena.mesh
```

Unpacking a packed mesh (from
[this repo](https://bitbucket.org/mlcui1/polyanya-triangulations-packed/)),
`arena.mesh.packed`, to `arena.mesh`:
```bash
$ ./bin/meshunpacker arena.mesh.packed
$ head arena.mesh
mesh
2
112
120
2
2
2
-1
4
1
```

Converting a triangulation, `arena.mesh` to a merged triangulation:
```bash
$ ./bin/meshmerger < arena.mesh > arena-merged.mesh
55;31;118
$ head arena-merged.mesh
mesh
2
112 55
2 2 2 -1 1
1 3 2 6 -1
2 3 5 5 6 -1 1 0
3 2 4 2 0 1 -1
3 1 2 2 -1
15 1 2 -1 2
15 3 4 -1 5 0 2
```
Note that `55;31;118` is output to stderr, representing
`number of polygons;number of dead ends;sum of polygon degrees` of the
outputted mesh.
