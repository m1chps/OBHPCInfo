#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "hash.h"

#include "rdtsc.h"

//
int main(int argc, char **argv)
{
  if (argc < 2)
    return printf("%s [input file]\n", argv[0]), -1;

  struct stat sb;

  if (stat(argv[1], &sb) == -1)
    return printf("Error: Cannot open file (%s)\n", argv[1]), -2;

  u_int64 len = sb.st_size;
  FILE *fd = fopen(argv[1], "rb");
  byte hash[MD5_H_SIZE] __attribute__((aligned(ALIGN)));

  if (!fd)
    ;

  byte *buffer = malloc(sizeof(byte) * len);

  fread(buffer, sizeof(byte), len, fd);
  
  double b = rdtsc();
  
  md5hash(buffer, len, hash);

  double a = rdtsc();
  
  printhash(hash, MD5_H_SIZE);

  printf("Cycles     : %lf\n", (a - b));
  printf("Cycles/Byte: %lf\n", (a - b) / len);
  printf("Bytes/Cycle: %lf\n", len / (a - b));

  free(buffer);

  fclose(fd);
  
  return 0;
}
