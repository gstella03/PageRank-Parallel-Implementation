#include "grafo.h"
#include <pthread.h>
#include <semaphore.h>
#define Buf_size 150

// struttura arco
typedef struct {
    int i;
    int j;
} arco;

// struct contenente i parametri di input dei thread consumatori
typedef struct {
  arco *buffer; 
  int *cindex;
  grafo *grafo; 
  pthread_mutex_t *pmutex_buf;
  pthread_mutex_t *pmutex_grafo;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;
} dati_consumatori;


// prototipo
void *cbody(void *arg);
