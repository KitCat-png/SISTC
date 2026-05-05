#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#define ID_MARIA "1231098"
#define ID_CAT "1170701"

int main(int argc, char *argv[]) {
    printf("1170701 - %s\n", __FILE__);
    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    char user_msg[2000];
    printf("Enter message: ");
    if (fgets(user_msg, sizeof(user_msg), stdin) == NULL) exit(1);
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(argv[1], argv[2], &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }

    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        exit(1);
    }

    // Send the sequence: ID\n + Message
    // change the ID here
    dprintf(s, "%s\n%s", ID_CAT, user_msg);

    FILE *fp = fdopen(s, "r+");
    if (!fp) exit(1);

    char linha1[2048], linha2[2048];
   
    // Read the two-line response from the server
    if (!fgets(linha1, sizeof(linha1), fp)) strcpy(linha1, "Error");
    if (!fgets(linha2, sizeof(linha2), fp)) strcpy(linha2, "Error");

    int nbytes = strlen(linha1) + strlen(linha2);

    // Remove \n for the final print
    linha1[strcspn(linha1, "\n")] = 0;
    linha2[strcspn(linha2, "\n")] = 0;

    // Required print format
    printf("Mensagem: %s\n" "Nome: %s\n""Total: %d\n", linha1, linha2, nbytes);

    fclose(fp);
    freeaddrinfo(res);
    return 0;
}
