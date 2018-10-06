/**
 *
 * 	@file  uart.c
 *
 *   @author
 *   @date
 */

#include "uart.h"
#include "lcd.h"
#include "button.h"
#include <inc/tm4c123gh6pm.h>
#include <timer.h>
#include <stdbool.h>    //has to be above #include interrupt.h
#include "driverlib/interrupt.h"
#define TIMER4B_PRESCALER 50

/**
 * @brief sets all necessary registers to enable the uart 1 module.
 */
void uart_init(void)
{

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;
    GPIO_PORTB_AFSEL_R |= (BIT0 | BIT1);
    GPIO_PORTB_PCTL_R |= 0x00000011;
    GPIO_PORTB_DEN_R |= (BIT0 | BIT1);
    GPIO_PORTB_DIR_R &= ~BIT0;
    GPIO_PORTB_DIR_R |= BIT1;

    uint16_t iBRD = 8;
    uint16_t fBRD = 44;
    UART1_CTL_R &= ~(UART_CTL_UARTEN);
    UART1_IBRD_R = iBRD;
    UART1_FBRD_R = fBRD;
    UART1_LCRH_R = UART_LCRH_WLEN_8;
    UART1_CC_R = UART_CC_CS_SYSCLK;
    UART1_CTL_R = (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN);
}
char message1[] = "Don't Press Me Ever Again\r\n";
char message2[] = "Important Files Deleted\r\n";
char message3[] = "*Monty Python Reference Here*\r\n";
char message4[] = "Blue, no green, Ahhhhh!!!\r\n";
char message5[] = "No\r\n";
char message6[] = "Yes\r\n";

void TIMER4B_Handler(void)
{
    TIMER4_ICR_R = TIMER_ICR_TBTOCINT; //clear flag
    int button = button_getButton();
    if (button == 6)
    {
       // uart_sendStr(message1);
    }
    if (button == 5)
    {
        //uart_sendStr(message2);
    }
    if (button == 4)
    {
       // uart_sendStr(message3);
    }
    if (button == 3)
    {
       // uart_sendStr(message4);
    }
    if (button == 2)
    {
       // uart_sendStr(message5);
    }
    if (button == 1)
    {
       // uart_sendStr(message6);
    }
}
void timer4B_init(void)
{
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R4; // Turn on clock to TIMER4

    //Configure the timer for input capture mode
    TIMER4_CTL_R &= ~(TIMER_CTL_TBEN); //disable timerB to allow us to change settings
    TIMER4_CFG_R |= TIMER_CFG_16_BIT; //set to 16 bit timer

    TIMER4_TBMR_R = TIMER_TBMR_TBMR_PERIOD; //set for periodic mode, down count

    TIMER4_TBPR_R = TIMER4B_PRESCALER - 1;  // 200 ms clock

    TIMER4_TBILR_R = (int) (16000000 / (TIMER4B_PRESCALER * 5)); // set the load value for the 0.2 second clock

    //clear TIMER3B interrupt flags
    TIMER4_ICR_R = (TIMER_ICR_TBTOCINT); //clears TIMER4 time-out interrupt flags
    TIMER4_IMR_R |= (TIMER_IMR_TBTOIM); //enable TIMER4(A&B) time-out interrupts

    //initialize local interrupts
    NVIC_EN2_R = 0x0000000C0; //#warning "enable interrupts for TIMER4A and TIMER4B" n = 0, 1, 2, 3, or 4
    //go to page 105 and find the corresponding interrupt numbers for TIMER4 A&B
    //then set those bits in the correct interrupt set EN register (p. 142)

    IntRegister(INT_TIMER4B, TIMER4B_Handler); //register TIMER4B interrupt handler

    IntMasterEnable(); //intialize global interrupts

    TIMER4_CTL_R |= (TIMER_CTL_TAEN | TIMER_CTL_TBEN); //Enable TIMER4A & TIMER4B
}

/**
 * @brief Sends a single 8 bit character over the uart 1 module.
 * @param data the data to be sent out over uart 1
 */
void uart_sendChar(char data)
{
    //wait until there is room to send data
    while (UART1_FR_R & 0x20)
    {
    }
    //send data
    UART1_DR_R = data;
}

/**
 * @brief polling receive an 8 bit character over uart 1 module.
 * @return the character received or a -1 if error occured
 */
char uart_receive(void)
{
    char data = 0;
    //wait to receive
    while (UART1_FR_R & UART_FR_RXFE)
    {
    }
    data = (char) (UART1_DR_R & 0xFF);
    return data;
}

/**
 * @brief sends an entire string of character over uart 1 module
 * @param data pointer to the first index of the string to be sent
 */
void uart_sendStr(const char *data)
{
    while (UART1_FR_R & 0x20)
    {
    }
    char toSend = *data;
    while (*data != '\0')
    {
        toSend = *data;
        uart_sendChar(toSend);
        data++;
    }
}

//int main(void)
//{
//    lcd_init();
//    uart_init();
//    button_init();
//    timer4B_init();
//    char localData[21];
//    int index = 0;
//    char enter;
//
//    char password[] = "password";
//    WiFi_start(password);
//
//    while (1)
//    {
//        enter = uart_receive();
//        if (enter != '\r')
//        {
//            localData[index] = enter;
//            uart_sendChar(localData[index]);
//            lcd_printf("index: %d \n char: %c", index, localData[index]);
//            index += 1;
//            if (index >= 21)
//            {
//                localData[20] = '\0';
//                lcd_printf("%s", localData);
//                index = 0;
//            }
//        }
//        else
//        {
//            uart_sendChar('\r');
//            uart_sendChar('\n');
//            lcd_printf("%.*s", index, localData);
//            index = 0;
//        }
//    }
//    WiFi_stop();
//}
