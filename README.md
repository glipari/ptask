ptask
=====

Periodic Real-Time Task interface to pthreads

Version 0.1, April 2013
Version 0.2, August 2013

Authors 
-------
- Giorgio Buttazzo (g.buttazzo@sssup.it)
- Giuseppe Lipari  (g.lipari@sssup.it)

License: GPL 3.0

------------------------------------------------------------------

PTASK is a simple wrapper to the pthread library. It is intended for
real-time programmers that wish to control the timing behaviour and
the synchronisation of threads. It is intended to be minimalistic, yet
extensible to more complex usage scenarios.

Currently it provides:

- An API for implementing periodic and aperiodic tasks;
- A simple API for group scheduling and synchronization;
- An API for mode changes. 

A manual is available in ptask_manual.pdf

------------------------------------------------------------------

INSTRUCTIONS

To compile the library the first time, enter into directory src/ and type:

  make depend; make 

this produces the library file libptask.a, that must be included into your
projects.

To compile the examples, use the same procedure in directory examples/. 
Before running the examples, remember to become super-user, otherwise
Linux will not allow you to create real-time tasks!

To compile the tests, enter directory test/ and once again run 

  make depend; make 

To run the tests, just execute the script

  ./runtests.sh

Happy programming!




