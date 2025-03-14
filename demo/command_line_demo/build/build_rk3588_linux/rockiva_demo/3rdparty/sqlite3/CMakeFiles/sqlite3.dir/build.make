# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mixtile/rockiva/demo/command_line_demo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux

# Include any dependencies generated for this target.
include rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/compiler_depend.make

# Include the progress variables for this target.
include rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/progress.make

# Include the compile flags for this target's objects.
include rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/flags.make

rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/src/sqlite3.c.o: rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/flags.make
rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/src/sqlite3.c.o: ../../rockiva_demo/3rdparty/sqlite3/src/sqlite3.c
rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/src/sqlite3.c.o: rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/src/sqlite3.c.o"
	cd /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/rockiva_demo/3rdparty/sqlite3 && /usr/bin/aarch64-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/src/sqlite3.c.o -MF CMakeFiles/sqlite3.dir/src/sqlite3.c.o.d -o CMakeFiles/sqlite3.dir/src/sqlite3.c.o -c /home/mixtile/rockiva/demo/command_line_demo/rockiva_demo/3rdparty/sqlite3/src/sqlite3.c

rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/src/sqlite3.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/sqlite3.dir/src/sqlite3.c.i"
	cd /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/rockiva_demo/3rdparty/sqlite3 && /usr/bin/aarch64-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/mixtile/rockiva/demo/command_line_demo/rockiva_demo/3rdparty/sqlite3/src/sqlite3.c > CMakeFiles/sqlite3.dir/src/sqlite3.c.i

rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/src/sqlite3.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/sqlite3.dir/src/sqlite3.c.s"
	cd /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/rockiva_demo/3rdparty/sqlite3 && /usr/bin/aarch64-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/mixtile/rockiva/demo/command_line_demo/rockiva_demo/3rdparty/sqlite3/src/sqlite3.c -o CMakeFiles/sqlite3.dir/src/sqlite3.c.s

# Object files for target sqlite3
sqlite3_OBJECTS = \
"CMakeFiles/sqlite3.dir/src/sqlite3.c.o"

# External object files for target sqlite3
sqlite3_EXTERNAL_OBJECTS =

rockiva_demo/3rdparty/sqlite3/libsqlite3.a: rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/src/sqlite3.c.o
rockiva_demo/3rdparty/sqlite3/libsqlite3.a: rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/build.make
rockiva_demo/3rdparty/sqlite3/libsqlite3.a: rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C static library libsqlite3.a"
	cd /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/rockiva_demo/3rdparty/sqlite3 && $(CMAKE_COMMAND) -P CMakeFiles/sqlite3.dir/cmake_clean_target.cmake
	cd /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/rockiva_demo/3rdparty/sqlite3 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sqlite3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/build: rockiva_demo/3rdparty/sqlite3/libsqlite3.a
.PHONY : rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/build

rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/clean:
	cd /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/rockiva_demo/3rdparty/sqlite3 && $(CMAKE_COMMAND) -P CMakeFiles/sqlite3.dir/cmake_clean.cmake
.PHONY : rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/clean

rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/depend:
	cd /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mixtile/rockiva/demo/command_line_demo /home/mixtile/rockiva/demo/command_line_demo/rockiva_demo/3rdparty/sqlite3 /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/rockiva_demo/3rdparty/sqlite3 /home/mixtile/rockiva/demo/command_line_demo/build/build_rk3588_linux/rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : rockiva_demo/3rdparty/sqlite3/CMakeFiles/sqlite3.dir/depend

