#include "server.h"
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>

/* 
 * Process the GET request, if it exists 
 */
void read_socket(struct client_info *connection) {
	int bytes;
	char buffer[BUFFERSIZE];
	int first = TRUE;
	int len = 0;

	printf("socket %d\n", connection->sd);
	while((bytes = read(connection->sd, buffer, BUFFERSIZE)) > 0) {
		printf("nothing\n");
		if(bytes <= 0){
			print_socket(connection);
			return;
		}
		else {
			buffer[bytes] = '\0';
		}
 
 		if(first == TRUE) {
 			if(is_valid_request(buffer, connection) == FALSE){
 				/* Call to print output and exit */
 				print_socket(connection);
 				return;
 			}
 			first = FALSE;
 		}

 		len = strlen(buffer);

 		/* Valid request. So keep processing input until blank line */
 		if((len == 1) && (buffer[0] == '\n')) {
 			print_socket(connection);
 			return;
 		}
 		if((len == 2) && (buffer[0] =='\r') && (buffer[1] == '\n')) {
 			print_socket(connection);
 			return;
 		} 	
	}
}

/*
 * Checks for GET command, document existence and HTTP 1.1
 */
int is_valid_request(char buffer[BUFFERSIZE], struct client_info *connection) {
	char *token;
	char GET[] = "GET";
	char HTTP[] = "HTTP/1.1";
	char count = 0;

	char *docpath;
	char *filename;
	char *type;
	char *standard;

	int pathlen = 0;
	errno = 0;

	int request = FALSE;
	int document = FALSE;
	int protocol = FALSE;

	token = strtok(buffer, " ");
	while(token != NULL) {
		/* Header should be in proper order for switch statement */
		switch(count) {
			case 0:
				/* Find GET request */
				if(strncmp(token, GET, 3) == 0){
					request = TRUE;
				}

				/* Save request type */
				type = (char *)malloc(strlen(token));
				memset(type, '\0', strlen(token));
				strcpy(type, token);
				connection->reqtype = type;

				break;
			case 1:
				/* Check for valid path */
				pathlen = strlen(connection->docs);
				pathlen = pathlen + strlen(token);
				
				docpath = (char *)malloc(pathlen);
				memset(docpath, '\0', pathlen);
				strcat(docpath, connection->docs);
				strcat(docpath, token);

				if((connection->dfp = fopen(docpath, "r")) != NULL) {
					document = TRUE;
					connection->docs = docpath;
				}
				else{
					/* Some error occured */
					if(errno == EINVAL){
						connection->request = FORBIDDEN;
					}
					else{
						connection->request = NOTFOUND;
					}
				}

				/* Save document name */
				filename = (char *)malloc(strlen(token));
				memset(filename, '\0', strlen(token));
				strcpy(filename, token);
				connection->filename = filename;

				break;
			case 2:
				/* Check for HTTP/1.1 */
				if(strncmp(token, HTTP, 8) == 0) {
					protocol = TRUE;
				}
				else{
					connection->request = BADREQUEST;
				}

				/* Save http protocol */
				standard = (char *)malloc(strlen(token)-1);
				strncpy(standard, token, strlen(token)-1);
				connection->protocol = standard;

				break;
		}

		count++;
		token = strtok(NULL, " ");
	}

	/* 
	 * If header in wrong order, or anything above fails, this is
	 * a bad request. 
	 */
	if((request == FALSE) || (document == FALSE) || (protocol == FALSE)) {
		return FALSE;
	}
	else{
		connection->request = GETREQUEST;
		return TRUE;
	}

}

/* 
 * Print the request result to the socket 
 */
void print_socket(struct client_info *connection) {
	char *getgood = "HTTP/1.1 200 OK\n";

	char *badreq = "HTTP/1.1 400 Bad Request\n";
	char *badtitle = "malformed Request";
	char *badbody = "Your browser sent a request I could not understand.\n";

	char *notfound = "HTTP/1.1 404 Not Found\n";
	char *nottitle = "Document not found";
	char *notbody = "You asked for a document that doesn't exist. That is so sad.\n";

	char *forbid = "HTTP/1.1 403 Forbidden\n";
	char *deniedtitle = "Permission Denied";
	char *deniedbody = "You asked for a document you are not permitted to see. It sucks to be you.\n";

	char *error = "HTTP/1.1 500 Internal Server Error\n";
	char *errortitle = "Oops. That Didn't work";
	char *errorbody = "I had some sort of problem dealing with your request. Sorry, I'm lame.\n";


	/* If valid GET request, print output */
	if(connection->request == GETREQUEST){
		write(connection->sd, getgood, sizeof(getgood));
		print_date(connection);
		print_content_type(connection);
		print_file_length(connection);
		print_file(connection);
		return;
	}
	else if(connection->request == BADREQUEST){
		handle_bad_request(connection, badreq, badtitle, badbody);
		return;
	}
	else if(connection->request == NOTFOUND){
		handle_bad_request(connection, notfound, nottitle, notbody);
		return;	
	}
	else if(connection->request == FORBIDDEN) {
		handle_bad_request(connection, forbid, deniedtitle, deniedbody);
		return;	
	}
	else{
		handle_bad_request(connection, error, errortitle, errorbody);
		return;	
	}
}

/*
 * Any request that is not a get request is sent here to print out
 */
void handle_bad_request(struct client_info *connection, char *type,
	char *title, char *body) {

	write(connection->sd, type, strlen(type));
	
	print_date(connection);
	print_content_type(connection);

	build_html(title, body, connection);

	print_html_length(connection);
	print_html(connection);

}

/* 
 * Print the file requested by GET 
 */
void print_file(struct client_info *connection) {
	char buffer[BUFFERSIZE];
	int wbytes = 0;
	FILE *fp;

	fp = fopen(connection->docs, "r");

	/* Use the document file path to send to sd */
	while(fgets(buffer, BUFFERSIZE, fp) != NULL){

		wbytes = write(connection->sd, buffer, strlen(buffer));

		if(wbytes < strlen(buffer)){
			/* Couldn't finish writing file */
			connection->bytes = wbytes;
			return;
		}
		connection->bytes += wbytes;
	}

	/* We are now done. Will return to main and complete log */
}

/* 
 * Print the html object associated with the structure 
 */
void print_html(struct client_info *connection) {
	int written = 0;

	send(connection->sd, connection->htmlobject, 
		connection->filesize, 0);

	connection->bytes = written;
}

/*
 * Print the date that the request was received
 */
void print_date(struct client_info *connection) {
	char date[] = "Date: ";

    time_t rawTime;
    struct tm *timeInfo;
    char buffer[BUFFERSIZE];

    /* Get time in GMT */
    time(&rawTime);
    timeInfo = gmtime(&rawTime);
    strftime(buffer, BUFFERSIZE, "%a %d %b %Y %X %Z", timeInfo);

	/* Date */
	write(connection->sd, date, strlen(date));
	write(connection->sd, buffer, strlen(buffer));
	write(connection->sd, "\n", 1);
}

/*
 * Prints the content type to the socket
 */
void print_content_type(struct client_info *connection) {
	char type[] = "Content-Type: text/html\n";
	
	if(write(connection->sd, type, sizeof(type)) < strlen(type)){
		return;
	}
}

/*
 * Prints the length for files that were found.
 */
void print_file_length(struct client_info *connection) {
	char length[] = "Content-Length: ";
	char *filelength;
	struct stat filestat;

	stat(connection->docs, &filestat);

	filelength = malloc( sizeof(intmax_t));
	memset(filelength, '\0', sizeof(intmax_t));
	sprintf(filelength, "%jd", (intmax_t)filestat.st_size);

	write(connection->sd, length, sizeof(length));
	write(connection->sd, filelength, sizeof(off_t));
	write(connection->sd, "\n\n", 2);

	connection->charfilesize = filelength;
}

/*
 * Prints the length for server generated HTML files.
 */
void print_html_length(struct client_info *connection) {
	char length[] = "Content-Length: ";
	char *filelength;
	int stringsize;

	/* Find the number of digits for converting to string */
	stringsize = floor(log10(abs(connection->filesize))) + 1;
	stringsize = stringsize*sizeof(char);

	filelength = malloc(stringsize);
	memset(filelength, '\0', stringsize);
	sprintf(filelength, "%d", connection->filesize);

	write(connection->sd, length, strlen(length));
	write(connection->sd, filelength, stringsize);
	write(connection->sd, "\n\n", 2);

	connection->charfilesize = filelength;
}

/* 
 * Builds the HTML response and sets the client_info structure
 */
void build_html(char *title, char *body, struct client_info *connection ){
	char htmlopen[] = "<html><body>\n";
	char headopen[] = "<h2>";
	char headclose[] = "</h2>\n";
	char htmlclose[] = "</body></html>\n";
	char *html;

	int size = strlen(htmlopen) + strlen(headopen) 
		+ strlen(headclose) + strlen(htmlclose) + strlen(title) 
		+ strlen(body);

	html = (char *)malloc(size);	
	memset(html, '\0', size);

	strcat(html, htmlopen);
	strcat(html, headopen);
	strcat(html, title);
	strcat(html, headclose);
	strcat(html, body);
	strcat(html, htmlclose);

	connection->htmlobject = html;
	connection->filesize = size;
}