/*
 * movement.h
 *
 *  Created on: Jan 21, 2018
 *      Author: kmontoya and Arnoldo
 */
#include "open_interface.h"

#ifndef MOVEMENT_H_
#define MOVEMENT_H_


//Moves the robot forward the given amount of centimeters
void move_forward(oi_t *sensor,int centimeters);

//Moves the robot backward the given amount of centimeters
void move_backward(oi_t *sensor,int centimeters);

//Turns robot clockwise by the given degrees.
void turn_clockwise(oi_t *sensor, int degrees);

//Turns robot counterclockwise by the given degrees.
void turn_counterclockwise(oi_t *sensor, int degrees);

//it moves in order to avoid an obstacle based on input[0 left, 1 right]
void avoid_obstacle(int direction);

//This helps get sensor data of the cliff sensors, in case they're where they're not supposed to
void get_cliff_sensor_data(oi_t *sensor);


#endif /* MOVEMENT_H_ */


