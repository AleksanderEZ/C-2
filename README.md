# C/2

C/2 as its own name indicates, is a C language clone. It has some features cut down so it can be implemented during the time of the course.

C/2 is compiled to the QMachine, a virtual machine with its own assembly code made by jfortes@dis.ulpgc.es

## Installation

You don't need to install anything. And a distribution of the QMachine already comes within the project. You'd need to have bison and flex installed, aswell as make. GCC is also needed.

## Usage

This repo already has a few sample source code files. If you want to write something in C/2 just add the file to the project and indicate its path in the Makefile TESTFILE field. The language specification may be added to the project.

```bash
#You can compile in debug mode
make debug
#You can compile and just run
make
```

## License

[MIT](https://choosealicense.com/licenses/mit/) 