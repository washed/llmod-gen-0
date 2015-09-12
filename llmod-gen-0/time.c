/*
 * time.c
 *
 * Created: 12.09.2015 17:07:59
 *  Author: washed
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "time.h"

static volatile uint32_t milliseconds = 0;
static volatile uint16_t microseconds = 0;

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