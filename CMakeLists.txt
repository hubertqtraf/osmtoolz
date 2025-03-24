cmake_minimum_required(VERSION 3.10)

project(osmtoolz)

find_package(Threads)
find_package(ZLIB)

# check the options: make VERBOSE=1
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-long-long -pedantic -std=c18 -O3 -g ")
endif()

# if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
#  include(CTest)
# endif()

#add_subdirectory (osm_cut)
#add_subdirectory (osm_nodes)
#add_subdirectory (osm_ways)
add_subdirectory (osm_split)

