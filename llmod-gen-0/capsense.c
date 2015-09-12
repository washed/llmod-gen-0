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

volatile uint8_t buttonState;
volatile uint8_t lastButtonState = LOW;
uint32_t lastDebounceTime = 0;
uint32_t debounceDelay = 250;
uint32_t button_reading;

uint8_t debounce( capsense_state_TypeDef* capsense_handle )
{
	uint8_t output = 0;
	int32_t capreading = 0;
	uint32_t capreference = 5;
		
	capreading = capacitiveSensor( capsense_handle, 30 );
	
	if ( capreading >= capreference )
	{
		button_reading = HIGH;
	}
	else
	{
		button_reading = LOW;
	}
	
	//TODO: Completely rework this horrible "debounce"-code!!!
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
				output = 1;
			}
		}
	}
	lastButtonState = button_reading;
	return output;
}

int32_t capacitiveSensor( capsense_state_TypeDef* capsense_handle, uint8_t samples )
{
	capsense_handle->total = 0;
	if (samples == 0) return 0;
	if (capsense_handle->error < 0) return -1;            // bad pin

	for (uint8_t i = 0; i < samples; i++)
	{    // loop for samples parameter - simple lowpass filter
		if (SenseOneCycle(capsense_handle) < 0)  return -2;   // variable over timeout
	}

		// only calibrate if time is greater than CS_AutocaL_Millis and total is less than 10% of baseline
		// this is an attempt to keep from calibrating when the sensor is seeing a "touched" signal

		if ( (millis() - capsense_handle->lastCal > capsense_handle->CS_Autocal_Millis) && abs(capsense_handle->total  - capsense_handle->leastTotal) < (int)(.10 * (float)capsense_handle->leastTotal) ) {

			capsense_handle->leastTotal = 0x0FFFFFFFL;          // reset for "autocalibrate"
			capsense_handle->lastCal = millis();
		}

	// routine to subtract baseline (non-sensed capacitance) from sensor return
	if (capsense_handle->total < capsense_handle->leastTotal) capsense_handle->leastTotal = capsense_handle->total;                 // set floor value to subtract from sensed value
	return(capsense_handle->total - capsense_handle->leastTotal);
}

int32_t capacitiveSensorRaw( capsense_state_TypeDef* capsense_handle, uint8_t samples )
{
	capsense_handle->total = 0;
	if (samples == 0) return 0;
	if (capsense_handle->error < 0) return -1;                  // bad pin - this appears not to work

	for (uint8_t i = 0; i < samples; i++) {    // loop for samples parameter - simple lowpass filter
		if (SenseOneCycle(capsense_handle) < 0)  return -2;   // variable over timeout
	}

	return capsense_handle->total;
}

void reset_CS_AutoCal( capsense_state_TypeDef* capsense_handle )
{
	capsense_handle->leastTotal = 0x0FFFFFFFL;
}

void set_CS_AutocaL_Millis( capsense_state_TypeDef* capsense_handle, uint32_t autoCal_millis )
{
	capsense_handle->CS_Autocal_Millis = autoCal_millis;
}

void set_CS_Timeout_Micros( capsense_state_TypeDef* capsense_handle, uint32_t timeout_micros)
{
	capsense_handle->CS_Timeout_Micros = timeout_micros;
}

void init_capsense( capsense_state_TypeDef* capsense_handle )
{
	//Init variables for capsense:
	capsense_handle->error = 1;

	capsense_handle->CS_Timeout_Micros = 300;
	capsense_handle->CS_Autocal_Millis = 20000;
		
	capsense_handle->leastTotal = 0x0FFFFFFFL;
	capsense_handle->lastCal = millis();
}

int32_t SenseOneCycle( capsense_state_TypeDef* capsense_handle )
{
	volatile uint32_t lastIncrementTime = 0;
	uint32_t minIncrementDelay = 10; //µs
	volatile uint32_t cycleTime = 0;
	uint32_t cycleTotal = 0;
	cli();
	PORTB &= ~(1<<PORTB1);
	DDRB &= ~(1<<DDB2);
	DDRB |= (1<<DDB2);
	PORTB &= ~(1<<PORTB2);
	_delay_us(10);
	DDRB &= ~(1<<DDB2);
	PORTB |= (1<<PORTB1);
	sei();
	
	while ( !(PINB & (1<<PINB2)) && ( cycleTotal < capsense_handle->CS_Timeout_Micros ) )
	{
		//Timestamp of the current measurement:
		cycleTime = (millis()*1000 + micros());
		
		//Increment the value only if it is time to increment:
		if ( (cycleTime - lastIncrementTime) >= minIncrementDelay )
		{
			cycleTotal += minIncrementDelay;
			lastIncrementTime = cycleTime;
		}
	}

	//Add this cycles total time to the accumulated total:
	capsense_handle->total += cycleTotal;

	cli();
	PORTB |= (1<<PORTB2);
	DDRB |= (1<<DDB2);
	PORTB |= (1<<PORTB2);
	DDRB &= ~(1<<DDB2);
	PORTB &= ~(1<<PORTB1);
	sei();

	DDRB |= (1<<DDB2);
	PORTB &= ~(1<<PORTB2);
	_delay_us(10);
	DDRB &= ~(1<<DDB2);
	
	_delay_us(100);

	if (cycleTotal >= capsense_handle->CS_Timeout_Micros)
	{
		return -2;     // total variable over timeout
	}
	else
	{
		return 1;
	}
}