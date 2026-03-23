#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Handler para evitar zombies e reportar terminações anormais
void sigchld_handler(int signum) { //this function runs when a child finishes, it cleans it up so it doesnt stay in the system as a zombie
    int status;
    pid_t pid;

    // waitpid com WNOHANG para não bloquear o pai caso o filho ainda esteja vivo
    // O loop while garante que limpamos todos os filhos que possam ter terminado quase ao mesmo tempo
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) { //this condition checks if any child is finished but doesnt wait around if theyre still running
        
        // Verifica se o filho terminou devido a um sinal (terminação anormal)
        if (WIFSIGNALED(status)) { //checks if a child died
            int sig_num = WTERMSIG(status);
            printf("\n[ALERTA] Processo filho %d terminou anormalmente!\n", pid);
            printf("Sinal: %d | Descrição: %s\n", sig_num, strsignal(sig_num)); //converts a signal number into a human-readable string,, stuff like "segmentation fault"
        }
    }
}

void fun1(int *d) { //sleep is usually interrupted by signals, this function makes it so its not interrupted by checking n
    ++d[0];
    int n = 3;
    do {
        n = sleep(n);
        // Se n > 0, o sleep foi interrompido por um sinal (como o SIGCHLD)
        if (n > 0) {
            printf("sleep interrompido. Segundos ainda a aguardar: %d\n", n);
        }
    } while (n > 0);
    printf("Novo valor gerado em fun1: %d\n", d[0]);
}

void fun2(int *d) {
    sleep(1);
    printf("Valor processado por fun2: %d\n", d[0]);
}

int main() {
    int dados = 0;

    printf("1170701 - %s\n", __FILE__);

    // Configura o atendimento do sinal SIGCHLD
    struct sigaction sa; 
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Reinicia chamadas de sistema se possível

    if (sigaction(SIGCHLD, &sa, NULL) == -1) { //registers sigchld_handler to watch for finishing children
        perror("sigaction");
        exit(1);
    }

    while(1) {
        fun1(&dados);
        
        pid_t r = fork();
        
        if (r == -1) {
            perror("fork");
            exit(1);
        }
        
        if (r == 0) { // Processo Filho
            fun2(&dados);
            exit(0);
        }
        
        //pai volta para o início do loop (fun1)
    }
    
    return 0;
}
