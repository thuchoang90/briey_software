#ifndef GPIO_H_
#define GPIO_H_


typedef struct
{
  volatile uint16_t INPUT;
  volatile uint16_t OUTPUT;
  volatile uint16_t OUTPUT_ENABLE;
} Gpio_Reg;

static uint16_t gpio_read(Gpio_Reg *reg){
	return reg->INPUT;
}

#endif /* GPIO_H_ */


