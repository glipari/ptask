# MAKEFILE DEI TEST DEL SISTEMA

LIB = -I../../src/
LIBFLAG = ../../src/libptask.a -L../../src/ 

all: migrationTimeTest schedulerTest protocolTest partitioningTest

migrationTimeTest: migrationTime.c
	gcc -Wall $(LIB) -o migrationTimeTest migrationTime.c $(LIBFLAG) -lptask -lrt -lpthread

schedulerTest: testSystem.o graphics.o 
	gcc -o schedulerTest testSystem.o graphics.o $(LIBFLAG) -lptask -lrt -lpthread -lm `allegro-config --libs`

protocolTest: testProt.o graphics.o 
	gcc -o protocolTest testProt.o graphics.o $(LIBFLAG) -lptask -lrt -lpthread -lm `allegro-config --libs`

partitioningTest: testPart.o animation.o graphics.o 
	gcc -o partitioningTest testPart.o animation.o graphics.o $(LIBFLAG) -lptask -lrt -lpthread -lm `allegro-config --libs`

testSystem.o: testSystem.c 
	gcc -Wall $(LIB) -c testSystem.c

testProt.o: testProt.c 
	gcc -Wall $(LIB) -c testProt.c

testPart.o: testPart.c 
	gcc -Wall $(LIB) -c testPart.c

graphics.o: graphics.c
	gcc -Wall $(LIB) -c graphics.c

animation.o: animation.c
	gcc -Wall $(LIB) -c animation.c

clean:
	rm -rf *.o migrationTimeTest schedulerTest protocolTest partitioningTest
.PHONY: clean
