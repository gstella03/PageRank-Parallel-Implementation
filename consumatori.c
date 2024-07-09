#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include "consumatori.h"

// funzione eseguita dai thread consumatori
void *cbody(void *arg)
{
  dati_consumatori *a = (dati_consumatori *)arg; 

  arco arc;
  do {
    sem_wait(a->sem_data_items);
    pthread_mutex_lock(a->pmutex_buf);
    arc = a->buffer[*(a->cindex) % Buf_size];
    *(a->cindex) +=1;
    pthread_mutex_unlock(a->pmutex_buf);
    sem_post(a->sem_free_slots);
    if (arc.i == -1 && arc.j == -1)
        break;
    if (arc.i == arc.j)
       continue;
    int i = arc.i;
    int j = arc.j;

    pthread_mutex_lock(a->pmutex_grafo);

    if (arco_gia_inserito(a->grafo,i,j)) {
        pthread_mutex_unlock(a->pmutex_grafo);
        continue;
    } else {
      aggiungi_arco(a->grafo,i,j);
    }
    pthread_mutex_unlock(a->pmutex_grafo);

  } while(true);

  return (void*)0 ; 
}

