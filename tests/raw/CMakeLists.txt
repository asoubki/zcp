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

# =======================================================================
#                             Generate Tests 
# =======================================================================
message("-- Generate raw unitary tests")


# Test MACROs
# -----------
macro (add_zcp_test testname thread bloc level type input md5)
    add_test(NAME ${testname}
        COMMAND ${CMAKE_COMMAND}
        -D PROGRAM=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp 
        -D THREAD=${thread}
        -D BLOCSIZE=${bloc}
        -D LEVEL=${level}
        -D TYPE=${type}
        -D INPUT=${CMAKE_SOURCE_DIR}/data/${input}
        -D OUTPUT=${CMAKE_BINARY_DIR}/tmp/${input}.${testname}.${bloc}.${level}.${thread}.raw
        -D CHECKSUM=${md5}
        -P ${CMAKE_CURRENT_LIST_DIR}/do_zcptest.cmake)
endmacro (add_zcp_test)


# LZ4 Raw Usecase
# --------------- 
# -- different bloc size
set(BLOC_SIZES 512 1024 64K 66000 256K 512K 1024K 1M 4M 8M)
# -- different compression levels
set(COMPRESS_LEVELS 1 2 3 4 5 6 7 8 9)
# -- different compression levels
set(MULTITHREAD_LEVELS 1 2 4 8 10 12 16 32)

# -- MD5 for raw compression
set(raw_BLOC_SIZES_MD5 fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
    fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
    fe3a5e6d9654f2446f1affadf44ecb0b)
set(raw_COMPRESS_LEVELS_MD5 fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
    fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
    )
set(raw_MULTITHREAD_LEVELS_MD5 fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
    fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b fe3a5e6d9654f2446f1affadf44ecb0b
    )

# Run Raw Tests 
# ------------- 
    
# Test Bloc sizes
set(INDEX 0)
foreach(bloc ${BLOC_SIZES})
    list (GET raw_BLOC_SIZES_MD5 ${INDEX} MD5)
    add_zcp_test(zcp_raw_bloc_${bloc}_t 1 ${bloc} 1 none bensh1.txt ${MD5})
    MATH(EXPR INDEX "${INDEX}+1")
endforeach()

# Test compression levels
set(INDEX 0)
foreach(level ${COMPRESS_LEVELS})
    list (GET raw_COMPRESS_LEVELS_MD5 ${INDEX} MD5)
    add_zcp_test(zcp_raw_level_${level}_t 1 64K ${level} none bensh1.txt ${MD5})
    MATH(EXPR INDEX "${INDEX}+1")
endforeach()

# Test multithread compression
set(INDEX 0)
foreach(mt ${MULTITHREAD_LEVELS})
    list (GET raw_MULTITHREAD_LEVELS_MD5 ${INDEX} MD5)
    add_zcp_test(zcp_raw_mt_${mt}_t ${mt} 64K 9 none bensh1.txt ${MD5})
    MATH(EXPR INDEX "${INDEX}+1")
endforeach()






















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