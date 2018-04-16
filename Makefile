# Paths
DSERV = Serveur
DCLT = Client
DOBJ = obj
DEXE = exe
DCLS = class

# Compiler flags
CFLAGS = -Wall -g

# Commands
CC = gcc $(CFLAGS)
SCA = scalac

# How to make Objects
$(DOBJ)/%.o: $(DSERV)/%.c
	$(CC) -o $@ -c $< -lpthread

#Targets
all: serveur client

serveur: $(DOBJ)/serveur.o
	$(CC) -o $(DEXE)/serveur $^ -lpthread

client:
	$(SCA) -d $(DCLS) $(DCLT)/Client.scala
	$(SCA) -d $(DCLS) $(DCLT)/Main.scala

clean:
	rm $(DOBJ)/*
	rm $(DEXE)/*
