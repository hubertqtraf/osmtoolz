gunzip -c $1_way_20.osm.gz > way.osm
sed '1d;2d;$d' way.osm > way1.osm
gunzip -c $1_rel_20.osm.gz > rel.osm
sed '1d;2d;$d' rel.osm > rel1.osm
gunzip -c $1_node_20.osm.gz > node.osm
sed '1d;2d;$d' node.osm > node1.osm
cat head.txt node1.osm way1.osm rel1.osm tail.txt > $2
rm way.osm way1.osm rel.osm rel1.osm node.osm node1.osm 
