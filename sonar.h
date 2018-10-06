/*
 * sonar.h
 *
 *  Created on: Mar 6, 2018
 *      Author: msueppel
 */

#ifndef SONAR_H_
#define SONAR_H_
#include <stdint.h>
#include <inc/tm4c123gh6pm.h>

void sonar_init(void);
void send_pulse(void);
void TIMER3B_Handler(void);
void timer3B_init();
int get_sonar(void);

#endif /* SONAR_H_ */
