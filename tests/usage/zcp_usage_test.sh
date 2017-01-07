#!/bin/bash
#
# Linux test script for zcp usage
#
# zcp
#    Copyright (C) 2013, Adil So <oilos.git@gmail.com>
#    All rights reserved.
#
#
# Usage
if [ $# -eq 0 ]
then
  echo "Usage : $0 {mode} {program} {args}"
fi


# Parce arguments
MODE=$1
GREPVALUE=$2
COMMAND=$3

# Run test
case "$MODE" in
    "CHECKARG")
        $COMMAND| grep "$GREPVALUE" | awk -F ":" '{print $2}' | tr -d '[:space:]'
        ERROR=$?
        ;;
    "SYNTAXERR")
        $COMMAND
        if [ $? -eq 0 ]; then
            ERROR=2
        else
            ERROR=0
        fi
        ;;
    *)
        (>&2 echo ">>> Unknown mode : $MODE")
        ERROR=1
        ;;
esac

exit $ERROR