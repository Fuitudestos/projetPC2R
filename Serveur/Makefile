# Paths
DSRC = src
DINC = inc
DOBJ = obj
DEXE = exe

# Compiler flags
CFLAGS = -I$(DINC) -Wall -g

# Commands
CC = gcc $(CFLAGS)

# How to make Objects
$(DOBJ)/%.o: $(DSRC)/%.c
	$(CC) -o $@ -c $< -lpthread

#Targets
all: serveur

serveur: $(DOBJ)/boggle.o $(DOBJ)/serveur.o
	$(CC) -o $(DEXE)/serveur $^ -lpthread

clean:
	rm $(DOBJ)/*
	rm $(DEXE)/*
