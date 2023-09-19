# ch32v203_make_projects
A simple make-based build flow for CH32V203 parts

## About
This repository demonstrates how to use the MRS Toolchain tools to implement
a very simple Make-based build flow for WCH-IC RISC-V MCUs.

## Prerequisites
You'll need to download the MRS Toolchain executables for your preferred OS and
install them in a known location. I got them here:

[MRS Toolchain](http://www.mounriver.com/download)

Navigate to the OS of your choice, download the "Toolchain & Debugger" package
and extract it somewhere that you can access from the Makefile.

## Installing
First clone the Github repo:
```
git clone https://github.com/emeb/ch32v203_make_projects.git
```

Then modify the `blink/Makefile` to point to the location of your toolchain installation
from above. Find the TOOLS line and insert the proper path.
```
TOOLS = <your tool path here>
```

## Organization
The repository is organized with subdirectories containing projects and common
code. Each project directory will have its own Makefile, linker script, debugger script
and source code. Common directories contain the WCH startup code and peripheral
libraries.

## Building
Change directory to the project you wish to build and run `make`. To program a
device connect the WCH-Linke programmer and run `make flash`.

## Adding projects
Create a new directory and copy the following files from the original `blink`
project:
```
Makefile
openocd_ch32.cfg
CH32V203x8.ld
system_ch32v20x.c
system_ch32v20x.h
ch32v20x_conf.h
```
Add your own `main.c` and any other source files. Modify the Makefile to add
all your own sources and add / remove any peripheral driver files your project
needs.

## Supporting other parts
This repository targets the CH32V203 family of parts only. To set up a similar
`make` flow for other WCH RISC-V MCUs you'll need to do the following:

* Extract the template for your desired family. The templates are found in
the Mounriver IDE installation, in the path `MRS_Community/template/wizard/WCH/RISC-V/`
and lower level directories per each family. Within those are directories
called `NoneOS` which are for simple bare-metal make-based builds, and these
also contain zipfiles for all variants within the family.
* Create a project directory. Copy files from the template `User`, `Debug` and
`Ld` directories. Copy the `Makefile` from this repository and modify it as
needed to use the files from the new family template.

