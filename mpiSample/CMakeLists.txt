#要求するcmakeのバージョン
cmake_minimum_required(VERSION 2.8)

#project name
PROJECT(mpiSample1)

# find MPI
find_package(MPI)

include_directories(/opt/local/mpich2/include)
link_directories(/opt/local/mpich2/lib)

add_executable(mpiSample1 mpiSample1.c)
target_link_libraries(mpiSample1 ${MPI_LIBRARIES})


