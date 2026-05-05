#ifndef INCLUDE_SOCK_AUX_H
#define INCLUDE_SOCK_AUX_H

#include <sys/socket.h>

int my_create_server_socket(char *port);
int my_connect(char *servername, char *port);
void print_socket_address(int sd);
void print_address(const struct sockaddr * clt_addr, socklen_t addrlen);

int myReadLine1(int, char *, int);
int myReadLine2(int, char *, int);
int myReadBlock(int, void *, int);


#endif
