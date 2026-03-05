#include <pthread.h>
#include <stdio.h>

#define NUM_HILOS 5

void* funcion1(void*arg){
    
    //casteo arg -> *void
    // transformo en *int
    // (int*) arg
    // desreferenciar
    int id=*(int*)arg;
    printf("Hola dsede el hilo numero:%d",id);
    printf("\n");
    return NULL;
}

int main(){
    pthread_t thread[NUM_HILOS];
    int ids[NUM_HILOS];
    for(int i=0;i<NUM_HILOS;i++){
        ids[i]=i;
        pthread_create(&thread[i],NULL,funcion1,&ids[i]); //duda: &i
    }

    for(int i=0;i<NUM_HILOS;i++){
        pthread_join(thread[i],NULL);
    }
    
    
    printf("Hilo principal\n");

    return 0;
}