/*
 * servo.c
 *
 *  Created on: Mar 20, 2018
 *      Author: msueppel
 */
#include "servo.h"
#include "button.h"
#include <stdio.h>
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <lcd.h>
#include <timer.h>
#include <math.h>
#include <stdbool.h>    //has to be above #include interrupt.h
#include "driverlib/interrupt.h"

unsigned period = 0x4E200;
unsigned midWidth = 0x5DC0;
int total = 90;
void servo_init(){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;  //provides clock to port B
    GPIO_PORTB_DEN_R |= 0b00100000;         //enables pin 5
    GPIO_PORTB_DIR_R |= 0b00100000; 		//sets pin 5 to output
    GPIO_PORTB_AFSEL_R |= 0b100000;       //enables alternate functions
    GPIO_PORTB_PCTL_R |= 0x700000;           //enables rx and tx
}
void timer1B_init(){
	//from page 724 of datasheet - setting up for PWM mode
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    TIMER1_CTL_R &= ~(TIMER_CTL_TBEN); //disable timerB to allow us to change settings
    TIMER1_CFG_R |= TIMER_CFG_16_BIT; //set to 16 bit timer
    TIMER1_TBMR_R = TIMER_TBMR_TBMR_PERIOD; //set for periodic mode, down count
    TIMER1_CTL_R |= 0b110000000000;
    TIMER1_TBMR_R = 0b1011;
    TIMER1_TBPR_R = 0x04 & 0xFF; //second 8 bits of 0x4E200
    TIMER1_TBILR_R = 0xE200 & 0xFFFF; //first 16 of 0x4E200 in ILR
	TIMER1_TBPMR_R = ((period - midWidth) >> 16) & 0xFF;
    TIMER1_TBMATCHR_R = period & 0xFFFF; //CHANGE ANDING SOME SORT OF VARIABLE THAT WILL BE SUBRACTED
    TIMER1_CTL_R |= TIMER_CTL_TBEN; //enable timerB
}

void move_servo(int degrees){
	int pulseWidth;

	//pulseWidth = degrees;
//    TIMER1_TBMATCHR_R = period - pulseWidth;                              // TIMER1_TBMATCHR_R
//    timer_waitMillis(250);
	//pulseWidth =(155.5556*(total+degrees)+6000); //Tested with cybot 15
	//pulseWidth =(155.5556*(degrees)+7866.667); //Tested with cybot 17 and 3
    //pulseWidth =(146.92*(degrees+total)+7555.5556); //Tested with cybot 2
  //  pulseWidth =(157.7125*(degrees+total)+7000); //Tested with cybot 2
	pulseWidth =(161.1111*(degrees)+9500); //Tested with cybot 4

	if(pulseWidth>=38500){
	    pulseWidth=38500;
	    total=180;
	}
	if(pulseWidth<=9500){
	    pulseWidth=9500;
	    total=0;
	}
	else{
	    total+=degrees;
	}
	TIMER1_TBMATCHR_R = period - pulseWidth;                              // TIMER1_TBMATCHR_R
	//timer_waitMillis(250);								//wait
}

//void main(void) {
//    timer1B_init();
//    servo_init();
//    button_init();
//    lcd_init();
//	int direction=1;
//	int button = 0;
//	move_servo(0);
//    while(1){
//		button = button_getButton();
//		if(button ==1){
//		    move_servo(1*direction);    //move 1 degree
//		}
//		else if(button ==2){
//		    move_servo(2.5*direction);  //move 2.5 degrees
//		}
//		else if(button ==3){
//		    move_servo(5*direction);    //move 5 degrees
//		}
//		else if(button ==4){
//			direction=direction*-1;     //change direction
//		}
//		else{
//			if(direction == 1){
//			    lcd_printf("Incrementing -- %d", total);
//			}
//			else if(direction==-1){
//                lcd_printf("Decrementing -- %d", total);
//			}
//		}
//    }
//
//}

