/*
 * servo.h
 *
 *  Created on: Mar 20, 2018
 *      Author: msueppel
 */

#ifndef SERVO_H_
#define SERVO_H_
#include <stdint.h>
#include <inc/tm4c123gh6pm.h>


void servo_init(void);
void timer1B_init(void);
void move_servo(int degrees);

#endif /* SERVO_H_ */
