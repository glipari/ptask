ptask
=====

Periodic Real-Time Task interface to pthreads

Version 0.1 

Authors 
-------

Giorgio Buttazzo (g.buttazzo@sssup.it)
Giuseppe Lipari (g.lipari@sssup.it)

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
