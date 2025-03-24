#!/bin/bash
#
# test split result
# usage:
#	start with file to split as parameter but without 'gz' extension
#	sh test.sh [testfile].osm
#
# this will open [testfile].osm.gz
#
./osm_split $1.gz
#
rm ./result/*.osm
#
gunzip ./result/way_20.osm.gz
gunzip ./result/rel_20.osm.gz
gunzip ./result/node_20.osm.gz
cat ./result/node_20.osm ./result/way_20.osm ./result/rel_20.osm > test.osm
#
gunzip $1.gz
#
diff -b $1 test.osm
#
gzip $1
#
gzip ./result/way_20.osm
gzip ./result/rel_20.osm
gzip ./result/node_20.osm
#
