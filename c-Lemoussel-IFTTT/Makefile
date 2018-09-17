CC = gcc
#CFLAGS=-Wall -ggdb -O0 -D DEBUG
CFLAGS=-O2 -Os -s -Wall
LDFLAGS=-lcurl -lconfig

all: HNAifttt
 
HNAifttt: HNAifttt.o json.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

HNAifttt.o: HNAifttt.c
	$(CC) $(CFLAGS) -c $< -o $@
json.o: json_h/json.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf HNAifttt *.o 
