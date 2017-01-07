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

# create working/tmp directory
# file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tmp)

# # Test MACROs
# # -----------
# if (WIN32)
#     set(CMAKE_ZCP_USAGE_TEST_SCRIPT ${CMAKE_SOURCE_DIR}/test/zcp_usage_test.bat)
# else()
#     set(CMAKE_ZCP_USAGE_TEST_SCRIPT ${CMAKE_SOURCE_DIR}/test/zcp_usage_test.sh)
# endif()

# macro (add_zcp_usage testname mode cmd grepvalue checkvalue)
#     add_test(NAME ${testname}
#         COMMAND ${CMAKE_COMMAND}
#         -D SHELL=${CMAKE_ZCP_USAGE_TEST_SCRIPT} 
#         -D MODE=${mode}
#         -D CMD=${cmd} 
#         -D GREPVALUE=${grepvalue}
#         -D CHECKVALUE=${checkvalue}
#         -P ${CMAKE_SOURCE_DIR}/do_zcptest_usage.cmake)
# endmacro (add_zcp_usage)

# macro (add_zcp_test testname thread bloc level type input md5)
#     add_test(NAME ${testname}
#         COMMAND ${CMAKE_COMMAND}
#         -D PROGRAM=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp 
#         -D THREAD=${thread}
#         -D BLOCSIZE=${bloc}
#         -D LEVEL=${level}
#         -D TYPE=${type}
#         -D INPUT=${CMAKE_SOURCE_DIR}/data/${input}
#         -D OUTPUT=${CMAKE_BINARY_DIR}/tmp/${input}.${testname}.${bloc}.${level}.${thread}.${type}
#         -D CHECKSUM=${md5}
#         -P ${CMAKE_SOURCE_DIR}/do_zcptest.cmake)
# endmacro (add_zcp_test)



# # CTest zcp Usage
# # ---------------
# # -- command line
# set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -p 15 -b 256K -l 9 -t lz4 ${CMAKE_SOURCE_DIR}/data/bensh1.txt  ${CMAKE_BINARY_DIR}/tmp/bensh1.tmp.lz4")
# # -- diffente usage tests
# set(USAGE_TYPES "input" "output" "type" "level" "thread" "blocsize")
# set(CHECK_VALUES "${CMAKE_SOURCE_DIR}/data/bensh1.txt" 
#                  "${CMAKE_BINARY_DIR}/tmp/bensh1.tmp.lz4" 
#                  "lz4" 
#                  "9" 
#                  "15" 
#                  "256K")

# # Check compress usage
# set(INDEX 0)
# foreach(usage ${USAGE_TYPES})
#     list (GET CHECK_VALUES ${INDEX} CHECK)
#     add_zcp_usage(zcp_usage_${usage} CHECKARG ${CMD} ${usage} ${CHECK})
#     MATH(EXPR INDEX "${INDEX}+1")
# endforeach()

# # Check decompress usage
# set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -d ${CMAKE_SOURCE_DIR}/data/bensh1.txt.lz4 ${CMAKE_BINARY_DIR}/tmp/bensh1.txt")
# # -- check input
# add_zcp_usage(zcp_usage_unzip_input CHECKARG ${CMD} "input" "${CMAKE_SOURCE_DIR}/data/bensh1.txt.lz4")
# # -- check output
# add_zcp_usage(zcp_usage_unzip_output CHECKARG ${CMD} "output" "${CMAKE_BINARY_DIR}/tmp/bensh1.txt")


# # CTest zcp Syntax Error
# # ----------------------
# # No input value
# set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -p 15 -b 256K -l 9 -t lz4")
# add_zcp_usage(zcp_syntax_input SYNTAXERR ${CMD} "INVALID" ">>> Syntax Error : no input file provided")
# # Invalid type
# set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -p 15 -b 256K -l 9 -t err ${CMAKE_SOURCE_DIR}/data/bensh1.txt")
# add_zcp_usage(zcp_syntax_type SYNTAXERR ${CMD} "INVALID" ">>> Syntax Error : invalid compression type")
# # Invalid type
# set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -d")
# add_zcp_usage(zcp_syntax_unzip_input1 SYNTAXERR ${CMD} "INVALID" ">>> Syntax Error : no input file provided")
# set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -d ${CMAKE_SOURCE_DIR}/data/bensh1")
# add_zcp_usage(zcp_syntax_unzip_input2 SYNTAXERR ${CMD} "INVALID" ">>> Syntax Error : output file must be set (no extension detected)")


# # CTest zcp Errors
# # ----------------
# ## Test Errors
# #add_zcp_usage(myTest_usage_compress_compress_error         1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
# #add_zcp_usage(myTest_usage_compress_write_error   1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
# #add_zcp_usage(myTest_usage_compress_thread_error   1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
# #add_zcp_usage(myTest_usage_compress_incompressible_error         1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
# #add_zcp_usage(myTest_usage_decompress_invalid_file       1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
# #add_zcp_usage(myTest_usage_decompress_write_error   1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
# #add_zcp_usage(myTest_usage_decompress_decompress_error         1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)

# # CTest zcp compress tests
# # ------------------------
# # -- different compression types
# set(COMPRESS_TYPE none lz4)
# # -- different bloc size
# set(BLOC_SIZES 512 1024 64K 66000 256K 512K 1024K 1M 4M 8M)
# # -- different compression levels
# set(COMPRESS_LEVELS 1 2 3 4 5 6 7 8 9)
# # -- different compression levels
# set(MULTITHREAD_LEVELS 1 2 4 8 10 12 16 32)

# # -- MD5 for none compression
# set(none_BLOC_SIZES_MD5 fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
#     fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
#     fe3a5e6d9654f2446f1affadf44ecb0b)
# set(none_COMPRESS_LEVELS_MD5 fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
#     fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
#     )
# set(none_MULTITHREAD_LEVELS_MD5 fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
#     fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
#     )
# # -- MD5 for lz4 compression
# set(lz4_BLOC_SIZES_MD5 5e4f3b896ccc093a724e9962cd020e0a cf809ba704e6a39540937bb7835aadc3 9e3035ee78d8ddc4f5307d1f24fc257c 9a319e5e8e851fa9200c8307714cff90
#     12659c8234efa72bcac4fb544b2d349d 183a8ddc8aa82008234cd06e6adb81df d193fa62c9270a6886be1858aadedcf5 d193fa62c9270a6886be1858aadedcf5 4234c8a2ec21f47071ef17f6acd5c126
#     4234c8a2ec21f47071ef17f6acd5c126)
# set(lz4_COMPRESS_LEVELS_MD5 9e3035ee78d8ddc4f5307d1f24fc257c 35d7287053e48bc8c7d4dd7cecb562de a9137b342f6760b3df138fb12705e48f 82d249fde5b9d0dff254fb6e55289c77
#     66027339ef46b57940d7b5151da8c4f6 1846a72bc50778451809cfcda1504c67 ac26698d371f6e52cf50c37b5679d0d0 3a74771e727553de811fd3f6a32da307 4b15062229e936bc640db4b4741f5190
#     )
# set(lz4_MULTITHREAD_LEVELS_MD5 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190
#     4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190
#     )

# foreach(type ${COMPRESS_TYPE})

#     # Test Bloc sizes
#     set(INDEX 0)
#     foreach(bloc ${BLOC_SIZES})
#         list (GET ${type}_BLOC_SIZES_MD5 ${INDEX} MD5)
#         add_zcp_test(zcp_${type}_bloc_${bloc}_t 1 ${bloc} 1 ${type} bensh1.txt ${MD5})
#         MATH(EXPR INDEX "${INDEX}+1")
#     endforeach()

#     # Test compression levels
#     set(INDEX 0)
#     foreach(level ${COMPRESS_LEVELS})
#         list (GET ${type}_COMPRESS_LEVELS_MD5 ${INDEX} MD5)
#         add_zcp_test(zcp_${type}_level_${level}_t 1 64K ${level} ${type} bensh1.txt ${MD5})
#         MATH(EXPR INDEX "${INDEX}+1")
#     endforeach()

#     # Test multithread compression
#     set(INDEX 0)
#     foreach(mt ${MULTITHREAD_LEVELS})
#         list (GET ${type}_MULTITHREAD_LEVELS_MD5 ${INDEX} MD5)
#         add_zcp_test(zcp_${type}_mt_${mt}_t ${mt} 64K 9 ${type} bensh1.txt ${MD5})
#         MATH(EXPR INDEX "${INDEX}+1")
#     endforeach()

# endforeach()






















#add_test(NAME Test1
#    COMMAND ${CMAKE_COMMAND}
#    -D PROGRAM=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp 
#    -D ARGUMENTS="-p 1 -b 64K -l 1 -t lz4 ${CMAKE_SOURCE_DIR}/data/bensh1.txt ${CMAKE_BINARY_DIR}/tmp/bensh1.txt.lz4"
#    -D THREAD=1
#    -D BLOCSIZE=64K
#    -D LEVEL=1
#    -D TYPE=lz4
#    -D INPUT=${CMAKE_SOURCE_DIR}/data/bensh1.txt
#    -D OUTPUT=${CMAKE_BINARY_DIR}/tmp/bensh1.txt.lz4
#    -D CHECKSUM=ac630a2c71a6984f1f082d6e9a2621e5
#    -P ${CMAKE_SOURCE_DIR}/dotest.cmake)

#add_test( "test1" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -p 1 -l 1 -t lz4 ${CMAKE_SOURCE_DIR}/data/bensh1.txt )
#file(MD5 ${CMAKE_SOURCE_DIR}/data/bensh1.txt MD5_RESULT)
#message("MD5_RESULT" = ${MD5_RESULT})









#define a macro to simplify adding tests, then use it
#macro (do_test arg result)
#  add_test (TutorialComp${arg} Tutorial ${arg})
#  set_tests_properties (TutorialComp${arg}
#    PROPERTIES PASS_REGULAR_EXPRESSION ${result})
#endmacro (do_test)
 
## do a bunch of result based tests
#do_test (25 "25 is 5")
#do_test (-25 "-25 is 0")


#execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files
#    output.txt ${SOURCEDIR}/expected.txt
#    RESULT_VARIABLE DIFFERENT)
#if(DIFFERENT)
#    message(FATAL_ERROR "Test failed - files differ")
#endif()




#set(Inputs 1 2 3)
#foreach(Input ${Inputs})
#  add_test(NAME Test${Input} COMMAND example ${Input})
#  set_tests_properties(Test${Input} PROPERTIES
#                       PASS_REGULAR_EXPRESSION "The result is ${Input}0")
#endforeach()