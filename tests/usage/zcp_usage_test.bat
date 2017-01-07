@ECHO OFF

REM 
REM Windows Test script for zcp usage
REM 
REM zcp
REM    Copyright (C) 2013, Adil So <oilos.git@gmail.com>
REM    All rights reserved.
REM 
REM



REM Parce arguments
set MODE=%1
set GREPVALUE=%2
set COMMAND=%3

echo %MODE%
echo %GREPVALUE%
echo %COMMAND%

%COMMAND%
REM # Run test
REM case "$MODE" in
REM     "CHECKARG")
REM         $COMMAND| grep "$GREPVALUE" | awk -F ":" '{print $2}' | tr -d '[:space:]'
REM         ERROR=$?
REM         ;;
REM     "SYNTAXERR")
REM         $COMMAND
REM         if [ $? -eq 0 ]; then
REM             ERROR=2
REM         else
REM             ERROR=0
REM         fi
REM         ;;
REM     *)
REM         (>&2 echo ">>> Unknown mode : $MODE")
REM         ERROR=1
REM         ;;
REM esac

REM exit $ERROR