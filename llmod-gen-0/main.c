/*
 * GccApplication1.c
 *
 * Created: 05.06.2015 08:40:06
 *  Author: washed
 */ 


/*
TODO:

-Replace damaged motor control IC
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

*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"
#include "capsense.h"
#include "llmod.h"
#include "time.h"

uint32_t lastModeSwitchTime = 0;
uint32_t modeSwitchDelay = 250;

int main(void)
{
	sys_init();

	init_llmod( &llmod );
	init_capsense( &capsense );

    while(1)
    {
		if ( debounce( &capsense ) != 0 )
		{
			llmod.current_mode++;
			if ( llmod.current_mode >= MAX_MODES )
				llmod.current_mode = 0;
		}

		if ((millis() - lastModeSwitchTime) > modeSwitchDelay)
		{
			lastModeSwitchTime = millis();
			run_llmod_statemachine( &llmod );
		}
	}
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

