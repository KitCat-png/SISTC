#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int my_create_server_socket(char *port);
void print_address(const struct sockaddr * clt_addr, socklen_t addrlen);

int main(int argc, char *argv[]) {
    printf("1170701 - %s\n", __FILE__);

    int new_socket_descriptor, socket_descriptor;
    struct sockaddr clt_addr;
    socklen_t addrlen;
 
    if(argc != 2) {
        printf("Usage: %s port_number\n", argv[0]);
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
       
    socket_descriptor = my_create_server_socket(argv[1]);

    while(1) {
        addrlen = sizeof(clt_addr);
        new_socket_descriptor = accept(socket_descriptor, &clt_addr, &addrlen);
       
        if(new_socket_descriptor < 0) {
            perror("accept");
            continue;
        }

        if (fork() == 0) {
            close(socket_descriptor);
            print_address(&clt_addr, addrlen);

            char filename[] = "fileXXXXXX";
            int fd = mkstemp(filename);
            if (fd < 0) { perror("mkstemp"); exit(1); }

            char buffer[1024];
            int n;
           
            while(1) {
                sleep(10); // Requirement for observation
                n = read(new_socket_descriptor, buffer, sizeof(buffer));
                if (n <= 0) break;
                write(fd, buffer, n);
            }

            close(fd);
            close(new_socket_descriptor);
            exit(0);
        }
        close(new_socket_descriptor);
    }
    return 0;
}

int my_create_server_socket(char *port) {
    int s, r;
    struct addrinfo hints, *a;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    r = getaddrinfo(NULL, port, &hints, &a);
    if (r != 0) { exit(1); }
    s = socket(PF_INET, SOCK_STREAM, 0);
    int so_reuseaddr = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
    bind(s, a->ai_addr, a->ai_addrlen);
    listen(s, 5);
    freeaddrinfo(a);
    return s;
}

void print_address(const struct sockaddr * clt_addr, socklen_t addrlen) {
    char hostname[256], port[6];
    getnameinfo(clt_addr, addrlen, hostname, sizeof(hostname), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);
    printf("Connection from %s:%s\n", hostname, port);
}
