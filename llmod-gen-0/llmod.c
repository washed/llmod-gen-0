/*
 * llmod.c
 *
 * Created: 12.09.2015 16:52:02
 *  Author: washed
 */ 



#include <avr/io.h>
#include "llmod.h"
#include "delay_mod.h"
#include "time.h"
#include "prng.h"

llmodTypeDef llmod;


void run_llmod_statemachine( llmodTypeDef* llmod_handle )
{
	uint32_t current_time;
	current_time = millis();
	
	switch ( llmod_handle->current_mode )
	{
		default:
		case MODE_IDLE:
		//In idle mode the motor is off:
		set_motor_off();
		break;
		
		case MODE_NORMAL_FWD:
		set_motor_speed(NORMAL_SPEED);
		set_motor_direction(FORWARD);
		break;
		
		case MODE_MAX_FWD:
		set_motor_speed(MAX_SPEED);
		set_motor_direction(FORWARD);
		break;
	
		//TODO: Add random modes!
		case MODE_RND_FWD:
		{
			if ( current_time - llmod_handle->last_rnd_change_time > llmod_handle->event_timer )
			{
				llmod_handle->last_rnd_change_time = current_time;
//				set_motor_speed( biased_random( llmod_handle->prng_handle, llmod_handle->biasTables[0] ) );
				set_motor_direction(FORWARD);
				//TODO: Make table for time bias!
				llmod_handle->event_timer = scale_and_offset( 5000 , 1000, get_random( llmod_handle->prng_handle ) );
			}
		}
		break;
		
		case MODE_RND_SNG_FWD:
		set_motor_off();
		break;
		
		case MODE_RND_SNG_FWD_REV:
		set_motor_off();
		break;
	}
}

void init_llmod( llmodTypeDef* llmod_handle )
{
	llmod_handle->current_mode = START_MODE;
	llmod_handle->current_direction = 0;
	llmod_handle->current_speed = 0;
	llmod_handle->last_rnd_change_time = 0;
	llmod_handle->event_timer = 0;
	llmod_handle->prng_handle = &prng1;
	
	//llmod_handle->biasTables[0] = &biasTableSpeed;
	//llmod_handle->biasTables[1] = &biasTableDirection;
	//llmod_handle->biasTables[2] = &biasTableTime;
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
	_delay_ms(pulse_delay);
	for ( uint32_t i = 0; i < number_pulses; i++ )
	{
		set_motor_speed(MAX_SPEED);
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