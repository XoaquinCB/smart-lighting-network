## Project structure

### General structure

The source code has been structured in a way that allows for separation between network layers whilst still trying to make it easy to integrate them all together into a full program to run on the microcontroller. There is also support for writing separate test programs to test any part of the code.

The `source/` directory is split into `application/` and `network_stack/`. The `source/application/` directory contains the code for running the main program. This includes the `main()` function and any supporting code needed for running the network stack. The `source/network_stack/` directory is further split into subdirectories for each layer in the stack.

Public header files are placed in the `include/` directory. Private header files should not be placed here.

### Supporting multiple executables

In order to support writing multiple executables (for writing tests), `*.target` files are used to define executable targets. Each `*.target` file simply includes a list of source files that should be compiled to generate the target. Each `*.target` file results in a corresponding `*.elf` file in the `build/` directory. Here is a target file:

```makefile
SOURCE_FILES := source_1.c source_2.c subdirectory/*.c
```

Note how you can use wildcards (`*`) if you want.

When using `make` to build the code, the target can be specified by setting the `TARGET` variable. This can either be done on the command line when calling make (`make TARGET=target_name`), or by creating a file called `config.mk` in the project's root directory and adding the line `TARGET?=target_name`. For example, for the main application target file `source/application/main.target` you would set `TARGET=application/main`.

### Network layers structure

The `application/main` target is the main target for the project, and currently specifies the following source files (this may change as the code evolves):

```
source/application/*.c
source/network_stack/app/*.c
source/network_stack/dll/*.c
source/network_stack/net/*.c
source/network_stack/phy/*.c
source/network_stack/tra/*.c
```

This means that for each network layer only the source files directly inside its directory are compiled into the main application. This allows subdirectories within each layer to be used to write separate targets (presumably for testing) without worriying about them affecting the main program.

### Config file

The `config.mk` file can be used to specify some options for make without having to type them into the command line every time. Currently the only two options that can be set in this file are `TARGET` and `PROGRAMMER`. `TARGET` specifies which target to build, and `PROGRAMMER` specifies the programmer option to pass to `avrdude` when programming the microcontroller. Make will generate the following default `config.mk` file it it doesn't exist, but the values can be safely changed to fit your needs:

```makefile
TARGET ?= application/main
PROGRAMMER ?= usbasp
```

Using `?=` means that it will only set the variable if it hasn't already been set, which allows the user to still specify the variable on the command line if they want to override this value. If you don't want this behaviour, use `:=` instead.

Note that this file should not be committed to the git repository.
