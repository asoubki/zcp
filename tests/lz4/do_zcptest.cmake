#
# cmake script to run unit tests
#
string(ASCII 27 Esc)
set(ColourReset "${Esc}[m")
set(ColourBold  "${Esc}[1m")
set(Red         "${Esc}[31m")

# check for arguments
# -------------------
if (NOT PROGRAM)
    message( FATAL_ERROR "Missing variable 'PROGRAM'" )
elseif (NOT THREAD)
    message( FATAL_ERROR "Missing variable 'THREAD'" )
elseif (NOT BLOCSIZE)
    message( FATAL_ERROR "Missing variable 'BLOCSIZE'" )
elseif (NOT LEVEL)
    message( FATAL_ERROR "Missing variable 'LEVEL'" )
elseif (NOT TYPE)
    message( FATAL_ERROR "Missing variable 'TYPE'" )
elseif (NOT INPUT)
    message( FATAL_ERROR "Missing variable 'INPUT'" )
elseif (NOT OUTPUT)
    message( FATAL_ERROR "Missing variable 'OUTPUT'" )
elseif (NOT CHECKSUM)
    message( FATAL_ERROR "Missing variable 'CHECKSUM'" )
endif()

# Check Compress mode
# -------------------
execute_process(COMMAND ${PROGRAM} -p ${THREAD} -l ${LEVEL} -b ${BLOCSIZE} -t ${TYPE} ${INPUT} ${OUTPUT}
                RESULT_VARIABLE OUTPUT_ERROR
                OUTPUT_VARIABLE OUTPUT_MESSAGE
                ERROR_VARIABLE  ERROR_MESSAGE)
# Check Program error
if (NOT ${OUTPUT_ERROR} EQUAL 0)
    message("Program finished with error :" ${OUTPUT_ERROR})
    # Remove color characters
    string(REPLACE "${Esc}[0m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
    string(REPLACE "${Esc}[1m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
    string(REPLACE "${Esc}[4m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
    string(REPLACE "${Esc}[31m" "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})

    string(REPLACE "${Esc}[0m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
    string(REPLACE "${Esc}[1m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
    string(REPLACE "${Esc}[4m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
    string(REPLACE "${Esc}[31m" "" ERROR_MESSAGE ${ERROR_MESSAGE})
    # Print output
    message("Program error message :" ${ERROR_MESSAGE})
    message(FATAL_ERROR "$Program output :" ${OUTPUT_MESSAGE})
endif()

# Check result
file(MD5 ${OUTPUT} MD5_RESULT)
if (NOT ${MD5_RESULT} STREQUAL ${CHECKSUM})
    message(FATAL_ERROR "Invalid compressed file (${OUTPUT}): MD5 = " ${MD5_RESULT} " instead of " ${CHECKSUM})
endif()


# Check Uncompress mode
# -------------------
execute_process(COMMAND ${PROGRAM} -d ${OUTPUT} ${OUTPUT}.out 
                RESULT_VARIABLE ERROR_CODE
                OUTPUT_VARIABLE OUTPUT_MESSAGE
                ERROR_VARIABLE  ERROR_MESSAGE)
                
# Check Program error
if (NOT ${ERROR_CODE} EQUAL 0)
    # Print error message
    if ( OUTPUT_MESSAGE )
        string(REPLACE "${Esc}[0m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
        string(REPLACE "${Esc}[1m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
        string(REPLACE "${Esc}[4m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
        string(REPLACE "${Esc}[31m" "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
        message("Program output :" ${OUTPUT_MESSAGE})
    endif()
    # Print output
    if ( ERROR_MESSAGE )
        string(REPLACE "${Esc}[0m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
        string(REPLACE "${Esc}[1m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
        string(REPLACE "${Esc}[4m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
        string(REPLACE "${Esc}[31m" "" ERROR_MESSAGE ${ERROR_MESSAGE})
        message("Program error message :" ${ERROR_MESSAGE})
    endif()
    # Print output
    message(FATAL_ERROR "Program returned an error (error = ${ERROR_CODE})")
endif()

# compare result file 
execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${OUTPUT}.out ${INPUT} 
                RESULT_VARIABLE ERROR_CODE
                OUTPUT_VARIABLE OUTPUT_MESSAGE
                ERROR_VARIABLE  ERROR_MESSAGE)
if (NOT ${ERROR_CODE} EQUAL 0)
    message(FATAL_ERROR "Invalid decompressed file")
endif()

