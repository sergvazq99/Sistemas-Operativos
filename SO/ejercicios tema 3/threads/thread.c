#include <pthread.h>
#include <stdio.h>

//compilación: gcc -pthread thread.c -o thread

/*pthread_create(a1,a2,a3,a4)
    a1: id del hilo
    a2: NULL
    a3: invoca una función anónima --> void *funcion(void *argumentos...)
            casting: ej: void *funcion(int*arg) --> int id=*(int*) arg
    a4: argumentos --> ej: int,string,char,......

    vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    >pthread_create(__,NULL,funcion,&a);<
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

pthread_join(a1,a2)
    a1: id del hilo
    a2: elementos que devuelve ese hilo
*/

void* funcion1(void*arg){
    printf("Hola dsede el hilo\n");
    return NULL;
}

int main(){
    pthread_t thread;
    pthread_create(&thread,NULL,funcion1,NULL); //Todos los campos son punteros
    pthread_join(thread,NULL);
    printf("Hilo principal\n");

    return 0;
}