cmake_minimum_required(VERSION 3.10)

# set the project name
project(osm_split)

# add the executable
add_executable(osm_split main.c osm_split.c ../common/read_version.c ../common/osm_info.c ../common/simple_sax.c ../common/zblock_io.c ../common/cmd_func.c)

target_link_libraries(osm_split ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(osm_split ${ZLIB_LIBRARIES})

