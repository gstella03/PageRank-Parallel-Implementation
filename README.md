


# PageRank-Parallel-Implementation

Questo progetto è un'implementazione parallela dell'algoritmo PageRank utilizzando il linguaggio C. Il programma legge un grafo da un file di input, calcola il PageRank dei nodi e stampa i risultati.

## Struttura del Progetto

Il progetto è suddiviso nei seguenti file:

- `main.c`: Contiene la funzione principale che gestisce l'input, l'inizializzazione del grafo e la gestione dei thread.
- `pagerank.c` e `pagerank.h`: Implementazione dell'algoritmo PageRank.
- `grafo.c` e `grafo.h`: Gestione della struttura del grafo e delle operazioni su di esso.
- `consumatori.c` e `consumatori.h`: Implementazione dei thread consumatori che processano gli archi del grafo.
- `gestore.c` e `gestore.h`: Gestione dei segnali durante l'esecuzione del programma.

### Rappresentazione del Grafo come Struttura Dati

In memoria, il grafo è rappresentato tramite le seguenti strutture dati:

```
typedef struct {
    int **adiacenza;
    int *size;
} inmap;

typedef struct {
    int N;
    int *out;
    int *ent; 
    inmap *in;
} grafo;

```

- `inmap` rappresenta la lista di adiacenza per gli archi entranti di ciascun nodo:
  - `adiacenza`: matrice di adiacenza per i nodi entranti.
  - `size`: array che tiene traccia della dimensione della lista di adiacenza per ciascun nodo.
  
- `grafo` rappresenta la struttura complessiva del grafo:
  - `N`: numero totale di nodi nel grafo.
  - `out`: array che tiene traccia del numero di archi uscenti per ciascun nodo.
  - `ent`: array che tiene traccia del numero di archi entranti per ciascun nodo.
  - `in`: puntatore a una struttura inmap che contiene le liste di adiacenza per gli archi entranti

### Parallelizzazione

Il programma è parallelizzato attraverso i thread in due zone: la prima durante la fase di lettura e creazione del grafo e la seconda durante il calcolo del PageRank.

#### Fase di Lettura e Creazione del Grafo

Questa fase è stata realizzata attraverso uno schema produttore-consumatore, con un thread produttore e t thread consumatori. Il thread produttore legge un arco alla volta dal file e lo passa ai thread consumatori, che si occupano della realizzazione del grafo attraverso una lista di adiacenza.

Per coordinare il thread produttore e i thread consumatori, ho utilizzato i semafori e due mutex come strumenti per la sincronizzazione e la mutua esclusione del buffer condiviso.

Semafori:

- `sem_free_slots`: inizializzato con la grandezza del buffer, tiene traccia del numero di slot liberi nel buffer
- `sem_data_items`: inizializzato a zero, tiene traccia del numero di elementi presenti nel buffer.

Mutex:

- `mutex`: per gestire la mutua esclusione del buffer.
- `mutex_g`: per gestire la mutua esclusione del grafo.

#### Fase di Calcolo del PageRank

Nella seconda fase del programma, il calcolo del PageRank è parallelizzato utilizzando i thread. Ciascuno dei taux thread partecipa al calcolo di diverse operazioni, inclusa l'inizializzazione di x, il calcolo del contributo dei dead end, il calcolo di xnext e il calcolo dell'errore. Anche in questa fase, si fa uso di meccanismi di sincronizzazione per garantire la corretta gestione dei dati condivisi.

Semafori:

- `sem_tasks`: Inizializzato a 0, tiene traccia del numero di task disponibili.
- `sem_done`: Inizializzato a 0, tiene traccia del numero di task completati.

Mutex:

- `mutex_task`: protegge l'accesso alla variabile task_id, che tiene traccia dei task assegnati ai thread.
- `mutex`: protegge l'accesso alle variabili condivise come sum_t e errore.

Per ogni tipo di calcolo da eseguire, il programma principale assegna una operazione da svolgere e incrementa il semaforo `sem_tasks` taux volte. I thread ausiliari, non appena rilevano la presenza di task disponibili, decrementano il semaforo `sem_tasks` e leggono l'ID del task e lo incrementano. Se l'operazione assegnata è diversa da -1, i thread calcolano i nodi su cui devono lavorare e eseguono il calcolo richiesto. Successivamente, incrementano il semaforo `sem_done`. Invece se l'operazione assegnata è uguale a -1 i thread incrementano il semaforo `sem_done` ed escono dal ciclo terminando il loro compito.

### Segnali

Il programma gestisce i segnali e permette di visualizzare lo stato corrente dell'algoritmo PageRank tramite il segnale `SIGUSR1`.


## Requisiti

- GCC (GNU Compiler Collection)
- Pthreads (POSIX Threads)
- Libreria standard di C

## Compilazione

Per compilare il progetto, utilizzare il comando `make`:

```bash
make
```

Questo comando compilerà tutti i file sorgente e genererà l'eseguibile `pagerank`.

## Utilizzo

Il programma accetta diverse opzioni tramite linea di comando:

```bash
./pagerank [-k K] [-m M] [-d D] [-e E] [-t T] infile
```

- `-k K`: Numero di nodi da visualizzare nel risultato finale (default: 3).
- `-m M`: Numero massimo di iterazioni per l'algoritmo PageRank (default: 100).
- `-d D`: Fattore di damping per l'algoritmo PageRank (default: 0.9).
- `-e E`: Tolleranza per la convergenza dell'algoritmo PageRank (default: 1.0e-7).
- `-t T`: Numero di thread consumatori (default: 3).
- `infile`: File di input contenente il grafo.

### Esempio di Esecuzione

```bash
./pagerank -k 5 -m 200 -d 0.85 -e 1.0e-6 -t 4 grafo.txt
```

## Formato del File di Input

Il file di input deve contenere una rappresentazione del grafo. La prima riga deve contenere tre numeri interi: il numero di nodi, il numero di nodi (ripetuto) e il numero totale di archi. Le righe successive devono contenere coppie di interi che rappresentano gli archi del grafo.

### Esempio di File di Input

```
4 4 5
1 2
2 3
3 4
4 1
1 3
```


## Output

Il programma stamperà:

- Il numero di nodi, di dead-end nodes e di archi validi.
- Se l'algoritmo è convergente o no.
- La somma dei rank (dovrebbe essere circa 1).
- I nodi con i rank più alti.


## Autori

- [Gabriele Stella]
