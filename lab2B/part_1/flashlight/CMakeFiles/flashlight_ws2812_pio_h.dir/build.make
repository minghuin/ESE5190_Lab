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
CMAKE_SOURCE_DIR = "/home/ncm656112/ESE519 Lab/lab2A/flashlight"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/ncm656112/ESE519 Lab/lab2A/flashlight"

# Utility rule file for flashlight_ws2812_pio_h.

# Include any custom commands dependencies for this target.
include CMakeFiles/flashlight_ws2812_pio_h.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/flashlight_ws2812_pio_h.dir/progress.make

CMakeFiles/flashlight_ws2812_pio_h: generated/ws2812.pio.h

generated/ws2812.pio.h: ws2812.pio
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir="/home/ncm656112/ESE519 Lab/lab2A/flashlight/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Generating generated/ws2812.pio.h"
	pioasm/pioasm -o c-sdk /home/ncm656112/ESE519\ Lab/lab2A/flashlight/ws2812.pio /home/ncm656112/ESE519\ Lab/lab2A/flashlight/generated/ws2812.pio.h

flashlight_ws2812_pio_h: CMakeFiles/flashlight_ws2812_pio_h
flashlight_ws2812_pio_h: generated/ws2812.pio.h
flashlight_ws2812_pio_h: CMakeFiles/flashlight_ws2812_pio_h.dir/build.make
.PHONY : flashlight_ws2812_pio_h

# Rule to build all files generated by this target.
CMakeFiles/flashlight_ws2812_pio_h.dir/build: flashlight_ws2812_pio_h
.PHONY : CMakeFiles/flashlight_ws2812_pio_h.dir/build

CMakeFiles/flashlight_ws2812_pio_h.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/flashlight_ws2812_pio_h.dir/cmake_clean.cmake
.PHONY : CMakeFiles/flashlight_ws2812_pio_h.dir/clean

CMakeFiles/flashlight_ws2812_pio_h.dir/depend:
	cd "/home/ncm656112/ESE519 Lab/lab2A/flashlight" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/ncm656112/ESE519 Lab/lab2A/flashlight" "/home/ncm656112/ESE519 Lab/lab2A/flashlight" "/home/ncm656112/ESE519 Lab/lab2A/flashlight" "/home/ncm656112/ESE519 Lab/lab2A/flashlight" "/home/ncm656112/ESE519 Lab/lab2A/flashlight/CMakeFiles/flashlight_ws2812_pio_h.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/flashlight_ws2812_pio_h.dir/depend

