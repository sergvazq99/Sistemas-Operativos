#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
    int id;
    int nivel;
}Usuario;

int main(){
    Usuario u1={001,333};
    Usuario u2={002,444};
    int fd=open("usuarios.db",O_RDONLY|O_WRONLY|O_TRUNC,0644);
    write(fd,&u1,sizeof(Usuario));
    write(fd,&u2,sizeof(Usuario));
    close(fd);

    fd=open("usuarios.db",O_RDONLY);
    lseek(fd,sizeof(Usuario),SEEK_SET);
    Usuario u;
    read(fd,&u,sizeof(Usuario));
    printf("Id: %d,Nivel :%d",u.id,u.nivel);

    close(fd);
    return 0;
}