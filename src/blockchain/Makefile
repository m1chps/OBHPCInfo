LIBPATH = ../yhash

all:
	gcc -std=c99 blockchain.c -g3 -o blockchain -lyhash -L=$(LIBPATH)

run:
	LD_LIBRARY_PATH=$(LIBPATH) ./blockchain

clean:
	rm -Rf *~ blockchain
