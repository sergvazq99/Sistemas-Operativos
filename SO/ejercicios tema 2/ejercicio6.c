#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int fd=open("salida_redireccionada.txt",O_CREAT|O_WRONLY|O_TRUNC,0644);
    dup2(fd,STDOUT_FILENO);
    close(fd);
    printf("Este mensaje irá al archivo y no a la pantalla\n");
    return 0;
}