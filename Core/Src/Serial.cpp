/*
 * Serial.cpp
 *
 *  Created on: Aug 24, 2020
 *      Author: Macbook
 */

#include "Serial.h"

Serial::Serial( UART_HandleTypeDef * uartx, Mode_print mode_p )
{
    // TODO Auto-generated constructor stub
    mode = mode_p;
    uart = uartx;
}

void Serial::write( uint8_t * text )
{
    uint8_t i = 0;

    if ( mode == PRINT_UART )
    {
        HAL_UART_Transmit( uart, text, strlen( (char*) text ), 1000 );
    }
    else
    {
        for ( i = 0; i < strlen( (char*) text ); i++ )
        {
            ITM_SendChar(text[i]);
        }
    }
}

