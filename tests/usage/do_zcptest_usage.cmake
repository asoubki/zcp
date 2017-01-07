#
# cmake script to run unit tests
#
string(ASCII 27 Esc)
set(ColourReset "${Esc}[m")
set(ColourBold  "${Esc}[1m")
set(Red         "${Esc}[31m")

# check for arguments
# -------------------
if (NOT SHELL)
    message( FATAL_ERROR "Missing variable 'SHELL'" )
elseif (NOT MODE)
    message( FATAL_ERROR "Missing variable 'MODE'" )
elseif (NOT CMD)
    message( FATAL_ERROR "Missing variable 'CMD'" )
elseif (NOT GREPVALUE)
    message( FATAL_ERROR "Missing variable 'GREPVALUE'" )
elseif (NOT CHECKVALUE)
    message( FATAL_ERROR "Missing variable 'CHECKVALUE'" )
endif()

# Run Program
# -----------
execute_process(COMMAND ${SHELL} ${MODE} "${GREPVALUE}" ${CMD}
                RESULT_VARIABLE ERROR_CODE
                OUTPUT_VARIABLE OUTPUT_MESSAGE
                ERROR_VARIABLE  ERROR_MESSAGE)

if ( OUTPUT_MESSAGE )
    string(REPLACE "${Esc}[0m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
    string(REPLACE "${Esc}[1m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
    string(REPLACE "${Esc}[4m"  "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
    string(REPLACE "${Esc}[31m" "" OUTPUT_MESSAGE ${OUTPUT_MESSAGE})
endif()
if ( ERROR_MESSAGE )
    string(REPLACE "${Esc}[0m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
    string(REPLACE "${Esc}[1m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
    string(REPLACE "${Esc}[4m"  "" ERROR_MESSAGE ${ERROR_MESSAGE})
    string(REPLACE "${Esc}[31m" "" ERROR_MESSAGE ${ERROR_MESSAGE})
    string(REPLACE "\n"         "" ERROR_MESSAGE ${ERROR_MESSAGE})
endif()      

# Check program return code
if (NOT ${ERROR_CODE} EQUAL 0)
    # Print error message
    if ( OUTPUT_MESSAGE )
        message("Program output :" ${OUTPUT_MESSAGE})
    endif()
    # Print output
    if ( ERROR_MESSAGE )
        message("Program error message :" ${ERROR_MESSAGE})
    endif()      
    message(FATAL_ERROR "Program returned an error (error = ${ERROR_CODE})")
endif()

# Init Comparaison value
if ( ${MODE} STREQUAL "CHECKARG" )
    set(COMPARE_VALUE ${OUTPUT_MESSAGE})
elseif( ${MODE} STREQUAL "SYNTAXERR" )
    set(COMPARE_VALUE ${ERROR_MESSAGE})
else()
    set(COMPARE_VALUE "")
endif()

#  Check program output
if( (COMPARE_VALUE) AND (NOT "${COMPARE_VALUE}" STREQUAL "${CHECKVALUE}"))
    message("       - input value    = " ${CHECKVALUE})
    message("       - detected value = " ${COMPARE_VALUE})
    message(FATAL_ERROR "Checked value do not match")
endif()

