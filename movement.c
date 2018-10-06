/*
 * movement.c
 *
 *  Created on: Jan 21, 2018
 *      Author: kmontoya & Amontoya
 */
#include "open_interface.h"
#include <uart.h>
#include <stdio.h>

void avoid_obstacle(int direction, oi_t *sensor);
void notify_user(oi_t *sensor);
void findMinMax(short *min, short*max, short nums[]);

char reverse_soundDuration[3] = {32,32,32};
char reverse_soundTone[3] = {73,73,73};

//Moves the robot forward the given amount of centimeters
void move_forward(oi_t *sensor, int centimeters)
{
    //Convert centimeters to millimeters
    int millimeters = centimeters * 10;

    //current distance sum
    int sum = 0;
    uint32_t em_stop;

    // move forward;
    oi_setWheels(150,150);

    while (sum < millimeters)
    {
        oi_update(sensor);
        short boundary = (sensor->cliffLeftSignal > 2700 | sensor->cliffFrontLeftSignal > 2700 | sensor->cliffFrontRightSignal > 2700 | sensor->cliffRightSignal > 2700);

        //Emergency stop trigger
        em_stop = sensor->bumpLeft | sensor->bumpRight | sensor->cliffLeft
                | sensor->cliffFrontLeft | sensor->cliffFrontRight
                | sensor->cliffRight | boundary;
        //If bumped on left or right side, avoid the obstacle
        if (em_stop)
        {
            oi_setWheels(0, 0);
            sum = millimeters;
            notify_user(sensor);
        }

        sum += sensor->distance;
    }

    oi_setWheels(0, 0); // stop
}

//Moves the robot backward the given amount of centimeters
void move_backward(oi_t *sensor, int centimeters)
{
    //Set distance in mm needed
    int millimeters = centimeters * (-10);

    //current distance sum
    int sum = 0;

    oi_loadSong(0, 3, reverse_soundTone, reverse_soundDuration);
    oi_play_song(0);

    // move forward;
    oi_setWheels(-250, -250);

    while (sum > millimeters)
    {
        oi_update(sensor);
        sum += sensor->distance;
    }

    oi_setWheels(0, 0); // stop
}

//Turns robot clockwise by the given degrees.
void turn_clockwise(oi_t *sensor, int degrees)
{
//Variable summing up angle that it has turned
    int angle_sum = 0;

//turn angle into negative
    degrees *= -1;

//Start turning clockWise
    oi_setWheels(-100, 100);

    while (angle_sum >= degrees)
    {
        oi_update(sensor);
        angle_sum += sensor->angle;
    }

    oi_setWheels(0, 0); // stop
}

//Turns robot counterclockwise by the given degrees.
void turn_counterclockwise(oi_t *sensor, int degrees)
{
//Variable summing up angle that it has turned
    int angle_sum = 0;

//Start turning clockWise
    oi_setWheels(100, -100);

    while (angle_sum <= degrees)
    {
        oi_update(sensor);
        angle_sum += sensor->angle;
    }

    oi_setWheels(0, 0); // stop

}

//it moves in order to avoid an obstacle based on input[0 left, 1 right]
void avoid_obstacle(int direction, oi_t *sensor)
{


    move_backward(sensor, 15);

    //Based off of direction, move left or right
    if (direction == 0)
    {
        turn_clockwise(sensor, 90);
        move_forward(sensor, 25);
        turn_counterclockwise(sensor, 90);
    }
    else
    {
        turn_counterclockwise(sensor, 90);
        move_forward(sensor, 25);
        turn_clockwise(sensor, 90);
    }

    //reset sensor values
    oi_update(sensor);
    oi_update(sensor);

}

/**
 * Function that will notify user if one of the emergency stop sensors was bumped
 */
void notify_user(oi_t *sensor)
{
    char transmit[50];

    if (sensor->bumpLeft)
        sprintf(transmit, "%s has been triggered!", "bumpLeft");
    else if (sensor->bumpRight)
        sprintf(transmit, "%s has been triggered!", "bumpRight");
    else if (sensor->cliffLeft)
        sprintf(transmit, "%s has been triggered!", "cliffLeft");
    else if (sensor->cliffFrontLeft)
        sprintf(transmit, "%s has been triggered!", "cliffFrontLeft");
    else if (sensor->cliffFrontRight)
        sprintf(transmit, "%s has been triggered!", "cliffFrontRight");
    else if (sensor->cliffRight)
        sprintf(transmit, "%s has been triggered!", "cliffRight");
    else if((sensor->cliffLeftSignal > 2700 | sensor->cliffFrontLeftSignal > 2700 | sensor->cliffFrontRightSignal > 2700 | sensor->cliffRightSignal > 2700))
        sprintf(transmit, "We've hit a white line");

    uart_sendStr(transmit);
}

/**
 * Function that will return data from the cliff sensors, basically telling us what it is for a certain amount of time.
 */
void get_cliff_sensor_data(oi_t *sensor)
{
    char transmit[150];
    short sensorArr[4][100];
    oi_setWheels(15, 15);
    char s = -1;
    while (++s < 100)
    {
        oi_update(sensor);
        timer_waitMillis(100);
        sensorArr[0][s] = sensor->cliffLeftSignal;
        sensorArr[1][s] = sensor->cliffFrontLeftSignal;
        sensorArr[2][s] = sensor->cliffFrontRightSignal;
        sensorArr[3][s] = sensor->cliffRightSignal;

        sprintf(transmit,
                "cliffLeftSignal is %d\n\rcliffFrontLeftSignal is %d\n\rcliffFrontRightSignal is %d\n\rcliffRightSignal is %d\n\n\n\r",
                sensorArr[0][s], sensorArr[1][s], sensorArr[2][s], sensorArr[3][s]);
        uart_sendStr(transmit);
    }
    oi_setWheels(0, 0);

    short i = 0, min = 0, max = 0;
    for (; i < 4; i++)
    {
        findMinMax(&min, &max, sensorArr[i]);
        sprintf(transmit, "Data at %d:\n\rmin is: %d\n\rmax is: %d\n\n\r", i, min, max);
        uart_sendStr(transmit);
    }

}

void findMinMax(short* min, short* max, short nums[])
{
    *min = nums[0];
    *max = nums[0];
    short i = 0;
    for(; i < 100; i++)
    {
        if(nums[i] > *max)
            *max = nums[i];
        if(nums[i] < *min)
            *min = nums[i];

    }
}
