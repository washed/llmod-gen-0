/*
 * capsense.c
 *
 * Created: 18.06.2015 23:58:36
 *  Author: washed
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "capsense.h"
#include "delay_mod.h"
#include "time.h"

capsense_state_TypeDef capsense;

void debounce_integrate( capsense_state_TypeDef* capsense_handle )
{
	if (capsense_handle->capsense_raw == 0)
	{
		if (capsense_handle->integrator > 0)
			capsense_handle->integrator--;
	}
	else if (capsense_handle->integrator < MAXIMUM)
		capsense_handle->integrator += 2;
		
	if (capsense_handle->integrator == 0)
		capsense_handle->capsense_debounced = 0;
	else if (capsense_handle->integrator >= MAXIMUM)
	{
		capsense_handle->capsense_debounced = 1;
		capsense_handle->integrator = MAXIMUM;  /* defensive code if integrator got corrupted */
	}
}

void init_capsense( capsense_state_TypeDef* capsense_handle )
{
	//Init variables for capsense:
	capsense_handle->chargedischarge_cycles = 50;
	capsense_handle->sensecycle_state = 0;
	capsense_handle->threshold = 0;
}

void sense_cycle( capsense_state_TypeDef* capsense_handle )
{
	
	//cli();

	//Start the loop:
	for ( uint32_t i = 0; i < capsense_handle->chargedischarge_cycles; )
	{
		if (SenseOneCycle(capsense_handle))
			i++;
	}
	
	capsense_handle->completed_cycles++;
	capsense_handle->calibration_sum += capsense_handle->sensed_pulsewidth;
	capsense_handle->calibration = capsense_handle->calibration_sum / capsense_handle->completed_cycles;	
	
	if ( capsense_handle->sensed_pulsewidth > capsense_handle->calibration )
		capsense_handle->capsense_realcount = capsense_handle->sensed_pulsewidth - capsense_handle->calibration;
	else
		capsense_handle->capsense_realcount = 0;
		
	if ( capsense_handle->capsense_realcount > capsense_handle->threshold )
		capsense_handle->capsense_raw = 1;
	else
		capsense_handle->capsense_raw = 0;

	//Restart the measurement:
	capsense_handle->sensecycle_state = CAPSENSE_PREPARE;
	//sei();
}

uint8_t SenseOneCycle( capsense_state_TypeDef* capsense_handle )
{
	uint8_t rVal = 0;
	switch (capsense_handle->sensecycle_state)
	{
		case CAPSENSE_PREPARE:
		{
			//Reset counter
			capsense_handle->sensed_pulsewidth = 0;
			
			//Tristate pulse pin:
			DDRB &= ~(1<<DDB1);
			PORTB &= ~(1<<PORTB1);
			
			//Tristate sense pin (input, no pull-up):
			DDRB &= ~(1<<DDB2);
			PORTB &= ~(1<<PORTB2);
			
			//Pulse pin as output low:
			DDRB |= (1<<DDB1);
			PORTB &= ~(1<<PORTB1);
			
			//Wait for it to discharge:
			capsense_handle->sensecycle_state = CAPSENSE_CHARGE;
			_delay_us(50);
		}
		break;
		
		case CAPSENSE_CHARGE:
		{
			//Set the pulse pin high:
			PORTB |= (1<<PORTB1);
			capsense_handle->sensecycle_state = CAPSENSE_WAITHIGH;
			_delay_us(50);
		}
		break;
		
		case CAPSENSE_WAITHIGH:
		{
			//Wait for the sense pin to go high:
			if ( (PINB & (1<<PINB2)) )
			{
				//Charge up completely, we need an intermediate step here (see ATTTiny85 datasheet section 10.2.3)
				PORTB |= (1<<PORTB2);
				DDRB |= (1<<DDB2);
				PORTB |= (1<<PORTB2);
				
				//Tristate sense pin:
				DDRB &= ~(1<<DDB2);
				PORTB &= ~(1<<PORTB2);
				
				//Set the pulse pin low:
				PORTB &= ~(1<<PORTB1);

				capsense_handle->sensecycle_state = CAPSENSE_WAITLOW;
				_delay_us(50);
			}
			else
			{
				capsense_handle->sensed_pulsewidth++;
			}
		}
		break;
		
		case CAPSENSE_WAITLOW:
		{
			//Wait for the sense pin to go low:
			if( !(PINB & (1<<PINB2)) )
			{
				//Discharge completely:
				DDRB |= (1<<DDB2);
				PORTB &= ~(1<<PORTB2);
				
				//Tristate sense pin:
				 
				DDRB &= ~(1<<DDB2);
				PORTB &= ~(1<<PORTB2);
				rVal = 1;
				capsense_handle->sensecycle_state = CAPSENSE_CHARGE;
				_delay_us(50);
			}
			else
			{
				capsense_handle->sensed_pulsewidth++;
			}
		}
		break;
	}
	
	return rVal;
}