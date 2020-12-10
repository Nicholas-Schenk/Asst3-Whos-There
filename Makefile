CC=gcc
INPUT=Asst3.c
OUTPUT=server
CFLAGS=-g -fsanitize=address -Wall

$(OUTPUT): $(INPUT) 
	$(CC) $(CFLAGS) -o $(OUTPUT) $(INPUT) 

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT)
