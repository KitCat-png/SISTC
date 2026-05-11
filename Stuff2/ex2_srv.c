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

    // ⭐ Why ignore these signals?
    // SIGPIPE: Prevents the server from crashing if it tries to write to a client that disconnected.
    // SIGCHLD: Automatically cleans up ("reaps") finished child processes so they don't become "zombies".
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
        
    // Sets up the listening socket
    socket_descriptor = my_create_server_socket(argv[1]);

    while(1) {
        addrlen = sizeof(clt_addr);
        // accept() blocks until a client connects. It returns a BRAND NEW socket dedicated to that client.
        new_socket_descriptor = accept(socket_descriptor, &clt_addr, &addrlen);
        
        if(new_socket_descriptor < 0) {
            perror("accept");
            continue;
        }

        // ⭐ Why fork?
        // fork() creates a child process to handle this specific client. 
        // This makes it a "Concurrent Server" that can handle multiple clients at once.
        if (fork() == 0) { 
            // -- WE ARE IN THE CHILD PROCESS NOW --
            
            // The child doesn't need the main listening socket, so we close it to save resources.
            close(socket_descriptor); 
            print_address(&clt_addr, addrlen);

            char filename[] = "fileXXXXXX";
            // mkstemp safely creates a unique temporary file. Great for avoiding collisions if multiple clients upload at once.
            int fd = mkstemp(filename);
            if (fd < 0) { perror("mkstemp"); exit(1); }

            char buffer[1024];
            int n;
            
            while(1) {
                sleep(10); // Simulates a slow server or network delay for testing/observation.
                n = read(new_socket_descriptor, buffer, sizeof(buffer));
                if (n <= 0) break; // Client disconnected or error
                write(fd, buffer, n); // Write received bytes to our temp file
            }

            close(fd);
            close(new_socket_descriptor);
            exit(0); // Child process dies here when client is done
        }
        
        // -- WE ARE IN THE PARENT PROCESS HERE --
        // The parent doesn't talk to the client, so it closes the client socket and loops back to accept() the next one.
        close(new_socket_descriptor);
    }
    return 0;
}

int my_create_server_socket(char *port) {
    int s, r;
    struct addrinfo hints, *a;
    
    // Setting up criteria for the network address we want
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_flags = AI_PASSIVE;     // Tells the OS we intend to bind and listen
    hints.ai_socktype = SOCK_STREAM; // TCP connection
    
    r = getaddrinfo(NULL, port, &hints, &a);
    if (r != 0) { exit(1); }
    
    s = socket(PF_INET, SOCK_STREAM, 0); // Create the actual socket
    
    int so_reuseaddr = 1;
    // Why SO_REUSEADDR?
    // If the server crashes, the OS keeps the port locked for a minute. 
    // This option lets you restart the server immediately without getting an "Address already in use" error.
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
    
    bind(s, a->ai_addr, a->ai_addrlen); // Assigns the port to the socket
    listen(s, 5); // Marks socket as passive and sets the queue limit (5 pending connections max)
    
    freeaddrinfo(a); // Clean up memory
    return s;
}

void print_address(const struct sockaddr * clt_addr, socklen_t addrlen) {
    char hostname[256], port[6];
    // Extracts the human-readable IP address and port from the raw socket address struct
    getnameinfo(clt_addr, addrlen, hostname, sizeof(hostname), port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);
    printf("Connection from %s:%s\n", hostname, port);
}
