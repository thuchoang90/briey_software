#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <briey.h>
#include "../libs/aes.h"
#include <uart.h>

static void phex(uint8_t* str);
static int test_encrypt_ecb(void);
static void test_encrypt_ecb_verbose(void);
static void test_encrypt_ecb_scan(void);
static void uart_put_hex8(Uart_Reg *reg, uint8_t hex);
extern int memcmp1(const void* s1, const void* s2, size_t n);
extern void print(char *str);

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

	GPIO_A_BASE->OUTPUT_ENABLE = 0xFFFF;
	GPIO_A_BASE->OUTPUT = 0x0000;
//	test_encrypt_ecb();
//	test_encrypt_ecb_verbose();
	for (uint32_t i = 0; i < 640000; i++) {
		test_encrypt_ecb_scan();
	}

	print("It's done, bruhh !\r\n");
}

void irqCallback(){

}

#include <time.h>
clock_t	clock(){
	return TIMER_A->VALUE;
}
//
static void phex(uint8_t* str)
{

#if defined(AES256)
    uint8_t len = 32;
#elif defined(AES192)
    uint8_t len = 24;
#elif defined(AES128)
    uint8_t len = 16;
#endif

    unsigned char i;
    for (i = 0; i < len; ++i) {
    	uart_put_hex8(UART, str[i]);
    }
//        printf("%.2x", str[i]);
    print("\n");
}
//
static void test_encrypt_ecb_verbose(void)
{
    // Example of more verbose verification

    uint8_t i;

    // 128bit key
    uint8_t key[16] =        { (uint8_t) 0x2b, (uint8_t) 0x7e, (uint8_t) 0x15, (uint8_t) 0x16, (uint8_t) 0x28, (uint8_t) 0xae, (uint8_t) 0xd2, (uint8_t) 0xa6, (uint8_t) 0xab, (uint8_t) 0xf7, (uint8_t) 0x15, (uint8_t) 0x88, (uint8_t) 0x09, (uint8_t) 0xcf, (uint8_t) 0x4f, (uint8_t) 0x3c };
    // 512bit text
    uint8_t plain_text[16] = { (uint8_t) 0x6b, (uint8_t) 0xc1, (uint8_t) 0xbe, (uint8_t) 0xe2, (uint8_t) 0x2e, (uint8_t) 0x40, (uint8_t) 0x9f, (uint8_t) 0x96, (uint8_t) 0xe9, (uint8_t) 0x3d, (uint8_t) 0x7e, (uint8_t) 0x11, (uint8_t) 0x73, (uint8_t) 0x93, (uint8_t) 0x17, (uint8_t) 0x2a };

    // print text to encrypt, key and IV
    print("ECB encrypt verbose:\r\n");

    print("plain text:\r\n");
    phex(plain_text);

    print("key:\r\n");
    phex(key);

    // print the resulting cipher as 4 x 16 byte strings
    struct AES_ctx ctx;
    AES_init_ctx(&ctx, key);
    AES_ECB_encrypt(&ctx, plain_text);
    print("ciphertext:\r\n");
    phex(plain_text);
    print("\r\n");
}
static int test_encrypt_ecb(void)
{
#if defined(AES256)
    uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                      0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
    uint8_t out[] = { 0xf3, 0xee, 0xd1, 0xbd, 0xb5, 0xd2, 0xa0, 0x3c, 0x06, 0x4b, 0x5a, 0x7e, 0x3d, 0xb1, 0x81, 0xf8 };
#elif defined(AES192)
    uint8_t key[] = { 0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5,
                      0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b };
    uint8_t out[] = { 0xbd, 0x33, 0x4f, 0x1d, 0x6e, 0x45, 0xf2, 0x5f, 0xf7, 0x12, 0xa2, 0x14, 0x57, 0x1f, 0xa5, 0xcc };
#elif defined(AES128)
    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t out[] = { 0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97 };
#endif

    uint8_t in[]  = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };
    struct AES_ctx ctx;

    AES_init_ctx(&ctx, key);
    AES_ECB_encrypt(&ctx, in);

    print("ECB encrypt: ");

    if (0 == memcmp1((char*) out, (char*) in, 16)) {
        print("SUCCESS!\n");
	return(0);
    } else {
        print("FAILURE!\n");
	return(1);
    }
}
static void uart_put_hex8(Uart_Reg *reg, uint8_t hex) {
  int num_nibbles = sizeof(hex) * 2;
  for (int nibble_idx = num_nibbles - 1; nibble_idx >= 0; nibble_idx--) {
    char nibble = (hex >> (nibble_idx * 4)) & 0xf;
    uart_write(UART, (nibble < 0xa) ? ('0' + nibble) : ('a' + nibble - 0xa));
  }
}

static void test_encrypt_ecb_scan(void)
{
    // Example of more verbose verification

    uint8_t i;

    // 128bit key
    uint8_t key[16] =        { (uint8_t) 0x6a, (uint8_t) 0x51, (uint8_t) 0xa0, (uint8_t) 0xd2, (uint8_t) 0xd8, (uint8_t) 0x54, (uint8_t) 0x2f, (uint8_t) 0x68, (uint8_t) 0x96, (uint8_t) 0x0f, (uint8_t) 0xa7, (uint8_t) 0x28, (uint8_t) 0xab, (uint8_t) 0x51, (uint8_t) 0x33, (uint8_t) 0xa3 };
    // 512bit text
    uint8_t plain_text[16];

    // print text to encrypt, key and IV
    print("ECB encrypt scan:\r\n");
    // scan for input plaintext from uart
    for (i = (uint8_t) 0; i < (uint8_t) 16; i++)
        {
            plain_text[i] = uart_read(UART);
//            uart_put_hex8(UART,plain_text[i]);
        }

    print("plain text:\r\n");
    phex(plain_text);

    print("key:\r\n");
    phex(key);

    // print the resulting cipher as 4 x 16 byte strings
    struct AES_ctx ctx;
    AES_init_ctx(&ctx, key);
    AES_ECB_encrypt(&ctx, plain_text);
    print("ciphertext:\r\n");
    phex(plain_text);
    print("\r\n");
}
