# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

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
CMAKE_COMMAND = /users/oahmed4/.local/bin/cmake

# The command to remove a file.
RM = /users/oahmed4/.local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /users/oahmed4/Strongly-Consistent-FS

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /users/oahmed4/Strongly-Consistent-FS/cmake/build

# Include any dependencies generated for this target.
include CMakeFiles/master.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/master.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/master.dir/flags.make

CMakeFiles/master.dir/master.cc.o: CMakeFiles/master.dir/flags.make
CMakeFiles/master.dir/master.cc.o: ../../master.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/users/oahmed4/Strongly-Consistent-FS/cmake/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/master.dir/master.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/master.dir/master.cc.o -c /users/oahmed4/Strongly-Consistent-FS/master.cc

CMakeFiles/master.dir/master.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/master.dir/master.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /users/oahmed4/Strongly-Consistent-FS/master.cc > CMakeFiles/master.dir/master.cc.i

CMakeFiles/master.dir/master.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/master.dir/master.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /users/oahmed4/Strongly-Consistent-FS/master.cc -o CMakeFiles/master.dir/master.cc.s

# Object files for target master
master_OBJECTS = \
"CMakeFiles/master.dir/master.cc.o"

# External object files for target master
master_EXTERNAL_OBJECTS =

master: CMakeFiles/master.dir/master.cc.o
master: CMakeFiles/master.dir/build.make
master: /usr/lib/x86_64-linux-gnu/libpthread.so
master: libwifs_grpc_proto.a
master: libprimarybackup_grpc_proto.a
master: /users/oahmed4/.local/lib/libgrpc++_reflection.a
master: /users/oahmed4/.local/lib/libgrpc++.a
master: /users/oahmed4/.local/lib/libprotobuf.a
master: /users/oahmed4/.local/lib/libgrpc.a
master: /users/oahmed4/.local/lib/libz.a
master: /users/oahmed4/.local/lib/libcares.a
master: /users/oahmed4/.local/lib/libaddress_sorting.a
master: /users/oahmed4/.local/lib/libre2.a
master: /users/oahmed4/.local/lib/libabsl_raw_hash_set.a
master: /users/oahmed4/.local/lib/libabsl_hashtablez_sampler.a
master: /users/oahmed4/.local/lib/libabsl_hash.a
master: /users/oahmed4/.local/lib/libabsl_city.a
master: /users/oahmed4/.local/lib/libabsl_low_level_hash.a
master: /users/oahmed4/.local/lib/libabsl_statusor.a
master: /users/oahmed4/.local/lib/libabsl_bad_variant_access.a
master: /users/oahmed4/.local/lib/libgpr.a
master: /users/oahmed4/.local/lib/libupb.a
master: /users/oahmed4/.local/lib/libabsl_status.a
master: /users/oahmed4/.local/lib/libabsl_random_distributions.a
master: /users/oahmed4/.local/lib/libabsl_random_seed_sequences.a
master: /users/oahmed4/.local/lib/libabsl_random_internal_pool_urbg.a
master: /users/oahmed4/.local/lib/libabsl_random_internal_randen.a
master: /users/oahmed4/.local/lib/libabsl_random_internal_randen_hwaes.a
master: /users/oahmed4/.local/lib/libabsl_random_internal_randen_hwaes_impl.a
master: /users/oahmed4/.local/lib/libabsl_random_internal_randen_slow.a
master: /users/oahmed4/.local/lib/libabsl_random_internal_platform.a
master: /users/oahmed4/.local/lib/libabsl_random_internal_seed_material.a
master: /users/oahmed4/.local/lib/libabsl_random_seed_gen_exception.a
master: /users/oahmed4/.local/lib/libabsl_cord.a
master: /users/oahmed4/.local/lib/libabsl_bad_optional_access.a
master: /users/oahmed4/.local/lib/libabsl_cordz_info.a
master: /users/oahmed4/.local/lib/libabsl_cord_internal.a
master: /users/oahmed4/.local/lib/libabsl_cordz_functions.a
master: /users/oahmed4/.local/lib/libabsl_exponential_biased.a
master: /users/oahmed4/.local/lib/libabsl_cordz_handle.a
master: /users/oahmed4/.local/lib/libabsl_str_format_internal.a
master: /users/oahmed4/.local/lib/libabsl_synchronization.a
master: /users/oahmed4/.local/lib/libabsl_stacktrace.a
master: /users/oahmed4/.local/lib/libabsl_symbolize.a
master: /users/oahmed4/.local/lib/libabsl_debugging_internal.a
master: /users/oahmed4/.local/lib/libabsl_demangle_internal.a
master: /users/oahmed4/.local/lib/libabsl_graphcycles_internal.a
master: /users/oahmed4/.local/lib/libabsl_malloc_internal.a
master: /users/oahmed4/.local/lib/libabsl_time.a
master: /users/oahmed4/.local/lib/libabsl_strings.a
master: /users/oahmed4/.local/lib/libabsl_throw_delegate.a
master: /users/oahmed4/.local/lib/libabsl_int128.a
master: /users/oahmed4/.local/lib/libabsl_strings_internal.a
master: /users/oahmed4/.local/lib/libabsl_base.a
master: /users/oahmed4/.local/lib/libabsl_spinlock_wait.a
master: /users/oahmed4/.local/lib/libabsl_raw_logging_internal.a
master: /users/oahmed4/.local/lib/libabsl_log_severity.a
master: /users/oahmed4/.local/lib/libabsl_civil_time.a
master: /users/oahmed4/.local/lib/libabsl_time_zone.a
master: /users/oahmed4/.local/lib/libssl.a
master: /users/oahmed4/.local/lib/libcrypto.a
master: CMakeFiles/master.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/users/oahmed4/Strongly-Consistent-FS/cmake/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable master"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/master.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/master.dir/build: master

.PHONY : CMakeFiles/master.dir/build

CMakeFiles/master.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/master.dir/cmake_clean.cmake
.PHONY : CMakeFiles/master.dir/clean

CMakeFiles/master.dir/depend:
	cd /users/oahmed4/Strongly-Consistent-FS/cmake/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /users/oahmed4/Strongly-Consistent-FS /users/oahmed4/Strongly-Consistent-FS /users/oahmed4/Strongly-Consistent-FS/cmake/build /users/oahmed4/Strongly-Consistent-FS/cmake/build /users/oahmed4/Strongly-Consistent-FS/cmake/build/CMakeFiles/master.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/master.dir/depend

