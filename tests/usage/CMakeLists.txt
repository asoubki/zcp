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
message("-- Generate zcp usage unitary tests")

# Test MACROs
# -----------
if (WIN32)
    set(CMAKE_ZCP_USAGE_TEST_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/zcp_usage_test.bat)
else()
    set(CMAKE_ZCP_USAGE_TEST_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/zcp_usage_test.sh)
endif()

macro (add_zcp_usage testname mode cmd grepvalue checkvalue)
    add_test(NAME ${testname}
        COMMAND ${CMAKE_COMMAND}
        -D SHELL=${CMAKE_ZCP_USAGE_TEST_SCRIPT} 
        -D MODE=${mode}
        -D CMD=${cmd} 
        -D GREPVALUE=${grepvalue}
        -D CHECKVALUE=${checkvalue}
        -P ${CMAKE_CURRENT_LIST_DIR}/do_zcptest_usage.cmake)
endmacro (add_zcp_usage)

# CTest zcp Usage
# ---------------
# -- command line
set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -p 15 -b 256K -l 9 -t lz4 ${CMAKE_SOURCE_DIR}/data/bensh1.txt  ${CMAKE_BINARY_DIR}/tmp/bensh1.tmp.lz4")
# -- diffente usage tests
set(USAGE_TYPES "input" "output" "type" "level" "thread" "blocsize")
set(CHECK_VALUES "${CMAKE_SOURCE_DIR}/data/bensh1.txt" 
                 "${CMAKE_BINARY_DIR}/tmp/bensh1.tmp.lz4" 
                 "lz4" 
                 "9" 
                 "15" 
                 "256K")

# Check compress usage
set(INDEX 0)
foreach(usage ${USAGE_TYPES})
    list (GET CHECK_VALUES ${INDEX} CHECK)
    add_zcp_usage(zcp_usage_${usage} CHECKARG ${CMD} ${usage} ${CHECK})
    MATH(EXPR INDEX "${INDEX}+1")
endforeach()

# Check decompress usage
set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -d ${CMAKE_SOURCE_DIR}/data/bensh1.txt.lz4 ${CMAKE_BINARY_DIR}/tmp/bensh1.txt")
# -- check input
add_zcp_usage(zcp_usage_unzip_input CHECKARG ${CMD} "input" "${CMAKE_SOURCE_DIR}/data/bensh1.txt.lz4")
# -- check output
add_zcp_usage(zcp_usage_unzip_output CHECKARG ${CMD} "output" "${CMAKE_BINARY_DIR}/tmp/bensh1.txt")


# CTest zcp Syntax Error
# ----------------------
# No input value
set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -p 15 -b 256K -l 9 -t lz4")
add_zcp_usage(zcp_syntax_input SYNTAXERR ${CMD} "INVALID" ">>> Syntax Error : no input file provided")
# Invalid type
set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -p 15 -b 256K -l 9 -t err ${CMAKE_SOURCE_DIR}/data/bensh1.txt")
add_zcp_usage(zcp_syntax_type SYNTAXERR ${CMD} "INVALID" ">>> Syntax Error : invalid compression type")
# Invalid type
set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -d")
add_zcp_usage(zcp_syntax_unzip_input1 SYNTAXERR ${CMD} "INVALID" ">>> Syntax Error : no input file provided")
set (CMD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp -d ${CMAKE_SOURCE_DIR}/data/bensh1")
add_zcp_usage(zcp_syntax_unzip_input2 SYNTAXERR ${CMD} "INVALID" ">>> Syntax Error : output file must be set (no extension detected)")


# CTest zcp Errors
# ----------------
## Test Errors
#add_zcp_usage(myTest_usage_compress_compress_error         1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
#add_zcp_usage(myTest_usage_compress_write_error   1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
#add_zcp_usage(myTest_usage_compress_thread_error   1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
#add_zcp_usage(myTest_usage_compress_incompressible_error         1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
#add_zcp_usage(myTest_usage_decompress_invalid_file       1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
#add_zcp_usage(myTest_usage_decompress_write_error   1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
#add_zcp_usage(myTest_usage_decompress_decompress_error         1 512   1 none bensh1.txt fe3a5e6d9654f2446f1affadf44ecb0b)
