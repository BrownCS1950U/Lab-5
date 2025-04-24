# Audio / Terrain Generation

This repository contains an implementation of 3D audio and quad based terrain generation.

## Usage

To compile, create a directory called `build` and create Makefile using the cmake build system

    mkdir build && cd build
    cmake ..
    make

Once the project is compiled, the binary is directed to a `bin` directory, and takes the `.obj` mesh as a command line argument

    ./bin/viewer sponza/sponza.obj

