/*
 * sonar.c
 *
 *  Created on: Mar 6, 2018
 *      Author: msueppel
 */

#include "sonar.h"
#include <stdio.h>
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <lcd.h>
#include <timer.h>
#include <math.h>
#include <stdbool.h>    //has to be above #include interrupt.h
#include "driverlib/interrupt.h"

volatile int rising_time; // start time of the return pulse
volatile int falling_time; // end time of the return pulse
volatile int update_flag = 1;
volatile int overflow;
int distance = 0, difference = 0, milliseconds = 0;

void TIMER3B_Handler(void)
{
    // TIMER3_ICR_R = TIMER_ICR_TBTOCINT; //clear flag
    if (TIMER3_RIS_R &= 0x400)
    {
        int event_time = TIMER3_TBR_R;
        if (update_flag == 0)
        {
            rising_time = event_time;
            update_flag = 1;
        }
        else if (update_flag == 1)
        {
            falling_time = event_time;
            update_flag = 2;
        }
    }
    TIMER3_ICR_R |= 0x400;
}

void sonar_init()
{
    //GPIO INIT
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;  //provides clock to port B
    GPIO_PORTB_DEN_R |= 0b00001000;         //enable pin 3
    GPIO_PORTB_DIR_R |= 0b00001000;         //sets pin 3 to output
    GPIO_PORTB_AFSEL_R |= 0b01000;        //port B pin 3 for alternate functions
    GPIO_PORTB_PCTL_R |= 0x7000;
}
void timer3B_init()
{
    //TIMER3 INIT
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3;
    TIMER3_CTL_R &= ~0x100;         //disable but preserve other bits
    TIMER3_CTL_R |= 0xC00;
    TIMER3_CFG_R |= 0x4;            //16-bit timer
    TIMER3_TBMR_R |= 0b10111;
    TIMER3_TBILR_R |= 0xFFFF;        //give max value for upper bound;
    TIMER3_ICR_R |= 0xF00;
    TIMER3_IMR_R |= 0b10000000000; // Timer B Capture Mode Event Interrupt Mask, enabled

    IntRegister(INT_TIMER3B, TIMER3B_Handler);
    NVIC_EN1_R |= 0b10000; //lab4a timer init, interrupt #36 (EN1 -> bits 32-63)
    IntMasterEnable(); //intialize global interrupts

    TIMER3_CTL_R |= 0b110100000000; //bits 11:10 are both edge select (=3), bit 8 is enable (1) for TimerB  
}
void send_pulse()
{
    GPIO_PORTB_AFSEL_R &= (~0x08); //disables alternate functions
    GPIO_PORTB_DIR_R |= 0x08; // set PB3 as output
    GPIO_PORTB_DATA_R |= 0x08; // set PB3 to high

    timer_waitMicros(5); // wait at least 5 microseconds based on data sheet

    GPIO_PORTB_DATA_R &= 0xF7; // set PB3 to low
    GPIO_PORTB_DIR_R &= 0xF7; // set PB3 as input
    GPIO_PORTB_AFSEL_R |= 0x08; // enables alternate functions
}

int get_sonar(void)
{

   // lcd_init();

    send_pulse();
    if (falling_time < rising_time)
    {
        difference = rising_time - falling_time; //returns # clock cycles
        overflow++;
    }
    else
    {
        difference = falling_time - rising_time;
    }
    update_flag = 0;  //reset
    distance = (difference * 34) / 32000; //32khz
   // milliseconds = difference / 16000;
   // lcd_printf("Distance: %d\nOverflow: %d\nTime: %d ms\nWidth: %d", distance,
    //           overflow, milliseconds, difference);
    //timer_waitMillis(250);

    return distance;
}
