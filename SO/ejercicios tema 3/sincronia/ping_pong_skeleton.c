#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// =========================
// TODO PASO 1: Estado global
// =========================
int turno = 0; // 0 = A, 1 = B
pthread_mutex_t mutex;
pthread_cond_t cond;

void* JugadorA(void* arg) {
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&mutex);

        while(turno!=0){
            pthread_cond_wait(&cond,&mutex);
        }

        printf("Ping\n");
        turno=1;

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* JugadorB(void* arg) {
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&mutex);

        while(turno!=1){
            pthread_cond_wait(&cond,&mutex);
        }

        printf("Pong\n");
        turno=0;

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t ha, hb;

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);

    pthread_create(&ha,NULL,JugadorA,NULL);
    pthread_create(&hb,NULL,JugadorB,NULL);
    
    pthread_join(ha,NULL);
    pthread_join(hb,NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}