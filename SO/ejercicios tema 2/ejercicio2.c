#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int fd=open("saludo.txt",O_RDONLY);
    if(fd==-1){
        printf("Error al abrir el archivo");
    }
    char buffer[100];
    ssize_t leidos=read(fd,buffer,10);
    buffer[leidos]='\0';
    printf("%s",buffer);
    close(fd);
    return 0;
}