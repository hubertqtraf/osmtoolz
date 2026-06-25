#!/bin/bash
# Example:
#./osm_merge/osm_merge -f "/run/media/.../osm/planet-220110/s_alps" "/run/media/.../osm/planet-220110/s_alps.osm"
# sh osm_cut.sh "/run/media/.../osm/planet-220110" s_alps
echo "$1"/$2.osm
cd ./osm_merge
sh omerge.sh "$1"/$2 "$1"/$2.osm
echo created: "$1"/$2.osm
