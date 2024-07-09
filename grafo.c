#include "grafo.h"

// funzioni di deallocazione del grafo
void distruggi(inmap *a,int n)
{
  for (int i = 0; i < n; i++) {
        free(a->adiacenza[i]);
  }
  free(a->adiacenza);
  free(a->size);
}
void distruggi_grafo(grafo *g) {
    distruggi(g->in,g->N);
    free(g->in);
    free(g->out);
    free(g->ent);
}

// funzione che controlla se l'arco è stato gia inserito
bool arco_gia_inserito(grafo *g, int i, int j) {
    for (int k = 0; k < g->ent[j]; k++) {
        if (g->in->adiacenza[j][k] == i) {
            return true;
        }
    }
    return false;
}

// funzione che aggiunge l'arco alla lista di adiacenza
void aggiungi_arco(grafo *g, int i, int j) {
    g->out[i]++;
    g->ent[j]++;
    int n = g->ent[j];
    if(n > g->in->size[j]){
        if (g->in->size[j] == 0){
            g->in->size[j] = 4;
        } else {
            g->in->size[j]= 2*g->in->size[j];
        }
        g->in->adiacenza[j] = realloc(g->in->adiacenza[j],g->in->size[j] * sizeof(int));
        if (g->in->adiacenza[j] == NULL) termina ("Errore allocazione");
    }
    g->in->adiacenza[j][n-1] = i;
}

// gestione errori
void termina(const char *messaggio){
  if(errno==0) 
     fprintf(stderr,"%s\n",messaggio);
  else 
    perror(messaggio);
  exit(1);
}
