/*
 * rc4.h
 *
 * Created: 12.09.2015 16:30:40
 *  Author: washed
 */ 


#ifndef RC4_H_
#define RC4_H_

#include <avr/io.h>

#define RC4_STATE_SIZE	256
#define RC4_KEY_SIZE	256
#define RC4_OUT_SIZE	256

extern uint8_t rc4_state[RC4_STATE_SIZE];
extern uint8_t rc4_key[RC4_KEY_SIZE];
extern uint8_t rc4_out_buffer[RC4_OUT_SIZE];

void key_schedule_algorithm( uint8_t state[], uint8_t key[], uint8_t len );
void prng( uint8_t state[], uint8_t out[], uint8_t len );

#endif /* RC4_H_ */