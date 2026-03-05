#include <stdio.h>
#include <pthread.h>

/*
🔔 signal despierta a un solo hilo
📢 broadcast despierta a todos los hilos que están esperando
*/

int cont=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //inicializar mutez

void*incrementar(void*arg){
    for(int i=0;i<1000000;i++){
        pthread_mutex_lock(&mutex); //bloqueo
        cont++;
        pthread_mutex_unlock(&mutex); //desbloqueo
    }
    return NULL;
}

int main(){
    pthread_t hilo1, hilo2;

    pthread_create(&hilo1, NULL, incrementar, NULL);
    pthread_create(&hilo2, NULL, incrementar, NULL);

    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);

    printf("Contador final: %d\n", cont);

    pthread_mutex_destroy(&mutex); //destruir mutez

    return 0;
}