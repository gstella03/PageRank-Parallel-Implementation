#include <pthread.h>

// struttura dati del thread gestore segnali
typedef struct {
    double *x;
    int *numiter;
    int N;
    pthread_mutex_t *mutex;
} dati_gestore_segnale;

// prototipo
void *gestore_segnali(void *arg);
