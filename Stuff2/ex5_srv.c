#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
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

int my_create_server_socket(char *port);

int main(int argc, char *argv[]) {
    printf("1170701 - %s\n", __FILE__);

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    signal(SIGCHLD, SIG_IGN); // Evita zombies

    int s = my_create_server_socket(argv[1]);

    while (1) {
        struct sockaddr clt_addr;
        socklen_t addrlen = sizeof(clt_addr);
        int ns = accept(s, &clt_addr, &addrlen);
        if (ns < 0) continue;

        if (fork() == 0) { // filho trata cliente
            close(s);

            msg1_t m1;
            msg2_t m2;
            memset(&m1, 0, sizeof(m1));
            memset(&m2, 0, sizeof(m2));

            FILE *fp = fdopen(ns, "r+");
            if (!fp) { close(ns); exit(1); }

            // --- RECEBER ------------------------------------------------------------------
            // ID
            if ((int)fread(m1.student_id, 1, 7, fp) != 7) { fclose(fp); exit(1); }

            // Text tamanho
            char nbytes_str[32];
            if (fgets(nbytes_str, sizeof(nbytes_str), fp) == NULL) { fclose(fp); exit(1); }
            int nb_text = atoi(nbytes_str);
            if (nb_text > (int)sizeof(m1.text) - 1) nb_text = sizeof(m1.text) - 1;

            int nr = fread(m1.text, 1, nb_text, fp);
            m1.text[nr] = '\0';

            // --- PROCESSAR ---------------------------------------------------------------
            // Text pra maiusculas
            int i;
            for (i = 0; i < nr; i++)
                m2.text[i] = toupper((unsigned char)m1.text[i]);
            m2.text[i] = '\0';

           if (strncmp(m1.student_id, ID_CAT, 7) == 0)
                strcpy(m2.student_name, "Catarina");

            // --- ENVIAR -----------------------------------------------------------------
            // 1) Text tamanho, bytes
            fprintf(fp, "%d\n", nr);
            fwrite(m2.text, 1, nr, fp);

            // 2) Nome tamanho, bytes
            int name_len = strlen(m2.student_name);
            fprintf(fp, "%d\n", name_len);
            fwrite(m2.student_name, 1, name_len, fp);
            fflush(fp);

            fclose(fp);
            exit(0);
        }
        close(ns); // pai fecha cópia do socket do cliente
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
