cmake_minimum_required(VERSION 3.10)

# set the project name
project(osm_huge)

# add the executable
add_executable(osm_huge main.c osm_cut_node_rect.c osm_cut_node_write.c ../common/std_param.c ../common/read_version.c ../common/osm_info.c ../common/simple_sax.c ../common/zblock_io.c ../common/cmd_func.c)

target_link_libraries(osm_huge ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(osm_huge ${ZLIB_LIBRARIES})

