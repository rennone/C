#要求するcmakeのバージョン
cmake_minimum_required(VERSION 2.8)

#project name
PROJECT(sample2)

# find MPI
find_package(MPI)

include_directories(/opt/local/mpich2/include)
link_directories(/opt/local/mpich2/lib)

add_executable(sample2 main.c)
target_link_libraries(sample2 ${MPI_LIBRARIES})


