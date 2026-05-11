#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct {
    char student_id[7]; // Exactly 7 bytes, no null terminator
    char text[2000]; 
} msg1_t;

typedef struct {
    char text[2000];
    char student_name[100];
} msg2_t;

// ⭐ Why do we need this custom myReadBlock function?
// TCP is a "stream" protocol. It guarantees bytes will arrive, but it doesn't guarantee they will all arrive in one single read() call. 
// This loop ensures we don't stop reading until we have downloaded the exact size of our struct.
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

    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    msg1_t m1;
    msg2_t m2;

    // ⭐ TEACHER QUESTION: Why memset the struct to 0?
    // C structs often have hidden "padding" bytes for memory alignment, which contain random garbage data.
    // Clearing the whole struct to 0 prevents us from accidentally sending that garbage memory over the network.
    memset(&m1, 0, sizeof(m1)); 

    // Use isep number, we change the number here
    // ⭐ Why memcpy instead of strcpy?
    // Because student_id has NO null terminator ('\0'). strcpy relies on null terminators to know when to stop, so it would overflow here!
    memcpy(m1.student_id, "1170701", 7); 

    printf("Enter text: ");
    fgets(m1.text, sizeof(m1.text), stdin);
    m1.text[strcspn(m1.text, "\n")] = 0; 

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(argv[1], argv[2], &hints, &res);

    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(s, res->ai_addr, res->ai_addrlen);

    // We write the raw memory block of the struct directly into the socket
    write(s, &m1, sizeof(msg1_t)); // Send the block

    // Wait until we have safely received the exact byte size of msg2_t
    if (myReadBlock(s, &m2, sizeof(msg2_t)) == sizeof(msg2_t)) {
        printf("Uppercase: %s\n", m2.text);
        printf("Student: %s\n", m2.student_name);
    }

    close(s);
    freeaddrinfo(res);
    return 0;
}
