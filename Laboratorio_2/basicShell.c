// Descrição: Implementação de um shell básico que executa comandos do sistema operacional.
// Autor: Matheus Andrade
// Data: 2024-10-24

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    char command[1024];
    pid_t pid;
    int status;

    while (1) {
        printf("basicShell> ");
        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0) {
            break;
        }

        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            continue;
        } else if (pid == 0) {
            char *args[] = {"/bin/sh", "-c", command, NULL};
            execvp(args[0], args);
            perror("Exec failed");
            _exit(1);
        } else {
            if (strchr(command, '&') == NULL) {
                waitpid(pid, &status, 0);
            }
        }
    }
}