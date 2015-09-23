# MAKEFILE DEL TEST 'TASK FUNCTIONS'

LIB = -I../../src/
LIBFLAG = ../../src/libptask.a -L../../src/ 

all: taskFunTest

taskFunTest: testTaskFun.o animation.o graphics.o
	gcc -o taskFunTest testTaskFun.o animation.o graphics.o $(LIBFLAG) -lptask -lrt -lpthread -lm `allegro-config --libs`

testTaskFun.o: testTaskFun.c testParameters.h animation.h graphics.h
	gcc -Wall $(LIB) -c testTaskFun.c

animation.o: animation.c
	gcc -Wall $(LIB) -c animation.c

graphics.o: graphics.c
	gcc -Wall $(LIB) -c graphics.c

clean:
	rm -rf *.o taskFunTest
.PHONY: clean
