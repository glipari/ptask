CC = gcc
LD = gcc
CFLAGS = -Wall -I../src/
STATIC_LIB = libptask.a
LDFLAGS = -L../src/ -lptask -lrt -lpthread -lm

# Add the name of the exec file
EXECS = ball pbarr_example 

SRCS_ball = ball.c
OBJS_ball = ${SRCS_ball:.c=.o}

SRCS_barr = pbarr_example.c 
OBJS_barr = ${SRCS_barr:.c=.o}

# Add the following two lines for every new program, listing the
# source files needed. For example, in order to add a "spike" example,
# uncomment the following two lines, and list the appropriate source
# files
# SRCS_spike = spike.c util.c 
# OBJS_spike = ${SRCS_ball:.c=.o}

all: $(EXECS)

-include Makefile.deps

depend:
	$(CC) $(CFLAGS) -MM *.[ch] > Makefile.deps

.c.o:
	$(CC) $(CFLAGS) -c $<

ball : $(OBJS_ball) 
	$(LD) -o $@ $^ $(LDFLAGS) `allegro-config --libs`

pbarr_example : $(OBJS_barr)
	$(LD) -o $@ $^ $(LDFLAGS)

# Add the following two lines for every new program. For example, in
# order to add a spike example, uncomment the following two lines.
# spike : $(OBJS_spike)
# 	$(LD) -o $@ $^ $(LDFLAGS)


clean:
	rm -rf *.o
	rm -rf $(EXECS)

cleanedit:
	rm -rf *~


