#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define CAPACITY 3
#define MAX_URL 256

typedef enum {
    CLASS_PRIORITY = 1,
    CLASS_ECONOMY  = 2
} req_class_t;

typedef struct {
    int id;
    req_class_t cls;
    char url[MAX_URL];
} request_t;

pthread_mutex_t mutex;

pthread_cond_t cond_PR;
pthread_cond_t cond_EC;

int espera_pr=0; 
int espera_ec=0; 

int contador_segmentos=0; 

int siguiente_pr=0; 
int siguiente_ec=0; 

int turno_pr=0; 
int turno_ec=0; 

/* =========================
   Utilidades
   ========================= */
static const char *class_str(req_class_t c) {
    return (c == CLASS_PRIORITY) ? "PRIORITY" : "ECONOMY";
}

static char *ltrim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

static void rtrim_inplace(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n-1])) {
        s[n-1] = '\0';
        n--;
    }
}

/* Carga playlist desde .m3u8:
   - Ignora comentarios (#...) y líneas vacías
   - Espera: CLASE URL
*/
static int load_playlist(const char *path, request_t **out, size_t *out_n) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    size_t cap = 16;
    size_t n = 0;
    request_t *arr = malloc(cap * sizeof(request_t));
    if (!arr) {
        perror("malloc");
        fclose(f);
        return -1;
    }

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, f) != -1) {
        char *p = ltrim(line);
        rtrim_inplace(p);

        if (*p == '\0') continue;     // vacía
        if (*p == '#') continue;      // comentario

        char cls[32] = {0};
        char url[MAX_URL] = {0};

        if (sscanf(p, "%31s %255s", cls, url) != 2) {
            // línea inválida, se ignora
            continue;
        }

        req_class_t type;
        if (strcasecmp(cls, "PRIORITY") == 0) {
            type = CLASS_PRIORITY;
        } else if (strcasecmp(cls, "ECONOMY") == 0) {
            type = CLASS_ECONOMY;
        } else {
            // clase desconocida -> ignorar
            continue;
        }

        if (n == cap) {
            cap *= 2;
            request_t *tmp = realloc(arr, cap * sizeof(request_t));
            if (!tmp) {
                perror("realloc");
                free(arr);
                free(line);
                fclose(f);
                return -1;
            }
            arr = tmp;
        }

        arr[n].id = (int)n + 1;
        arr[n].cls = type;
        strncpy(arr[n].url, url, MAX_URL - 1);
        arr[n].url[MAX_URL - 1] = '\0';
        n++;
    }

    free(line);
    fclose(f);

    if (n == 0) {
        fprintf(stderr, "Error: playlist vacía o sin líneas válidas.\n");
        free(arr);
        return -1;
    }

    *out = arr;
    *out_n = n;
    return 0;
}


void enter_priority(void) {
    pthread_mutex_lock(&mutex);
    int ticket=siguiente_pr;
    siguiente_pr++;
    espera_pr++;

    while(turno_pr!=ticket||contador_segmentos==CAPACITY){
        pthread_cond_wait(&cond_PR,&mutex);
    }

    espera_pr--;
    contador_segmentos++;
    turno_pr++;
    pthread_mutex_unlock(&mutex);
}

void enter_economy(void) {
    pthread_mutex_lock(&mutex);
    int ticket=siguiente_ec;
    siguiente_ec++;
    espera_ec++;

    while(turno_ec!=ticket||contador_segmentos==CAPACITY||espera_pr>0){
        pthread_cond_wait(&cond_EC,&mutex);
    }

    espera_ec--;
    contador_segmentos++;
    turno_ec++;
    pthread_mutex_unlock(&mutex);
}

void leave_player(void) {
    pthread_mutex_lock(&mutex);
    contador_segmentos--;

    if(espera_pr>0){
        pthread_cond_broadcast(&cond_PR);
    }
    else{
        pthread_cond_broadcast(&cond_EC);
    }

    pthread_mutex_unlock(&mutex);
}


static void *worker(void *arg) {
    request_t *req = (request_t*)arg;

    if(req->cls==CLASS_PRIORITY){
        enter_priority();
    }
    else{
        enter_economy();
    }

    printf("[START] T%02d | %s | %s\n", req->id, class_str(req->cls), req->url);

    int t = (rand() % 3) + 1;
    sleep(t);

    printf("[DONE ] T%02d | %s | Archivo descargado y reproducido correctamente %s\n",req->id,class_str(req->cls),req->url);
    leave_player();
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <playlist.m3u8>\n", argv[0]);
        return 1;
    }

    srand((unsigned)time(NULL));

    request_t *playlist = NULL;
    size_t playlist_len = 0;

    if (load_playlist(argv[1], &playlist, &playlist_len) != 0) {
        return 1;
    }

    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond_PR,NULL);
    pthread_cond_init(&cond_EC,NULL);

    pthread_t *th = calloc(playlist_len, sizeof(pthread_t));
    if (!th) {
        perror("calloc");
        free(playlist);
        return 1;
    }

    for (size_t i = 0; i < playlist_len; i++) {
        pthread_create(&th[i],NULL,worker,&playlist[i]);
    }

    for (size_t i = 0; i < playlist_len; i++) {
        pthread_join(th[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_PR);
    pthread_cond_destroy(&cond_EC);

    free(th);
    free(playlist);

    printf("\n[OK] Playlist finalizada: Archivo descargado y reproducido correctamente\n");
    return 0;
}



/*EXPLICACIÓN

2.- Variables compartidas del sistema:

    pthread_mutex_t mutex: cerrojo general
    pthread_cond_t cond_PR: variable condicional para peticiones PRIORITY
    pthread_cond_t cond_EC: variable condicional para peticiones ECONOMY

    int espera_pr: número de esperas de PRIORITY
    int espera_ec: número de esperas de ECONOMY

    int contador_segmentos: control de segmentos totales

    int siguiente_pr: turno siguiente para peticiones PRIORITY
    int siguiente_ec: turno siguiente para peticiones ECONOMY

    int turno_pr: turno actual para peticiones PRIORITY
    int turno_ec: turno actual para peticiones ECONOMY

    Deben protegerse con mutex para que en ningún caso se acceda más de una vez a los recursos compartidos, sino
    podría ocurrir una condición de carrera, ya que sino varios hilos podrían aumentar o decrementar de forma incorrecta 
    las esperas, el número de segmentos o que el número de turno sea incorrectp.
    
3.- 


4.- Para ambas peticiones tenemos un sistema de tickets/turnos, el cual en cada llegada se asigna un turno por orden y se aumenta una espera. 
    Mientras en ambas solicitudes se supere la capacidad, o no sea su turno o en el caso de las peticiones ECONOMY (que no haya ninguna petición PRIORITY en espera antes)
    debe esperar. Luego se decrementa una espera y se aumenta el turno.
    
    
    
    






*/