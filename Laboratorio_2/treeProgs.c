// Descrição: Programa que cria uma árvore de processos de acordo com a profundidade informada pelo usuário.
// Autor: Matheus Andrade
// Data: 2024-10-24

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int n;

    pid_t proccessPid = getpid();

    printf("Processo executando com pid %d.\nDeseja uma árvore de quantos níveis de profundidade?\n>", proccessPid);
    scanf("%d", &n);

    int i = 0;
    while (i < n - 1) {
        pid_t pid1 = 0;
        pid_t pid2 = 0;

        pid1 = fork();
        if(pid1) {
            pid2 = fork();
            pid1 = 0;
        }
        if(pid2) break;
        i++;
    }

    pid_t pidProc = getpid();

    if(pidProc == proccessPid) {
        sleep(1);
        printf("Exibindo a árvore de processos:\n");
        char pidStr[10];
        sprintf(pidStr, "%d", proccessPid);
        char *args[] = {"pstree", "-c", pidStr, NULL};
        execvp("pstree", args);
        perror("execvp failed");
    } else {
        sleep(2);
    }

    return 0;
}