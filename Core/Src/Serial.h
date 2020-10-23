/*
 * Serial.h
 *
 *  Created on: Aug 24, 2020
 *      Author: Macbook
 */

#ifndef SRC_SERIAL_H_
#define SRC_SERIAL_H_

#include "Print.h"
#include "stm32f1xx_hal.h"

typedef enum
{
    PRINT_DEBUGGING = 0x00,
    PRINT_UART      = 0x01,
    PRINT_USB       = 0x02
} Mode_print;

class Serial: public Print
{
private:

    uint8_t mode;

public:

    UART_HandleTypeDef *uart;
    Serial( UART_HandleTypeDef * uartx, Mode_print mode = PRINT_UART );
    virtual void write( uint8_t * text );

};

#endif /* SRC_SERIAL_H_ */
