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
	Uart_Config uartConfig;
	uartConfig.dataLength = 8;
	uartConfig.parity = NONE;
	uartConfig.stop = ONE;
	uartConfig.clockDivider = (CORE_HZ / 8 / 115200) - 1;
	uart_applyConfig(UART,&uartConfig);

	print("Well, hello there ! こんにちは。\r\n");
	print("University of Electro-Communications (UEC), Tokyo, Japan\r\n");
	print("電気通信大学、東京都、日本\r\n");
	print("PHAM LAB ! 範研究室 !\r\n");
}


void irqCallback(){

}
