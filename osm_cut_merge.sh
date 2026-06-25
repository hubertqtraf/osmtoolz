#!/bin/bash
# Example:
#./osm_cut_merge/osm_cut -f "/run/media/.../osm/planet-220110/s_alps" -r 3,15,43,50 -o "/run/media/.../osm/planet-220110/alps2"
# sh osm_cut_mege.sh "/run/media/.../osm/planet-220110" planet-220110a -30,30,30,80 test
echo "$1"/$4.osm
./osm_cut/osm_cut -f "$1"/$2 -r $3 -o "$1"/$4
DATA="$(pwd)/osm_merge"
cd ./osm_merge
sh omerge.sh "$1"/$4 "$1"/$4.osm
echo created: "$1"/$4.osm
