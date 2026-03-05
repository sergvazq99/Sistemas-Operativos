#include <pthread.h>
#include <stdio.h>

#define NUM_HILOS 5
int cont=0; //variable global

void* funcion1(void*arg){

    for(int i=0;i<1000;i++){
        cont++;
    }

   pthread_exit(NULL);
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
    
    
    printf("Valor de contador:%d\n",cont);

    return 0;
}