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

#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include "server.h"

pthread_mutex_t logLock;
sig_atomic_t processes;

/*
 * A signal handler for SIGCHLD to handle zombies when a fork is
 * destroyed. Following Bob Becks example in server.c
 */
static void kidhandler(int signum) {
        waitpid(WAIT_ANY, NULL, WNOHANG);
}

/*
 * A simple server using forks
 */
void initiate_server_f(struct server_settings *settings){
        struct sockaddr_in sockname, client;
        struct sigaction action;
        socklen_t clientLen;
        int socketDescriptor;
        pid_t pid;

        memset(&sockname, 0, sizeof(sockname));

        /*
         * Pack the socket structure with IPv4 settings on localhost,
         * then create a socket and return the descriptor
         */
        sockname.sin_family = AF_INET;
        sockname.sin_port = htons(settings->port);
        sockname.sin_addr.s_addr = htonl(INADDR_ANY);
        socketDescriptor = socket(AF_INET,SOCK_STREAM,0);

        /* Have failed to retrieve socket descriptor */
        if(socketDescriptor == -1){
                err(1, "Socket failed");
        }

        /* Attempt to assign the socket to the localhost */
        if(bind(socketDescriptor, (struct sockaddr *) &sockname, 
                sizeof(sockname)) == -1) {
                err(1, "Bind failed");
        }

        /* Socket now allows incoming connection requests */
        if(listen(socketDescriptor, 3) == -1) {
                err(1, "Listen failed");
        }

        /* Handle zombies and legitimate system calls */
        action.sa_handler = kidhandler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = SA_RESTART;
        if(sigaction(SIGCHLD, &action, NULL) == -1) {
                err(1, "sigaction failed");
        }

        /* Initialize mutex. Needed to limit *fp access for log file */
        pthread_mutex_init(&logLock, NULL);
        processes = 0;

        /* Create infinite polling loop for server to accept connections */
        for(;;) {
                int clientSocketDesc;
                clientLen = sizeof(&client);
                clientSocketDesc = accept(socketDescriptor, (struct sockaddr *)&client, &clientLen);

                /*
                 * Fork connections connected to socket to handle multiple
                 * requests.
                 */
                if (clientSocketDesc == -1) {
                        err(1, "Accepting connection failed");
                }
                else if(processes < MAXPROCESSES){
                        pid = fork();
                        if(pid == -1) {
                                err(1, "Fork failed");
                        }
                        
                        if(pid == 0) {
                                processes+=1;

                                struct client_info *con = (struct client_info *)
                                        malloc(sizeof(struct client_info));
                                memset(con, '\0', sizeof(struct client_info));
                                
                                /* Pass the attributes into the client structure */
                                con->client = client;
                                con->path = settings->path;
                                con->docs = settings->docs;
                                con->sd = clientSocketDesc;

                                /*
                                 * read_socket() determines request and
                                 * prints results to the socket.
                                 */
                                read_socket(con);

                                /* Only allow one proccess to write to log */
                                pthread_mutex_lock(&logLock);
                                
                                handlelog(con);

                                pthread_mutex_unlock(&logLock);

                                processes-=1;
                                exit(0);
                        }                        
                }


                close(clientSocketDesc);
        }
        
}
