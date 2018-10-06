/*
 * ir.c
 *
 *  Created on: Feb 27, 2018
 *      Author: msueppel
 */
#include <math.h>
#include "ir.h"
#include "lcd.h"
#include "Timer.h"

void ADC_init()
{
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;  //provides clock to port B
    SYSCTL_RCGCADC_R |= 0x1;                   //provides clock to ADC0

    GPIO_PORTB_AFSEL_R |= 0b00010000; //sets PB4 to alternate function 0x11          originally 0x4;
    GPIO_PORTB_DEN_R &= 0b11101111; //sets PB4's digital function to disabled    originally 0b11111011
    GPIO_PORTB_AMSEL_R |= 0b00010000; //enables PB4's analog function
    GPIO_PORTB_DIR_R &= 0b11101111; //sets PB4 to input
    GPIO_PORTB_ADCCTL_R = 0x00; //pin set to not trigger ADC

    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN1;                   //disable SS1 sample sequencer to configure it   0x2 for SS1
    ADC0_EMUX_R = ADC_EMUX_EM1_PROCESSOR;                //initialize the ADC trigger source as processor (default)
    ADC0_SSMUX1_R |= 0x000A;                             //set 1st sample to use the AIN10 ADC pin   PB4
    ADC0_SSCTL1_R |= (ADC_SSCTL0_IE0 | ADC_SSCTL0_END0); //enable raw interrupt status
    ADC0_SAC_R |= ADC_SAC_AVG_32X;                      //enable oversampling to average  0x6
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN1;                    //re-enable ADC0 SS1 0b10 0x2
}

int ADC_read()
{
    int value = 0;
    ADC0_PSSI_R = ADC_PSSI_SS1; //begin sampling SS1
    while ((ADC0_RIS_R & ADC_RIS_INR1) == 0)        //waits until conversion is complete
    {

    }
    value = ADC0_SSFIFO1_R;   //contains conversion result for SS1
    return value;
}

//int main(void)
//{
//    lcd_init();
//    ADC_init();
//    int numInput=0;
//    int total=0;
//    int quanV = 0;
//    double dist = 0.0;
//    while (1)
//    {
//        timer_waitMillis(250);
//        while(numInput<16){
//            total+=ADC_read();
//            numInput++;
//        }
//        quanV = total/16;
//       // quanV = ADC_read();
//        total=0;
//        numInput = 0;
//       // dist = 86801 * pow(quanV, -1.132);//
//        dist = 10294 * pow(quanV, -0.945);
//        lcd_printf("%d, %g", quanV, dist);
//    }
//}
//
