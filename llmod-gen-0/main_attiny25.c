/*******************************************************************************
*   Atmel Corporation:  http://www.atmel.com
*   Support email:  touch@atmel.com
******************************************************************************/
/*  License
*   Copyright (c) 2010, Atmel Corporation All rights reserved.
*
*   Redistribution and use in source and binary forms, with or without
*   modification, are permitted provided that the following conditions are met:
*
*   1. Redistributions of source code must retain the above copyright notice,
*   this list of conditions and the following disclaimer.
*
*   2. Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*
*   3. The name of ATMEL may not be used to endorse or promote products derived
*   from this software without specific prior written permission.
*
*   THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
*   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
*   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
*   SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
*   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
*   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*----------------------------------------------------------------------------
compiler information
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
include files
----------------------------------------------------------------------------*/
#if defined(__ICCAVR__)
   #include <ioavr.h>
   #include <intrinsics.h>
#else
   #error 'This compiler is not supported at the moment.'
#endif

#include "touch_api_2kdevice.h"
/*----------------------------------------------------------------------------
manifest constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
type definitions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
prototypes
----------------------------------------------------------------------------*/
/*  initialise host app, pins, watchdog, etc    */
static void init_system( void );
/*  configure timer ISR to fire regularly   */
static void init_timer_isr( void );
/*  Assign the parameters values to global configuration parameter structure    */
static void qt_set_parameters( void );
/*  Configure the sensors */
static void config_sensors(void);

/*  Configure the sensors for  Keys configuration */
static void config_keys(void);

#if defined(__ICCAVR__)
  #pragma vector=TIM0_COMPA_vect
  __interrupt static void timer_isr( void );
#endif

/*----------------------------------------------------------------------------
Structure Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
macros
----------------------------------------------------------------------------*/
/*  Total ticks per msec.  */
#define TICKS_PER_MS                4u
#define GET_SENSOR_STATE(SENSOR_NUMBER) qt_measure_data.qt_touch_status.sensor_states[(SENSOR_NUMBER/8)] & (1 << (SENSOR_NUMBER % 8))
#define GET_ROTOR_SLIDER_POSITION(ROTOR_SLIDER_NUMBER) qt_measure_data.qt_touch_status.rotor_slider_values[ROTOR_SLIDER_NUMBER]


/*----------------------------------------------------------------------------
global variables
----------------------------------------------------------------------------*/
/* Timer period in msec. */
uint16_t qt_measurement_period_msec = 25u;

/*----------------------------------------------------------------------------
extern variables
----------------------------------------------------------------------------*/
/* This configuration data structure parameters if needs to be changed will be
changed in the qt_set_parameters function */
extern qt_touch_lib_config_data_t qt_config_data;
/* measurement data */
extern qt_touch_lib_measure_data_t qt_measure_data;

/* Get sensor delta values */
extern int16_t qt_get_sensor_delta( uint8_t sensor);

#ifdef QTOUCH_STUDIO_MASKS
extern TOUCH_DATA_T SNS_array[2];
extern TOUCH_DATA_T SNSK_array[2];
#endif

/*----------------------------------------------------------------------------
static variables
----------------------------------------------------------------------------*/
/* flag set by timer ISR when it's time to measure touch */
static volatile uint8_t time_to_measure_touch = 0u;
/* current time, set by timer ISR */
static volatile uint16_t current_time_ms_touch = 0u;

/*============================================================================
Name    :   main
------------------------------------------------------------------------------
Purpose :   main code entry point
Input   :
Output  :
Notes   :
============================================================================*/
int main( void )
{
   /*status flags to indicate the re-burst for library*/
   uint16_t status_flag = 0u;
   uint16_t burst_flag = 0u;
#ifdef QTOUCH_STUDIO_MASKS
  SNS_array[0]=0x01;
  SNS_array[1]=0x00;
  SNSK_array[0]=0x02;
  SNSK_array[1]=0x00;
#endif

    /* initialise host app, pins, watchdog, etc */
   init_system();

   /* Configure the Sensors as keys or Keys With Rotor/Sliders in this function */
   config_sensors();

   /* initialise touch sensing */
   qt_init_sensing();

   /*  Set the parameters like recalibration threshold, Max_On_Duration etc in this function by the user */
   qt_set_parameters( );

   /* configure timer ISR to fire regularly */
   init_timer_isr();

   /*  Address to pass address of user functions   */
   /*  This function is called after the library has made capacitive measurements,
   *   but before it has processed them. The user can use this hook to apply filter
   *   functions to the measured signal values.(Possibly to fix sensor layout faults)    */
    qt_filter_callback = 0;

   /* enable interrupts */
   __enable_interrupt();

   /* loop forever */
   for( ; ; )
   {
      if( time_to_measure_touch )
      {
         /*  clear flag: it's time to measure touch  */
         time_to_measure_touch = 0u;

         do {
            /*  one time measure touch sensors    */
            status_flag = qt_measure_sensors( current_time_ms_touch );
            burst_flag = status_flag & QTLIB_BURST_AGAIN;

            /*Time critical host application code goes here*/

         }while (  burst_flag) ;
  			
      }

      /*  Time Non-critical host application code goes here  */

   }
}

/*============================================================================
Name    :   qt_set_parameters
------------------------------------------------------------------------------
Purpose :   This will fill the default threshold values in the configuration
data structure.But User can change the values of these parameters .
Input   :
Output  :
Notes   :   initialize configuration data for processing
============================================================================*/
static void qt_set_parameters( void )
{
   qt_config_data.qt_di                     = DEF_QT_DI;
   qt_config_data.qt_neg_drift_rate         = DEF_QT_NEG_DRIFT_RATE;
   qt_config_data.qt_pos_drift_rate         = DEF_QT_POS_DRIFT_RATE;
   qt_config_data.qt_max_on_duration        = DEF_QT_MAX_ON_DURATION;
   qt_config_data.qt_drift_hold_time        = DEF_QT_DRIFT_HOLD_TIME;
   qt_config_data.qt_recal_threshold        = DEF_QT_RECAL_THRESHOLD;
   qt_config_data.qt_pos_recal_delay        = DEF_QT_POS_RECAL_DELAY;
}

/*============================================================================
Name    :   config_sensors
------------------------------------------------------------------------------
Purpose :   Configure the sensors
Input   :
Output  :
Notes   :
============================================================================*/
static void config_sensors(void)
{

      config_keys();
 
}
/*============================================================================
Name    :   config_keys
------------------------------------------------------------------------------
Purpose :   Configure the sensors as keys only
Input   :
Output  :
Notes   :
============================================================================*/

static void config_keys(void)
{
  qt_enable_key( CHANNEL_0, NO_AKS_GROUP, 10u, HYST_6_25 );
  
  #if(QT_NUM_CHANNELS >= 2u)
  qt_enable_key( CHANNEL_1, NO_AKS_GROUP, 10u, HYST_6_25 );
  #endif
  
}


/*============================================================================
Name    :   init_timer_isr
------------------------------------------------------------------------------
Purpose :   configure timer ISR to fire regularly
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/
static void init_timer_isr( void )
{
   /*  set timer compare value (how often timer ISR will fire) */
   OCR0A = ( TICKS_PER_MS * qt_measurement_period_msec);
   /*  enable timer ISR */
   TIMSK |= (1u << OCIE0A);
   /*  timer prescaler = system clock / 1024  */
   TCCR0B |= (1u << CS02) | (1u << CS00);
   /*  timer mode = CTC (count up to compare value, then reset)    */
   TCCR0A |= (1u << WGM01);
}

/*============================================================================
Name    :   init_system
------------------------------------------------------------------------------
Purpose :   initialise host app, pins, watchdog, etc
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/
static void init_system( void )
{
   /*  run at 4MHz */
   CLKPR = 0x80u;
   CLKPR = 0x01u;

   /*  disable pull-ups    */
   MCUCR |= (1u << PUD);
   //   MCUCR |= (1u << JTD);
   //   MCUCR |= (1u << JTD);

}

/*============================================================================
Name    :   timer_isr
------------------------------------------------------------------------------
Purpose :   timer 1 compare ISR
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/
#if defined(__ICCAVR__)
   #pragma vector=TIM0_COMPA_vect
   __interrupt static void timer_isr( void )
#endif
{
   /*  set flag: it's time to measure touch    */
   time_to_measure_touch = 1u;
   /*  update the current time */
   current_time_ms_touch += qt_measurement_period_msec;
}
