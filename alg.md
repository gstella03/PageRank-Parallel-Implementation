# La Ricerca Sequenziale

La ricerca sequenziale (o lineare) è un metodo semplice per trovare un elemento in una lista, iterando su ogni elemento fino a trovare quello cercato o raggiungere la fine della lista.

```c
int seqSearch(int k, int A[], int n) {
    for (int i = 0; i < n; i++) {
        if (A[i] == k) {
            return i; // Elemento trovato, ritorna l'indice
        }
    }
    return -1; // Elemento non trovato
}
```
### Complessità

- Temporale: La complessità temporale della ricerca sequenziale è `𝑂(𝑛)`, dove 𝑛 è la lunghezza della lista. Nel peggiore dei casi, l'algoritmo deve esaminare ogni elemento una volta.

- Spaziale: La complessità spaziale è `𝑂(1)` poiché utilizza solo una quantità costante di spazio aggiuntivo per le variabili.

# Ricerca Binaria

La ricerca binaria è un algoritmo di ricerca efficiente utilizzato per trovare un elemento in una lista ordinata. 
L'idea di base è di dividere ripetutamente la lista a metà, confrontando l'elemento di mezzo con l'elemento cercato, 
e restringendo la ricerca alla metà in cui l'elemento deve trovarsi.

```c
int binSearch(int k, int A[], int n) {
    int left = 0;
    int right = n - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (A[mid] == k)
            return mid; // Elemento trovato, ritorna l'indice

        if (A[mid] < k)
            left = mid + 1; // Cerca nella metà destra
        else
            right = mid - 1; // Cerca nella metà sinistra
    }

    return -1; // Elemento non trovat
}

```
### Complessità

- Temporale: La complessità temporale della ricerca binaria è `𝑂(log 𝑛)`, dove 𝑛 è la lunghezza della lista. Ad ogni iterazione, il numero di elementi da considerare si riduce della metà.

- Spaziale: La complessità spaziale è `𝑂(1)` poiché utilizza solo una quantità costante di spazio aggiuntivo per le variabili `left`, `right` e `mid`.

