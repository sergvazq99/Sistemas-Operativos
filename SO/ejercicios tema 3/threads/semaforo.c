#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h> // Para el exit

#define NUM_HILOS 5
#define NUM_RECURSOS 2 // Recursos disponibles

sem_t semaforo; // Declaración del semáforo

void *hilo(void *arg) {
    int id = *(int *)arg;
    printf("Hilo %d: Intentando adquirir semáforo...\n", id);
    sem_wait(&semaforo); // Esperar (P)
    printf("Hilo %d: Semáforo adquirido. Usando recurso...\n", id);
    sleep(1); // Simular uso del recurso
    printf("Hilo %d: Liberando semáforo.\n", id);
    sem_post(&semaforo); // Señalar (V)
    pthread_exit(NULL);
}

int main() {
    pthread_t hilos[NUM_HILOS];
    int ids[NUM_HILOS];
    int i;

    // Inicializar el semáforo con 2 unidades
    if (sem_init(&semaforo, 0, NUM_RECURSOS) != 0) {
        perror("sem_init failed");
        exit(1);
    }

    // Crear los hilos
    for (i = 0; i < NUM_HILOS; i++) {
        ids[i] = i;
        if (pthread_create(&hilos[i], NULL, hilo, &ids[i]) != 0) {
            perror("pthread_create failed");
            exit(1);
        }
    }

    // Esperar a que todos los hilos terminen
    for (i = 0; i < NUM_HILOS; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Destruir el semáforo
    sem_destroy(&semaforo);

    return 0;
}