#!/bin/bash
POLYANYA_FOLDER="`dirname $0`/../.."
MESH_FOLDER="$POLYANYA_FOLDER/meshes"
MAP_FOLDER="$POLYANYA_FOLDER/utils/maps"
for set in `ls $MAP_FOLDER`; do
    mkdir -p "$MESH_FOLDER/$set"
    for map in `ls maps/$set`; do
        echo "converting $set/$map"
        $POLYANYA_FOLDER/utils/scripts/gridmap2mesh.sh < $MAP_FOLDER/$set/$map > $MESH_FOLDER/$set/${map::-4}.mesh
        # is_correct=`($POLYANYA_FOLDER/utils/scripts/gridmap2mesh.sh < $MAP_FOLDER/$set/$map) | head -n 1`
        # if [[ $is_correct != "mesh" ]]; then
        #     echo "ERROR!!"
        # fi
    done
done
