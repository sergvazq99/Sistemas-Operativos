#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// TODO PASO 1: Estado del monitor
int cruzando = 0;
int direccion = 0; // 0 neutro, 1 N->S, 2 S->N
int esperaNorte = 0;
int esperaSur = 0;

#define NN 5
#define NS 5

pthread_mutex_t mutex;
pthread_cond_t cond_norte;
pthread_cond_t cond_sur;

void* CocheDesdeNorte(void* arg) {

    int coche=*(int*)arg;

    pthread_mutex_lock(&mutex);
    esperaNorte++;

    while(direccion==2||(direccion==1&&esperaSur>0)){
        pthread_cond_wait(&cond_norte,&mutex);
    }

    esperaNorte--;
    direccion=1;
    cruzando++;

    printf("Coche entrando desde norte a sur numero: %d, cruzando: %d,direccion: %d, esperas en Norte: %d, esperas en Sur: %d\n"
    ,coche,cruzando,direccion,esperaNorte,esperaSur);
    pthread_mutex_unlock(&mutex);
    
    pthread_mutex_lock(&mutex);
    cruzando--;

    printf("Coche saliendo desde norte a sur numero: %d, cruzando: %d,direccion: %d, esperas en Norte: %d, esperas en Sur: %d\n"
    ,coche,cruzando,direccion,esperaNorte,esperaSur);

    if(cruzando==0){
        direccion=0;
        if(esperaSur>0){
            pthread_cond_broadcast(&cond_sur);
        }
        else{
            pthread_cond_broadcast(&cond_norte);
        }
    }
    
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* CocheDesdeSur(void* arg) {

    int coche=*(int*)arg;

    pthread_mutex_lock(&mutex);
    esperaSur++;

    while(direccion==1||(direccion==2&&esperaNorte>0)){
        pthread_cond_wait(&cond_sur,&mutex);
    }

    esperaSur--;
    direccion=2;
    cruzando++;

    printf("Coche entrando desde sur a norte numero: %d, cruzando: %d,direccion: %d, esperas en Norte: %d, esperas en Sur: %d\n"
    ,coche,cruzando,direccion,esperaNorte,esperaSur);
    pthread_mutex_unlock(&mutex);
   
    pthread_mutex_lock(&mutex);
    cruzando--;

    printf("Coche saliendo desde sur a norte numero: %d, cruzando: %d,direccion: %d, esperas en Norte: %d, esperas en Sur: %d\n"
    ,coche,cruzando,direccion,esperaNorte,esperaSur);

    if(cruzando==0){
        direccion=0;
        if(esperaNorte>0){
            pthread_cond_broadcast(&cond_norte);
        }
        else{
            pthread_cond_broadcast(&cond_sur);
        }
    }

    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond_norte,NULL);
    pthread_cond_init(&cond_sur,NULL);

    
    pthread_t cochesNorte[NN];
    pthread_t cochesSur[NS];

    int idN[NN];
    int idS[NS];

    for(int i=0;i<NN;i++){
        pthread_create(&cochesNorte[i],NULL,CocheDesdeNorte,&idN[i]);
    }
    for(int i=0;i<NS;i++){
        pthread_create(&cochesSur[i],NULL,CocheDesdeSur,&idS[i]);
    }

    for(int i=0;i<NN;i++){
        pthread_join(cochesNorte[i],NULL);
    }
    for(int i=0;i<NS;i++){
        pthread_join(cochesSur[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_norte);
    pthread_cond_destroy(&cond_sur);
    
    return 0;
}