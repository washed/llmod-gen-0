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
#define F_CPU 8000000UL
#include "delay_mod.h"
#include "capsense.h"

/*
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

void beep(unsigned int note, unsigned int duration);
void play();
*/

#define MAX_SPEED		0xFF
#define MIN_SPEED		0x0

#define NORMAL_SPEED	0x9B

#define MAX_MODES		6
#define START_MODE		0

#define TEMP_TIMER_RST	1000

#define LOW				1
#define HIGH			0

typedef enum llmod_modes
{
	
	MODE_IDLE			= 0,
	MODE_NORMAL_FWD,
	MODE_MAX_FWD,
	MODE_NORMAL_REV,
	MODE_RND_FWD,
	MODE_RND_SNG_FWD,
	MODE_RND_SNG_FWD_REV
	
}llmod_modes;

typedef enum llmod_direction
{
	
	FORWARD		= 0,
	BACKWARD	= 1
	
}llmod_direction;

typedef struct llmod_state
{
	
	llmod_modes			current_mode;
	llmod_direction		current_direction;
	uint8_t				current_speed;
	llmod_modes			last_mode;
	
}llmod_state;

capsense_state_TypeDef capsense;
llmod_state llmod;

volatile uint32_t milliseconds = 0;
volatile uint16_t microseconds = 0;

volatile uint8_t buttonState;
volatile uint8_t lastButtonState = LOW;
uint32_t lastDebounceTime = 0;
uint32_t debounceDelay = 250;
uint32_t button_reading;

uint32_t lastModeSwitchTime = 0;
uint32_t modeSwitchDelay = 250;

uint32_t millis();
void main_state_machine();
void init_llmod( llmod_state* llmod_handle );
void set_motor_direction( llmod_direction direction );
void set_motor_off();
void set_motor_speed( uint8_t speed );
void pulse_motor( uint32_t number_pulses, uint32_t pulse_length, uint32_t pulse_delay );

int main(void)
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
	
	init_llmod(START_MODE);
	main_state_machine( &llmod );
	
	init_capsense( &capsense );
	int32_t capreading = 0;
	uint32_t capreference = 5;
	
    while(1)
    {
		capreading = capacitiveSensor( &capsense, 30 );
		
		if ( capreading >= capreference )
		{
			button_reading = HIGH;
		}
		else
		{
			button_reading = LOW;
		}
		
		if ((millis() - lastDebounceTime) > debounceDelay)
		{
			// whatever the reading is at, it's been there for longer
			// than the debounce delay, so take it as the actual current state:

			// if the button state has changed:
			if (button_reading != buttonState)
			{
				buttonState = button_reading;

				// only toggle the MODE if the new button state is HIGH
				if (buttonState == HIGH)
				{
					
					llmod.current_mode++;
					if ( llmod.current_mode >= MAX_MODES )
						llmod.current_mode = 0;
				}
			}
		}
		lastButtonState = button_reading;
		
		if ((millis() - lastModeSwitchTime) > modeSwitchDelay)
		{
			lastModeSwitchTime = millis();
			main_state_machine( &llmod );
		}
	}
}

void main_state_machine( llmod_state* llmod_handle )
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

ISR(TIMER1_COMPA_vect)
{
	microseconds += 10;
	if ( microseconds == 1000 )
	{
		microseconds = 0;
		milliseconds++;
	}
}

uint32_t millis()
{
	return milliseconds;
}

uint16_t micros()
{
	return microseconds;
}


/*
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
*/