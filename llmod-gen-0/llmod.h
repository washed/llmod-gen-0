/*
 * llmod.h
 *
 * Created: 12.09.2015 16:54:18
 *  Author: washed
 */ 


#ifndef LLMOD_H_
#define LLMOD_H_

#define F_CPU 8000000UL

#define MODE_SWITCH_TIME	250
#define MAX_SPEED			0xFF
#define MIN_SPEED			0x0

#define NORMAL_SPEED		0x9B

#define MAX_MODES			5
#define START_MODE			0

#define TEMP_TIMER_RST		1000

#define PRNG_SEED_W			1
#define PRNG_SEED_Z			2

#define PROB_NUMBERS_COUNT 256


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
	uint32_t			last_rnd_change_time;
	uint32_t			event_timer;
	uint32_t			m_w;
	uint32_t			m_z;
};


typedef struct probabilityPairTypeDef probabilityPairTypeDef;

struct probabilityPairTypeDef
{

	uint8_t probability;
	uint8_t number;

};

extern probabilityPairTypeDef probabilityTable[PROB_NUMBERS_COUNT];
extern llmod_state llmod;


uint32_t get_random( llmod_state* llmod_handle );
int32_t scale_and_offset( uint32_t scale, uint32_t offset, uint32_t input );
uint8_t prng_is_seeded( llmod_state* llmod_handle );
void seed_random( llmod_state* llmod_handle, uint32_t seed );
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