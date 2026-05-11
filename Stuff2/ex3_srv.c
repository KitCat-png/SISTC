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

    // ⭐ Why SIG_IGN for SIGCHLD?
    // It tells the OS to automatically clean up child processes when they finish, preventing "zombie" processes from hogging memory.
    signal(SIGCHLD, SIG_IGN); 
    int s = my_create_server_socket(argv[1]);

    while (1) {
        // ⭐ What does this accept block do?
        // 1. The Caller ID: Sets up empty memory to store the incoming client's IP and port.
        struct sockaddr clt_addr;
        socklen_t addrlen = sizeof(clt_addr);
        
        // 2. The Wait & Dedicated Line: accept() pauses until a client connects. 
        // It creates 'ns', a BRAND NEW socket strictly for talking to this specific client.
        // The original socket 's' stays at the front door listening for more people.
        int ns = accept(s, &clt_addr, &addrlen);
        
        // 3. Error Handling
        if (ns < 0) continue;

        if (fork() == 0) { // Concurrent handling: Child process takes over this specific client
            close(s); // Child doesn't need the main listening socket
            
            // ⭐ Why fdopen()?
            // Sockets are raw file descriptors (integers). fdopen converts it to a standard C file stream (FILE *).
            // This lets you use easy functions like fgets() and fprintf() instead of raw read() and write().
            FILE *fp = fdopen(ns, "r+");
            char id[100], msg[2000], name[100];

            // Read ID and Message line by line
            if (fgets(id, sizeof(id), fp) && fgets(msg, sizeof(msg), fp)) {
                
                // ⭐ What does strcspn do here?
                // fgets() captures the '\n' (Enter key) at the end of the input. 
                // strcspn finds that '\n' and we replace it with '0' (null terminator) to clean the string.
                id[strcspn(id, "\n")] = 0; 
                msg[strcspn(msg, "\n")] = 0;

                // Determine name based on ID
                if (strcmp(id, ID_CAT) == 0) strcpy(name, "Catarina");
                else if (strcmp(id, ID_MARIA) == 0) strcpy(name, "Maria");
                else strcpy(name, "Unknown Student");

                // convert message to uppercase
                for (int i = 0; msg[i]; i++) msg[i] = toupper(msg[i]);

                //Send Response back to the client
                fprintf(fp, "%s\n%s\n", msg, name);
            }

            fclose(fp); // This also automatically closes the underlying socket (ns)
            exit(0); // Child is done
        }
        close(ns); // Parent closes the connected socket and waits for the next client
    }
    return 0;
}

int my_create_server_socket(char *port) {
    int s;
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // Server mode (listening)

    getaddrinfo(NULL, port, &hints, &res);
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
    int opt = 1;
    // Allows immediate reuse of the port if the server crashes/restarts
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    bind(s, res->ai_addr, res->ai_addrlen);
    listen(s, 5); // Queue up to 5 connections
    freeaddrinfo(res);
    return s;
}
