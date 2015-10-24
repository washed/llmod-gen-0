/*
 * GccApplication1.c
 *
 * Created: 05.06.2015 08:40:06
 *  Author: washed
 */ 


/*
TODO:

-Replace damaged motor control IC -> Solved, Pins were shorted to GND plane. See comment about OSHPark PCBs.
-Verify touch button functionality and experiment with different laminated sensors
-Ground connection for laminated touch sensors?
-Add a source of randomness
-Add random modes
-Test everything
-Characterize battery life with different types of batteries
-Test compatibility/order the other battery clips
-Test PCB mounting with countersunk M3 screw
-Are the "fins" on the PCB necessary to hold it in place?
-Update board shape to improve fit
-Next generation?

-REMEMBER: OSHPARK CANT HANDLE SMALL DISTANCES!!! BIGGER CLEARANCE!!!

*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"
#include "capsense.h"
#include "llmod.h"
#include "time.h"
#include "delay_mod.h"

//#define DEBUG_CAPSENSE_DEBOUNCE
//#define DEBUG_CAPSENSE_COUNT

#if (defined(DEBUG_CAPSENSE_DEBOUNCE) && defined (DEBUG_CAPSENSE_COUNT))
#error "Only one of those capsense debug modes can be used at any time!"
#endif

#if (defined(DEBUG_CAPSENSE_DEBOUNCE) || defined (DEBUG_CAPSENSE_COUNT))
#warning "Cap sense debugging is activated!"
#endif

uint32_t current_time = 0;
uint32_t last_time_sense = 0;
uint32_t last_time_switch = 0;

int main(void)
{
	sys_init();

	init_llmod( &llmod );
	init_capsense( &capsense );

#ifdef DEBUG_CAPSENSE_DEBOUNCE
	uint32_t current_micros;
	uint32_t delay1 = 50;
	while (1)
	{
		current_time = millis();
		if ( current_time - last_time_sense > SENSE_INTERVAL )
		{
			
			last_time_sense = current_time;		
			if ( debounce_capsense() == 1 )
			{
				//for ( uint32_t i = 0; i < cap_value; i++ )
				{
					current_micros = micros();
					PORTB |= (1<<PB3);
					while ( micros() - current_micros < delay1 );
					
					current_micros = micros();
					PORTB &= ~(1<<PB3);
					while ( micros() - current_micros < delay1 );
				}

			}
			
		}
	}
#elif defined(DEBUG_CAPSENSE_COUNT)
	uint32_t cap_value;
	while (1)
	{
		current_time = millis();
		if ( current_time - last_time_sense > SENSE_INTERVAL )
		{
			
			last_time_sense = current_time;
			cap_value = capacitiveSensor( &capsense, DEFAULT_SAMPLES );
			if ( cap_value >= 1 )
			{
				for ( uint32_t i = 0; i < cap_value; i++ )
				{
					current_micros = micros();
					PORTB |= (1<<PB3);
					while ( micros() - current_micros < delay1 );
					
					current_micros = micros();
					PORTB &= ~(1<<PB3);
					while ( micros() - current_micros < delay1 );
				}

			}
			
		}
	}
#else
	while (1)
	{
		current_time = millis();
		if ( (current_time - last_time_sense) > SENSE_INTERVAL )
		{
			last_time_sense = current_time;
			if ( debounce_capsense() == 1 )
			{
				//Switch was pressed, has enough time passed to change the mode?
				current_time = millis();
				if (( current_time - last_time_switch) > MODE_SWITCH_TIME)
				{
					last_time_switch = current_time;
					llmod.current_mode++;
					if ( llmod.current_mode > MAX_MODES )
						llmod.current_mode = 0;
						
					if ( (llmod.current_mode == MODE_RND_FWD) && (llmod.current_mode == MODE_RND_SNG_FWD) && (llmod.current_mode == MODE_RND_SNG_FWD_REV) )
					{
						//Seed the PRNG with the current system time:
						seed_random( &llmod, millis() );
					}
					pulse_motor( llmod.current_mode, 100, 50 );
				}
			}
		}
		run_llmod_statemachine( &llmod );
	}
	#endif
}

void sys_init()
{
	//Disable global Interrupts
	cli();
	
	//Set clock prescaler to 1 (8MHz)
	CLKPR = 1<<CLKPCE;
	CLKPR = 0;
	
	//Enable PLL, wait for it to achieve lock and then enable the clock for Timer/Counter 1:
	PLLCSR = (1 << PLLE);
	while ( (PLLCSR & (1<<PLOCK)) == 0 );
	PLLCSR |= (1 << PCKE);

	//Configure Ports PB0, PB3 and PB4 as output for motor control:
	DDRB |= (1<<DDB0) | (1<<DDB3) | (1<<DDB4);
	
	//1ms CTC interrupt:
	TCCR1 = (1<<CTC1)|(5<<CS10);   // CTC  mode
	OCR1C = 39;  // 10µs
	TIMSK |= (1<<OCIE1A);
	
	sei();
	
	//PWM for speed setting:
	TCCR0A = 3<<COM0A0 | 3<<WGM00;
	TCCR0B = 0<<WGM02 | 2<<CS00;
}