CC = gcc

EXE = omelette
DEBUGEXE = omelette_debug

PEXTFLAGS = -DUSE_PEXT -mbmi2
CFLAGS = -std=gnu11 -Wall -O2 -g -lpthread $(PEXTFLAGS) -DNDEBUG
DEBUGFLAGS = -std=gnu11 -Wall -O0

OBJ = attacks.o bitboards.o evaluate.o main.o movegen.o movepicker.o position.o search.o threads.o tt.o uci.o

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(DEBUGEXE): $(OBJ)
	$(CC) $(DEBUGFLAGS) -o $@ $(OBJ)


.PHONY:	clean build debug

clean:
	-rm *.o

build:
	$(EXE)

debug:
	$(DEBUGEXE)
