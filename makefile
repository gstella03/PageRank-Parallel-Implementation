CC=gcc
CFLAGS=-std=c11 -Wall -g -O -pthread
LDLIBS=-lm -lrt -pthread

EXECS=pagerank

all: $(EXECS)

pagerank: main.o pagerank.o consumatori.o grafo.o gestore.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.c pagerank.h consumatori.h grafo.h gestore.h
	$(CC) $(CFLAGS) -c $<

pagerank.o: pagerank.c pagerank.h grafo.h gestore.h
	$(CC) $(CFLAGS) -c $<

gestore.o: gestore.c gestore.h
	$(CC) $(CFLAGS) -c $<

consumatori.o: consumatori.c consumatori.h grafo.h
	$(CC) $(CFLAGS) -c $<

grafo.o: grafo.c grafo.h
	$(CC) $(CFLAGS) -c $<


clean: 
	rm -f *.o $(EXECS)
	