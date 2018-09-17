CC = gcc
CFLAGS=-Wall -ggdb -O0 -D DEBUG
#CFLAGS=-O2 -Os -s -Wall
LDFLAGS=-lcurl -lconfig

all: HNAnative
 
HNAnative: HNAnative.o json.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

HNAnative.o: HNAnative.c
	$(CC) $(CFLAGS) -c $< -o $@
json.o: json_h/json.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf HNAnative *.o 
