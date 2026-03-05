#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/*Cuando un exec() funciona, reemplaza completamente el proceso actual y nunca vuelve.
Solo se ejecuta un exec() por proceso.*/

int main() {
    pid_t pid = fork();

    if (pid == 0) { // Hijo
        printf("Soy el hijo antes de exec\n");

        // --- 1. execl ---
        execl("/bin/ls", "ls", "-l", NULL); // exec(lista)

        // --- 2. execlp ---
        execlp("ls", "ls", "-l", NULL); // exec(lista+path)

        // --- 3. execv ---
        char *args3[] = {"ls", "-l", NULL}; // exec(vector)
        execv("/bin/ls", args3);

        // --- 4. execvp ---
        char *args4[] = {"ls", "-l", NULL}; // exec(vector+path)
        execvp("ls", args4);

        // --- 5. execve ---
        char *args5[] = {"ls", "-l", NULL}; // exec(vector+entorno)
        char *envp[] = {NULL};
        execve("/bin/ls", args5, envp);

        printf("Si exec funciona, esto NO se imprime\n");
    } else { // Padre
        wait(NULL);
        printf("Soy el padre, el hijo terminó\n");
    }

    return 0;
}