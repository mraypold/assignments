# Michael Raypold
# CMPUT 379
# UnixID : raypold

all:saucer

saucer: saucer.o draw.o threads.o
	gcc -Wall -o saucer saucer.o draw.o threads.o -lncurses -lpthread -g

saucer.o: saucer.c saucer.h
	gcc -Wall -c saucer.c -lncurses -lpthread -g

draw.o: draw.c saucer.h
	gcc -Wall -c draw.c -lncurses -lpthread -g

threads.o: threads.c saucer.h
	gcc -Wall -c threads.c -lncurses -lpthread -g

clean:
	-rm -f *.o *~ saucer core
