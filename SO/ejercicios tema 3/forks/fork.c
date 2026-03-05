#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    /*---------------- PROCESO PADRE E HIJO -----------------*/
    // se crean 2 procesos SIEMPRE
    pid_t pid = fork();

    // 0 = hijo
    if (pid == 0) {
        printf("Soy el proceso hijo\n");
    } 
    // >0 = padre
    else if (pid > 0) {
        printf("Soy el proceso padre\n");
    } 
    // <0 = error
    else {
        printf("Error al crear el proceso\n");
    }

    /*--------------- PROCESO PADRE,HIJO,NIETO ----------------*/

    pid_t pid1=fork(); // proceso nuevo

    // 0 = hijo
    if(pid1==0){
        printf("Soy el hijo\n");

        pid_t pid2=fork(); // hijo crea al nieto

        // el hijo del hijo = nieto
        if(pid2==0){
            printf("Soy el nieto\n");
        }
        else{ // sigue siendo el hijo, porque el padre del nieto es el hijo
            print("Sigo siendo el hijo\n");
        }
    }
    else{ // >0 = padre
        printf("Soy el padre\n");
    }

    return 0;
}