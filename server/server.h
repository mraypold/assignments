#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <netinet/in.h>

#define MAXCONNECTIONS 256

#define MAXTHREADS 8
#define MAXPROCESSES 256
#define MAXPATHSIZE 256

#define MAXBUFFER 80
#define BUFFERSIZE 255

#define SERVER_F 0
#define SERVER_P 1
#define SERVER_S 2

#define TRUE 1
#define FALSE 0

#define GETREQUEST 1
#define BADREQUEST 2
#define NOTFOUND 3
#define FORBIDDEN 4
#define SERVERERROR 5



struct server_settings {
        int type;
        u_short port;
        char *path;
        char *docs;
        char *clientAddress;
};

struct client_info {
		char *path;
		char *docs;
		char *filename;
		FILE *dfp;
		int filesize;
		char *charfilesize;
		struct sockaddr_in client;
		int sd;
		int bytes;
		int request;
		char *reqtype;
		char *protocol;
		char *htmlobject;
};

struct thread_data {
		int thread_id;
		struct client_info *client;
};

/* server_f prototypes */
void initiate_server_f(struct server_settings*);

/* server_p prototypes */
void initiate_server_p(struct server_settings*);
void *newconnection(void*);

/* server_s prototypes */
void initiate_server_s(struct server_settings*);

/* log_handler prototypes */
void handlelog(struct client_info*);

/* http prototypes */
int is_valid_request(char buffer[BUFFERSIZE], struct client_info*);
void read_socket(struct client_info*);
void print_socket(struct client_info*);
void handle_bad_request(struct client_info*, char*, char*, char*);
void print_file(struct client_info*);
void print_html(struct client_info*);
void print_date(struct client_info*);
void print_content_type(struct client_info*);
void print_file_length(struct client_info*);
void print_html_length(struct client_info*);
void build_html(char*, char*, struct client_info*);

#endif
