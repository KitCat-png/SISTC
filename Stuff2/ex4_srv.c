#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//Structure definitions
typedef struct {
    char student_id[7];
    char text[2000];
} msg1_t;

typedef struct {
    char text[2000];
    char student_name[100];
} msg2_t;

// Helper to read the bytes
int myReadBlock(int s, void *buf, int count) {
    int r, nread = 0;
    while(nread < count) {
        r = read(s, (char*)buf + nread, count - nread);
        if(r <= 0) break;
        nread += r;
    }
    return nread;
}

int main(int argc, char *argv[]) {
    printf("1170701 - %s\n", __FILE__);

    //The server must accept the port as an argument
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Prevent zombie processe
    signal(SIGCHLD, SIG_IGN); 

    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    //avoid "Address already in use" errors
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(s, 5);
    printf("Server listening on port %s ...\n", argv[1]);

    while (1) {
        struct sockaddr clt_addr;
        socklen_t addrlen = sizeof(clt_addr);
        int ns = accept(s, &clt_addr, &addrlen);
        
        if (ns < 0) continue;

        //Create a new process for each client
        if (fork() == 0) { 
            close(s); // Child doesn't need the listener
            
            msg1_t m1;
            msg2_t m2;
            
            // Clear m2 to prevent garbage characters 
            memset(&m2, 0, sizeof(m2)); 

            //Read the incoming msg1_t block
            if (myReadBlock(ns, &m1, sizeof(msg1_t)) == sizeof(msg1_t)) {
                
                // Convert text to uppercase
                int i;
                for (i = 0; m1.text[i] != '\0' && i < 1999; i++) {
                    m2.text[i] = (char)toupper((unsigned char)m1.text[i]);
                }
                m2.text[i] = '\0'; // Ensure it's clean

                //Identify student by ID
                // We check the first 7 chars since there's no null terminator
                if (strncmp(m1.student_id, "1170701", 7) == 0) {
                    strcpy(m2.student_name, "Catarina");
                } 
                else if (strncmp(m1.student_id, "1231098", 7) == 0) {
                    strcpy(m2.student_name, "Maria");
                } 
                else {
                    strcpy(m2.student_name, "Unknown Student");
                }

                //Send back the msg2_t block
                write(ns, &m2, sizeof(msg2_t));
            }
            
            close(ns);
            exit(0);
        }
        close(ns); // Parent process closes its copy of the client socket
    }
    return 0;
}
