#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "sock_aux.h"

int main(int argc, char *argv[]) {
    printf("1170701 - %s\n", __FILE__);
    if (argc != 4) {
        printf("Usage: %s server_ip port filename\n", argv[0]);
        exit(1);
    }

    // Opens the local file in Read-Only mode
    int fd = open(argv[3], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // Custom helper function (likely wraps socket() and connect()) to establish the TCP link to the server
    int s = my_connect(argv[1], argv[2]);
    if (s == -1) {
        close(fd);
        exit(1);
    }

    char buffer[4096];
    int n;
    
    // Read up to 4096 bytes from the local file at a time
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        int total = 0;
        
        // Why this inner while loop?
        // Network sockets don't guarantee that a single write() will send ALL the bytes you asked it to send.
        // This loop forces the program to keep calling write() on the remaining data until the entire chunk ('n' bytes) has been successfully pushed to the socket.
        while (total < n) {
            // Write starting from the offset of what we've already sent
            int w = write(s, buffer + total, n - total);
            if (w <= 0) { // Error handling if the connection drops mid-transfer
                perror("write socket");
                close(fd);
                close(s);
                exit(1);
            }
            total += w; // Accumulate how many bytes were successfully sent
        }
    }

    printf("Ficheiro '%s' enviado com sucesso.\n", argv[3]);
    close(fd);
    close(s);
    return 0;
}
