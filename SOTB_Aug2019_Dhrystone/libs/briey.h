/*
 * briey.h
 *
 *  Created on: Aug 24, 2016
 *      Author: clp
 */

#ifndef BRIEY_H_
#define BRIEY_H_

#include "timer.h"
#include "prescaler.h"
#include "interrupt.h"
#include "uart.h"
#include "gpio.h"

#define ONCHIP_RAM_ADDR		0x80000000
#define ONCHIP_RAM_SIZE		0x10000
#define ONCHIP_RAM			((volatile uint32_t*)(0x80000000))

#define GPIO_A_BASE         ((Gpio_Reg*)(0xF0000000))
#define SPI			        ((Spi_Reg *)(0xF0002000))
#define UART                ((Uart_Reg*)(0xF0010000))

#define TIMER_PRESCALER ((Prescaler_Reg*)0xF0020000)
#define TIMER_INTERRUPT ((InterruptCtrl_Reg*)0xF0020010)
#define TIMER_A ((Timer_Reg*)0xF0020040)
#define TIMER_B ((Timer_Reg*)0xF0020050)
#define TIMER_C ((Timer_Reg*)0xF0020060)
#define TIMER_D ((Timer_Reg*)0xF0020070)

#define UART_SAMPLE_PER_BAUD 8


#endif /* BRIEY_H_ */
