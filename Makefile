# Paths
DSERV = Serveur
DCLT = Client
DOBJ = obj
DEXE = exe

# Compiler flags
CFLAGS = -Wall -g

# Commands
CC = gcc $(CFLAGS)

# How to make Objects
$(DOBJ)/serveur.o: $(DSERV)/serveur.c
	$(CC) -o $@ -c $<

#Targets
all: serveur

serveur : $(DOBJ)/serveur.o
	$(CC) -o $(DEXE)/serveur $^

clean:
	rm $(DOBJ)/*
	rm $(DEXE)/*
