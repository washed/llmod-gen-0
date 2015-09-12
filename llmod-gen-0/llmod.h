/*
 * llmod.h
 *
 * Created: 12.09.2015 16:54:18
 *  Author: washed
 */ 


#ifndef LLMOD_H_
#define LLMOD_H_

#define F_CPU 8000000UL

#define MAX_SPEED		0xFF
#define MIN_SPEED		0x0

#define NORMAL_SPEED	0x9B

#define MAX_MODES		6
#define START_MODE		0

#define TEMP_TIMER_RST	1000

#define PRNG_SEED_W		1
#define PRNG_SEED_Z		2

#ifdef ENABLE_MUSIC
#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880
#endif

typedef enum llmod_modes llmod_modes;
typedef enum llmod_direction llmod_direction;
typedef struct llmod_state llmod_state;

enum llmod_modes
{
	
	MODE_IDLE			= 0,
	MODE_NORMAL_FWD,
	MODE_MAX_FWD,
	MODE_NORMAL_REV,
	MODE_RND_FWD,
	MODE_RND_SNG_FWD,
	MODE_RND_SNG_FWD_REV
	
};

enum llmod_direction
{
	
	FORWARD		= 0,
	BACKWARD	= 1
	
};

struct llmod_state
{
	
	llmod_modes			current_mode;
	llmod_direction		current_direction;
	uint8_t				current_speed;
	llmod_modes			last_mode;
	
};



extern llmod_state llmod;

void run_llmod_statemachine( llmod_state* llmod_handle );
void init_llmod( llmod_state* llmod_handle );
void set_motor_direction( llmod_direction direction );
void set_motor_off();
void set_motor_speed( uint8_t speed );
void pulse_motor( uint32_t number_pulses, uint32_t pulse_length, uint32_t pulse_delay );

#ifdef ENABLE_MUSIC
void beep(unsigned int note, unsigned int duration);
void play();
#endif

#endif /* LLMOD_H_ */