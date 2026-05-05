#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define ID_CAT "1170701"
#define ID_MARIA "1231098"

int my_create_server_socket(char *port);

int main(int argc, char *argv[]) {
    printf("1170701 - %s\n", __FILE__);

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    signal(SIGCHLD, SIG_IGN); // Prevent zombies
    int s = my_create_server_socket(argv[1]);

    while (1) {
        struct sockaddr clt_addr;
        socklen_t addrlen = sizeof(clt_addr);
        int ns = accept(s, &clt_addr, &addrlen);
        if (ns < 0) continue;

        if (fork() == 0) { // Concurrent handling
            close(s);
           
            // Using fdopen for easier line-by-line reading
            FILE *fp = fdopen(ns, "r+");
            char id[100], msg[2000], name[100];

            // Read ID and Message
            if (fgets(id, sizeof(id), fp) && fgets(msg, sizeof(msg), fp)) {
                id[strcspn(id, "\n")] = 0; // Remove newline
                msg[strcspn(msg, "\n")] = 0;

                // Determine name based on ID
                if (strcmp(id, ID_CAT) == 0) strcpy(name, "Catarina");
                else if (strcmp(id, ID_MARIA) == 0) strcpy(name, "Maria");
                else strcpy(name, "Unknown Student");

                // convert message to uppercase
		for (int i = 0; msg[i]; i++) msg[i] = toupper(msg[i]);

                //Send Response
                fprintf(fp, "%s\n%s\n", msg, name);
            }

            fclose(fp);
            exit(0);
        }
        close(ns);
    }
    return 0;
}

int my_create_server_socket(char *port) {
    int s;
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &res);
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
   
    bind(s, res->ai_addr, res->ai_addrlen);
    listen(s, 5);
    freeaddrinfo(res);
    return s;
}
