#include <stdio.h>
#include <pthread.h>

// =========================
// TODO PASO 1: Estado global
// =========================
int llegados = 0;
const int N = 5;
pthread_mutex_t mutex;
pthread_cond_t cond_barrera;

void* hilo_amigo(void* arg) {
    pthread_mutex_lock(&mutex);

    llegados++;

    if (llegados < N) {
        while(llegados<N){
            pthread_cond_wait(&cond_barrera,&mutex);
        }
    } else {
        pthread_cond_broadcast(&cond_barrera);
    }

    pthread_mutex_unlock(&mutex);

    printf("Hilo %lu: Pase la barrera.\n", pthread_self());
    return NULL;
}

int main() {
    pthread_t h[N];

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond_barrera,NULL);

    for(int i=0;i<N;i++){
        pthread_create(&h[i],NULL,hilo_amigo,NULL);
    }
    for(int i=0;i<N;i++){
        pthread_join(h[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_barrera);
    return 0;
}