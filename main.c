#include <stdio.h>
#include <ctype.h>
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include "servo.h"
#include "sonar.h"
#include "button.h"
#include "ir.h"
#include "uart.h"
#include "open_interface.h"
#include "movement.h"
#include <lcd.h>
#include <timer.h>
#include <math.h>
#include <stdbool.h>    //has to be above #include interrupt.h
#include "driverlib/interrupt.h"

//Functions being used
void receive_data(char* data);
void sweep_data(void);
void sensors_data(oi_t *sensor);
void check_area(void);
void clear_array();
void scanOnce(void);
int goal_check(void);
void visualize(void);
void clear_display(void);
void goal(void);


//Variables
#define PI 3.14159265
int ser_total = 0;
int direction = 1;

//object detection variables
int obj_see = 0, degree_count = 0, total_obj_count = 0, smallest_degree = 180,
        smallest_obj_num = 0, starting_angle = 0, ending_angle = 0;
double mid_angle;
double smallest_dist_width = 0;

int scan_done = 0;

char victory_songTone[16] = {67, 67, 72, 71, 67, 67, 72,71, 67,67, 72, 71, 84, 83, 79, 81};
char victory_songDuration[16] = { 8, 8, 16, 16, 8, 8, 16, 16, 8, 8, 16, 32, 8, 8, 8, 64};

char str[80];
double temp;
int obj_distance[5] = { 99, 99, 99, 99, 99 };
int obj_angle[5] = { 199, 199, 199, 199, 199 };
double obj_radius[5] = { 99, 99, 99, 99, 99 };
char display[80][161];
int gpon;
char dtm;
int dtml;
short rob_angle, rob_x, rob_y;

/**
 *Our Main Function that will receive data, and make decisions based off
 *what it receives.

 Commands:
 s | s1 = return bump and cliff sensor data
 s2     = sweep the area, and return that data
 s3     = kind of sweep the bottom cliff sensors, returning that data for a certain amount of time
 mfX     = move forward by X0 cm(mf1 moves forward 10cm)  [up to 30cm]
 mbX     = move backward by X0 cm(mb3 moves backward 30cm)[up to 30cm]
 trX      = turn right(clockwise) by X0 degrees(can be negative)[up to 90 degrees]
 tlX      = turn right(clockwise) by X0 degrees(can be negative)[up to 90 degrees]
 */
int main(void)
{
    //Initializations
    timer1B_init(); //servo timer
    timer3B_init(); //sonar timer
    timer4B_init(); //uart timer
    sonar_init();
    servo_init();
    button_init();
    uart_init();
    lcd_init();
    ADC_init();

    char data[3];
    char command = '0';
    char distance = 0, degrees = 0;
    //Initialize variables
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    while (1)
    {
        receive_data(data);
        command = tolower(data[0]);
        switch (command)
        {
        case 'c':
            //C     = sweep the area, and return that data
            clear_display();
            clear_array();
            move_servo(0);
            timer_waitMillis(500);
            check_area();
            move_servo(0);
            scan_done = 0;
            ser_total = 0;
            visualize();
            total_obj_count = 0;
            timer_waitMillis(500);
            break;
        case 's':
            if (data[1] == '2')
            {
                char transmit[150];
                oi_update(sensor_data);
                timer_waitMillis(100);
                short sensorArr0 = sensor_data->cliffLeftSignal;
                short sensorArr1 = sensor_data->cliffFrontLeftSignal;
                short sensorArr2 = sensor_data->cliffFrontRightSignal;
                short sensorArr3 = sensor_data->cliffRightSignal;
                sprintf(transmit,
                        "cliffLeftSignal is %d\n\rcliffFrontLeftSignal is %d\n\rcliffFrontRightSignal is %d\n\rcliffRightSignal is %d\n\n\n\r",
                        sensorArr0, sensorArr1, sensorArr2, sensorArr3);
                uart_sendStr(transmit);
            }
            else if (data[1] == '3')
            {
                get_cliff_sensor_data(sensor_data);
            }
            else
                //s | s1 = return bump and cliff sensor data, and return it
                sensors_data(sensor_data);
            break;
        case 'm':
            if (data[2] >= '0' && data[2] <= '9') //Valid number
            {
                //Calculate distance
                distance = (data[2] - '0') * 10;

                if (tolower(data[1]) == 'f')
                    //fX     = move forward by X0 cm(f1 moves forward 10cm)  [up to 90cm]
                    move_forward(sensor_data, distance);

                else if (tolower(data[1]) == 'b')
                    //bX     = move backward by X0 cm(b3 moves backward 30cm)[up to 90cm]
                    move_backward(sensor_data, distance);
            }
            break;
        case 't':
            if (data[2] >= '0' && data[2] <= '9') //insure it is a valid number
            {
                //Calculate degrees
                degrees = (data[2] - '0') * 10;

                if (tolower(data[1]) == 'r')
                    //trX      = turn right(clockwise) by X0 degrees(can be negative)[up to 90 degrees]
                    turn_clockwise(sensor_data, degrees);

                else if (tolower(data[1]) == 'l')
                    //tlX      = turn left(counterClockwise) by X0 degrees(can be negative)[up to 90 degrees]
                    turn_counterclockwise(sensor_data, degrees);
            }

        case 'g':
            goal();
            break;
        case 'd':
            clear_display();
            visualize();
            break;
        default:
            //Print Invalid command
            uart_sendStr("Invalid command\n\r");
        }
    }
}

/**
 * Function that will receive data, the data will be used for commands.
 * When sending data, * will end a command
 */
void receive_data(char* data)
{
    char i = 0;
    data[i++] = uart_receive();
    while (data[i-1] != 10 && i < 3)
    {
        data[i++] = uart_receive();
    }
    timer_waitMillis(10);
    char s = ~(UART1_FR_R & UART_FR_RXFE) & UART_FR_RXFE;
    //Consume all remaining characters
    while (s)
    {
        timer_waitMillis(10);
        (UART1_DR_R & 0xFF);
        s = ~(UART1_FR_R & UART_FR_RXFE) & UART_FR_RXFE;

    }
}

/**
 * This function will Grab data from all bump and cliff sensors, and will return the data
 * through uart, display it on terminal
 */
void sensors_data(oi_t *sensor)
{
    //Variables being used
    char transmit[150];

    //Update sensor
    oi_update(sensor);

    //Create string being transmitted
    sprintf(transmit,
            "bumpLeft\t\tbumpRight\n\r%d\t\t\t%d\n\r\n\rcliffLeft\t\tcliffRight\n\r%d\t\t\t%d\n\r\n\rcliffFrontLeft\t\tcliffFrontRight\n\r%d\t\t\t%d\r\n",
            sensor->bumpLeft, sensor->bumpRight, sensor->cliffLeft,
            sensor->cliffRight, sensor->cliffFrontLeft,
            sensor->cliffFrontRight);

    //Transmit Data
    uart_sendStr(transmit);
}

void check_area(void)
{
    while (1)
    {
        scanOnce();
        gpon = goal_check();
        if (gpon >= 0)
        {
            if (obj_angle[gpon] >= 90)
            {
                dtm = 'L';
                dtml = cos((180 - obj_angle[gpon]) * PI / 180)
                        * obj_distance[gpon] + 17;
            }
            else
            {
                dtm = 'R';
                dtml = cos((obj_angle[gpon]) * PI / 180) * obj_distance[gpon]
                        + 17;
            }
        }
        if (scan_done)
        {
            sprintf(str,
                    "D: %d R: %g A: %d\r\nD: %d R: %g A: %d\r\nD: %d R: %g A: %d\r\nD: %d R: %g A: %d\r\n move=%c Distance=%d",
                    obj_distance[0], obj_radius[0], obj_angle[0],
                    obj_distance[1], obj_radius[1], obj_angle[1],
                    obj_distance[2], obj_radius[2], obj_angle[2],
                    obj_distance[3], obj_radius[3], obj_angle[3], dtm, dtml);
            uart_sendStr(str);
            dtml=1000;
            break;
        }
    }
}

/**
 * This function will sweep, find objects, and return them through uart to be displayed.
 */
void scanOnce(void)
{
    //IR variables
    double ir_dist;
    int ir_num_input = 0;
    int ir_total = 0;
    int ir_quanV;

    int sonar_dist;

    ser_total += (1 * direction);
    //180
    if (ser_total >= 180)
    {
        scan_done = 1;
    }
    move_servo(ser_total);
    //IR
    while (ir_num_input < 16)
    {
        ir_total += ADC_read();
        ir_num_input++;
    }
    ir_quanV = ir_total / 16;
    ir_total = 0;
    ir_num_input = 0;

    // ir_dist = 10294 * pow(ir_quanV, -0.945); //bot 2
    // ir_dist = 189031 * pow(ir_quanV, -1.239); //bot 17 ds
//    ir_dist = 119254 * pow(ir_quanV, -1.188);        //bot 13
//    ir_dist = 112048 * pow(ir_quanV, -1.164); //bot 3
//      ir_dist = 56362 * pow(ir_quanV, -1.074); //bot 5
      ir_dist = 64460 * pow(ir_quanV, -1.099); //bot 4

    //SONAR
    sonar_dist = get_sonar();

    if ((ir_dist <= 50) && (ser_total > 3))
    {
        if (obj_see == 0)
        {
            starting_angle = ser_total;

        }

        obj_see = 1;
        degree_count += 1;
    }
    if ((obj_see == 1) && (ir_dist > 50))
    {
        obj_angle[total_obj_count] = starting_angle;
        obj_see = 0;
        ending_angle = ser_total;
        mid_angle = (ending_angle - starting_angle) / 2.0;
        obj_distance[total_obj_count] = sonar_dist;
        temp = tan(mid_angle * PI / 180);
        obj_radius[total_obj_count] = temp * 2.0 * sonar_dist;
        degree_count = 0;
        ending_angle = 0;
        total_obj_count++;
        lcd_printf("%g %g", temp, mid_angle);
    }
    sprintf(str, "Angle: %d degrees IR Dist: %g Ping Distance: %d\r\n",
            ser_total, ir_dist, sonar_dist);
    uart_sendStr(str);
    timer_waitMillis(125);
}

int goal_check(void)
{
    int i = 0;
    for (i = 0; i < 5; i++)
    {
        if (obj_radius[i] <= 7)
        {
            return i;
        }
    }
    return -1;
}

void clear_array(){
    int w = 0;
    for (w = 0; w <= 5; w++)
    {
        obj_distance[w] = 99;
        obj_radius[w] = 99;
        obj_angle[w] = 199;
    }
}

/**
 * This will print to console whatever the robot is currently seeing.
 * It will grab what's in the obj_angle & obj_distance, convert to x & y coordinates
 * and draw them that far apart from our Robot
 */
void visualize(void)
{
    //add our position to display
    display[1][80]= '^';
    display[0][80]= 'O';
    display[0][81]= ')';
    display[0][79]= '(';
    //Add items position to display
//    obj_distance[0], obj_radius[0], obj_angle[0],
    short i = 0, y = 79;
    char transmit[161];
    for (i = 0; i < 4; i++)
    {
        if (obj_distance[i] < 50)
        {
            short curY = (short) ((((float) obj_distance[i])
                    * sin((((float) obj_angle[i]) * M_PI) / 180.0)) + .5);
            short curX = (short) ((((float) obj_distance[i])
                    * cos((((float) obj_angle[i]) * M_PI) / 180.0)) + .5);
            char display_char = obj_radius[i] < 10? '0': 'X';
            display[0 + curY][80 + curX] = display_char;
            display[1 + curY][80 + curX] = display_char;
            display[0 + curY][81 + curX] = display_char;
            display[1 + curY][81 + curX] = display_char;
        }
    }

    //Print Display
    while (y > -1)
    {
        sprintf(transmit, "%s", display[y]);
        uart_sendStr(display[y--]);
    }
}


/**
 * This is a helper function that clears the display every time it is about to display something.
 */
void clear_display(void)
{
    int x = 0, y = 0;
    for (; x < 161; x++)
    {
        for (y = 0; y < 80; y++)
        {
            if (x == 160)
                display[y][x] = '\0';
            else if (x == 158)
                display[y][x] = '\n';
            else if (x == 159)
                display[y][x] = '\r';
            else
                display[y][x] = ' ';
        }
    }
}

/**
 * This is the helper function that will flash the led, and play music once we know we've reached our goal.
 */
void goal(void)
{

    /// \brief Set the LEDS on the Create
    /// \param play_led 0=off, 1=on
    /// \param advance_led 0=off, 1=on
    /// \param power_color (0-255), 0=green, 255=red
    /// \param power_intensity (0-255) 0=off, 255=full intensity
//    void oi_setLeds(uint8_t play_led, uint8_t advance_led, uint8_t power_color, uint8_t power_intensity)

    oi_loadSong(1, 16, victory_songTone, victory_songDuration);
    oi_play_song(1);
    char color = 0, i = 0;
    for (; i < 100; i++)
    {
        oi_setLeds(1, 1, ((color += 20)%255), 255);
        timer_waitMillis(100);
        oi_setLeds(0, 1, color, 0);
        timer_waitMillis(100);
    }
}

