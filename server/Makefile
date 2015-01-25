# Michael Raypold
# CMPUT 379
# UnixID : raypold

server: server.o server_f.o server_p.o server_s.o log_handler.o http.o
	gcc -Wall -o server server.o server_f.o server_p.o server_s.o log_handler.o http.o -lm -lpthread

server.o: server.c server.h
	gcc -Wall -c server.c

server_f.o: server_f.c server.h
	gcc -Wall -c server_f.c

server_p.o: server_p.c server.h
	gcc -Wall -c server_p.c -lpthread

server_s.o: server_s.c server.h
	gcc -Wall -c server_s.c

log_handler.o: log_handler.c server.h
	gcc -Wall -c log_handler.c

http.o: http.c server.h
	gcc -Wall -c http.c -lm

clean:
	-rm -f *.o server core
