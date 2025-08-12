
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <sys/socket.h>

void add_to_pfds(struct pollfd pfds[], int newfd, int *fd_count) { 
    printf("adding the connection to the fd : %d \n", newfd);
    pfds[*fd_count].fd = newfd;
    pfds[*fd_count].events = POLLIN;
    pfds[*fd_count].revents = 0;
    (*fd_count)++;
    // for(int i = 0; i < 5; i++) {
    //     // printf("pfds element: %d \n", i);
    //     // printf("pfds value: %d \n", pfds[i].fd);
    // }
}

void handle_client_data(int listener, int fd_count, struct pollfd pfds[], int i) {

    // receive message from the connected client
    char msgbuf[245];
    int rmsg;
    // while ((rmsg = recv(a, msgbuf, sizeof(msgbuf) - 1, 0)) > 0) {
    //     msgbuf[rmsg] = '\0';  // Null-terminate the buffer
    //     printf("received message from the client: %s\n", msgbuf);
    // }
    rmsg = recv(pfds[i].fd, msgbuf, sizeof(msgbuf) -1, 0);
    if (rmsg == 0) {
        printf("connection closed by the client \n");
    } else if (rmsg == -1) {
        perror("error in receiving msgs");
    }
    else {
        printf("received from the client %d: %s", pfds[i].fd, msgbuf);
    }

}

int create_listener(void) {
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, "3100", &hints, &ai)) != 0) {
        fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype,
                          p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            // close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    freeaddrinfo(ai); // All done with this

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}


void accept_connection(int fd, int *fd_count, struct pollfd pfds[]) {
    socklen_t socklen;
    struct sockaddr_storage remoteaddr;

    int a = accept(fd, (struct sockaddr *)&remoteaddr, &socklen);
    if (a == -1) {
        perror("some error occured in accepting the connection");
    }

    add_to_pfds(pfds, a, fd_count);

    char *smsg;
    snprintf(smsg,40,"thank you for connecting; %d;xyz \n", a);
    int msg = send(a, smsg, 40, 0);
    if (msg == -1) {
        perror("sending message");
    }
}


void process_connections_dynamic_memory(int num_events, struct pollfd **pfds, int listener) {

     // for(int i = 0; i < *fd_count; i++) {

        // Check if someone's ready to read
        if ((*pfds)[0].revents & (POLLIN | POLLHUP)) {
            // We got one!!

            if ((*pfds)[0].fd == listener) {
                // If we're the listener, it's a new connection
                // accept_connection(listener);
            } else {
                // Otherwise we're just a regular client
                // handle_client_data(listener, fd_count, *pfds, &i);
            }
        }
    // }
    // for (int i = 0; i < num_events; i++) {
    // printf("inside processing connection \n");
    // if((*pfds)[0].revents != 0) {
    //     // means here the event has happnened
    //     if ((*pfds)[0].fd == listener){
    //         // this event was sent from an existing connection
    //         // recv();
    //     }
    //     else {
    //         printf("accepting the connection");
    //         // pfds[i] = 
    //         // add the new connection to the array
    //         int listener = create_listener();
    //         accept_connection(pfds[0].fd);
    //     }
    //     num_events++;
    //     // }
    // }

    // if(pfds[0].revents && POLLIN) {
    //     printf("we have received some sort of event \n");

    //     int a = accept(s, res->ai_addr, &sockelen);
    //     if (a == -1) {
    //         perror("error occured at accepting the connection");
    //     }

    //     int msg = send(a, "Hey there! thank you for connecting to me", 50, 0);
    //     if (msg == -1) {
    //         perror("error sending the message");
    //     }
    //     printf("waiting for new connections... \n");
    // }
    
}

void process_connection_stack(int num_events, struct pollfd pfds[], int listener, int *fd_count) {
    for (int i = 0; i < *fd_count; i++) {
        if (pfds[i].revents & (POLLIN | POLLHUP)) {
            if (pfds[i].fd == listener) {
                // If we're the listener, it's a new connection
                accept_connection(listener, fd_count, pfds);
            } else {
                // printf("I am inside the else condition, event has occured at client : %d\n", pfds[i].fd);
                handle_client_data(listener, *fd_count, pfds, i);
            }
        } 
    }
    // Check if someone's ready to read
}


int main () {
    // firstly initialize a socket listner

    int fd_size = 5;
    int fd_count = 0;
    // struct pollfd *pfds = malloc(sizeof *pfds * fd_size);
    struct pollfd pfds[fd_size];

    socklen_t sockelen;

    
    int s = create_listener();

    pfds[0].events = POLLIN;
    pfds[0].fd = s;

    fd_count = 1;

    printf("starting the polling \n");

    while (1) {

        printf("waiting for another event to appear... \n");

        int num_events = poll(pfds, fd_count, -1);
        if (num_events == -1) {
            perror("something went wrong in polling \n");
        }
    
        printf("processing connections \n");
        int revents = num_events;
        // process_connections(revents, &pfds, s);
        process_connection_stack(revents, pfds, s, &fd_count);
    }




}
