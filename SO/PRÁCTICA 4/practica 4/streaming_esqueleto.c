/*
 * streaming_esqueleto.c
 *
 * Qué debe hacerse:
 * - Completar la sincronización para controlar el acceso concurrente al reproductor.
 * - Respetar la capacidad máxima, la prioridad entre clases y el orden de llegada dentro de cada clase.
 * - Mantener la estructura base del fichero sin rehacer el programa con otro diseño.
 *
 * Qué lee:
 * - Un fichero de entrada tipo .m3u8 simplificado, donde cada línea válida tiene:
 *     CLASE URL
 * - Ejemplos de clase esperada: PRIORITY o ECONOMY.
 *
 * Por qué lo lee así:
 * - Porque la práctica modela un manifiesto sencillo de trabajo: cada línea representa una solicitud
 *   independiente que después se convertirá en un hilo.
 * - Se ignoran líneas vacías y comentarios para permitir playlists legibles y fáciles de probar.
 *
 * Importante:
 * - Este archivo es un esqueleto básico.
 * - Los bloques marcados como TODO deben completarse.
 * - Los comentarios explican el propósito de cada parte, pero no resuelven la práctica.
 */

#define _GNU_SOURCE                 // Activa extensiones GNU necesarias para funciones como getline.
#include <stdio.h>                  // Entrada/salida estándar: printf, fprintf, perror, FILE, fopen.
#include <stdlib.h>                 // Utilidades generales: malloc, realloc, calloc, free, rand, srand.
#include <pthread.h>                // API de hilos POSIX: pthread_t, mutex, variables de condición.
#include <string.h>                 // Manipulación de cadenas: strlen, strncpy, strcasecmp.
#include <unistd.h>                 // Funciones POSIX como sleep.
#include <time.h>                   // Manejo de tiempo: time, usado para sembrar aleatoriedad.
#include <ctype.h>                  // Clasificación de caracteres: isspace.

#define CAPACITY 3                  // Capacidad máxima del reproductor: cuántas solicitudes pueden estar activas a la vez.
#define MAX_URL 256                 // Tamaño máximo reservado para almacenar una URL de segmento.

typedef enum {                      // Enumeración para representar la clase de cada solicitud.
    CLASS_PRIORITY = 1,            // Clase de alta prioridad.
    CLASS_ECONOMY  = 2             // Clase de baja prioridad.
} req_class_t;                      // Nombre del tipo enumerado.

typedef struct {                    // Estructura que representa una línea válida de la playlist.
    int id;                         // Identificador lógico de la solicitud, útil para trazas.
    req_class_t cls;                // Clase de servicio de la solicitud: PRIORITY o ECONOMY.
    char url[MAX_URL];              // URL o ruta asociada al segmento que se va a simular.
} request_t;                        // Nombre del tipo estructura.

/* =========================
   Sincronización global
   ========================= */

pthread_mutex_t mutex;

pthread_cond_t cond_PR;
pthread_cond_t cond_EC;

int espera_pr=0; // contador de espera para PRIORITY
int espera_ec=0; // contador de espera para ECONOMY

int contador_segmentos=0; // contador de capacidad

int siguiente_pr=0; // ticket para PRIORITY
int siguiente_ec=0; // ticket para ECONOMY

int turno_pr=0; // turno para PRIORITY
int turno_ec=0; // turo para ECONOMY


/* =========================
   Utilidades
   ========================= */

static const char *class_str(req_class_t c) {   // Convierte una clase interna a texto legible.
    return (c == CLASS_PRIORITY)                // Si la clase es PRIORITY...
           ? "PRIORITY"                         // ...devuelve la cadena "PRIORITY".
           : "ECONOMY";                        // En caso contrario devuelve "ECONOMY".
}

static char *ltrim(char *s) {                   // Elimina espacios iniciales de una cadena modificando el puntero de inicio.
    while (*s && isspace((unsigned char)*s))    // Mientras no sea fin de cadena y el carácter actual sea espacio...
        s++;                                    // ...avanza al siguiente carácter.
    return s;                                   // Devuelve el puntero al primer carácter no blanco.
}

static void rtrim_inplace(char *s) {            // Elimina espacios finales de una cadena modificándola en el sitio.
    size_t n = strlen(s);                       // Calcula la longitud actual de la cadena.
    while (n > 0 && isspace((unsigned char)s[n-1])) { // Mientras haya caracteres y el último sea espacio...
        s[n-1] = '\0';                         // ...lo sustituye por fin de cadena.
        n--;                                    // Reduce la longitud lógica y sigue comprobando.
    }
}

/*
 * Carga una playlist desde un fichero .m3u8 simplificado.
 *
 * Parámetros:
 * - path: ruta del fichero a abrir.
 * - out: dirección donde se devolverá el puntero al array dinámico de solicitudes.
 * - out_n: dirección donde se devolverá cuántas solicitudes válidas se han cargado.
 *
 * Devuelve:
 * - 0 si todo va bien.
 * - -1 si ocurre algún error o si no hay líneas válidas.
 */
static int load_playlist(const char *path, request_t **out, size_t *out_n) {
    FILE *f = fopen(path, "r");                // Abre el fichero en modo lectura.
    if (!f) {                                   // Si no pudo abrirse...
        perror("fopen");                       // ...muestra el motivo del error del sistema.
        return -1;                              // ...y devuelve error.
    }

    size_t cap = 16;                            // Capacidad inicial del array dinámico de solicitudes.
    size_t n = 0;                               // Número de solicitudes válidas cargadas hasta el momento.
    request_t *arr = malloc(cap * sizeof(request_t)); // Reserva memoria para la capacidad inicial.
    if (!arr) {                                 // Si la reserva falla...
        perror("malloc");                      // ...informa del error.
        fclose(f);                              // Cierra el fichero ya abierto.
        return -1;                              // Devuelve error.
    }

    char *line = NULL;                          // Puntero que getline reservará o redimensionará automáticamente.
    size_t len = 0;                             // Tamaño del buffer gestionado por getline.

    while (getline(&line, &len, f) != -1) {     // Lee el fichero línea a línea hasta EOF o error.
        char *p = ltrim(line);                  // Elimina espacios a la izquierda moviendo el puntero útil.
        rtrim_inplace(p);                       // Elimina espacios y salto de línea al final.

        if (*p == '\0') continue;              // Si la línea quedó vacía, se ignora.
        if (*p == '#') continue;                // Si la línea empieza por comentario, se ignora.

        char cls[32] = {0};                     // Buffer temporal para leer el texto de la clase.
        char url[MAX_URL] = {0};                // Buffer temporal para leer la URL.

        if (sscanf(p, "%31s %255s", cls, url) != 2) { // Intenta extraer exactamente dos campos: clase y URL.
            // Línea inválida: no tiene el formato mínimo esperado.
            continue;                           // Se ignora y se sigue con la siguiente línea.
        }

        req_class_t type;                       // Variable temporal donde se guardará la clase ya convertida.
        if (strcasecmp(cls, "PRIORITY") == 0) { // Compara sin distinguir mayúsculas/minúsculas con PRIORITY.
            type = CLASS_PRIORITY;              // Si coincide, asigna la clase interna PRIORITY.
        } else if (strcasecmp(cls, "ECONOMY") == 0) { // Si no, prueba con ECONOMY.
            type = CLASS_ECONOMY;               // Si coincide, asigna la clase interna ECONOMY.
        } else {
            // Clase desconocida: la línea no forma parte del formato aceptado.
            continue;                           // Se ignora sin abortar toda la carga.
        }

        if (n == cap) {                         // Si el array está lleno...
            cap *= 2;                           // ...duplica la capacidad para seguir creciendo.
            request_t *tmp = realloc(arr, cap * sizeof(request_t)); // Reajusta el bloque de memoria.
            if (!tmp) {                         // Si realloc falla...
                perror("realloc");             // ...informa del error.
                free(arr);                      // Libera el array anterior.
                free(line);                     // Libera el buffer usado por getline.
                fclose(f);                      // Cierra el fichero.
                return -1;                      // Devuelve error.
            }
            arr = tmp;                          // Si todo fue bien, actualiza el puntero al nuevo bloque.
        }

        arr[n].id = (int)n + 1;                 // Asigna un identificador consecutivo comenzando en 1.
        arr[n].cls = type;                      // Guarda la clase convertida.
        strncpy(arr[n].url, url, MAX_URL - 1); // Copia la URL al campo fijo de la estructura.
        arr[n].url[MAX_URL - 1] = '\0';        // Fuerza terminación nula por seguridad.
        n++;                                    // Incrementa el número de solicitudes cargadas.
    }

    free(line);                                 // Libera el buffer interno usado para leer líneas.
    fclose(f);                                  // Cierra el fichero porque ya no hace falta.

    if (n == 0) {                               // Si no se obtuvo ninguna línea válida...
        fprintf(stderr, "Error: playlist vacía o sin líneas válidas.\n"); // Informa del problema.
        free(arr);                              // Libera el array reservado.
        return -1;                              // Devuelve error.
    }

    *out = arr;                                 // Devuelve al llamador el puntero al array construido.
    *out_n = n;                                 // Devuelve al llamador cuántos elementos válidos hay.
    return 0;                                   // Indica éxito.
}

/*
 * Entrada al reproductor para solicitudes PRIORITY.
 *
 * Parámetros:
 * - No recibe parámetros porque trabaja contra estado global compartido.
 *
 */
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

/*
 * Entrada al reproductor para solicitudes ECONOMY.
 *
 * Parámetros:
 * - No recibe parámetros porque consulta y modifica estado compartido global.
 */
void enter_economy(void) {
    pthread_mutex_lock(&mutex);
    int ticket=siguiente_ec;
    siguiente_ec++;
    espera_ec++;

    while(turno_ec!=ticket||contador_segmentos==CAPACITY||espera_ec>0){
        pthread_cond_wait(&cond_EC,&mutex);
    }

    espera_ec--;
    contador_segmentos++;
    turno_ec++;
    pthread_mutex_unlock(&mutex);
}

/*
 * Salida del reproductor.
 *
 * Parámetros:
 * - No recibe parámetros porque actúa sobre el estado global de concurrencia.
 */
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

/*
 * Función principal que ejecuta cada hilo.
 *
 * Parámetro:
 * - arg: puntero genérico que realmente apunta a un request_t.
 *
 * Devuelve:
 * - NULL, porque el hilo no necesita devolver un valor útil al hacer join.
 */
static void *worker(void *arg) {
    request_t *req = (request_t*)arg;           // Convierte el puntero genérico al tipo real esperado por el hilo.

    if(req->cls==CLASS_PRIORITY){
        enter_priority();
    }
    else{
        enter_economy();
    }

    printf("[START] T%02d | %s | %s\n",      // Imprime una traza de comienzo de procesamiento.
           req->id,                             // Primer argumento de formato: id de la solicitud.
           class_str(req->cls),                 // Segundo argumento: clase convertida a texto.
           req->url);                           // Tercer argumento: URL de la solicitud.

    int t = (rand() % 3) + 1;                   // Genera un tiempo aleatorio entre 1 y 3 segundos.
    sleep(t);                                   // Simula la descarga/reproducción bloqueando este hilo ese tiempo.

    printf("[DONE ] T%02d | %s | Archivo descargado y reproducido correctamente %s\n",req->id,class_str(req->cls),req->url);
    leave_player();
    return NULL;                                // El hilo termina sin devolver información adicional.
}

/*
 * Punto de entrada del programa.
 *
 * Parámetros:
 * - argc: número de argumentos recibidos por línea de comandos.
 * - argv: vector de cadenas con los argumentos; argv[0] es el nombre del programa.
 *
 * Uso esperado:
 * - ./programa playlist.m3u8
 */
int main(int argc, char **argv) {
    if (argc != 2) {                            // Comprueba que se ha pasado exactamente un argumento además del nombre del programa.
        fprintf(stderr, "Uso: %s <playlist.m3u8>\n", argv[0]); // Muestra el formato correcto de ejecución.
        return 1;                               // Sale con código de error.
    }

    srand((unsigned)time(NULL));                // Inicializa la semilla del generador aleatorio con la hora actual.

    request_t *playlist = NULL;                 // Aquí se guardará el array dinámico de solicitudes leídas.
    size_t playlist_len = 0;                    // Aquí se guardará cuántas solicitudes válidas se leyeron.

    if (load_playlist(argv[1], &playlist, &playlist_len) != 0) { // Carga el fichero cuyo nombre viene en argv[1].
        return 1;                               // Si falla la carga, termina con error.
    }
    
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond_PR,NULL);
    pthread_cond_init(&cond_EC,NULL);

    pthread_t *th = calloc(playlist_len, sizeof(pthread_t)); // Reserva un vector para guardar los identificadores de los hilos.
    if (!th) {                                  // Si falla la reserva...
        perror("calloc");                      // ...informa del error.
        free(playlist);                         // Libera la memoria de la playlist ya cargada.
        return 1;                               // Sale con error.
    }

    for(int i=0;i<playlist_len;i++){
        pthread_create(&th[i],NULL,worker,&playlist[i]);
    }

    for(int i=0;i<playlist_len;i++){
        pthread_join(th[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_PR);
    pthread_cond_destroy(&cond_EC);

    free(th);                                   // Libera el vector de identificadores de hilo.
    free(playlist);                             // Libera la memoria dinámica de la playlist.

    printf("\n[OK] Playlist finalizada: Archivo descargado y reproducido correctamente\n"); // Mensaje final global.
    return 0;                                   // Termina correctamente.
}