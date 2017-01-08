#   zcp
#   Copyright (C) 2013, Adil So <oilos.git@gmail.com>
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
#   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# =======================================================================
#                        Check necessary packages
# =======================================================================
cmake_minimum_required(VERSION 3.0.0)

#FIND_PACKAGE(Eigen3) : @todo
#find_package (Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

# @todo : make Cmake Hirarchic (tree structure)

# =======================================================================
#               Configure compiler depending on the plateform
# =======================================================================
if(WIN32)
    set(PLATFORM "Win32")
else()
    set(PLATFORM "Linux")
endif()    
message("-- Detected Plateform is ${PLATFORM}")

# =======================================================================
#                           Set Project variables
# ======================================================================= 
project(zcp VERSION 1.0.0)

# set output paths
set(CMAKE_DIST_DIR ${CMAKE_BINARY_DIR}/dist)
if (NOT DEFINED CMAKE_PACKAGE_DIR)
  set(CMAKE_PACKAGE_DIR ${CMAKE_DIST_DIR}/pkg)
endif()

# create working/tmp directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)
file(MAKE_DIRECTORY ${CMAKE_PACKAGE_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_DIST_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_DIST_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_DIST_DIR}/bin)

# =======================================================================
#                            Compile project
# =======================================================================
# Compilation flags & includes 
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall -g")
endif ()

# Source files
set (SOURCE_FILES
        src/main.cpp
        src/zfile.cc
        src/zfilelz4.cc
        src/lz4/lz4.c
        src/lz4/lz4hc.c
        src/lz4/xxhash.c
        )

# generate executable
add_executable(zcp src/main.cpp)

# generate libraries
add_library(zfile SHARED ${SOURCE_FILES})
add_library(zfile_s STATIC ${SOURCE_FILES})

set_target_properties (zfile PROPERTIES
    VERSION ${PROJECT_VERSION})

set_target_properties (zfile_s PROPERTIES
    VERSION ${PROJECT_VERSION})


# Link flags
target_link_libraries(zcp pthread zfile)
target_link_libraries(zfile pthread)

# =======================================================================
#                             Generate Pacakge
# =======================================================================
message(${CMAKE_PACKAGE_DIR})
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_PACKAGE_DIR})
include(CPack)

install (TARGETS zfile zfile_s
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         RUNTIME DESTINATION lib)

install (TARGETS zcp RUNTIME DESTINATION bin)

install(FILES src/zfile.hh
        DESTINATION include)

# =======================================================================
#                             Generate Tests 
# =======================================================================
include(CTest)
enable_testing()


# Include tests
add_subdirectory(tests)
add_subdirectory(tests/lz4)
add_subdirectory(tests/usage)
add_subdirectory(tests/raw)
