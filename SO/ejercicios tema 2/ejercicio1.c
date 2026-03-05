#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int fd=open("saludo.txt",O_CREAT|O_WRONLY|O_TRUNC,0644);
    if(fd==-1){
        printf("Error al abrir el archivo");
        exit(-1);
    }
    char*msg="Hola mundo\n";
    write(fd,msg,strlen(msg));
    close(fd);
    return 0;
}