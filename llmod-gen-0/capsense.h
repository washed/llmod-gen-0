/*
 * capsense.h
 *
 * Created: 19.06.2015 00:01:57
 *  Author: washed
 */ 


#ifndef CAPSENSE_H_
#define CAPSENSE_H_

#include <avr/io.h>

#define F_CPU 8000000UL

#define LOW				1
#define HIGH			0

typedef struct capsense_state_TypeDef capsense_state_TypeDef;

struct capsense_state_TypeDef
{
	int8_t		error;
	uint32_t	leastTotal;
	uint32_t	CS_Timeout_Micros;
	uint32_t	CS_Autocal_Millis;
	uint32_t	lastCal;
	uint32_t	total;
	
};

extern capsense_state_TypeDef capsense;

uint8_t debounce( capsense_state_TypeDef* capsense_handle );
int32_t capacitiveSensor( capsense_state_TypeDef* capsense_handle, uint8_t samples );
int32_t capacitiveSensorRaw( capsense_state_TypeDef* capsense_handle, uint8_t samples );
void reset_CS_AutoCal( capsense_state_TypeDef* capsense_handle );
void set_CS_AutocaL_Millis(  capsense_state_TypeDef* capsense_handle, uint32_t autoCal_millis );
void set_CS_Timeout_Micros( capsense_state_TypeDef* capsense_handle, uint32_t timeout_millis);
void init_capsense();
int32_t SenseOneCycle( capsense_state_TypeDef* capsense_handle );

#endif /* CAPSENSE_H_ */