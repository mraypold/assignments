# Michael Raypold
# CMPUT 379
# UnixID : raypold

assignment: main.o memchunk.o
	gcc -Wall -m32 -o assignment main.o memchunk.o

main.o: main.c memchunk.h
	gcc -Wall -c -m32 main.c        

memchunk.o: memchunk.c memchunk.h
	gcc -Wall -c -m32 memchunk.c

clean:
	-rm -rf *.o assignment core
