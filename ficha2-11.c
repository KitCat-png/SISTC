#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void fun1(int *d) {
    ++d[0];
    sleep(2);
    printf("Novo valor gerado em fun1: %d\n", d[0]);
}

void fun2(int *d) {
    sleep(1);
    printf("Valor processado por fun2: %d\n", d[0]);
}

int main() {
    printf("1170701 - %s\n", __FILE__);
    int dados = 0;

    while(1) {
        fun1(&dados);

        pid_t pid = fork();

        if (pid == 0) {
            // --- Processo Filho ---
            fun2(&dados); // Executa fun2 com a cópia atual de 'dados'
            exit(0); 
		}
        printf("Pai: A avançar para a próxima iteração...\n");
    }
    return 0;
}
