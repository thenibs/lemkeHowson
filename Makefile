CC=g++
CFLAGS=-c -Wall -std=c++17
LFLAGS=-static

all: lemkeHowson testLemke

lemkeHowson: lemke.o
	$(CC) lemke.o -o lemkeHowson $(LFLAGS) -O

lemke.o: main.cpp
	$(CC) main.cpp -o lemke.o $(CFLAGS)

testLemke: test.o
	$(CC) test.o -o testLemke $(LFLAGS) -O

test.o: test.cpp
	$(CC) test.cpp -o test.o $(CFLAGS)

clean:
	rm -f *.o lemkeHowson testLemke dump
