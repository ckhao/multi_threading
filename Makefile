CC=gcc 

all: ass2
	
ass2:
	$(CC) -o mts ass2.c -lpthread 

.PHONY: clean

clean:
	-rm -f *.o *.exe

