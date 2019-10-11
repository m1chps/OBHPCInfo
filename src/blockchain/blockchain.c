/*
  Blockchain concept code.
  
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "../yhash/hash.h" //For SHA256 hashing function

#define SHA256_BYTE_HASH_LEN 32
#define MAX_DATA_LEN         2048

#define DIFF_LEVEL           2 //Blockchain difficulty level 

#define N 10

//
typedef struct block_s {

  time_t block_time;                                 //8    bytes
  uint64_t block_index;                              //8    bytes
  unsigned char block_data[MAX_DATA_LEN];            //2048 bytes
  unsigned char block_hash[SHA256_BYTE_HASH_LEN];         //32   bytes
  unsigned char prev_block_hash[SHA256_BYTE_HASH_LEN];    //32   bytes
  
  //8 + 2048 + 32 + 32 = 2128 bytes = 2KB + 80B

  int _nonce; //Mining fail counter
  int _difficulty; //Mining difficulty level

  struct block_s *next_block; //
  
} block_t;

//
typedef struct blockchain_s {
  
  block_t *gen_block;
  
} blockchain_t;

//Reward
static uint64_t reward;

//Prototypes
void mine_block(block_t *b);
void print_block(block_t *b);
blockchain_t *init_blockchain();
void compute_block_hash(block_t *b);
void print_blockchain(blockchain_t *bc);
void release_blockchain(blockchain_t *bc);
int add_block(blockchain_t *bc, block_t *b);
block_t *create_block(uint64_t index, unsigned char *data);
void print_hash(const byte *restrict hash, const u_int size);

//Create a block
block_t *create_block(uint64_t index, unsigned char *data)
{
  block_t *b = malloc(sizeof(block_t));

  if (b)
    {
      b->block_index = index;
      b->block_time  = time(NULL);
      strncpy(b->block_data, data, strlen(data));

      b->_difficulty = DIFF_LEVEL;
      b->_nonce = -1;

      b->next_block = NULL;
    }
  
  return b;
}

//Genesis blockchain
blockchain_t *init_blockchain()
{
  blockchain_t *bc = malloc(sizeof(blockchain_t));

  if (bc)
    {
      //Genesis block
      bc->gen_block = create_block(0, "Genesis block");
      memset(bc->gen_block->prev_block_hash, 0, SHA256_BYTE_HASH_LEN);
    }
  
  return bc;
}

//
void release_blockchain(blockchain_t *bc)
{
  //Free all blocks first!
  free(bc);
}

//Compute block hash
void compute_block_hash(block_t *b)
{  
  unsigned hash_data_len = sizeof(b->block_index)    + sizeof(b->block_time)  +
                           strlen(b->block_data)     + sizeof(b->_nonce)      +
                           SHA256_BYTE_HASH_LEN;
  unsigned char hash_data[hash_data_len + 1];
  
  //Pack it all up in a byte stream
  memcpy(hash_data, &b->block_index, sizeof(b->block_index));
  
  memcpy(hash_data + sizeof(b->block_index), &b->block_time, sizeof(b->block_time));

  memcpy(hash_data + sizeof(b->block_index) + sizeof(b->block_time), b->block_data, strlen(b->block_data));

  memcpy(hash_data + sizeof(b->block_index) + sizeof(b->block_time) + strlen(b->block_data), &b->_nonce, sizeof(b->_nonce));
  
  memcpy(hash_data + sizeof(b->block_index) + sizeof(b->block_time) + strlen(b->block_data) + sizeof(b->_nonce), b->prev_block_hash, SHA256_BYTE_HASH_LEN);

  hash_data[hash_data_len] = 0;

  //Compute hash
  sha256hash(hash_data, hash_data_len, b->block_hash);
}

/*
  Mining challenge.
  
  The nonce will be incremented until a block matching the challenge is found.
  The example challenge of this code uses leading zeroes with the number of zeroes 
  representing the challenge diffculty (by default set to 2). If a block meets the 
  criteria (difficulty number of leading zeroes), it will be added and reward incremented.

 */
void mine_block(block_t *b)
{
  unsigned char zeroes[b->_difficulty + 1];

  memset(zeroes, '0', b->_difficulty); 
  zeroes[b->_difficulty] = 0; //Closing the string
  
  do
    {      
      b->_nonce++; 
      compute_block_hash(b);
    }
  while (strncmp(b->block_hash, zeroes, b->_difficulty));
}

//
int add_block(blockchain_t *bc, block_t *b)
{
  if (bc)
    {
      block_t *tmp = bc->gen_block;

      //Reach the end block
      while (tmp && tmp->next_block)
	tmp = tmp->next_block;

      if (tmp)
	{
	  mine_block(b);
	  
	  memcpy(b->prev_block_hash, tmp->block_hash, SHA256_BYTE_HASH_LEN);
	  tmp->next_block = b;
	  
	  //Apply reward
	  reward++;
	}
      else
	return 0;
    }
  else
    return 0;
}

//
void print_hash(const byte *restrict hash, const u_int size)
{
  for (int i = 0; i < size; i++)
    printf("%2.2x", hash[i]);
}

//
void print_block(block_t *b)
{
  printf(" # Block index\t:\t%" PRIu64 "\n", b->block_index);
  printf("\tblock time              :\t%llu\n", b->block_time);
  printf("\tblock data              :\t%s\n", b->block_data);
  printf("\tblock nonce             :\t%d\n", b->_nonce);
  printf("\tblock difficulty        :\t%d\n", b->_difficulty);
  printf("\tblock previous hash     :\t");   print_hash(b->prev_block_hash, SHA256_BYTE_HASH_LEN); printf("\n");
  printf("\tblock hash              :\t");   print_hash(b->block_hash, SHA256_BYTE_HASH_LEN);      printf("\n");
}

//
void print_blockchain(blockchain_t *bc)
{
  if (bc)
    {
      printf("Printing blockchain: BEGIN\n");
      printf("\t reward                 :\t%" PRIu64 "\n", reward);

      block_t *tmp = bc->gen_block;
      
      while (tmp)
	{
	  print_block(tmp);
	  tmp = tmp->next_block;
	}
      printf("Printing blockchain: END\n\n");
    }
}

//
int main(int argc, char **argv)
{
  char bname[10];
  unsigned nb_gen_blocks = 0;
  blockchain_t *bc = init_blockchain();
  
  while (nb_gen_blocks < N) //
    {

      sprintf(bname, "Block %d", nb_gen_blocks);
      
      block_t *b = create_block(1, bname);
      add_block(bc, b);
      
      print_blockchain(bc);

      nb_gen_blocks++;
    }
  
  release_blockchain(bc);
  
  return 0;
}
