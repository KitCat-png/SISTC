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

    // Abrir ficheiro a enviar
    int fd = open(argv[3], O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // Conectar ao servidor
    int s = my_connect(argv[1], argv[2]);
    if (s == -1) {
        close(fd);
        exit(1);
    }

    // Ler do ficheiro e enviar para o servidor
    char buffer[4096];
    int n;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        int total = 0;
        while (total < n) {
            int w = write(s, buffer + total, n - total);
            if (w <= 0) {
                perror("write socket");
                close(fd);
                close(s);
                exit(1);
            }
            total += w;
        }
    }

    printf("Ficheiro '%s' enviado com sucesso.\n", argv[3]);
    close(fd);
    close(s);
    return 0;
}
