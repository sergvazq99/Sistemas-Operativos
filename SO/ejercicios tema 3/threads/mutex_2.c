#include <pthread.h>
#include <stdio.h>

#define NUM_HILOS 5
int cont=0; //variable global, incrementada por todos los hilos
pthread_mutex_t cerrojo = PTHREAD_MUTEX_INITIALIZER; // mutex para proteger "cont" y "turno"
pthread_cond_t cond=PTHREAD_COND_INITIALIZER; // variable de condición para coordinar turnos
int turno=0; //variable global, indica qué hilo puede ejecutar en este momento

/*siempre que se tenga sistema de TURNOS ---> signal*/

void*funcion_hilo(void*arg){
    int identificador_hilo=*(int *)arg;
    pthread_mutex_lock(&cerrojo); // bloquea

    while(turno!=identificador_hilo){
        pthread_cond_wait(&cond, &cerrojo);
    }
    
    printf("Hola desde el hilo n: %d\n",identificador_hilo);

    for(int i=0;i<1000;i++){
        cont++;
    }
    turno++; // pasa el turno al siguiente hilo
    pthread_cond_broadcast(&cond); // despierta a todos los hilos que esperan
    pthread_mutex_unlock(&cerrojo); // desbloquea
    return NULL;
}


int main(){
    pthread_t hilo[NUM_HILOS];
    pthread_mutex_init(&cerrojo,NULL);
    pthread_cond_init(&cond,NULL);

    int ids[NUM_HILOS];

    for(int i=0;i<NUM_HILOS;i++){
        ids[i] = i;
        pthread_create(&hilo[i], NULL, funcion_hilo, &ids[i]);
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilo[i], NULL);
    }

    printf("Valor final de cont: %d\n", cont);

    return 0;
}

/* FUNCIONAMIENTO DEL EJERCICIO (broadcast)*/

/*H0 ejecuta
│
├─> broadcast despierta H1, H2, H3, H4
│     └─> solo H1 entra (turno==1), el resto vuelven a dormir
│
H1 ejecuta
│
├─> broadcast despierta H2, H3, H4
│     └─> solo H2 entra (turno==2)
│
H2 ejecuta
│
├─> broadcast despierta H3, H4
│     └─> solo H3 entra (turno==3)
│
H3 ejecuta
│
├─> broadcast despierta H4
│     └─> solo H4 entra (turno==4)
│
H4 ejecuta
└─> fin*/


/*FUNCIONAMIENTO DEL EJERCICIO (signal)*/

/*H0 ejecuta
│
└─> signal despierta H1 (solo uno)
    H1 ejecuta
    └─> signal despierta H2
        H2 ejecuta
        └─> signal despierta H3
            H3 ejecuta
            └─> signal despierta H4
                H4 ejecuta*/