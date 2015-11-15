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

#define LOW						1
#define HIGH					0

#define CAPSENSE_THRESHOLD		15
#define DEFAULT_SAMPLES			5

#define DEBOUNCE_TIME		0.3
#define SAMPLE_FREQUENCY	100
#define DROPBACK			0
#define MAXIMUM				2*(DEBOUNCE_TIME * SAMPLE_FREQUENCY)

typedef struct capsense_state_TypeDef capsense_state_TypeDef;
typedef struct Debouncer_TypeDef Debouncer_TypeDef;
typedef enum capsense_sensecycle_stateENUM capsense_sensecycle_stateENUM;

enum capsense_sensecycle_stateENUM
{
	CAPSENSE_PREPARE = 0,
	CAPSENSE_CHARGE,
	CAPSENSE_WAITHIGH,
	CAPSENSE_WAITLOW
};
struct capsense_state_TypeDef
{
	uint32_t						threshold;
	uint32_t						capsense_realcount;
	uint8_t							capsense_raw;
	uint8_t							capsense_debounced;
	uint32_t						integrator;
	uint32_t						chargedischarge_cycles;
	uint32_t						sensed_pulsewidth;
	uint32_t						calibration;
	uint32_t						completed_cycles;
	uint32_t						calibration_sum;
	capsense_sensecycle_stateENUM	sensecycle_state;
};

extern capsense_state_TypeDef capsense;

uint8_t SenseOneCycle( capsense_state_TypeDef* capsense_handle );

#endif /* CAPSENSE_H_ */
