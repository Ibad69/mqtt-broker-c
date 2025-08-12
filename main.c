// #include <cstdlib>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

int main (void) {
    
    // the struct is named addrinfo 

    struct addrinfo hints, *res;
    socklen_t socklen; 

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM; //for TCP
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;

    if ((getaddrinfo(NULL, "3100", &hints, &res)) == -1) {
        perror("gett address info failed");
    };

    printf("new connection is being made \n");

    // initialize a socket with the address info
    // return us a file descriptor
    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == -1 ) {
        perror("socket connection failed");
    }

    int yes=1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
    }
    // bind this socket to a port

    int b = bind(s, res->ai_addr, res->ai_addrlen);
    if (b == -1) {
        perror("binding the socket has failed");
    }


    int l = listen(s, 5);
    if (l == -1) {
        perror("listener is not working correctly");
    }

    // accept the connections itself
    while (1) {
        printf("hi connection came \n");
        int a = accept(s, res->ai_addr, &socklen);
        if (a == -1) {
            perror("connection accepted");
        }

        int msg = send(a, "hello world", 15, 0);
        printf("active connections at the moment...\n");
        if (msg == -1) {
            perror("error sending sms to the connection");
        }
    }


    return 0;
}
