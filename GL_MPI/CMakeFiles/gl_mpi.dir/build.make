# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/local/bin/cmake

# The command to remove a file.
RM = /opt/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /opt/local/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/REN/C/GL_MPI

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/REN/C/GL_MPI

# Include any dependencies generated for this target.
include CMakeFiles/gl_mpi.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/gl_mpi.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/gl_mpi.dir/flags.make

CMakeFiles/gl_mpi.dir/main.c.o: CMakeFiles/gl_mpi.dir/flags.make
CMakeFiles/gl_mpi.dir/main.c.o: main.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/REN/C/GL_MPI/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/gl_mpi.dir/main.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/gl_mpi.dir/main.c.o   -c /Users/REN/C/GL_MPI/main.c

CMakeFiles/gl_mpi.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gl_mpi.dir/main.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Users/REN/C/GL_MPI/main.c > CMakeFiles/gl_mpi.dir/main.c.i

CMakeFiles/gl_mpi.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gl_mpi.dir/main.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Users/REN/C/GL_MPI/main.c -o CMakeFiles/gl_mpi.dir/main.c.s

CMakeFiles/gl_mpi.dir/main.c.o.requires:
.PHONY : CMakeFiles/gl_mpi.dir/main.c.o.requires

CMakeFiles/gl_mpi.dir/main.c.o.provides: CMakeFiles/gl_mpi.dir/main.c.o.requires
	$(MAKE) -f CMakeFiles/gl_mpi.dir/build.make CMakeFiles/gl_mpi.dir/main.c.o.provides.build
.PHONY : CMakeFiles/gl_mpi.dir/main.c.o.provides

CMakeFiles/gl_mpi.dir/main.c.o.provides.build: CMakeFiles/gl_mpi.dir/main.c.o

# Object files for target gl_mpi
gl_mpi_OBJECTS = \
"CMakeFiles/gl_mpi.dir/main.c.o"

# External object files for target gl_mpi
gl_mpi_EXTERNAL_OBJECTS =

gl_mpi: CMakeFiles/gl_mpi.dir/main.c.o
gl_mpi: CMakeFiles/gl_mpi.dir/build.make
gl_mpi: /opt/local/mpich2/lib/libmpichcxx.a
gl_mpi: /opt/local/mpich2/lib/libpmpich.a
gl_mpi: /opt/local/mpich2/lib/libmpich.a
gl_mpi: /opt/local/mpich2/lib/libopa.a
gl_mpi: /opt/local/mpich2/lib/libmpl.a
gl_mpi: /usr/lib/libpthread.dylib
gl_mpi: CMakeFiles/gl_mpi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable gl_mpi"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gl_mpi.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/gl_mpi.dir/build: gl_mpi
.PHONY : CMakeFiles/gl_mpi.dir/build

CMakeFiles/gl_mpi.dir/requires: CMakeFiles/gl_mpi.dir/main.c.o.requires
.PHONY : CMakeFiles/gl_mpi.dir/requires

CMakeFiles/gl_mpi.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/gl_mpi.dir/cmake_clean.cmake
.PHONY : CMakeFiles/gl_mpi.dir/clean

CMakeFiles/gl_mpi.dir/depend:
	cd /Users/REN/C/GL_MPI && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/REN/C/GL_MPI /Users/REN/C/GL_MPI /Users/REN/C/GL_MPI /Users/REN/C/GL_MPI /Users/REN/C/GL_MPI/CMakeFiles/gl_mpi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/gl_mpi.dir/depend
