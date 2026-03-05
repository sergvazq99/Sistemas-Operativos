#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int fd=open("antiguo.txt",O_CREAT|O_RDONLY|O_TRUNC,0644);
    close(fd);
    int cambio=rename("antiguo.txt","nuevo.txt");
    if(cambio==0){
        printf("Archivo cambiado con exito\n");
    }
    int borrado=unlink("nuevo.txt");
    if(borrado==0){
        printf("Archivo borrado con exito\n");
    }
    
    return 0;
}