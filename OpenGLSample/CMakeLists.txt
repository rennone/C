#要求するcmakeのバージョン
cmake_minimum_required(VERSION 2.8)

#プロジェクト名
project(texture)

find_package(GLUT)
find_package(OpenGL)
include_directories(~/glew-1.10.0/include)
link_directories(~/glew-1.10.0/lib)

add_executable(texture texture.c)
target_link_libraries(texture libGLEW.a)
target_link_libraries(texture ${GLUT_LIBRARY} ${OPENGL_LIBRARY} m)
