CC = gcc
LD = gcc
CFLAGS = -Wall -I.
SRCS = ptask.c pbarrier.c ptime.c pmutex.c tstat.c rtmode.c
OBJS = ${SRCS:.c=.o}
DOBJ = ${SRCS:.c=.do}
STATIC_LIB = libptask.a
SHARED_LIB = libptask.so.1

$(STATIC_LIB) : $(OBJS)
	$(AR) rcs libptask.a $(OBJS)

$(SHARED_LIB) : $(DOBJ) 
	$(CC) -shared -Wl,-soname,libptask.so.1 \
	      -o libptask.so.1.0.1 $(DOBJ)

-include Makefile.deps

depend: 
	$(CC) $(CFLAGS) -MM *.[ch] > Makefile.deps

.c.o:
	$(CC) $(CFLAGS) -c $<

%.do : %.c 
	$(CC) $(CFLAGS) -c -fPIC $< -o $@

all: $(STATIC_LIB) $(SHARED_LIB)

clean: 
	rm -f $(OBJS)
	rm -f $(DOBJ)

cleanedit: 
	rm -f *~

cleanall: clean cleanedit
	rm -f $(STATIC_LIB) '$(SHARED_LIB).0.1'

