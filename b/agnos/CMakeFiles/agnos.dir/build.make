# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.30.1/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.30.1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/o/projects/asciiPainter

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/o/projects/asciiPainter/b

# Include any dependencies generated for this target.
include agnos/CMakeFiles/agnos.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include agnos/CMakeFiles/agnos.dir/compiler_depend.make

# Include the progress variables for this target.
include agnos/CMakeFiles/agnos.dir/progress.make

# Include the compile flags for this target's objects.
include agnos/CMakeFiles/agnos.dir/flags.make

agnos/CMakeFiles/agnos.dir/agnosUnix.cpp.o: agnos/CMakeFiles/agnos.dir/flags.make
agnos/CMakeFiles/agnos.dir/agnosUnix.cpp.o: /Users/o/projects/asciiPainter/agnos/agnosUnix.cpp
agnos/CMakeFiles/agnos.dir/agnosUnix.cpp.o: agnos/CMakeFiles/agnos.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/o/projects/asciiPainter/b/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object agnos/CMakeFiles/agnos.dir/agnosUnix.cpp.o"
	cd /Users/o/projects/asciiPainter/b/agnos && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT agnos/CMakeFiles/agnos.dir/agnosUnix.cpp.o -MF CMakeFiles/agnos.dir/agnosUnix.cpp.o.d -o CMakeFiles/agnos.dir/agnosUnix.cpp.o -c /Users/o/projects/asciiPainter/agnos/agnosUnix.cpp

agnos/CMakeFiles/agnos.dir/agnosUnix.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/agnos.dir/agnosUnix.cpp.i"
	cd /Users/o/projects/asciiPainter/b/agnos && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/o/projects/asciiPainter/agnos/agnosUnix.cpp > CMakeFiles/agnos.dir/agnosUnix.cpp.i

agnos/CMakeFiles/agnos.dir/agnosUnix.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/agnos.dir/agnosUnix.cpp.s"
	cd /Users/o/projects/asciiPainter/b/agnos && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/o/projects/asciiPainter/agnos/agnosUnix.cpp -o CMakeFiles/agnos.dir/agnosUnix.cpp.s

# Object files for target agnos
agnos_OBJECTS = \
"CMakeFiles/agnos.dir/agnosUnix.cpp.o"

# External object files for target agnos
agnos_EXTERNAL_OBJECTS =

agnos/libagnos.a: agnos/CMakeFiles/agnos.dir/agnosUnix.cpp.o
agnos/libagnos.a: agnos/CMakeFiles/agnos.dir/build.make
agnos/libagnos.a: agnos/CMakeFiles/agnos.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/o/projects/asciiPainter/b/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libagnos.a"
	cd /Users/o/projects/asciiPainter/b/agnos && $(CMAKE_COMMAND) -P CMakeFiles/agnos.dir/cmake_clean_target.cmake
	cd /Users/o/projects/asciiPainter/b/agnos && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/agnos.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
agnos/CMakeFiles/agnos.dir/build: agnos/libagnos.a
.PHONY : agnos/CMakeFiles/agnos.dir/build

agnos/CMakeFiles/agnos.dir/clean:
	cd /Users/o/projects/asciiPainter/b/agnos && $(CMAKE_COMMAND) -P CMakeFiles/agnos.dir/cmake_clean.cmake
.PHONY : agnos/CMakeFiles/agnos.dir/clean

agnos/CMakeFiles/agnos.dir/depend:
	cd /Users/o/projects/asciiPainter/b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/o/projects/asciiPainter /Users/o/projects/asciiPainter/agnos /Users/o/projects/asciiPainter/b /Users/o/projects/asciiPainter/b/agnos /Users/o/projects/asciiPainter/b/agnos/CMakeFiles/agnos.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : agnos/CMakeFiles/agnos.dir/depend

