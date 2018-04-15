#ifndef MP3_BOARD_H
#define MP3_BOARD_H

#include "stm32f10x.h"


#define __PIN(a) GPIO_Pin_ ## a
#define _PIN(a) __PIN(a)

#define __PORT(a) GPIO ## a
#define _PORT(a) __PORT(a)

/*vs1003*/

#define PIN_XRESET 	4
#define PIN_CS   	2
#define PIN_DREQ 	1
#define PIN_XDCS 	0
#define PIN_SCLK 	5
#define PIN_SI 		7
#define PIN_SO 		6

#define PORT_XRESET C
#define PORT_CS   	B
#define PORT_DREQ 	B
#define PORT_XDCS 	B
#define PORT_SPI 	A

/*button*/

#define PIN_BTN_1 15
#define PIN_BTN_2 14
#define PIN_BTN_3 13

#define PORT_BTN_1 C
#define PORT_BTN_2 C
#define PORT_BTN_3 C


/**************************************/
#define PIN_ADC_XP 0
#define PIN_ADC_XN 2
#define PIN_ADC_YP 1
#define PIN_ADC_YN 3

#define PORT_ADC A

#define PIN_EC11_A 7
#define PIN_EC11_B 6
#define PIN_BTN    5

#define PORT_EC11 A

#define PIN_USART_RX  10
#define PIN_USART_TX  9
#define PIN_USART_RTS 12

#define PORT_USART A

#define PIN_TFT_DB10 8  
#define PIN_TFT_DB11 9
#define PIN_TFT_DB12 10
#define PIN_TFT_DB13 11
#define PIN_TFT_DB14 12
#define PIN_TFT_DB15 13
#define PIN_TFT_DB16 14
#define PIN_TFT_DB17 15

#define PORT_TFT_DB B

#define PIN_TFT_RESET 	7
#define PIN_TFT_NE1 	10
#define PIN_TFT_CLE_A16 8
#define PIN_TFT_NWE 	9
#define PIN_TFT_NOE 	6


#define PORT_TFT_RESET 	 C
#define PORT_TFT_NE1 	 A
#define PORT_TFT_CLE_A16 A
#define PORT_TFT_NWE 	 A
#define PORT_TFT_NOE	 C
/***************************************/


void board_init(void);





#endif