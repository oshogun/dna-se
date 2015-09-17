FLAGS=-Wall -Wno-unused-result -std=c11 -O2

CC=gcc

RM=rm -f

EXEC=dna-seq

all: $(EXEC)

dna.o: src/dna.c
	$(CC) $(FLAGS) -c src/dna.c

$(EXEC): dna.o
	$(CC) $(FLAGS) dna.o -o $(EXEC)
	$(RM) dna.o

run:
	./$(EXEC)

clean:
	$(RM) dna.o $(EXEC)
