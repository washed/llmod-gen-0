
#include "rc4.h"

uint8_t rc4_state[RC4_STATE_SIZE];
uint8_t rc4_key[RC4_KEY_SIZE];
uint8_t rc4_out_buffer[RC4_OUT_SIZE];

// Key Scheduling Algorithm 
// Input: state - the state used to generate the keystream
//        key - Key to use to initialize the state 
//        len - length of key in bytes  

void key_schedule_algorithm( uint8_t state[], uint8_t key[], uint8_t len)
{
   uint8_t i,j=0,t;
   
   for (i=0; i < 255; i++)
      state[i] = i;
	  
   for (i=0; i < 255; i++)
   {
      j = (j + state[i] + key[i % len]) % 256;
      t = state[i];
      state[i] = state[j];
      state[j] = t;
   }
}

// Pseudo-Random Generator Algorithm 
// Input: state - the state used to generate the keystream 
//        out - Must be of at least "len" length
//        len - number of bytes to generate

void prng( uint8_t state[], uint8_t out[], uint8_t len)
{
   uint8_t i=0, j=0, x, t;
   
   for (x=0; x < len; ++x)  {
      i = (i + 1) % 256; 
      j = (j + state[i]) % 256; 
      t = state[i]; 
      state[i] = state[j]; 
      state[j] = t; 
      out[x] = state[(state[i] + state[j]) % 256];
   }   
}  