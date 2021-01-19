#!/bin/bash
# Be sure to set the stack limit!
POLYANYA_FOLDER="`dirname $0`/../.."
MESH_FOLDER="$POLYANYA_FOLDER/meshes"
MAP_FOLDER="$POLYANYA_FOLDER/utils/maps"
for map in $MAP_FOLDER/random/random512-{10,15,20,25,30,35}*; do
    bname=`basename $map`
    echo "converting $bname"
    $POLYANYA_FOLDER/utils/scripts/gridmap2mesh.sh < $map > $MESH_FOLDER/random/${bname::-4}.mesh
done
