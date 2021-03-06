# MiKTeX

MiKTeX is both a C/C++ implemenentation of TeX & Friends and a TeX distribution.

You can find more information here:

* [https://miktex.org](https://miktex.org "MiKTeX project page")
* [https://miktex.org/howto/build-win](https://miktex.org/howto/build-win "Building MiKTeX (Windows)")
* [https://miktex.org/howto/build-unx](https://miktex.org/howto/build-unx "Building MiKTeX (Unix-alike platforms)")

## What you will find here

This readme file is located in the top-level directory of the the
MiKTeX source code tree.  You use the source code to build and install
a minimal TeX system.  MiKTeX provides an integrated package manager,
i.e., this minimal TeX system can serve as a starting point for a much
larger (complete) TeX system.

## Prerequisites

* C++ compiler

  You will need a modern C/C++ compiler.  The minimum required
  standard version is C++14.

* CMake

  MiKTeX is built by using CMake, which will create Makefiles suited
  for your local system.
  
* Various build utilities

  You will need utilities like Bison, Flex, etc.  Please see the
  top-level `CMakeLists.txt`.

* Libraries

  On Unix-alike platforms, you will need development packages (header
  files) for a handful of popular libraries. Please see the top-level
  `CMakeLists.txt` for more information.

## Running CMake

It is recommended that you build outside the source code
directory.  For a server (non-UI) build, run this:

    cmake -DWITH_UI_QT=OFF ../source

If you are doing an out-of-source build, then you have to specify the
path to the MiKTeX source code directory (`../source` in the example
above).

CMake for Windows may require a generator specification.  If you want
to generate Makefiles for `nmake`, run this:

    cmake -G "NMake Makefiles" ../source

## Building MiKTeX

Run the make utility to build MiKTeX, for example:

    make

or, if you build on Windows:

    nmake

## Installing

CMake creates a standard `install` target which you can use to install
everything in the standard location.  On Linux, you run

    sudo make install

This will install MiKTeX in the `/usr/local`-prefixed directory
tree.

## First steps

### Initialize the package database

MiKTeX has the ability to install missing packages "on-the-fly".  The
package database must be initialized for this to work:

    sudo mpm --admin --update-db

In order to enable the automatic package installer, run:

    sudo initexmf --admin --set-config-value [MPM]AutoInstall=1
   
### Symbolic links

Some MiKTeX binaries are prefixed with `miktex-` in order to allow a
peaceful coexistence with another TeX distribution.  For example, the
pdfTeX binary is named `miktex-pdftex`.  You can create symbolic links
targetting the `miktex-` prefixed binaries:

    sudo initexmf --admin --force --mklinks

After running this command, the pdfTeX engine can be invoked via `pdftex`.

### Installing packages

You use the MiKTeX packagement utility to install some required packages:

    sudo mpm --admin --install amsfonts
    sudo mpm --admin --install cm
    sudo mpm --admin --install ltxbase
    sudo mpm --admin --install miktex-misc
    sudo mpm --admin --install miktex-pdftex

Run this, to create font/language configuration files used by the TeX engines:

    sudo initexmf --admin --mkmaps --mklinks
   
### Running (pdf)latex for the first time

You can now test your installation by running

    pdflatex sample2e
