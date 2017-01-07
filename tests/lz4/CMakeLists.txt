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

message("-- Generate lz4 unitary tests")

# Test MACROs
# -----------
macro (add_zcp_test testname thread bloc level type input md5)
    add_test(NAME ${testname}
        COMMAND ${CMAKE_COMMAND}
        -D PROGRAM=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zcp 
        -D THREAD=${thread}
        -D BLOCSIZE=${bloc}
        -D LEVEL=${level}
        -D TYPE=lz4
        -D INPUT=${CMAKE_SOURCE_DIR}/data/${input}
        -D OUTPUT=${CMAKE_BINARY_DIR}/tmp/${input}.${testname}.${bloc}.${level}.${thread}.lz4
        -D CHECKSUM=${md5}
        -P ${CMAKE_CURRENT_LIST_DIR}/do_zcptest.cmake)
endmacro (add_zcp_test)

# LZ4 Tests Usecase
# ----------------- 
# -- different bloc size
set(BLOC_SIZES 512 1024 64K 66000 256K 512K 1024K 1M 4M 8M)
# -- different compression levels
set(COMPRESS_LEVELS 1 2 3 4 5 6 7 8 9)
# -- different compression levels
set(MULTITHREAD_LEVELS 1 2 4 8 10 12 16 32)

# -- MD5 for lz4 compression
set(lz4_BLOC_SIZES_MD5 5e4f3b896ccc093a724e9962cd020e0a cf809ba704e6a39540937bb7835aadc3 9e3035ee78d8ddc4f5307d1f24fc257c 9a319e5e8e851fa9200c8307714cff90
    12659c8234efa72bcac4fb544b2d349d 183a8ddc8aa82008234cd06e6adb81df d193fa62c9270a6886be1858aadedcf5 d193fa62c9270a6886be1858aadedcf5 4234c8a2ec21f47071ef17f6acd5c126
    4234c8a2ec21f47071ef17f6acd5c126)
set(lz4_COMPRESS_LEVELS_MD5 9e3035ee78d8ddc4f5307d1f24fc257c 35d7287053e48bc8c7d4dd7cecb562de a9137b342f6760b3df138fb12705e48f 82d249fde5b9d0dff254fb6e55289c77
    66027339ef46b57940d7b5151da8c4f6 1846a72bc50778451809cfcda1504c67 ac26698d371f6e52cf50c37b5679d0d0 3a74771e727553de811fd3f6a32da307 4b15062229e936bc640db4b4741f5190
    )
set(lz4_MULTITHREAD_LEVELS_MD5 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190
    4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190 4b15062229e936bc640db4b4741f5190
    )

# Run lz4 Tests 
# ------------- 

# Test Bloc sizes
set(INDEX 0)
foreach(bloc ${BLOC_SIZES})
    list (GET lz4_BLOC_SIZES_MD5 ${INDEX} MD5)
    add_zcp_test(zcp_lz4_bloc_${bloc}_t 1 ${bloc} 1 lz4 bensh1.txt ${MD5})
    MATH(EXPR INDEX "${INDEX}+1")
endforeach()

# Test compression levels
set(INDEX 0)
foreach(level ${COMPRESS_LEVELS})
    list (GET lz4_COMPRESS_LEVELS_MD5 ${INDEX} MD5)
    add_zcp_test(zcp_lz4_level_${level}_t 1 64K ${level} lz4 bensh1.txt ${MD5})
    MATH(EXPR INDEX "${INDEX}+1")
endforeach()

# Test multithread compression
set(INDEX 0)
foreach(mt ${MULTITHREAD_LEVELS})
    list (GET lz4_MULTITHREAD_LEVELS_MD5 ${INDEX} MD5)
    add_zcp_test(zcp_lz4_mt_${mt}_t ${mt} 64K 9 lz4 bensh1.txt ${MD5})
    MATH(EXPR INDEX "${INDEX}+1")
endforeach()
