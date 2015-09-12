/*
 * llmod.c
 *
 * Created: 12.09.2015 16:52:02
 *  Author: washed
 */ 



#include <avr/io.h>
#include "llmod.h"
#include "delay_mod.h"

llmod_state llmod;

uint32_t m_w = PRNG_SEED_W;
uint32_t m_z = PRNG_SEED_Z;

void run_llmod_statemachine( llmod_state* llmod_handle )
{
	//Pulse the motor to indicate state change
	if ( llmod_handle->current_mode != llmod_handle->last_mode )
	pulse_motor( llmod_handle->current_mode, 500, 500 );
	
	switch ( llmod_handle->current_mode )
	{
		default:
		case MODE_IDLE:
		//In idle mode the motor is off:
		set_motor_off();
		break;
		
		case MODE_NORMAL_FWD:
		//TODO: FIX DIRECTION WITH NEW MOTOR DRIVER!
		set_motor_speed(NORMAL_SPEED);
		set_motor_direction(BACKWARD);
		break;
		
		case MODE_MAX_FWD:
		set_motor_speed(MAX_SPEED);
		set_motor_direction(BACKWARD);
		break;
		
		case MODE_NORMAL_REV:
		set_motor_speed(NORMAL_SPEED);
		set_motor_direction(BACKWARD);
		break;
		
		//TODO: Add random modes!
		case MODE_RND_FWD:
		set_motor_off();
		break;
		
		case MODE_RND_SNG_FWD:
		set_motor_off();
		break;
		
		case MODE_RND_SNG_FWD_REV:
		set_motor_off();
		break;
	}
	llmod_handle->last_mode = llmod_handle->current_mode;
}

void init_llmod( llmod_state* llmod_handle )
{
	llmod_handle->current_mode = START_MODE;
	llmod_handle->current_direction = 0;
	llmod_handle->current_speed = 0;
}

void set_motor_direction( llmod_direction direction )
{
	switch ( direction )
	{
		case FORWARD:
		PORTB &= ~(1<<PB3);
		PORTB |= (1<<PB4);
		break;
		
		case BACKWARD:
		PORTB |= (1<<PB3);
		PORTB &= ~(1<<PB4);
		break;
	}
}

void pulse_motor( uint32_t number_pulses, uint32_t pulse_length, uint32_t pulse_delay )
{
	set_motor_speed(MAX_SPEED);
	for ( uint32_t i = 0; i < number_pulses; i++ )
	{
		set_motor_direction(BACKWARD);
		_delay_ms(pulse_length);
		set_motor_off();
		_delay_ms(pulse_delay);
	}
	
}

void set_motor_off()
{
	PORTB &= ~(1<<PB3);
	PORTB &= ~(1<<PB4);
	set_motor_speed(MIN_SPEED);
}

void set_motor_speed( uint8_t speed )
{
	if ( speed <= 0xFF )
	OCR0A = 0xFF - speed;
}

uint32_t getRandom()
{
	m_z = 36969L * (m_z & 65535L) + (m_z >> 16);
	m_w = 18000L * (m_w & 65535L) + (m_w >> 16);
	return (m_z << 16) + m_w;  /* 32-bit result */
}

#ifdef ENABLE_MUSIC
void play()
{
	beep(a, 500);
	beep(a, 500);
	beep(a, 500);
	beep(f, 350);
	beep(cH, 150);
	beep(a, 500);
	beep(f, 350);
	beep(cH, 150);
	beep(a, 650);
	
	_delay_ms(150);
	//end of first bit
	
	beep(eH, 500);
	beep(eH, 500);
	beep(eH, 500);
	beep(fH, 350);
	beep(cH, 150);
	beep(gS, 500);
	beep(f, 350);
	beep(cH, 150);
	beep(a, 650);
	
	_delay_ms(150);
	//end of second bit...
	
	beep(aH, 500);
	beep(a, 300);
	beep(a, 150);
	beep(aH, 400);
	beep(gSH, 200);
	beep(gH, 200);
	beep(fSH, 125);
	beep(fH, 125);
	beep(fSH, 250);
	
	_delay_ms(250);
	
	beep(aS, 250);
	beep(dSH, 400);
	beep(dH, 200);
	beep(cSH, 200);
	beep(cH, 125);
	beep(b, 125);
	beep(cH, 250);
	
	_delay_ms(250);
	
	beep(f, 125);
	beep(gS, 500);
	beep(f, 375);
	beep(a, 125);
	beep(cH, 500);
	beep(a, 375);
	beep(cH, 125);
	beep(eH, 650);
	
	//end of third bit... (Though it doesn't play well)
	//let's repeat it
	
	beep(aH, 500);
	beep(a, 300);
	beep(a, 150);
	beep(aH, 400);
	beep(gSH, 200);
	beep(gH, 200);
	beep(fSH, 125);
	beep(fH, 125);
	beep(fSH, 250);
	
	_delay_ms(250);
	
	beep(aS, 250);
	beep(dSH, 400);
	beep(dH, 200);
	beep(cSH, 200);
	beep(cH, 125);
	beep(b, 125);
	beep(cH, 250);
	
	_delay_ms(250);
	
	beep(f, 250);
	beep(gS, 500);
	beep(f, 375);
	beep(cH, 125);
	beep(a, 500);
	beep(f, 375);
	beep(cH, 125);
	beep(a, 650);
	//end of the song
	
}

void beep(unsigned int note, unsigned int duration)
{
	int i;
	long delay = (long)(500000/note);  //This is the semiperiod of each note.
	long time = (long)((duration*1000)/(delay*2));  //This is how much time we need to spend on the note.
	for (i=0;i<time;i++)
	{
		PORTB |= (1<<PB3);     //Set P1.2...
		_delay_us(delay);   //...for a semiperiod...
		PORTB &= ~(1<<PB3);   //...then reset it...
		_delay_us(delay);   //...for the other semiperiod.
	}
	_delay_ms(20); //Add a little delay to separate the single notes
}
#endif