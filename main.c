#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h> 
#include "pagerank.h"
#include "grafo.h"
#include "consumatori.h"
#include "gestore.h"

typedef struct {
    int r;
    int c;
    int n;
} tripla;

typedef struct {
    int i;
    double n;
} pair;

//funzione di confronto per qsort
int confronta(pair *a, pair *b)
{
  if(a->n < b->n) return 1;
  else if(a->n > b->n) return -1;
  return 0; 
}

// funzione che inizializza il grafo
void inizializza_grafo (grafo *g, int n){
    g->N=n;
    g->out=malloc(n*sizeof(int));
    if (g->out == NULL) termina("Errore allocazione");
    g->ent=malloc(n*sizeof(int));
    if (g->ent == NULL) termina("Errore allocazione");
    g->in=malloc(sizeof(inmap));
    if (g->in == NULL) termina("Errore allocazione");
    g->in->adiacenza=malloc(n*sizeof(int*));
    if (g->in->adiacenza == NULL) termina("Errore allocazione");
    g->in->size=malloc(n*sizeof(int));
    if (g->in->size == NULL) termina("Errore allocazione");
    for(int i=0;i<n;i++){
        g->out[i]=0;
        g->ent[i]=0;
        g->in->size[i]=0;
        g->in->adiacenza[i]=NULL;
    }
}

int main(int argc, char *argv[])
{
    // valori di default
    int k=3;
    int m=100;
    double d=0.9;
    double e=1.0e-7;
    int tc=3;
    char *infile = NULL;

    // gestione opzioni linea di comando
    int opt;
    while ((opt = getopt(argc, argv, "k:m:d:e:t:")) != -1) {
        switch (opt) {
            case 'k':
                k = atoi(optarg);
                break;
            case 'm':
                m = atoi(optarg);
                break;
            case 'd':
                d = atof(optarg);
                break;
            case 'e':
                e = atof(optarg);
                break;
            case 't':
                tc = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usare: %s [-k K] [-m M] [-d D] [-e E] [-t T] infile\n", argv[0]);
                exit(1);
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "Input file mancante\n");
        fprintf(stderr, "Usare: %s [-k K] [-m M] [-d D] [-e E] [-t T] infile\n", argv[0]);
        exit(1);
    }

    // apro file
    infile = argv[optind];
    FILE *f = fopen(infile,"r");
    if(f==NULL) termina("Errore apertura file");

    // GESTIONE THREAD
    arco buffer[Buf_size];
    grafo g;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_g = PTHREAD_MUTEX_INITIALIZER;
    sem_t sem_free_slots, sem_data_items;
    int p_index = 0;
    int c_index = 0;
    sem_init(&sem_free_slots, 0, Buf_size);
    sem_init(&sem_data_items, 0, 0);

    dati_consumatori ac[tc];
    pthread_t cons[tc]; 

    // creo tutti i consumatori
    for(int i=0;i<tc;i++) {
        ac[i].buffer = buffer;
        ac[i].cindex = &c_index;
        ac[i].pmutex_buf = &mutex;
        ac[i].pmutex_grafo = &mutex_g;
        ac[i].grafo = &g;
        ac[i].sem_data_items = &sem_data_items;
        ac[i].sem_free_slots = &sem_free_slots; 
        pthread_create(&cons[i],NULL,cbody,&ac[i]);
    }

    // leggo file e passo gli archi
    char *line=NULL;    
    size_t n=0;
    int prima = true;
    while (true) {
        ssize_t e = getline(&line,&n,f);
        if(e<0) { 
            free(line);  
            break;  
        }
        if(line[0]=='%')
            continue;
        tripla t;
        arco a;
        if(prima){
            if (sscanf(line, "%d %d %d", &t.r, &t.c, &t.n) != 3) {
                free(line);
                termina("Errore: Formato della prima linea non valido");
            }
            prima = false;
            if (t.r < 0 && t.c < 0) termina("Errore: numero di righe e di colonne non valide");
            if (t.r != t.c) termina("Errore: righe e colonne non coincidono");
            if (t.n < 0) termina("Errore: numero totale di archi non valido");
            pthread_mutex_lock(&mutex_g);
            inizializza_grafo(&g,t.r);
            pthread_mutex_unlock(&mutex_g);
        } else {
            if (sscanf(line, "%d %d", &a.i, &a.j) != 2) {
                free(line);
                termina("Errore: Formato della linea archi non valido");
            }
            if(a.i > t.r || a.j > t.r) termina("Errore: archi illegali");
            a.i=a.i-1;
            a.j=a.j-1;
            
            sem_wait(&sem_free_slots);
            pthread_mutex_lock(&mutex);
            buffer[p_index % Buf_size] = a;
            p_index +=1;
            pthread_mutex_unlock(&mutex);
            sem_post(&sem_data_items);
        }
    }

    // deallocazione
    fclose(f);

    // segnalo ai consumatori che possono terminare
    arco a;
    a.i=a.j=-1;
    for(int i=0;i<tc;i++) {
        sem_wait(&sem_free_slots);
        pthread_mutex_lock(&mutex);
        buffer[p_index++ % Buf_size]= a;
        pthread_mutex_unlock(&mutex);
        sem_post(&sem_data_items);
    }

    // attendo i consumatori 
    for(int i=0;i<tc;i++) {
        pthread_join(cons[i],NULL);
    }

    // deallocazione
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_g);
    sem_destroy(&sem_data_items);
    sem_destroy(&sem_free_slots);

    // realloco ala dimensione minima
    for (int i=0;i<g.N;i++){
        g.in->adiacenza[i]=realloc(g.in->adiacenza[i],g.ent[i]*sizeof(int));
        if(g.in->adiacenza[i]==NULL) termina("Errore allocazione");
    }

    // conto archi validi e dead-end nodes
    int n_archi=0;
    int n_d_nodi=0;
    for (int i=0;i<g.N;i++){
        n_archi+=g.out[i];
        if (g.out[i]==0){
            n_d_nodi++;
        }
    }
    // output lettura grafo
    printf("Number of nodes: %d\n",g.N);
    printf("Number of dead-end nodes: %d\n",n_d_nodi);
    printf("Number of valid arcs: %d\n",n_archi);
    
    // blocco tutti i segnali tranne sigint
    sigset_t mask;
    sigfillset(&mask);  
    sigdelset(&mask,SIGINT); 
    pthread_sigmask(SIG_BLOCK,&mask,NULL); 


    int iter;
    double *x=pagerank(&g,d,e,m,tc,&iter);
    double sum=0.0;
    pair *arr=malloc(g.N*sizeof(pair));
    if(arr==NULL) termina("Errore allocazione");
    for(int i=0;i<g.N;i++){
        sum+=x[i];
        pair p;
        p.i=i;
        p.n=x[i];
        arr[i]=p;
    }
    qsort(arr,g.N,sizeof(pair), (__compar_fn_t) &confronta);

    //output calcolo pagerank
    if (iter == m){
        printf("Did not converge after %d iterations\n",m);
    } else {
        printf("Converged after %d iterations\n",iter);
    }
    printf("Sum of ranks: %4.4f   (should be 1)\n",sum);
    printf("Top %d nodes:\n",k);
    for(int i=0;i<k;i++){
        printf("  %d %f\n",arr[i].i,arr[i].n);
    }

    // deallocazione
    distruggi_grafo(&g);
    free(x);
    free(arr);

    return 0;
}

