#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int fd=open("hueco.bin",O_CREAT|O_RDONLY|O_WRONLY|0644);
    write(fd,"A",1);
    lseek(fd,1000,SEEK_CUR);
    write(fd,"Z",1);
    close(fd);
    return 0;
}