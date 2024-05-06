# Eric's Allolib playground

This is a fork of [Allolib Playground](https://github.com/AlloSphere-Research-Group/allolib_playground) for my personal projects in CMPSC 190D S24 at UCSB. 

TODO: the projects will be put in this readme when i actually make them

## Setup

Get all dependencies and tools required by allolib. See the [readme file for allolib](https://github.com/AlloSphere-Research-Group/allolib/blob/master/readme.md).

Run the init.sh script to get the allolib and al_ext libraries or execute in
a bash shell:

    git submodule update --recursive --init

## Building and running applications

The allolib playground provides basic facilties for building and debugging 
single file applications. On a bash shell on Windows, Linux and OS X do:

    ./run.sh path/to/file.cpp

This will build allolib, and create an executable for the file.cpp called 'file' inside the '''path/to/bin''' directory. It will then run the application.

## Allolib API documentation

You can find the allolib API doxygen documentation at:
[https://allosphere-research-group.github.io/allolib-doc/]()
