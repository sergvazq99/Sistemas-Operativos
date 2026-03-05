#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// TODO PASO 1: Estado compartido
int lectores_activos = 0;
int escribiendo = 0;
int escritores_esperando = 0;

pthread_mutex_t mutex;
pthread_cond_t c_lect;
pthread_cond_t c_escr;

void* Lector(void* arg) {
    int id=*(int*)arg;

    pthread_mutex_lock(&mutex);
    while(escribiendo||escritores_esperando>0){
        pthread_cond_wait(&c_lect,&mutex);
    }

    lectores_activos++;
    printf("[L%d] entra a leer (lectores_activos=%d)\n", id, lectores_activos);
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&mutex);
    lectores_activos--;
    printf("[L%d] sale de leer (lectores_activos=%d)\n", id, lectores_activos);

    if(lectores_activos==0){
        pthread_cond_signal(&c_escr);
    }
    
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* Escritor(void* arg) {
    int id = *(int*)arg;

    pthread_mutex_lock(&mutex);
    escritores_esperando++;

    while(escribiendo||lectores_activos>0){
        pthread_cond_wait(&c_escr,&mutex);
    }
    escritores_esperando--;
    escribiendo=1;
    printf("[E%d] entra a escribir\n", id);
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&mutex);
    escribiendo = 0;
    printf("[E%d] sale de escribir\n", id);

    pthread_cond_broadcast(&c_lect);
    pthread_cond_signal(&c_escr);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main() {
    const int NL = 4;
    const int NE = 2;

    pthread_t hilos_lectores[NL];
    pthread_t hilos_escritores[NE];

    int idL[NL];
    int idE[NE];

    for(int i=0;i<NL;i++){
        pthread_create(&hilos_lectores[i],NULL,Lector,&idL[i]);
    }

    for(int i=0;i<NE;i++){
        pthread_create(&hilos_escritores[i],NULL,Escritor,&idE[i]);
    }

    for(int i=0;i<NL;i++){
        pthread_join(hilos_lectores[i],NULL);
    }

    for(int i=0;i<NE;i++){
        pthread_join(hilos_escritores[i],NULL);
    }


    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&c_lect,NULL);
    pthread_cond_init(&c_escr,NULL);
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&c_lect);
    pthread_cond_destroy(&c_escr);
    return 0;
}