# ptask

Periodic Real-Time Task interface to pthreads

* Version 0.1, April 2013
* Version 0.2, August 2013
* Version 0.3, September 2015
* Version 0.4, July 2017

----
## Authors

* Giorgio Buttazzo (g.buttazzo@sssup.it)
* Giuseppe Lipari  (g.lipari@sssup.it)

## Contributors

* Alessio Balsini (a.balsini@sssup.it)
* Alexandre Becart and Benoit Delecroix (integration SCHED_DEADLINE)

License: GPL 3.0

----
## Introduction

PTASK is a simple wrapper to the pthread library. It is intended for
real-time programmers that wish to control the timing behaviour and
the synchronisation of threads. It is intended to be minimalistic, yet
extensible to more complex usage scenarios.

Currently it provides:

- An API for implementing periodic and aperiodic tasks;
- A simple API for group scheduling and synchronization;
- An API for mode changes. 

A manual is available in `docs/ptask_manual_x.x.pdf`.

----
## Instructions

### Prerequisites

* Allegro 4 libraries
* CMake 3.1+

### Compiling

To compile the library the first time, enter the main `ptask` folder
(from now identified with `ptask/`) and type:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

this produces the library file `ptask/build/src/libptask.a`, that must be
included into your projects.

This also compiles all the:
* **examples** (`ptask/build/src/examples`);
* **tests** (`ptask/build/src/tests`).

To compile the examples, use the same procedure in directory
examples/.  Before doing that, make sure you have already installed
the Allegro library, version 4 (http://liballeg.org/download.html). On
Ubuntu systems, you can install it with

```
  sudo apt-get install liballegro4-dev
```

Once you compiled the sources, remember to become **super-user**,
otherwise Linux will not allow you to create real-time tasks!

To run the tests, execute the script

```
$ ptask/build/src/tests/runtest.sh
```

or, from the `ptask/build/` directory, run

```
$ sudo make test
```

Happy programming!
