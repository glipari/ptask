# MAKEFILE TEST PARAMS

LIB = -I../../src/
LIBFLAG = ../../src/libptask.a -L../../src/ 


paramTest: testParam.o animation.o graphics.o
	gcc -o paramTest testParam.o animation.o graphics.o $(LIBFLAG) -lptask -lrt -lpthread -lm `allegro-config --libs`

testParam.o: testParam.c testParameters.h animation.h graphics.h
	gcc -Wall $(LIB) -c testParam.c

animation.o: animation.c
	gcc -Wall $(LIB) -c animation.c

graphics.o: graphics.c
	gcc -Wall $(LIB) -c graphics.c

clean:
	rm -rf *.o paramTest
.PHONY: clean
