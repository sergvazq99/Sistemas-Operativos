#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define K 10
int buffer[K];
int count = 0;
int in = 0, out = 0;

pthread_mutex_t mutex;
pthread_cond_t no_lleno;
pthread_cond_t no_vacio;

void* Productor(void* arg) {
    int item=rand()%100;
    
    pthread_mutex_lock(&mutex);

    while(count==K){
        pthread_cond_wait(&no_lleno,&mutex);
    }

    buffer[in]=item;
    in=(in+1)%K;
    count++;

    printf("[Productor] produce %d | count=%d | in=%d out=%d\n", item, count, in, out);

    pthread_cond_signal(&no_vacio);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* Consumidor(void* arg) {
    pthread_mutex_lock(&mutex);

    while(count==0){
        pthread_cond_wait(&no_vacio,&mutex);
    }

    int item=buffer[out];
    out=(out+1)%K;
    count--;

    printf("[Consumidor] extrae %d | count=%d | in=%d out=%d\n", item, count, in, out);
    
    pthread_cond_signal(&no_lleno);
    pthread_mutex_unlock(&mutex);

    printf("Procesando el item: %d\n",item);
    return NULL;
}

int main() {
    pthread_t productor,consumidor;

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&no_lleno,NULL);
    pthread_cond_init(&no_vacio,NULL);

    pthread_create(&productor,NULL,Productor,NULL);
    pthread_create(&consumidor,NULL,Consumidor,NULL);

    pthread_join(productor,NULL);
    pthread_join(consumidor,NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_lleno);
    pthread_cond_destroy(&no_vacio);
    return 0;
}