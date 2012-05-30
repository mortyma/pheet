# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cederman/workspace/pheet

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cederman/workspace/pheet

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running interactive CMake command-line interface..."
	/usr/bin/cmake -i .
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/cederman/workspace/pheet/CMakeFiles /home/cederman/workspace/pheet/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/cederman/workspace/pheet/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named pheet_test

# Build rule for target.
pheet_test: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 pheet_test
.PHONY : pheet_test

# fast build rule for target.
pheet_test/fast:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/build
.PHONY : pheet_test/fast

test/Test.o: test/Test.cpp.o
.PHONY : test/Test.o

# target to build an object file
test/Test.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/Test.cpp.o
.PHONY : test/Test.cpp.o

test/Test.i: test/Test.cpp.i
.PHONY : test/Test.i

# target to preprocess a source file
test/Test.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/Test.cpp.i
.PHONY : test/Test.cpp.i

test/Test.s: test/Test.cpp.s
.PHONY : test/Test.s

# target to generate assembly for a file
test/Test.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/Test.cpp.s
.PHONY : test/Test.cpp.s

test/graph_bipartitioning/BranchBound/BasicLowerBound.o: test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.o
.PHONY : test/graph_bipartitioning/BranchBound/BasicLowerBound.o

# target to build an object file
test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.o
.PHONY : test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.o

test/graph_bipartitioning/BranchBound/BasicLowerBound.i: test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.i
.PHONY : test/graph_bipartitioning/BranchBound/BasicLowerBound.i

# target to preprocess a source file
test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.i
.PHONY : test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.i

test/graph_bipartitioning/BranchBound/BasicLowerBound.s: test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.s
.PHONY : test/graph_bipartitioning/BranchBound/BasicLowerBound.s

# target to generate assembly for a file
test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.s
.PHONY : test/graph_bipartitioning/BranchBound/BasicLowerBound.cpp.s

test/graph_bipartitioning/BranchBound/BasicNextVertex.o: test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.o
.PHONY : test/graph_bipartitioning/BranchBound/BasicNextVertex.o

# target to build an object file
test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.o
.PHONY : test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.o

test/graph_bipartitioning/BranchBound/BasicNextVertex.i: test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.i
.PHONY : test/graph_bipartitioning/BranchBound/BasicNextVertex.i

# target to preprocess a source file
test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.i
.PHONY : test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.i

test/graph_bipartitioning/BranchBound/BasicNextVertex.s: test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.s
.PHONY : test/graph_bipartitioning/BranchBound/BasicNextVertex.s

# target to generate assembly for a file
test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.s
.PHONY : test/graph_bipartitioning/BranchBound/BasicNextVertex.cpp.s

test/graph_bipartitioning/GraphBipartitioningTests.o: test/graph_bipartitioning/GraphBipartitioningTests.cpp.o
.PHONY : test/graph_bipartitioning/GraphBipartitioningTests.o

# target to build an object file
test/graph_bipartitioning/GraphBipartitioningTests.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/GraphBipartitioningTests.cpp.o
.PHONY : test/graph_bipartitioning/GraphBipartitioningTests.cpp.o

test/graph_bipartitioning/GraphBipartitioningTests.i: test/graph_bipartitioning/GraphBipartitioningTests.cpp.i
.PHONY : test/graph_bipartitioning/GraphBipartitioningTests.i

# target to preprocess a source file
test/graph_bipartitioning/GraphBipartitioningTests.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/GraphBipartitioningTests.cpp.i
.PHONY : test/graph_bipartitioning/GraphBipartitioningTests.cpp.i

test/graph_bipartitioning/GraphBipartitioningTests.s: test/graph_bipartitioning/GraphBipartitioningTests.cpp.s
.PHONY : test/graph_bipartitioning/GraphBipartitioningTests.s

# target to generate assembly for a file
test/graph_bipartitioning/GraphBipartitioningTests.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/graph_bipartitioning/GraphBipartitioningTests.cpp.s
.PHONY : test/graph_bipartitioning/GraphBipartitioningTests.cpp.s

test/inarow/InARowTests.o: test/inarow/InARowTests.cpp.o
.PHONY : test/inarow/InARowTests.o

# target to build an object file
test/inarow/InARowTests.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/inarow/InARowTests.cpp.o
.PHONY : test/inarow/InARowTests.cpp.o

test/inarow/InARowTests.i: test/inarow/InARowTests.cpp.i
.PHONY : test/inarow/InARowTests.i

# target to preprocess a source file
test/inarow/InARowTests.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/inarow/InARowTests.cpp.i
.PHONY : test/inarow/InARowTests.cpp.i

test/inarow/InARowTests.s: test/inarow/InARowTests.cpp.s
.PHONY : test/inarow/InARowTests.s

# target to generate assembly for a file
test/inarow/InARowTests.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/inarow/InARowTests.cpp.s
.PHONY : test/inarow/InARowTests.cpp.s

test/lupiv/LUPivTests.o: test/lupiv/LUPivTests.cpp.o
.PHONY : test/lupiv/LUPivTests.o

# target to build an object file
test/lupiv/LUPivTests.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/lupiv/LUPivTests.cpp.o
.PHONY : test/lupiv/LUPivTests.cpp.o

test/lupiv/LUPivTests.i: test/lupiv/LUPivTests.cpp.i
.PHONY : test/lupiv/LUPivTests.i

# target to preprocess a source file
test/lupiv/LUPivTests.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/lupiv/LUPivTests.cpp.i
.PHONY : test/lupiv/LUPivTests.cpp.i

test/lupiv/LUPivTests.s: test/lupiv/LUPivTests.cpp.s
.PHONY : test/lupiv/LUPivTests.s

# target to generate assembly for a file
test/lupiv/LUPivTests.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/lupiv/LUPivTests.cpp.s
.PHONY : test/lupiv/LUPivTests.cpp.s

test/n-queens/NQueensTests.o: test/n-queens/NQueensTests.cpp.o
.PHONY : test/n-queens/NQueensTests.o

# target to build an object file
test/n-queens/NQueensTests.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/n-queens/NQueensTests.cpp.o
.PHONY : test/n-queens/NQueensTests.cpp.o

test/n-queens/NQueensTests.i: test/n-queens/NQueensTests.cpp.i
.PHONY : test/n-queens/NQueensTests.i

# target to preprocess a source file
test/n-queens/NQueensTests.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/n-queens/NQueensTests.cpp.i
.PHONY : test/n-queens/NQueensTests.cpp.i

test/n-queens/NQueensTests.s: test/n-queens/NQueensTests.cpp.s
.PHONY : test/n-queens/NQueensTests.s

# target to generate assembly for a file
test/n-queens/NQueensTests.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/n-queens/NQueensTests.cpp.s
.PHONY : test/n-queens/NQueensTests.cpp.s

test/pheet_tests.o: test/pheet_tests.cpp.o
.PHONY : test/pheet_tests.o

# target to build an object file
test/pheet_tests.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/pheet_tests.cpp.o
.PHONY : test/pheet_tests.cpp.o

test/pheet_tests.i: test/pheet_tests.cpp.i
.PHONY : test/pheet_tests.i

# target to preprocess a source file
test/pheet_tests.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/pheet_tests.cpp.i
.PHONY : test/pheet_tests.cpp.i

test/pheet_tests.s: test/pheet_tests.cpp.s
.PHONY : test/pheet_tests.s

# target to generate assembly for a file
test/pheet_tests.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/pheet_tests.cpp.s
.PHONY : test/pheet_tests.cpp.s

test/sor/SORTests.o: test/sor/SORTests.cpp.o
.PHONY : test/sor/SORTests.o

# target to build an object file
test/sor/SORTests.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/sor/SORTests.cpp.o
.PHONY : test/sor/SORTests.cpp.o

test/sor/SORTests.i: test/sor/SORTests.cpp.i
.PHONY : test/sor/SORTests.i

# target to preprocess a source file
test/sor/SORTests.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/sor/SORTests.cpp.i
.PHONY : test/sor/SORTests.cpp.i

test/sor/SORTests.s: test/sor/SORTests.cpp.s
.PHONY : test/sor/SORTests.s

# target to generate assembly for a file
test/sor/SORTests.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/sor/SORTests.cpp.s
.PHONY : test/sor/SORTests.cpp.s

test/sorting/SortingTests.o: test/sorting/SortingTests.cpp.o
.PHONY : test/sorting/SortingTests.o

# target to build an object file
test/sorting/SortingTests.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/sorting/SortingTests.cpp.o
.PHONY : test/sorting/SortingTests.cpp.o

test/sorting/SortingTests.i: test/sorting/SortingTests.cpp.i
.PHONY : test/sorting/SortingTests.i

# target to preprocess a source file
test/sorting/SortingTests.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/sorting/SortingTests.cpp.i
.PHONY : test/sorting/SortingTests.cpp.i

test/sorting/SortingTests.s: test/sorting/SortingTests.cpp.s
.PHONY : test/sorting/SortingTests.s

# target to generate assembly for a file
test/sorting/SortingTests.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/sorting/SortingTests.cpp.s
.PHONY : test/sorting/SortingTests.cpp.s

test/uts/RecursiveSearch/rng/brg_sha1.o: test/uts/RecursiveSearch/rng/brg_sha1.cpp.o
.PHONY : test/uts/RecursiveSearch/rng/brg_sha1.o

# target to build an object file
test/uts/RecursiveSearch/rng/brg_sha1.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/RecursiveSearch/rng/brg_sha1.cpp.o
.PHONY : test/uts/RecursiveSearch/rng/brg_sha1.cpp.o

test/uts/RecursiveSearch/rng/brg_sha1.i: test/uts/RecursiveSearch/rng/brg_sha1.cpp.i
.PHONY : test/uts/RecursiveSearch/rng/brg_sha1.i

# target to preprocess a source file
test/uts/RecursiveSearch/rng/brg_sha1.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/RecursiveSearch/rng/brg_sha1.cpp.i
.PHONY : test/uts/RecursiveSearch/rng/brg_sha1.cpp.i

test/uts/RecursiveSearch/rng/brg_sha1.s: test/uts/RecursiveSearch/rng/brg_sha1.cpp.s
.PHONY : test/uts/RecursiveSearch/rng/brg_sha1.s

# target to generate assembly for a file
test/uts/RecursiveSearch/rng/brg_sha1.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/RecursiveSearch/rng/brg_sha1.cpp.s
.PHONY : test/uts/RecursiveSearch/rng/brg_sha1.cpp.s

test/uts/RecursiveSearch/uts.o: test/uts/RecursiveSearch/uts.cpp.o
.PHONY : test/uts/RecursiveSearch/uts.o

# target to build an object file
test/uts/RecursiveSearch/uts.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/RecursiveSearch/uts.cpp.o
.PHONY : test/uts/RecursiveSearch/uts.cpp.o

test/uts/RecursiveSearch/uts.i: test/uts/RecursiveSearch/uts.cpp.i
.PHONY : test/uts/RecursiveSearch/uts.i

# target to preprocess a source file
test/uts/RecursiveSearch/uts.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/RecursiveSearch/uts.cpp.i
.PHONY : test/uts/RecursiveSearch/uts.cpp.i

test/uts/RecursiveSearch/uts.s: test/uts/RecursiveSearch/uts.cpp.s
.PHONY : test/uts/RecursiveSearch/uts.s

# target to generate assembly for a file
test/uts/RecursiveSearch/uts.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/RecursiveSearch/uts.cpp.s
.PHONY : test/uts/RecursiveSearch/uts.cpp.s

test/uts/UTSImp.o: test/uts/UTSImp.cpp.o
.PHONY : test/uts/UTSImp.o

# target to build an object file
test/uts/UTSImp.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/UTSImp.cpp.o
.PHONY : test/uts/UTSImp.cpp.o

test/uts/UTSImp.i: test/uts/UTSImp.cpp.i
.PHONY : test/uts/UTSImp.i

# target to preprocess a source file
test/uts/UTSImp.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/UTSImp.cpp.i
.PHONY : test/uts/UTSImp.cpp.i

test/uts/UTSImp.s: test/uts/UTSImp.cpp.s
.PHONY : test/uts/UTSImp.s

# target to generate assembly for a file
test/uts/UTSImp.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/UTSImp.cpp.s
.PHONY : test/uts/UTSImp.cpp.s

test/uts/UTSTests.o: test/uts/UTSTests.cpp.o
.PHONY : test/uts/UTSTests.o

# target to build an object file
test/uts/UTSTests.cpp.o:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/UTSTests.cpp.o
.PHONY : test/uts/UTSTests.cpp.o

test/uts/UTSTests.i: test/uts/UTSTests.cpp.i
.PHONY : test/uts/UTSTests.i

# target to preprocess a source file
test/uts/UTSTests.cpp.i:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/UTSTests.cpp.i
.PHONY : test/uts/UTSTests.cpp.i

test/uts/UTSTests.s: test/uts/UTSTests.cpp.s
.PHONY : test/uts/UTSTests.s

# target to generate assembly for a file
test/uts/UTSTests.cpp.s:
	$(MAKE) -f CMakeFiles/pheet_test.dir/build.make CMakeFiles/pheet_test.dir/test/uts/UTSTests.cpp.s
.PHONY : test/uts/UTSTests.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... pheet_test"
	@echo "... rebuild_cache"
	@echo "... test/Test.o"
	@echo "... test/Test.i"
	@echo "... test/Test.s"
	@echo "... test/graph_bipartitioning/BranchBound/BasicLowerBound.o"
	@echo "... test/graph_bipartitioning/BranchBound/BasicLowerBound.i"
	@echo "... test/graph_bipartitioning/BranchBound/BasicLowerBound.s"
	@echo "... test/graph_bipartitioning/BranchBound/BasicNextVertex.o"
	@echo "... test/graph_bipartitioning/BranchBound/BasicNextVertex.i"
	@echo "... test/graph_bipartitioning/BranchBound/BasicNextVertex.s"
	@echo "... test/graph_bipartitioning/GraphBipartitioningTests.o"
	@echo "... test/graph_bipartitioning/GraphBipartitioningTests.i"
	@echo "... test/graph_bipartitioning/GraphBipartitioningTests.s"
	@echo "... test/inarow/InARowTests.o"
	@echo "... test/inarow/InARowTests.i"
	@echo "... test/inarow/InARowTests.s"
	@echo "... test/lupiv/LUPivTests.o"
	@echo "... test/lupiv/LUPivTests.i"
	@echo "... test/lupiv/LUPivTests.s"
	@echo "... test/n-queens/NQueensTests.o"
	@echo "... test/n-queens/NQueensTests.i"
	@echo "... test/n-queens/NQueensTests.s"
	@echo "... test/pheet_tests.o"
	@echo "... test/pheet_tests.i"
	@echo "... test/pheet_tests.s"
	@echo "... test/sor/SORTests.o"
	@echo "... test/sor/SORTests.i"
	@echo "... test/sor/SORTests.s"
	@echo "... test/sorting/SortingTests.o"
	@echo "... test/sorting/SortingTests.i"
	@echo "... test/sorting/SortingTests.s"
	@echo "... test/uts/RecursiveSearch/rng/brg_sha1.o"
	@echo "... test/uts/RecursiveSearch/rng/brg_sha1.i"
	@echo "... test/uts/RecursiveSearch/rng/brg_sha1.s"
	@echo "... test/uts/RecursiveSearch/uts.o"
	@echo "... test/uts/RecursiveSearch/uts.i"
	@echo "... test/uts/RecursiveSearch/uts.s"
	@echo "... test/uts/UTSImp.o"
	@echo "... test/uts/UTSImp.i"
	@echo "... test/uts/UTSImp.s"
	@echo "... test/uts/UTSTests.o"
	@echo "... test/uts/UTSTests.i"
	@echo "... test/uts/UTSTests.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

