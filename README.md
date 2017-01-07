# zcp

multi-thread, multi-format compression tool and library (implementation in C++03/C++98)


## Pre-Requirement

 - CMake 3.0.0 or higher

## Building for for Windows Desktop

  - Under construction

## Building for Linux

 - Run `build.sh dist`.
 - `./build.${PLATEFORM}` will be created.
   it is possible to specify a destination directory using the argument 
     + `--build=BUILD` for build directory
     + `--prefix=PREFIX` for install directory
     + `--package=PACKAGE` for package directory

## install for Linux

 - Run `build.sh install`
 - The application and libraries will be installe in `/usr/bin` or in thre PREFIX if 
   specified during the dist

## Generate package

 - Run `build.sh package`
 - The pacakges will be generated under `./build.${PLATEFORM}/dist/pkg` or in thre PACKAGE if 
   specified during the dist

## Notes
 - To undo the build operation, `build.sh` handles 2 arguments : 
     + `clean` to clean compilation files
     + `cleanall` to clean all intermediate directory and get a clean workspace
        
## See also

 - [lz4 Extremely Fast Compression algorithm](https://github.com/lz4/lz4)

