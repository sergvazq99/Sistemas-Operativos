#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int fd=open("saludo.txt",O_WRONLY);
    if(fd==-1){
        printf("Error al abrir el archivo");
    }

    off_t pos=lseek(fd,5,SEEK_SET);
    write(fd,"Clase",5);
    close(fd);
    return 0;
}