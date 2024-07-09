#ifndef GRAFO_H
#define GRAFO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>

// struttura inmap
typedef struct {
    int **adiacenza;
    int *size;
}inmap;

// struttura grafo
typedef struct {
    int N;
    int *out;
    int *ent; //numero archi entranti nel nodo i
    inmap *in;
} grafo;

// prototipi
void termina(const char *messaggio);
void aggiungi_arco(grafo *g, int origine, int destinazione);
bool arco_gia_inserito(grafo *g, int origine, int destinazione);
void distruggi(inmap *a,int n);
void distruggi_grafo(grafo *g);

#endif
