#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include "gestore.h"

// funzione thread gestore di segnali
void *gestore_segnali(void *arg) {
    dati_gestore_segnale *d = (dati_gestore_segnale *)arg;
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask,SIGINT);
    int s;

    while (true) {
        int e=sigwait(&mask, &s);
        if(e!=0) perror("Errore sigwait");
        if (s == SIGUSR1) {
            pthread_mutex_lock(d->mutex);
            int iter = *(d->numiter);
            int max_idx = 0;
            double max_pr = d->x[0];
            for (int i = 1; i < d->N; i++) {
                if (d->x[i] > max_pr) {
                    max_idx = i;
                    max_pr = d->x[i];
                }
            }
            fprintf(stderr, "Iterazione corrente: %d, Nodo con maggiore PageRank: %d, Valore PageRank: %f\n", iter, max_idx, max_pr);
            pthread_mutex_unlock(d->mutex);
        } 
    }

    return NULL;
}
