#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int fd=open("numeros.dat",O_CREAT|O_RDONLY|O_TRUNC,0644);
    int entero=123456;
    write(fd,&entero,sizeof(int));

    close(fd);
    printf("número %d",entero);
    return 0;
}