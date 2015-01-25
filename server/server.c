/*
 * Copyright (c) 2014 Michael Raypold <raypold@ualberta.ca>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * The main loop that takes as arguments the server type, port number and 
 * location of on disk for documents and logs.
 *
 * Server arguments are server_f, server_p and server_s
 *
 * Example, "./server_f 8080 docs logs/log.txt"
 *
 */

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include "server.h"

/* Parse command line input and initialize the appropriate server */
int main(int argc, char *argv[]){
        FILE *fp;
        DIR *dir;

        struct server_settings *settings = (struct server_settings *)
                malloc(sizeof(struct server_settings));
                
        if(argc != 5){
            printf("Incorrect command line arguments\n");
            exit(0);
        }

        /* Get command line arguments with minimal error checking */      
        while(optind < argc){
            /* Get server type */
            if(optind == 1){
                if(strncmp(argv[optind], "server_f", 8) == 0){
                    settings->type = 0;
                }
                else if(strncmp(argv[optind], "server_p", 8) == 0){
                    settings->type = 1;
                }
                else if(strncmp(argv[optind], "server_s", 8) == 0){
                    settings->type = 2;
                }
                else{
                    printf("Unspecified server type\n");
                    exit(0);
                }
            }

            /* Get port number */
            if(optind == 2){
                settings->port = atoi(argv[optind]);
            }

            /* Get document location */
            if(optind == 3) {
                settings->docs = argv[optind];
            }

            /* Get log location */
            if(optind == 4){
                settings->path = argv[optind];
            }

            optind++;
        }

        /* Determine if correct document directory */
        dir = opendir(settings->docs);

        if(dir == NULL) {
            printf("Documents directory is not available!\n");
            printf("Exiting server\n");
            exit(0);
        }
        closedir(dir);

        /* Determine if correct directory for logfile location */
        fp = fopen(settings->path, "a");

        if(fp == NULL) {
            printf("Directory and log file do not exist!\n");
            printf("Exiting server\n");
            exit(0);
        }
        fclose(fp);

        /* Initiate the appropriate server */
        switch(settings->type) {
            case SERVER_F:
                initiate_server_f(settings);
                break;
            case SERVER_P:
                initiate_server_p(settings);
                break;
            case SERVER_S:
                initiate_server_s(settings);
                break;
            default:
                printf("Did not recognize server type\n");
                exit(0);
                break;
        }
        return 0;
}