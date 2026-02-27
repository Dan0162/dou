#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/mman.h>
#include <inttypes.h>

#define TAMANIOPAGINA 4096
#define ITERACIONES 5

int num = 25;
volatile int turno = 0;

typedef struct {
    int id;
    uint8_t *num_compartido;
} DatosHilo;

void *trabajo_hilo(void *arg)
{
    DatosHilo *datos = (DatosHilo *)arg;

    for (int i = 0; i < ITERACIONES; i++) {
        while (turno != datos->id);

        *datos->num_compartido = (uint8_t)(20 + (datos->id * 20) + i);
        num = 25 + (datos->id * 25) + i;

        printf("Hilo %d en sección crítica (iteración %d)\n", datos->id, i);

        turno = 1 - datos->id;
    }

    return NULL;
}

int main   (int argc, char ** argv)
{
    uint8_t *num_compartido = mmap(NULL, TAMANIOPAGINA,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (num_compartido == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    *num_compartido = 22;

    pthread_t hilo0, hilo1;
    DatosHilo datos0 = {0, num_compartido};
    DatosHilo datos1 = {1, num_compartido};

    pthread_create(&hilo0, NULL, trabajo_hilo, &datos0);
    pthread_create(&hilo1, NULL, trabajo_hilo, &datos1);

    pthread_join(hilo0, NULL);
    pthread_join(hilo1, NULL);

    printf("Num en memoria no compartida:   %i    PID: %d\n", num, getpid());

    printf("Num en memoria compartida:      %" PRIu8   "    PID: %d\n", *num_compartido,getpid());

    munmap(num_compartido, TAMANIOPAGINA);

    return 0;
}
