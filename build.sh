#!/bin/bash
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

set -e # exit on error

# Colors 
BOLD="\033[1m"
RESET="\033[0m"
BLUE="\033[34m"
RED="\033[31m"

# Functions
function do_doc() {
    doxygen
    if [ -d doc ]
    then
        mkdir -p $DOC_CPP
        cp -R doc/* $DOC_CPP
    else
        exit 1
    fi
}

function usage {
  echo -e $BOLD"NAME"$RESET
  echo -e "\t" $BOLD "build.sh" $RESET "\t project build script"
  echo -e ""
  echo -e $BOLD"SYNOPSIS"$RESET
  echo -e "\t" $BOLD "build.sh" $RESET "\t [OPTION]"
  echo -e ""
  echo -e $BOLD"OPTIONS"$RESET
  echo -e "\t" $BOLD "configure" $RESET "\t configure project"
  echo -e "\t" $BOLD "build" $RESET "\t compile and build project"
  echo -e "\t" $BOLD "clean" $RESET "\t clean compile files"
  echo -e "\t" $BOLD "cleanall" $RESET "\t clean all generated file (start from the beginning)"
  echo -e "\t" $BOLD "dist" $RESET "\t configure, build & generate doc files"
  echo -e "\t" $BOLD "test" $RESET "\t run unitary test (ctest)"
  echo -e "\t" $BOLD "package" $RESET "\t generate the package"
  echo -e ""

  echo -e $BOLD"specify directories:"$RESET
  echo -e "\t" $BOLD "--prefix=PREFIX" $RESET "\t install into the given directory. otherwise"
  echo -e "\t\t\t\t in [/usr/local]"
  echo -e "\t" $BOLD "--package=PACKAGE" $RESET "\t generate package in the given directory"  
  echo -e "\t" $BOLD "--build=BUILD" $RESET "\t generate & compile project in the given directory"  
  echo -e ""
  echo -e ""
  exit 1
}

if [ $# -eq 0 ]
then
  usage
fi




SOURCE_DIR=`(cd $(dirname $0); pwd)`
PLATFORM=`uname`
BUILD_DIR="${SOURCE_DIR}/build.${PLATFORM}"
DOC_CPP="${SOURCE_DIR}/doc"

for option
do
    case $option in 
        --prefix=*)
            INSTALL_DIR=`expr "x$option" : "x--prefix=\(.*\)"`
            INSTALL_DIR=`(mkdir $INSTALL_DIR 1>/dev/null 2>&1; cd $INSTALL_DIR; pwd)`
            ;;
        --package=*)
            PACKAGE_DIR=`expr "x$option" : "x--package=\(.*\)"`
            PACKAGE_DIR=`(mkdir $PACKAGE_DIR 1>/dev/null 2>&1; cd $PACKAGE_DIR; pwd)`
            ;;
        --build=*)
            BUILD_DIR=`expr "x$option" : "x--build=\(.*\)"`
            BUILD_DIR=`(mkdir $BUILD_DIR 1>/dev/null 2>&1; cd $BUILD_DIR; pwd)`
            ;;
        configure|build|clean|cleanall|dist|doc|install|package|test)
            ;;
        *)
            usage
            ;;
    esac        
done

if [ -z "$PACKAGE_DIR" ]; then
    PACKAGE_DIR=$BUILD_DIR
fi
if [ -z "$INSTALL_DIR" ]; then
    INSTALL_DIR=$BUILD_DIR
fi


for target in "$@"
do
    case "$target" in

        configure)
            echo -e ""
            echo -e $BLUE$BOLD"CMake : Generate make files "$RESET
            echo -e $BLUE$BOLD"--------------------------- "$RESET
            (mkdir -p $BUILD_DIR; cd $BUILD_DIR; cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PACKAGE_DIR=$PACKAGE_DIR -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -G "Unix Makefiles" ..)
            ;;

        build)
            echo -e ""
            echo -e $BLUE$BOLD"Build Project"$RESET
            echo -e $BLUE$BOLD"-------------"$RESET
            if [ ! -d $BUILD_DIR ]; then
                echo -e $RED ">>> Run 'build.sh configure' first " $RESET
                echo " "
                break
            fi
            (cd $BUILD_DIR; make -f Makefile)
            ;;

        clean)
            echo -e ""
            echo -e $BLUE$BOLD"Clean build & dist repo "$RESET
            echo -e $BLUE$BOLD"----------------------- "$RESET
            echo -e ""
            echo "Clean files ..."
            if [ -d $BUILD_DIR ]; then
                (cd $BUILD_DIR ; make clean)
            fi 
	          ;;

        cleanall)
            echo -e ""
            echo -e $BLUE$BOLD"Clean build & dist repo "$RESET
            echo -e $BLUE$BOLD"----------------------- "$RESET
            echo -e ""
            echo "Clean build directory ..."
            rm -rf ${BUILD_DIR} 1>/dev/null 2>&1 
            echo "Clean package directory ..."
            rm -rf ${PACKAGE_DIR} 1>/dev/null 2>&1 
            #echo "Clean install directory ..."
            #rm -rf ${PACKAGE_DIR} 1>/dev/null 2>&1 
            echo "Clean doc directory ..."
            rm -rf ${DOC_CPP} 1>/dev/null 2>&1 
	          ;;            

        dist)
            echo -e ""
            echo -e $BLUE$BOLD"CMake : Generate make files "$RESET
            echo -e $BLUE$BOLD"--------------------------- "$RESET
            (mkdir -p $BUILD_DIR; cd $BUILD_DIR; cmake -G "Unix Makefiles" ..)

            echo -e ""
            echo -e ""
            echo -e $BLUE$BOLD"Build Project"$RESET
            echo -e $BLUE$BOLD"-------------"$RESET
            (cd $BUILD_DIR; make -f Makefile)

            echo -e ""
            echo -e ""
            echo -e $BLUE$BOLD"Build Doc"$RESET
            echo -e $BLUE$BOLD"---------"$RESET
            #do_doc
            ;;

        test)
            echo -e ""
            echo -e $BLUE$BOLD"Run CTests"$RESET
            echo -e $BLUE$BOLD"----------"$RESET
            if [ ! -d $BUILD_DIR ]; then
                echo -e $RED ">>> Run 'build.sh configure' first " $RESET
                echo " "
                break
            fi
            if [ ! -f $BUILD_DIR/dist/bin/zcp ]; then
                echo -e $RED ">>> Run 'build.sh build' first " $RESET
                echo " "
                break
            fi
            (cmake -E chdir $BUILD_DIR ctest -j6 -C Release -T test --output-on-failure)
	           ;;
        doc)
            do_doc
            ;;
        install|package)
            if [ ! -d $BUILD_DIR ]; then
                echo -e $RED ">>> Run 'build.sh configure' first " $RESET
                echo " "
                break
            fi
            if [ ! -f $BUILD_DIR/dist/bin/zcp ]; then
                echo -e $RED ">>> Run 'build.sh build' first " $RESET
                echo " "
                break
            fi
            (cd $BUILD_DIR && make $target)
            ;;
        *)
            ;;
    esac
done

echo -e ""
echo -e $BLUE$BOLD"[END]"$RESET
    


exit 0
