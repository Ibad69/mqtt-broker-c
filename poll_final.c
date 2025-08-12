// final polling 

// #include <cstdlib>
// #include <cstdint>
// #inclu
// #include <cstddef>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdint.h>
#include <errno.h>


typedef enum MQTT_STATES {
    MQTT_PUBLISH_DUP,
    MQTT_PUBLISH_QOS_MASK,
    MQTT_PUBLISH_RETAIN
} mqtt_states;

typedef enum PACKET_TYPE {
    CONNACK = 2,
    CONNECT = 1,
    PUBACK = 4,
    PUBLISH = 3,
    PUBREC = 5,
    PUBCOMP = 7,
    PUBREL = 6,
    SUBSCRIBE = 8,
    SUBACK = 9,
    UNSUBSCRIBE = 10,
    UNSUBACK = 11,
    DISCONNECT = 14,
    AUTH = 15,
    PINGREQ = 12,
    PINGRESP = 13,
} packet_type;

enum MQTTerrors {
    MQTT_OK = 1
};

typedef struct PACKET_HEADER {
    enum PACKET_TYPE control_type;
    uint32_t control_flags:4;
    uint32_t remaining_length;
} packet_header;

typedef struct MQTTPACKET {
   packet_header header;
   uint64_t variable_header;
   uint64_t payload;
   uint16_t topic_name_size;
   const void* topic_name;
} mqtt_packet;


typedef struct MESSAGE {
    char *content;
    int sent_from_socket;
} message;

typedef struct SUBSCRIBER {
    int socket_id;
} subscriber;

typedef struct TOPIC {
    char *name;
    int subcsriber_count;
    message *messages;
    subscriber *subscribers;
} topic;

typedef struct CLIENT {
    int fd;
    uint8_t *buf;
} client;



void add_new_subscriber(subscriber *subc, int socket_id, topic *topc) {
    topc->subscribers[topc->subcsriber_count] = *subc;
    topc->subcsriber_count++;
}

void remove_subscriber(subscriber *subc, int socket_id, topic *topc) {

}

void add_message_to_topic(topic *topc, message *msg) {

}

ssize_t pack_packet_header(uint8_t *buf, size_t bufsz, mqtt_packet *packet) {
   *buf = packet->header.control_type << 4; 
   *buf = packet->header.control_flags & 0x0F;
    uint8_t remaining_length = packet->header.remaining_length;
    const uint8_t *start = buf;

    do {
        bufsz--;
        buf++;
        if (bufsz == 0) return 0;

        *buf  = remaining_length & 0x7F;
        if(remaining_length > 127) *buf |= 0x80;
        remaining_length = remaining_length >> 7;

    }while (*buf & 0x80);
    bufsz--;
    buf++;
    return buf - start;
}

void unpack_sub_response(const uint8_t *buf, mqtt_packet *packet, size_t bufsz) {
    // // response->dup_flag = (fixed_header->control_flags & MQTT_PUBLISH_DUP) >> 3;
    // // response->qos_level = (fixed_header->control_flags & MQTT_PUBLISH_QOS_MASK) >> 1;
    // // response->retain_flag = fixed_header->control_flags & MQTT_PUBLISH_RETAIN;

    // uint8_t dup_flag = (packet->header.control_flags & MQTT_PUBLISH_DUP ) >> 3;
    // uint8_t qos_level = (packet->header.control_flags& MQTT_PUBLISH_QOS_MASK) >> 1;
    // uint8_t retain_flag = (packet->header.control_flags & MQTT_PUBLISH_RETAIN);

    uint16_t integer_htons;
    memcpy(&integer_htons, buf, 2uL);
    packet->topic_name_size = ntohs(integer_htons);
    buf+=4;
    packet->topic_name = buf;
    printf("the buffer is now %c \n", *buf);
    buf += packet->topic_name_size;
}

ssize_t pack_connack_uint16(uint8_t *buf) {
    uint16_t integer_htons;
    memcpy(buf, &integer_htons, 2uL);
    return 2;
}

void unpack_publish_response(const uint8_t *buf, mqtt_packet *packet, size_t bufsz) {
    // // response->dup_flag = (fixed_header->control_flags & MQTT_PUBLISH_DUP) >> 3;
    // // response->qos_level = (fixed_header->control_flags & MQTT_PUBLISH_QOS_MASK) >> 1;
    // // response->retain_flag = fixed_header->control_flags & MQTT_PUBLISH_RETAIN;

    uint8_t dup_flag = (packet->header.control_flags & MQTT_PUBLISH_DUP ) >> 3;
    uint8_t qos_level = (packet->header.control_flags& MQTT_PUBLISH_QOS_MASK) >> 1;
    uint8_t retain_flag = (packet->header.control_flags & MQTT_PUBLISH_RETAIN);

    uint16_t integer_htons;
    memcpy(&integer_htons, buf, 2uL);
    packet->topic_name_size = ntohs(integer_htons);
    buf+=2;
    packet->topic_name = buf;
    printf("the buffer is now %c \n", *buf);
    buf += packet->topic_name_size;
}

void unpack_packet_headerV2(const uint8_t *buf, mqtt_packet *packet, size_t bufsz) { 

    packet->header.control_type = *buf >> 4;
    packet->header.control_flags = *buf & 0x0F;
    packet->header.remaining_length = 0;

    int lshift = 0;

    do {
        if (lshift == 28) {
            perror("invalid remaining_length");
        }

        if (bufsz ==0) return;

        bufsz--;
        buf++;

        packet->header.remaining_length += (uint32_t) + ((*buf & 0x7F) << lshift);
        lshift += 7;

    } while (*buf & 0x80);
}

ssize_t unpack_packet_header (const uint8_t *buf, mqtt_packet *packet, size_t bufsz) { 

    packet->header.control_type = *buf >> 4;
    packet->header.control_flags = *buf & 0x0F;
    packet->header.remaining_length = 0;

    const uint8_t *start = buf;
    ssize_t consumed = 0;

    int lshift = 0;

    do {
        printf("moved how many times? \n");
        if (lshift == 28) {
            perror("invalid remaining_length");
        }

        --bufsz;
        ++buf;
        if (bufsz == 0) return 0;

        packet->header.remaining_length += (uint32_t) + ((*buf & 0x7F) << lshift);
        lshift += 7;

    } while (*buf & 0x80);

    // consuming the last byte
    --bufsz;
    ++buf;

    // printf("buf - start", )
    return buf - start;
}

void unpack_response(packet_header *ph, uint8_t *buf) {

        // we do this because we want the actual value or number for this so we make it as we use to send it 
        // bcos at the time of sending we use plain 2 which is directly proportional to 00000010 
        // if we receive something like 00100000 we have to shift it to the right to get the correct value
        // this will have the packet type header or the header with flags
    // unsigned int charvalue = packet_type >> 4;
    ph->control_type = (*buf >> 4);
    ph->control_flags = (*buf & 0x0F);

    // now parse the reamaining length
}


unsigned char* generate_packet_type (packet_type type) {
    // snprintf(smsg,40,"thank you for connecting; %d;xyz \n", a);
    // unsigned char connack[] = { 0x20, 0x02, 0x00, 0x00 };
    unsigned char *connack = malloc(4);
    if (connack == NULL) {
        printf("error occured in allocating memory");
        return NULL;
    }
    switch(type) {
        case CONNACK:
            connack[0] = type << 4;
            connack[1] = 0x02;
            connack[2] = 0x00;
            connack[3] = 0x00;
            return connack;
        case PINGRESP:
            connack[0] = type << 4;
            return connack;
    }
}


unsigned char decode_packet_type (char* buf) {
}

long int unpacki32(unsigned char *buf)
{
    unsigned long int i2 = ((unsigned long int)buf[0]<<24) |
        ((unsigned long int)buf[1]<<16) |
        ((unsigned long int)buf[2]<<8)  |
        buf[3];
    long int i;

    // change unsigned numbers to signed
    if (i2 <= 0x7fffffffu) { i = i2; }
    else { i = -1 - (long int)(0xffffffffu - i2); }

    return i;
}

int get_listener() {

    struct addrinfo hints, *ai, *p;

    int listener;
    int yes=1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;

    if ((getaddrinfo(NULL, "3001", &hints, &ai)) != 0) {
        perror("error occured in initializing struct");
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

void del_from_pfds(struct pollfd *pfds, int *pfd_i, int *fd_count) {
    // Copy the one from the end over this one
    pfds[*pfd_i] = pfds[*fd_count-1];

    (*fd_count)--; 
}

ssize_t recv_all(void* buf, struct pollfd *pfds, size_t bufsz, int *pfd_i, int *fd_count) { 
    
    // uint8_t buff[256];
    // this is an array but can pretend to be the pointer to the first element of the array if needed
    // c decays this automatically for us to a (void *)&buff[0];

    const void *const start = buf;
    int nbytes = recv(pfds[*pfd_i].fd, buf, bufsz, 0);
    int sender_fd = pfds[*pfd_i].fd;
    printf("looping inside of recv functino call \n");

    if (nbytes <= 0) { // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", sender_fd);
            // break;
        } 
        if (nbytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                /* should call recv later again */
                perror("errorinrecv");
                // break;
            }
            /* an error occurred that wasn't "nothing to read". */
            // error = MQTT_ERROR_SOCKET_ERROR;
            // break;
        }
        close(pfds[*pfd_i].fd); // Bye!

        printf("removing from the array \n");
        del_from_pfds(pfds, pfd_i, fd_count);

        // printf("closed this client removed from pfds breaking? \n");
        // reexamine the slot we just deleted
        (*pfd_i)--;
        // break;
    } 

    // printf("buf at this time : %s \n", (char*)buf);
    // printf("busz at this time: %zu \n", bufsz);

    buf = (char*)buf + nbytes;
    bufsz -= (unsigned long)nbytes;

    return (char*)buf - (const char*)start;
}

ssize_t send_all(int socfd, void *buf, size_t len, int flags) {
    size_t sent = 0;
    ssize_t rv = send(socfd, (char*)buf + sent, len - sent, flags);
    if (rv == -1) {
        perror("errorinsendall");
    }
}



void handle_client_data(int listener, 
                        int *fd_count, struct pollfd *pfds, 
                        int *pfd_i, topic *topics, 
                        uint8_t *buf, ssize_t *bufsz, 
                        uint8_t *sbuf, ssize_t *sbufsz
                        )
{
    mqtt_packet packet;
    mqtt_packet s_packet;
    ssize_t rv = recv_all(buf, pfds, *bufsz, pfd_i, fd_count); 
    uint8_t *sbuf_cursor = sbuf;

    // printf("the bytes that are there before == %zu \n", rv);
    // for(int i = 0; i < rv; i++) {
    //     printf("buf character inside loop: %c \n", buf[i]);
    //     printf("bytes %02X", buf[i]);
    // }
    // buf += rv;
    // bufsz -= (unsigned long)rv;
    // printf("the element buf[0]: %d \n", buf[0]);
    // printf("buf is pointing to the element %d \n", *buf);
    // printf("bufsz before %zu", bufsz);

    ssize_t rvv = unpack_packet_header(buf, &packet, *bufsz);
    buf += rvv;

    // printf("the bytes that are there == %zu \n", rvv);
    // printf("packet header: %d \n", packet.header.control_type);
    // printf("reamaining length is : %d \n", packet.header.remaining_length);

    switch(packet.header.control_type) {
        // packet type / header buf[0]
        // remaining_length buf[1]
        // variable header buf[2] buf[3]
        // payload buf[4] 
        case SUBSCRIBE: { 
            // printf("inside subscribe request the name of the topic : %s \n", topics[0].name);
            unpack_sub_response(buf, &packet, *bufsz);
            // unsigned header_size = buf[0];
            // unsigned remaining_length = buf[1];
            // for (int i = 7; i >= 1; i--){
            //     printf("going through the remaining_length byte : bit => %d value => %d \n", i, (remaining_length >> i) & 1);
            // }
            // unsigned long int h = ((unsigned long int)buf[0]>>25 | (unsigned long int)buf[1]>>16 |(unsigned long int)buf[2]>>8 | buf[3]);
            // printf("print the value: %lu \n", h);
            break;
        }
        case CONNECT: {

            s_packet.header.control_type = CONNACK;
            s_packet.header.control_flags = 0x00;
            s_packet.header.remaining_length = 2;

            ssize_t rv = pack_packet_header(sbuf_cursor, *sbufsz, &s_packet);
            printf("received from packing packet header: %zu \n", rv);
            sbuf_cursor+=rv;
            sbuf_cursor = 0x00;
            sbuf_cursor++;
            sbuf_cursor = 0x00;
            sbuf_cursor += pack_connack_uint16(buf);
            size_t sent = 0;
            printf("pfdsaddress %d \n", pfds[*pfd_i].fd);

            send_all(pfds[*pfd_i].fd, sbuf, 245, 0);
            break;
        }
        case PUBLISH: {
            printf("publish message \n");
            unpack_publish_response(buf, &packet, *bufsz);
            printf("topic name finally : %s \n", (char*)packet.topic_name);
            break;
        }
        case PINGREQ: { 
            unsigned char *packet = generate_packet_type(PINGRESP);
            // printf("sending a ping res response to the connected client \n");
            if ((send(pfds[*pfd_i].fd, packet, sizeof packet, 0)) == -1){
                perror("hcsend");
            };
            break;
        }
        case PUBACK: { 
            // printf("puback \n");
            // if ((send(pfds[*pfd_i].fd, packet, sizeof packet, 0)) == -1){
            //     perror("hcsend");
            // };
        }
    }
    // // Send to everyone!
    // // for(int j = 0; j < *fd_count; j++) {
    // //     int dest_fd = pfds[j].fd;

    // //     // Except the listener and ourselves
    // //     if (dest_fd != listener && dest_fd != sender_fd) {
    // //         if (send(dest_fd, buf, nbytes, 0) == -1) {
    // //             perror("send");
    // //         }
    // //     }
    // // }
}

// handle client data v1 old

void handle_client_datav1(int listener, int *fd_count, struct pollfd *pfds, int *pfd_i, topic *topics)
{
    char buf[32];    // Buffer for client data
    int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);

    int sender_fd = pfds[*pfd_i].fd;

    printf("the n bytes received %d \n", nbytes);

    if (nbytes <= 0) { // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed
            printf("pollserver: socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }

        close(pfds[*pfd_i].fd); // Bye!

        del_from_pfds(pfds, pfd_i, fd_count);

        // reexamine the slot we just deleted
        (*pfd_i)--;

    } else { 
        // we do this because we want the actual value or number for this so we make it as we use to send it 
        // bcos at the time of sending we use plain 2 which is directly proportional to 00000010 
        // if we receive something like 00100000 we have to shift it to the right to get the correct value
        // this will have the packet type header or the header with flags
        unsigned char packet_type = buf[0];
        unsigned int charvalue = packet_type >> 4;

        printf("the packet type is ? %d \n", charvalue);
        printf("buf ,%c \n" , buf[7]);

        switch(charvalue) {
            // packet type / header buf[0]
            // remaining_length buf[1]
            // variable header buf[2] buf[3]
            // payload buf[4] 
            case SUBSCRIBE: { 
                printf("the name of the topic : %s \n", topics[0].name);
                printf("this is a subscribe request:  %d \n", buf[3]);
                unsigned header_size = buf[0];
                unsigned remaining_length = buf[1];
                // for (int i = 7; i >= 1; i--){
                //     printf("going through the remaining_length byte : bit => %d value => %d \n", i, (remaining_length >> i) & 1);
                // }
                // unsigned long int h = ((unsigned long int)buf[0]>>25 | (unsigned long int)buf[1]>>16 |(unsigned long int)buf[2]>>8 | buf[3]);
                // printf("print the value: %lu \n", h);
            }
            case PINGREQ: { 
                unsigned char *packet = generate_packet_type(PINGRESP);
                printf("sending a ping res response to the connected client \n");
                if ((send(pfds[*pfd_i].fd, packet, sizeof packet, 0)) == -1){
                    perror("hcsend");
                };
            }
        }
        // Send to everyone!
        // for(int j = 0; j < *fd_count; j++) {
        //     int dest_fd = pfds[j].fd;

        //     // Except the listener and ourselves
        //     if (dest_fd != listener && dest_fd != sender_fd) {
        //         if (send(dest_fd, buf, nbytes, 0) == -1) {
        //             perror("send");
        //         }
        //     }
        // }
    }
}


void add_to_pfds(int newfd, struct pollfd **pfds, int *fd_count, int *fd_size) {
    // TODO;realloc if size is full

    printf("adding to array \n");

    if (*fd_size == *fd_count) {
        *fd_size *= 2;
        *pfds = realloc(*pfds, sizeof(**pfds)*(*fd_size)); 
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*pfds)[*fd_count].revents = 0;
    (*fd_count)++;

    uint8_t buf[256];

    // client *sclient = malloc(sizeof(client));
    // sclient->buf = buf;

}

int accept_connection(struct pollfd **pfds, int *fd_count, int *fd_size, int listener, int *i) {

    struct sockaddr_storage storage;
    socklen_t socklen;

    int a = accept(listener, (struct sockaddr *)&storage, &socklen);
    if (a == -1){
        perror("accept");
        // return;
    }

    // char buf[256];

    // int nbytes = recv(a, buf, sizeof buf, 0);

    // if (nbytes <= 0) {
    //     if (nbytes == 0) {
    //         printf("socket hung up \n");
    //     }
    //     else {
    //         perror("accerecv");
    //     }
    // } 
    // else {
    //     printf("conn: recv from fd : %.*s \n",  nbytes, buf);
    // }

    add_to_pfds(a, pfds, fd_count, fd_size);

    // char *smsg = "Thank you for connecting!";
    unsigned char *connack = generate_packet_type(CONNACK);
    printf("char at 0 index %d \n", connack[0]);
    int msg = send(a, connack, sizeof(connack), 0);
    if (msg == -1) {
        perror("send");
    }

    free(connack);
    return a;
}

void process_connection(struct pollfd **pfds, int *fd_size, int *fd_count, int listener, topic *topics) {
    for (int i = 0; i < *fd_count; i++) {
        // printf("printing the value that the pfds is containing : %d", pfds[i].revents);
        if ((*pfds)[i].revents & (POLLIN | POLLHUP))  {
            if ((*pfds)[i].fd == listener) {
                printf("I am accepting this connection \n");
                int acc = accept_connection(pfds, fd_count, fd_size, listener, &i);
            }
            else {
                uint8_t buf[256];
                ssize_t bufsz = sizeof(buf);
                uint8_t sbuf[256];
                ssize_t sbufsz = sizeof(sbuf);
                handle_client_data(listener, fd_count, *pfds, &i, topics, buf, &bufsz, sbuf, &sbufsz);
                // handle_client_datav1(listener, fd_count, *pfds, &i, topics);
            }
        }
    }
}

int main () {

    printf("polling is now starting \n");

    int fd_size = 5;
    int fd_count = 0;

    subscriber *subcs = malloc(5);
    if (subcs == NULL) {
        printf("error allocating subscribers");
    }

    message *msgs = malloc(100);
    if (msgs == NULL) {
        printf("error allocating msgs");
    }

    topic tp = {.name = "data", .subcsriber_count = 0, .subscribers = subcs, .messages = msgs };
    topic *topics = malloc(sizeof(topic));
    if (topics == NULL) {
        printf("error allocating topics");
    }

    int topic_count = 0;
    topics[topic_count] = tp;

    struct pollfd *pfds = malloc(fd_size*2);
    if (pfds == NULL) {
        printf("error allocating pfds");
    }
    // struct pollfd pfds[fd_size];


    int listener = get_listener();

    pfds[0].fd = listener;
    pfds[0].events = POLLIN;
    fd_count = 1;

    for (;;) {
        printf("in main loop before poll \n");
        int pollres = poll(pfds, fd_count, -1);
        if (pollres == -1) {
            perror("polling");
        }
        process_connection(&pfds, &fd_size, &fd_count, listener, topics);
    }

    free(pfds);
    free(topics);
    free(subcs);
    free(msgs);

    return 0;
}
