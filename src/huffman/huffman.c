/*
  Huffman compression

  Stream to compress: AAAABBCABBCDCCDA
  -------------------

  . Count frequencies & sort
  
  A frequency: 6
  B frequency: 4
  C frequency: 4
  D frequency: 2

  . Build encoding tree

  A: 6
  B: 4
  C: 4 __  0
  D: 2 __|---(CD: 6)
           1
  Sort

  A : 6
  CD: 6 __  0
  B : 4 __|---(CDB: 10)
            1

  Sort

  CDB: 10 __ 0
  A  : 6  __|---(CDBA: 16)
             1

  Encoding/Decoding tree
     
	       CDBA
	      /    \
	  0  /      \  1
	   CDB       A
	   /  \
	0 /    \ 1
	 CD     B
	/  \
     0 /    \ 1
      C      D
  
  Caracter codes

  A encoding: 1
  B encoding: 01
  C encoding: 000
  D encoding: 001
  
  High frequency characters get the sortest encoding
  Low frequency characters get the longest encoding

  Compressed stream: 11110101000101010010000000011
  ------------------
  
  1, 1, 1, 1, 01, 01, 000, 1, 01, 01, 001, 000, 000, 001, 1
  
  1, A
  1, A
  1, A
  1, A
  0, 1, B
  0, 1, B
  0, 0, 0, C
  1, A
  0, 1, B
  0, 1, B
  0, 0, 1, D
  0, 0, 0, C
  0, 0, 0, C
  0, 0, 1, D
  1, A  

  Evaluate compression ratio in bits
  
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
#define max(a, b) ((a) > (b)) ? a : b

//
#define MAX_CHARS  256
#define BUFF_SIZE  1024

//
typedef struct node_s { unsigned char c; unsigned f; struct node_s *left; struct node_s *right; } node_t;

//
typedef struct freq_tab_s { char c[MAX_CHARS]; unsigned f[MAX_CHARS]; unsigned nb_entries; unsigned size; } freq_tab_t;

//
typedef struct freq_stack_s { node_t *stack[MAX_CHARS]; int sp; } freq_stack_t;

//
freq_tab_t freq_tab;

//
freq_stack_t freq_stack;

//
void stack_init()
{ freq_stack.sp = -1; }

//
unsigned push(node_t *n)
{
  if (freq_stack.sp < MAX_CHARS - 1)
    {
      freq_stack.stack[++freq_stack.sp] = n;

      return 1;
    }
  else
    return 0;
}

//
unsigned pop(node_t **n)
{
  if (freq_stack.sp >= 0)
    {
      (*n) = freq_stack.stack[freq_stack.sp--];

      return 1;
    }
  else
    {
      *(n) = NULL;
      return 0;
    }
}

//
void freq_init()
{
  freq_tab.size = 0;
  freq_tab.nb_entries = 0;
  
  for (unsigned i = 0; i < MAX_CHARS; i++)
    freq_tab.f[i] = 0;
}

//
void freq_count(unsigned char *buffer, unsigned bufflen)
{
  for (unsigned i = 0; i < bufflen; i++)
    {
      freq_tab.c[buffer[i]] = buffer[i];
      freq_tab.f[buffer[i]]++;
    }
}

//
void freq_sort()
{
  unsigned tmp_f = 0;
  unsigned char tmp_c = 0; 
  
  for (unsigned i = 0; i < MAX_CHARS; i++)
    for (unsigned j = i; j < MAX_CHARS; j++)
      if (freq_tab.f[i] < freq_tab.f[j])
	{
	  tmp_f         = freq_tab.f[i];
	  freq_tab.f[i] = freq_tab.f[j];
	  freq_tab.f[j] = tmp_f;

	  tmp_c         = freq_tab.c[i];
	  freq_tab.c[i] = freq_tab.c[j];
	  freq_tab.c[j] = tmp_c;
	}
}

//
void stack_sort()
{
  node_t *tmp = NULL;

  for (unsigned i = 0; i <= freq_stack.sp; i++)
    for (unsigned j = i; j <= freq_stack.sp; j++)
      if (freq_stack.stack[i]->f < freq_stack.stack[j]->f)
      {
	tmp = freq_stack.stack[i];
	freq_stack.stack[i] = freq_stack.stack[j];
	freq_stack.stack[j] = tmp;
      }
}

//
unsigned freq_clean()
{
  unsigned i = 0;
  node_t *n = NULL;
  
  while (freq_tab.f[i])
    {
      n = malloc(sizeof(node_t));

      n->c = freq_tab.c[i];
      n->f = freq_tab.f[i];
      n->left = n->right = NULL;

      push(n);
      
      freq_tab.nb_entries++;
      freq_tab.size += freq_tab.f[i++];
    }
  
  return i;
}

//
void tree_build()
{
  node_t *n1 = NULL, *n2 = NULL, *n = NULL;

  while (freq_stack.sp > 0)
    {      
      pop(&n2);
      pop(&n1);
      
      n = malloc(sizeof(node_t));
      
      n->left  = n1;
      n->right = n2;
      n->f     = n1->f + n2->f;
      
      push(n);
      
      stack_sort();
    }
}

void tree_print(node_t *n, unsigned depth)
{  
  if (n)
    {      
      for (unsigned i = 0; i < depth; i++)
	printf("|");
      
      if (!n->left && !n->right)
	if (n->c == 0x0A)
	  printf(" (RET)");
	else
	  if (n->c == 0x09)
	    printf(" (TAB)");
	  else
	    printf(" (%c)", n->c);
      
      printf(" %u\n", n->f);

      tree_print(n->left , depth + 1);
      tree_print(n->right, depth + 1);
    }
}

//
void print_encoding(node_t *n, unsigned *e, unsigned pos, unsigned *size)
{
  if (n)
    {
      if (n->left)
	{
	  e[pos] = 0;
	  print_encoding(n->left, e, pos + 1, size);
	}

      if (n->right)
	{
	  e[pos] = 1;
	  print_encoding(n->right, e, pos + 1, size);
	}
      
      //If leaf
      if (!n->left && !n->right)
	{
	  *size += (pos * n->f);
	  
	  if (n->c == 0x0A)
	    printf("RET (0x%02x): ", n->c);
	  else
	    if (n->c == 0x09)
	      printf("TAB (0x%02x): ", n->c);
	    else
	      if (n->c == 0x0D)
		printf("EOL (0x%02x): ", n->c);
	      else
		printf("%c (0x%02x): ", n->c, n->c);
	  
	  for (unsigned i = 0; i < pos; i++)
	    printf("%u ", e[i]);

	  
	  printf("\n");
	}
    }
}

//
int main(int argc, char **argv)
{
  if (argc < 2)
    return printf("OUPS: %s [file]\n", argv[0]), -1;
  
  unsigned read_bytes = 0;
  FILE *fd = fopen(argv[1], "rb");
  unsigned char buffer[BUFF_SIZE + 1];

  freq_init();
  stack_init();
  
  while ((read_bytes = fread(buffer, sizeof(unsigned char), BUFF_SIZE, fd)))
    freq_count(buffer, read_bytes);
  
  freq_sort();

  freq_clean();
  
  printf("#Entries: %u\n", freq_tab.nb_entries);
  
  for (unsigned i = 0; i < freq_tab.nb_entries; i++)
    if (freq_tab.c[i] == 0x0A)
      printf("%3u: 0x%02x (RET) %u\n", i, (unsigned char)freq_tab.c[i], freq_tab.f[i]);
    else
      if (freq_tab.c[i] == 0x09)
	printf("%3u: 0x%02x (TAB) %u\n", i, (unsigned char)freq_tab.c[i], freq_tab.f[i]);
      else
	printf("%3u: 0x%02x (%c) %u\n", i, (unsigned char)freq_tab.c[i], freq_tab.c[i], freq_tab.f[i]);

  printf("\n");
  
  stack_sort();
  
  tree_build();
  
  node_t *n = NULL;

  if (pop(&n))
    tree_print(n, 0);
  
  printf("\n");

  unsigned e[MAX_CHARS];
  unsigned huff_size = 0;
  
  print_encoding(n, e, 0, &huff_size);

  //huff_size is in bits, orig_size is in bytes 
  printf("\nOriginal size: in bits ( %u );\t\tin bytes (%lu KB, %lu MB)\n"
	   "Huffman  size: in bits ( %u );\t\tin bytes (%lu KB, %lu MB)\n\n",
	 (freq_tab.size << 3), (freq_tab.size >> 10), (freq_tab.size >> 20),
	 huff_size, (huff_size >> 13), (huff_size >> 23));

  printf("Orig / huff ratio\t\t: %lf\n", (double)(freq_tab.size << 3) / (double)huff_size);
  printf("Space Savings\t\t\t: %lf %%\n", (((double)huff_size * 100 / (double)(freq_tab.size << 3))));
  
  fclose(fd);
  
  return 0;
}
