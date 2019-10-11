#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define swap()					\
  {						\
    temp = lrot(A, 5) + F + E + g + M[i];	\
    E = D;					\
    D = C;					\
    C = lrot(B, 30);				\
    B = A;					\
    A = temp;					\
  }  						
    
//
void sha1hash(byte *restrict msg, const u_int64 len, byte *restrict hash)
{
  byte *newmsg = NULL;
  u_int64 newlen = len;
  u_int M[80] __attribute__((aligned(ALIGN))), F, A, B, C, D, E, temp, g,  
        a0 = 0x67452301, b0 = 0xEFCDAB89, c0 = 0x98BADCFE, d0 = 0x10325476, e0 = 0xC3D2E1F0;
  
  //Padding !
  while ((++newlen & 63) != 56);
  
  newmsg = malloc(newlen + 8); //8 => u_int64 - Replace with aligned malloc 
  memcpy(newmsg, msg, len);    //Replace with fast memcpy
  
  *(newmsg + len) = 0x80; //Padding with '1' bit
  
  for (int i = len + 1; i < newlen; i++) //Padding with '0' bit
    *(newmsg + i) = 0x00;
  
  u_int642byte_be(len << 3, newmsg + newlen);
  
  //Hashing  
  for (int block = 0; block < newlen; block += 64)
    {
      break16(byte2u_int_be, M, newmsg + block);
      
      //
      for (int i = 16; i < 80; i++)
	M[i] = lrot((M[i - 3] ^ M[i - 8] ^ M[i - 14] ^ M[i - 16]), 1);
      
      //Vectorizable !
      /* for (int i = 32; i < 80; i++) */
      /* 	M[i] = lrot(M[i - 6] ^ M[i - 16] ^ M[i - 28] ^ M[i - 32], 2); */
      
      A = a0, B = b0, C = c0, D = d0, E = e0;
      
      //Break the loop to avoid index range check 
      for (int i = 0; i < 20; i++)
	{
	  F = (B & C) | ((~B) & D), g = 0x5A827999;
	  swap();
	}
      
      for (int i = 20; i < 40; i++)
	{
	  F = B ^ C ^ D, g = 0x6ED9EBA1;
	  swap();
	}
      
      for (int i = 40; i < 60; i++)
	{
	  F = (B & C) | (B & D) | (C & D), g = 0x8F1BBCDC;
	  swap();
	}
      
      for (int i = 60; i < 80; i++)
	{
	  F = B ^ C ^ D, g = 0xCA62C1D6;
	  swap();
	}
      
      a0 += A;
      b0 += B;
      c0 += C;
      d0 += D;
      e0 += E;
    }
  
  u_int2byte_be(a0, hash     );
  u_int2byte_be(b0, hash +  4);
  u_int2byte_be(c0, hash +  8);
  u_int2byte_be(d0, hash + 12);
  u_int2byte_be(e0, hash + 16);
}
