Compass
=======

Overview
-------

Compass is a solver for the Orienteering Problem written in C.

It includes some routines originally released in the Concorde solver: 2-opt, 2.5-opt, 3-opt and Lin-Kernighan for TSP, dynamic k-d trees and the functions to calculate the edge length. Note that this routines were released under an Academic Licence.

Compass is distributed under the GNU General Public License.


Instructions to Build from Source
---------------------------------

### Installing dependencies

  Compass uses autotools
```sh
sudo apt-get install m4
```

  Compass uses the GSL library for sampling distribution.

```sh
sudo apt-get install libgsl0-dev libatlas-base-dev ibbfd-dev libiberty-dev
```

### Availability

The source code is available in:

```sh
git clone https://github.com/bcamath-ds/compass
```

### Building Compass

If the configure script is absent you can generate using the GNU Autotools:

```bash
autoheader
libtoolize
aclocal
automake --add-missing
autoconf
```

To build the compass binary type:
```bash
./configure
make
```
 Additional build instructions are in the INSTALL file.

Directory Layout
----------------

__Simplified directory layout (only essential files/directories):__

```
ROOT                          Root directory
├── AUTHORS                   Authors file
├── compass                   Compass binay
├── configure                 Configure
├── configure.ac              Autotools file to generate configure
├── COPYING                   Copyright information
├── INSTALL                   Installation instructions
├── LICENSE                   License details
├── Makefile                  Running "make" uses this file
├── Makefile.am               Autotools file to generate the Makefile
├── README.md                 This file
└── src
     ├── compass.c            Compass main file
     ├── compass.h            Compass header file
     │
     ├── data                 **Data**
     │   ├── data.c
     │   ├── data.h           Data header file
     │   ├── delaunay.c       Delaunay triangulations
     │   ├── delaunay.h       Delaunay header file
     │   ├── edgelen-cc.c
     │   ├── kdtree
     │   │   ├── kdbuild.c
     │   │   ├── kdnear.c
     │   │   ├── kdspan.c
     │   │   ├── kdtree.h
     │   │   └── kdtwoopt.c
     │   ├── near.c
     │   ├── neigh.h
     │   └── xnear.c
     │
     ├── env                  Envirment utilities: alloc, time, errors...
     │   └── ...
     ├── Makefile.am
     ├── Makefile.in
     │
     ├── op                   **Orienteering Problem**
     │   ├── ea               Evolutionary Algorithm
     │   │   ├── add.c        Add operator
     │   │   ├── crossover.c  Crossover operator
     │   │   ├── drop.c       Drop operator
     │   │   ├── ea.c         EA main file
     │   │   ├── ea.h         EA header file
     │   │   ├── mutation.c   Mutation operator
     │   │   └── selection.c  Parent selection
     │   ├── init             Solution initialization
     │   │   ├── init.c       Initialization main file
     │   │   └── select.c     Node selection
     │   ├── io.c             Read/write OPLib
     │   ├── op.c             OP main file
     │   ├── op.h             OP header file
     │   ├── prob.c           OP structures
     │   └── solution.c       Solution manipulations
     │
     ├── prob.c               Compass problem structures
     │
     ├── tsp                  **Travelling Salesperson Problem**
     │   ├── init             Solution initialization
     │   │   └── init.c       Initialization main file
     │   ├── linkern          Lin-Kernighan
     │   │   ├── flip_two.c
     │   │   ├── linkern.c    Lin-Kernighan main file
     │   │   └── linkern.h    Lin-Kernighan header file
     │   ├── ls               TSP local searchs
     │   │   └── ls.c         Local search main file
     │   ├── prob.c           TSP structures
     │   ├── tsp.c            TSP main file
     │   └── tsp.h            TSP header file
     │
     └── util                 Other utilities
         └── ...
```
