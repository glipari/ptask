CC = gcc
LD = gcc
CFLAGS = -Wall
SRCS = ptask.c
OBJS = ${SRCS:.c=.o}
DOBJ = ${SRCS:.c=.do}
STATIC_LIB = libptask.a
SHARED_LIB = libptask.so.1



$(STATIC_LIB) : $(OBJS)
	$(AR) rcs libptask.a $(OBJS)

$(SHARED_LIB) : $(DOBJ) 
	$(CC) -shared -Wl,-soname,libptask.so.1 \
	      -o libptask.so.1.0.1 $(DOBJ)

.c.o:
	$(CC) $(CFLAGS) -c $<

%.do : %.c 
	$(CC) $(CFLAGS) -c -fPIC $< -o $@

all: $(STATIC_LIB) $(SHARED_LIB)

clean: 
	rm -f $(OBJS)
	rm -f $(DOBJ)

cleanall: clean
	rm -f $(STATIC_LIB) $(SHARED_LIB).0.1