cmake_minimum_required(VERSION 3.10)

# set the project name
project(osm_cut)

# add the executable
add_executable(osm_cut main.c osm_cut_node.c osm_cut_way.c osm_rel_write.c osm_cut_rel.c osm_node_write.c osm_way_write.c ../common/std_param.c ../common/read_version.c ../common/osm_info.c ../common/simple_sax.c ../common/zblock_io.c ../common/cmd_func.c)

target_link_libraries(osm_cut ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(osm_cut ${ZLIB_LIBRARIES})
