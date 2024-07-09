#ifndef PAGERANK_H
#define PAGERANK_H
#include "grafo.h"

// struttura dati dei thread ausilliari per il calcolo del pagerank
typedef struct {
    grafo *g;
    double d;
    double *x;
    double *y;
    double *xnext;
    double teleport;
    double *sum_t;
    double *errore;
    sem_t *sem_tasks;
    sem_t *sem_done;
    pthread_mutex_t *mutex_task;
    pthread_mutex_t *mutex;
    int *task_id;
    int *operazione;
    int n_thread;
    int nodi;
    int resto;
} dati_thread_pagerank;

// prototipi
double *pagerank(grafo *g, double d, double eps, int maxiter, int taux, int *numiter);
void *tbody(void *arg);

#endif
