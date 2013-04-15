CFLAGS = -Wall -I../src/
LDFLAGS=-L ../src -lptask -lpthread -lrt
CC=gcc
LD=gcc

.SUFFIXES:
.SUFFIXES: .o .c

EXECS = time key key_opt

SRCS_time = time.c 
OBJS_time = ${SRCS_time:.c=.o}

SRCS_key = key.c 
OBJS_key = ${SRCS_key:.c=.o}

SRCS_key_opt = key_opt.c 
OBJS_key_opt = ${SRCS_key_opt:.c=.o}

all: $(EXECS)

-include Makefile.deps

depend:
	$(CC) $(CFLAGS) -MM *.[ch] > Makefile.deps

.c.o: ../src/libptask.a
	$(CC) $(CFLAGS) -c $<

time: $(OBJS_time)
	$(LD) -o $@ $^ $(LDFLAGS) 

key: $(OBJS_key)
	$(LD) -o $@ $^ $(LDFLAGS) 

key_opt: $(OBJS_key_opt)
	$(LD) -o $@ $^ $(LDFLAGS) 

clean:
	rm -rf *.o
	rm -rf $(EXECS)

cleanedit:
	rm -rf *~

