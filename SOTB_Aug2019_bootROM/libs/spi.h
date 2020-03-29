/*
 * spi.h
 *
 *  Created on: May 24, 2019
 *      Author: thuc
 */

#ifndef LIBS_SPI_H_
#define LIBS_SPI_H_


#define SD_ACK		0x00
#define SD_IDLE		0x01
#define SD_PATTERN	0x000001aa
#define SD_CCS		0x40000000
#define SD_WAIT		0x80
#define SD_INVALID	0xff


typedef struct {
  volatile int32_t DATA;
  volatile int32_t STATUS;
  volatile int32_t CONFIG;
  volatile int32_t CLOCK_DIVIDER;
  volatile int32_t ssSetup;
  volatile int32_t ssHold;
  volatile int32_t ssDisable;
} Spi_Reg;

typedef struct {
	int8_t interrupt_enable;
	int32_t clockDivider;
} Spi_Config;


#endif /* LIBS_SPI_H_ */
