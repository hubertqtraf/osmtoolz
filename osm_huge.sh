#!/bin/bash
# Example:
#./osm_huge/osm_huge -f "/run/media/.../osm/planet-220110/s_alps" -r 3,15,43,50 -o "/run/media/.../osm/planet-220110/alps2"
# sh osm_huge.sh "/run/media/.../osm/planet-220110" planet-220110a -30,30,30,80 test
echo "$1"/$2.osminfo.gz
./osm_huge/osm_huge -f "$1"/$2 -r $3 -o "$1"/$4
echo ln -s $2_rel_20.osm.gz $4_rel_20.osm.gz
cd "$1"
#pwd
ln -s $2_rel_20.osm.gz $4_rel_20.osm.gz
