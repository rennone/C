#要求するcmakeのバージョン
cmake_minimum_required(VERSION 2.8)

#プロジェクト名
project(fdtd)

# MPI, GLUT, OpenGLのフォルダを見つける?
find_package(MPI REQUIRED)
find_package(GLUT)
find_package(OpenGL)

include_directories(/opt/local/include)
include_directories(/opt/local/mpich2/include)
include_directories(~/glew-1.10.0/include)
link_directories(/opt/local/mpich2/lib)
link_directories(~/glew-1.10.0/lib)
add_executable(fdtd main.c simulator.c fdtdTE.c fdtdTE_upml.c fdtdTM.c fdtdTM_upml.c field.c drawer.c models.c noModel.c circleModel.c)

#最適化オプション?
#set(CMAKE_BUILD_TYPE release)

target_link_libraries(fdtd ${MPI_LIBRARIES})
target_link_libraries(fdtd ${GLUT_LIBRARY} ${OPENGL_LIBRARY} m)
target_link_libraries(fdtd GLEW.a)

#以下はいらない
#include_directories(MPI_INCLUDE_PATH)
#set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
#set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} &{MPI_LINK_FLAGS})
#set(CMAKE_C_COMPILE_FLAGS ${CMAKE_C_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
#set(CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS} &{MPI_LINK_FLAGS})
#enable_testing()
#add_test(SimpleText ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4 ${CMAKE_CURRENT_BINARY_DIR}/hello)
#SET(CMAKE_C_COMPILER mpicc)

