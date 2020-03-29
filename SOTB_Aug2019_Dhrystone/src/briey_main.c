#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <briey.h>

extern void main2();

void print(char *str){
	while(*str){
		uart_write(UART,*(str++));
	}
}

int main() {
	uint32_t CORE_HZ;
	uint16_t gpio_in;

	gpio_in = (GPIO_A_BASE->INPUT)&0xff;
	CORE_HZ = (gpio_in + 1) * 1000000;

	Uart_Config uartConfig;
	uartConfig.dataLength = 8;
	uartConfig.parity = NONE;
	uartConfig.stop = ONE;
	uartConfig.clockDivider = (CORE_HZ / 8 / 115200) - 1;
	uart_applyConfig(UART,&uartConfig);

	print("\n------- in RAM now -------\r\n");
	print("Hello again !\r\n");

	interruptCtrl_init(TIMER_INTERRUPT);
	prescaler_init(TIMER_PRESCALER);
	timer_init(TIMER_A);

	TIMER_A->LIMIT = ~0;
	TIMER_A->CLEARS_TICKS = 0x00010001;

	main2(CORE_HZ);
}

void irqCallback(){

}

#include <time.h>
clock_t	clock(){
	return TIMER_A->VALUE;
}
