#include "server.h"
#include <time.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>

#define MAXBUFFER 80
/*
 * Uses open file descriptor to print log information related to server
 */
//void handlelog(struct server_settings *settings) {
void handlelog(struct client_info *info) {
        time_t rawTime;
        struct tm *timeInfo;
        char buffer[MAXBUFFER];
        FILE *fp;

        fp = fopen(info->path, "a");

        /* Print the time */
        time(&rawTime);
        timeInfo = gmtime(&rawTime);
        strftime(buffer, MAXBUFFER, "%a %d %b %Y %X %Z", timeInfo);

        fprintf(fp, "%s\t", buffer);

        /* Print client settings */                
        fprintf(fp, "%s\t", inet_ntoa(info->client.sin_addr));

        /* Print the request type */
        fprintf(fp, "%s %s %s\t", info->reqtype, info->filename,
        	info->protocol);

        /* Print the results */
        switch(info->request){
        	case GETREQUEST:
        	    fprintf(fp, "%s ", "200 OK");
        		/* Print bytes transfered */
				fprintf(fp, "%d/", info->bytes);
				fprintf(fp, "%s\n", info->charfilesize);
        	    break;
        	case BADREQUEST:
        	    fprintf(fp, "%s\n", "400 Bad Request");
        	    break;
        	case NOTFOUND:
        	    fprintf(fp, "%s\n", "404 Not Found");
        	    break;
        	case FORBIDDEN:
        	    fprintf(fp, "%s\n", "403 Forbidden");
        	    break;
        	default:
        	    fprintf(fp, "%s\n", "500 Internal Server Error");
        	    break;
        }

}