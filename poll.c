// this implmentation will cover polling as generally as possible
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <sys/socket.h>

int main () {

    int fd_size = 5;
    int fd_count = 0;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    struct addrinfo hints, *res;

    socklen_t socklen;
    
    memset(&hints, 0, sizeof hints); 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;

    if ((getaddrinfo(NULL, "3100", &hints, &res)) == -1) {
        perror("the first listener is causing issues");
    }
    
    // poll generally will be a system call which will let our operating system handle the dirty work

    // how polling works
    // the poll() call gives us the number of elements that needs to process or are available to be processed?
    // pollfd struct has (fd, events, revents) events that have been processed, revents that needs to be processed
    // we need poll in the main loop and keep on processing the connections
    // the processing connections is generally going to be going through the connections basically in the pollfd array
    // the processing connection will either handle a new connection or an old one we will only be working for a new one
    // new connection will or increase the fd count and add in the array of fds
    // add in array of fds will generally me handled by a separate function to add in the array

    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == -1) {
        perror("error creating socket");
    }

    int yes=1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
    }

    int b = bind(s, res->ai_addr, res->ai_addrlen);
    if (b == -1) {
        perror("error in binding");
    }

    int l = listen(s, 5);
    if (s == -1) {
        perror("error creating socket");
    }



    pfds[0].events = POLLIN;
    pfds[0].fd = s;

    printf("polling has started... \n");

    for(;;){

        int num_event = poll(pfds, 1, 0);
        if (num_event == -1) {
            perror("something went wrong in polling");
        }
        else {
            if (pfds[0].revents && POLLIN) {
                printf("File descriptor %d is ready to read\n",
                       pfds[0].fd);
                int a = accept(s, res->ai_addr, &socklen);
                if (a == -1) {
                    perror("connection accepted");
                }

                // send a message to the connected client!
                int msg = send(a, "hello world", 15, 0);
                printf("active connections at the moment...\n");
                if (msg == -1) {
                    perror("error sending sms to the connection");
                }

                // receive message from the connected client
                char msgbuf[245];
                int rmsg;

                // while ((rmsg = recv(a, msgbuf, sizeof(msgbuf) - 1, 0)) > 0) {
                //     msgbuf[rmsg] = '\0';  // Null-terminate the buffer
                //     printf("received message from the client: %s\n", msgbuf);
                // }

                rmsg = recv(a, msgbuf, sizeof(msgbuf) -1, 0);
                if (rmsg == 0) {
                    printf("connection closed by the client \n");
                } else if (rmsg == -1) {
                    perror("error in receiving msgs");
                }
                else {
                    printf("received from the client : %s", msgbuf);
                }
            }
        }

    }


    return 0;
}
