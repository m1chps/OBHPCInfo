GCC=gcc
CFLAGS=-std=c99 -Ofast
LFLAGS=-fPIC -shared

all : clean md5 sha1 sha224 sha256 libyhash

md5 :
	$(GCC) $(CFLAGS) common.c md5hash.c md5main.c -o md5hash

sha1 :
	$(GCC) $(CFLAGS) common.c sha1hash.c sha1main.c -o sha1hash

sha224 :
	$(GCC) $(CFLAGS) common.c sha224hash.c sha224main.c -o sha224hash

sha256 :
	$(GCC) $(CFLAGS) common.c sha256hash.c sha256main.c -o sha256hash

libyhash :
	$(GCC) $(LFLAGS) $(CFLAGS) common.c md5hash.c sha1hash.c sha224hash.c sha256hash.c -o libyhash.so

clean :
	@rm -rf *~ *.o *.so md5hash sha1hash sha224hash sha256hash
