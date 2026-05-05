#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct {
    char student_id[7];
    char text[2000];
} msg1_t;

typedef struct {
    char text[2000];
    char student_name[100];
} msg2_t;

#define ID_CAT "1170701"

int main(int argc, char *argv[]) {
    printf("1170701 - %s\n", __FILE__);

    if (argc != 3) {
        printf("Formato correto: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    msg1_t m1;
    msg2_t m2;
    memset(&m1, 0, sizeof(m1));
    memset(&m2, 0, sizeof(m2));

    memcpy(m1.student_id, ID_CAT, 7);

    // Ler
    if (fgets(m1.text, sizeof(m1.text), stdin) == NULL) exit(1);
    int text_len = strlen(m1.text);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(argv[1], argv[2], &hints, &res) != 0) {
        perror("getaddrinfo"); exit(1);
    }
    int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect"); exit(1);
    }
    freeaddrinfo(res);

    FILE *fp = fdopen(s, "r+");
    if (!fp) { perror("fdopen"); exit(1); }

    // --- ENVIAR ---
    // ID
    fwrite(m1.student_id, 1, 7, fp);

    // Text
    fprintf(fp, "%d\n", text_len);

    // Text tamanho
    fwrite(m1.text, 1, text_len, fp);
    fflush(fp);

    // --- RECEBER -------------------------------------------------------------------
    char nbytes_str[32];
    int nbytes = 0;

    // Text Maiusculas
    if (fgets(nbytes_str, sizeof(nbytes_str), fp) == NULL) { fclose(fp); exit(1); }
    int nb_text = atoi(nbytes_str);
    int nr = fread(m2.text, 1, nb_text, fp);
    m2.text[nr] = '\0';
    nbytes += strlen(nbytes_str) + nr;

    // Nome
    if (fgets(nbytes_str, sizeof(nbytes_str), fp) == NULL) { fclose(fp); exit(1); }
    int nb_name = atoi(nbytes_str);
    nr = fread(m2.student_name, 1, nb_name, fp);
    m2.student_name[nr] = '\0';
    nbytes += strlen(nbytes_str) + nr;

    printf("Mensagem: %s\n""Nome: %s\n""Total: %d\n", m2.text, m2.student_name, nbytes);

    fclose(fp);
    return 0;
}
