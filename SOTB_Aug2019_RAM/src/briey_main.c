#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <briey.h>

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
}


void irqCallback(){

}
