#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "pagerank.h"
#include "gestore.h"

// funzione eseguita dai thread ausilliari
void *tbody(void *arg) {
    dati_thread_pagerank *dc = (dati_thread_pagerank *)arg;
    grafo *g = dc->g;
    double d = dc->d;
    double *x = dc->x;
    double *y = dc->y;
    double *xnext = dc->xnext;
    double teleport = dc->teleport;

    while (true) {
        // attendo che ci sia un task da eseguire
        sem_wait(dc->sem_tasks);

        // leggo il prossimo task
        pthread_mutex_lock(dc->mutex_task);
        int task = (*dc->task_id);
        (*dc->task_id)++;
        pthread_mutex_unlock(dc->mutex_task);

        // calcolo nodi su cui lavorare
        int start_node = dc->nodi*task;
        int end_node = dc->nodi*(task+1);
        if(task == (dc->n_thread-1))
            end_node += dc->resto;

        int operazione = *(dc->operazione);

        // condizione di uscita
        if (operazione == -1) {
            sem_post(dc->sem_done);
            break;
        }

        if (operazione == 1) {
            // inizializzazione di x[j]
            for (int j = start_node; j < end_node; j++) {
                x[j] = 1.0 / g->N;
            }

        } else if (operazione == 2) {
            // calcolo contributo dei nodi dead end
            double local_sum_t = 0.0;
            for (int j = start_node; j < end_node; j++) {
                if (g->out[j] == 0) {
                    local_sum_t += x[j];
                    y[j] = 0;
                } else {
                    y[j] = x[j] / g->out[j];
                }
            }
            pthread_mutex_lock(dc->mutex);
            *(dc->sum_t) += local_sum_t;
            pthread_mutex_unlock(dc->mutex);

        } else if (operazione == 3) {
            // calcolo xnext[j]
            for (int j = start_node; j < end_node; j++) {
                xnext[j] = teleport + d * (*(dc->sum_t) / g->N);
                for (int k = 0; k < g->ent[j]; k++) {
                    int i = g->in->adiacenza[j][k]; 
                    xnext[j] += d * y[i];
                }
            }

        } else if (operazione == 4) {
            // calcolo errore
            double local_err = 0.0;
            for (int j = start_node; j < end_node; j++) {
                local_err += fabs(xnext[j] - x[j]);
                x[j] = xnext[j];
            }
            pthread_mutex_lock(dc->mutex);
            *(dc->errore) += local_err;
            pthread_mutex_unlock(dc->mutex);
        }
        // Segnalo che il task è completato
        sem_post(dc->sem_done);
    }

    return (void*)0 ;
}

double *pagerank(grafo *g, double d, double eps, int maxiter, int taux, int *numiter) {
    double *x = malloc(g->N * sizeof(double));
    if (x == NULL) termina("Errore allocazione");
    double *y = malloc(g->N * sizeof(double));
    if (y == NULL) termina("Errore allocazione");
    double *xnext = malloc(g->N * sizeof(double));
    if (xnext == NULL) termina("Errore allocazione");
    double teleport = (1.0 - d) / g->N;
    double err, sum_t;
    int iter = 0;
    int ope;

    // gestione thread ausilliari
    sem_t sem_tasks, sem_done;
    pthread_mutex_t mutex_p = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_t = PTHREAD_MUTEX_INITIALIZER;
    sem_init(&sem_tasks, 0, 0);
    sem_init(&sem_done, 0, 0);
    int task_id;

    // creo tutti i thread ausilliari
    dati_thread_pagerank td[taux];
    pthread_t tp[taux];

    int nodi = g->N / taux;
    int resto = g->N-(taux*nodi);


    for (int i = 0; i < taux; i++) {
        td[i].g = g;
        td[i].d = d;
        td[i].x = x;
        td[i].y = y;
        td[i].errore = &err;
        td[i].operazione = &ope;
        td[i].sum_t = &sum_t;
        td[i].xnext = xnext;
        td[i].teleport = teleport;
        td[i].sem_tasks = &sem_tasks;
        td[i].sem_done = &sem_done;
        td[i].mutex = &mutex_p;
        td[i].mutex_task = &mutex_t;
        td[i].task_id = &task_id;
        td[i].n_thread = taux;
        td[i].nodi = nodi;
        td[i].resto= resto;

        pthread_create(&tp[i], NULL, tbody, &td[i]);
    }

    // inizializzo x[i] parallelamente
    task_id = 0;
    ope = 1;
    for (int j = 0; j < taux; j++) {
        sem_post(&sem_tasks);
    }
    for (int j = 0; j < taux; j++) {
        sem_wait(&sem_done);
    }

    // creo thread gestore
    dati_gestore_segnale dg;
    pthread_t gestore;
    dg.mutex = &mutex_p;
    dg.N = g->N;
    dg.x = x;
    dg.numiter = &iter;
    pthread_create(&gestore, NULL, gestore_segnali, &dg);

    do {
        // calcolo il contributo dei dead end parallelamente
        sum_t = 0.0;
        task_id = 0;
        ope = 2;
        for (int j = 0; j < taux; j++) {
            sem_post(&sem_tasks);
        }
        for (int j = 0; j < taux; j++) {
            sem_wait(&sem_done);
        }

        // calcolo xnext parallelamente
        task_id = 0;
        ope = 3;
        for (int j = 0; j < taux; j++) {
            sem_post(&sem_tasks);
        }
        for (int j = 0; j < taux; j++) {
            sem_wait(&sem_done);
        }

        // calcolo errore parallelamente
        task_id = 0;
        err = 0.0;
        ope = 4;
        for (int j = 0; j < taux; j++) {
            sem_post(&sem_tasks);
        }
        for (int j = 0; j < taux; j++) {
            sem_wait(&sem_done);
        }

        iter++;
    } while (err > eps && iter < maxiter);
    *numiter = iter;

    // termino i thread ausilliari
    ope = -1;
    for (int t = 0; t < taux; t++) {
        sem_post(&sem_tasks);
    }
    for (int t = 0; t < taux; t++) {
        pthread_join(tp[t], NULL);
    }

    // termino thread gestore
    pthread_cancel(gestore);
    pthread_join(gestore, NULL);

    // deallocazione
    sem_destroy(&sem_tasks);
    sem_destroy(&sem_done);
    pthread_mutex_destroy(&mutex_p);
    pthread_mutex_destroy(&mutex_t);
    free(y);
    free(xnext);

    return x;
}
