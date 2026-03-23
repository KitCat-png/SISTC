#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void myHandler(int signum) {
    printf("signal %d received\n", signum);
}

int main() {

		printf("1170701 - %s\n", __FILE__);
		sigset_t mask_initial, mask_child;

    //bloquear SIGINT e SIGQUIT inicialmente
    sigemptyset(&mask_initial);
    sigaddset(&mask_initial, SIGINT);
    sigaddset(&mask_initial, SIGQUIT);
    sigprocmask(SIG_BLOCK, &mask_initial, NULL);

    //ignorar SIGTERM em todos os processos
    signal(SIGTERM, SIG_IGN);

    //configurar myHandler para SIGCHLD antes da criação de processos
    signal(SIGCHLD, myHandler);

    while(1){
        
        pid_t r = fork();

        if (r == 0) { // Processo FILHO
            
				//bloquear SIGTSTP adicionalmente no filho
            sigemptyset(&mask_child);
            sigaddset(&mask_child, SIGTSTP);
            sigprocmask(SIG_BLOCK, &mask_child, NULL);

            //reset SIGCHLD
            signal(SIGCHLD, SIG_DFL);

            exit(0); 
        }
        
        // Processo PAI continua o loop
    }

    return 0;
}
