#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <briey.h>

// GPT represents GUIDs with the first three blocks as little-endian
// 5b193300-fc78-40cd-8002-e86c45580b47
const gpt_guid gpt_guid_sifive_fsbl = {{
  0x00, 0x33, 0x19, 0x5b, 0x78, 0xfc, 0xcd, 0x40, 0x80, 0x02, 0xe8, 0x6c, 0x45, 0x58, 0x0b, 0x47
}};

void print(char *str) {
	while(*str) { uart_write(UART,*(str++)); }
}

static inline void check_available(void) {
	uint16_t r;
	do { //check command fifo
		r = SPI->STATUS >> 16;
	} while (r < 0x20); //make sure that no command in the fifo
}

static uint8_t sd_read(void) {
	uint32_t r;
	uint16_t n = 10000;
	do {
		SPI->DATA = 0x010000ff;
		check_available();
		r = SPI->DATA;
		if (r&0x80000000)
			return (r&0xff);
	} while (--n > 0);
	print("sd_card timeout\r\n");
	return (r&0xff);
}

static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc) {
	uint8_t r;
	SPI->DATA = 0x40 | (0x3f&cmd); //first two bits must be 01
	SPI->DATA = arg >> 24;
	SPI->DATA = arg >> 16;
	SPI->DATA = arg >> 8;
	SPI->DATA = arg;
	SPI->DATA = crc;
	do {
		r = sd_read();
	} while ((r==SD_WAIT) || (r==SD_INVALID) );
	return r;
}

static void sd_poweron(uint32_t CORE_HZ) {
	uint8_t i;
	// SPI frequency = FCLK / (2 * clockDivider)
	// set clock to 300KHz
	SPI->CLOCK_DIVIDER = CORE_HZ / 300000 / 2;
	//at power on, SS & MOSI lines must high. Just apply the clock
	SPI->DATA = 0x10000000; //disable the SS line 0
	for (i = 10; i > 0; i--) {
		SPI->DATA = 0xff;
	}
	SPI->DATA = 0x11000000; //enable the SS line 0
}

static uint8_t sd_cmd0(void) {
	print("CMD0: ");
	return (sd_send_cmd(0,0,0x95) == SD_IDLE);
}

static uint8_t sd_cmd8(void) {
	uint32_t r;
	print("CMD8: ");
	if (sd_send_cmd(8,SD_PATTERN,0x87) != SD_IDLE)
		return 0;
	else {
		r = sd_read();
		r = (r<<8) | sd_read();
		r = (r<<8) | sd_read();
		r = (r<<8) | sd_read();
		return (r == SD_PATTERN);
	}
}

static uint8_t sd_acmd41(void) {
	uint8_t r;
	print("ACMD41: ");
	do { // init the sd card by a series of CMD55-CMD41
		sd_send_cmd(55,0,0x65); //CMD55
		r = sd_send_cmd(41,SD_CCS,0x77); //CMD41 with HCS=1
	} while (r == SD_IDLE);
	return (r == SD_ACK);
}

static uint8_t sd_cmd58(void) {
	uint32_t r;
	print("CMD58: ");
	if (sd_send_cmd(58,0,0xfd) != SD_ACK)
		return 0;
	r = sd_read(); //get the first byte
	//flush the other three
	SPI->DATA = 0xff;
	SPI->DATA = 0xff;
	SPI->DATA = 0xff;
	//check the bit [31]: card power up status bit
	// and the bit [30]: CCS flag
	return (r == 0xc0);
}

static uint8_t sd_cmd16(void) {
	uint8_t r;
	print("CMD16: ");
	return (sd_send_cmd(16, 512, 0x15) == SD_ACK);
}

static uint16_t crc16_round(uint16_t crc, uint8_t data) {
	crc = (uint8_t)(crc >> 8) | (crc << 8);
	crc ^= data;
	crc ^= (uint8_t)(crc >> 4) & 0xf;
	crc ^= crc << 12;
	crc ^= (crc & 0xff) << 5;
	return crc;
}

#define SPIN_SHIFT	6
#define SPIN_UPDATE(i)	(!((i) & ((1 << SPIN_SHIFT)-1)))
#define SPIN_INDEX(i)	(((i) >> SPIN_SHIFT) & 0x3)

static const char spinner[] = { '-', '/', '|', '\\' };

#define GPT_BLOCK_SIZE 512

static uint8_t crc7(uint8_t prev, uint8_t in) {
  // CRC polynomial 0x89
  uint8_t remainder = prev & in;
  remainder ^= (remainder >> 4) ^ (remainder >> 7);
  remainder ^= remainder << 4;
  return remainder & 0x7f;
}

int sd_copy(void* dst, uint32_t src_lba, size_t size) {
	volatile uint8_t *p;
	if (dst==NULL)
		p = (void*)ONCHIP_RAM_ADDR;
	else
		p = dst;
	uint32_t i = size;

	uint8_t crc = 0;
	crc = crc7(crc, (0x40 | (0x3f&18)));
	crc = crc7(crc, src_lba >> 24);
	crc = crc7(crc, (src_lba >> 16) & 0xff);
	crc = crc7(crc, (src_lba >> 8) & 0xff);
	crc = crc7(crc, src_lba & 0xff);
	crc = (crc << 1) | 1;

	uint8_t rc;
	rc = sd_send_cmd(18,src_lba,crc);
	if ((rc != SD_ACK) && (rc != SD_IDLE)) {
		return 0;
	}

	do {
		uint16_t crc = 0, crc_exp;
		int16_t n = 512;

		while (sd_read() != 0xfe);
		do {
			uint8_t x = sd_read();
			*p++ = x;
			crc = crc16_round(crc, x);
		} while (--n > 0);

		crc_exp = sd_read();
		crc_exp = (crc_exp << 8) | sd_read();

		if (crc != crc_exp) {
			print("\b- CRC mismatch ");
			sd_send_cmd(0x4C, 0, 0x01);
			return 0;
		}

		if (SPIN_UPDATE(i)) {
			uart_write(UART,'\b');
			uart_write(UART,spinner[SPIN_INDEX(i)]);
		}
	} while (--i > 0);

	rc = sd_send_cmd(0x4C, 0, 0x01);
	while (rc != SD_ACK) {
		rc = sd_read();
	}
	print("\b ");
	return 1;
}

static gpt_partition_range find_sd_gpt_partition (
  uint64_t partition_entries_lba,
  uint32_t num_partition_entries,
  uint32_t partition_entry_size,
  const gpt_guid* partition_type_guid,
  void* block_buf  // Used to temporarily load blocks of SD card
) {
  // Exclusive end
  uint64_t partition_entries_lba_end = (
    partition_entries_lba +
    (num_partition_entries * partition_entry_size + GPT_BLOCK_SIZE - 1) / GPT_BLOCK_SIZE
  );
  uint32_t num_entries;
  num_entries = GPT_BLOCK_SIZE / partition_entry_size;
  for (uint64_t i = partition_entries_lba; i < partition_entries_lba_end; i++) {
    sd_copy(block_buf, i, 1);
    gpt_partition_range range = gpt_find_partition_by_guid (
      block_buf, partition_type_guid, num_entries
    );
    if (gpt_is_valid_partition_range(range)) {
      return range;
    }
  }
  return gpt_invalid_partition_range();
}

int main() {
	uint32_t r, CORE_HZ;
	uint16_t gpio_in;

	gpio_in = (GPIO_A_BASE->INPUT)&0xff;
	CORE_HZ = (gpio_in + 1) * 1000000;

	Uart_Config uartConfig;
	uartConfig.dataLength = 8;
	uartConfig.parity = NONE;
	uartConfig.stop = ONE;
	uartConfig.clockDivider = (CORE_HZ / 8 /115200) - 1;
	uart_applyConfig(UART,&uartConfig);

	/* [0]: cpol; [1]: cpha => sdcard mode is both zero
		   [31..4]: ssActiveHigh: only 1 spi device. and it actives low */
	SPI->CONFIG = 0;

	/* [0]: command fifo empty interrupt enable
	   [1]: read fifo emptu interrupt enable
	   [8]: command fifo empty interrupt pending
	   [9]: read fifo empty interrupt pending */
	SPI->STATUS = 0;

	print("\n------------------- in BootROM now --------------------\r\n");
	print("__________________PHAM LAB ! 範研究室__________________\r\n");
	print("| Tokyo, Japan                         | 東京  日本   |\r\n");
	print("| Hello !                              | こんにちは！ |\r\n");
	print("| University of Electro-Communications | 電気通信大学 |\r\n");
	print("| Cee you again !                  :)  |   ¯\\_ツ_/¯   |\r\n");
	print("¯¯↑¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\r\n");

	print("\nINIT\r\n");
	sd_poweron(CORE_HZ);

	if (sd_cmd0()) print("ok\r\n"); //reset sd card
	else print("fail\r\n");

	if (sd_cmd8()) print("ok\r\n"); //check status sd card (type)
	else print("fail\r\n");

	if (sd_acmd41()) print("ok\r\n"); //init sd card
	else print("fail\r\n");

	if (sd_cmd58()) print("ok\r\n"); //init success
	else print("fail\r\n");

	if (sd_cmd16()) print("ok\r\n");	//set block length
	else print("fail\r\n");

	uint8_t gpt_buf[GPT_BLOCK_SIZE];
	int error;

	// SPI frequency = FCLK / (2 * clockDivider)
	// set clock to 20MHz
	SPI->CLOCK_DIVIDER = CORE_HZ / 20000000 / 2;

	print("copy sd header: ");
	error = sd_copy(gpt_buf, GPT_HEADER_LBA, 1);
	if (error) print("done\r\n");
	else { print("fail\r\n"); return 1; }

	print("finding partition: ");
	gpt_partition_range part_range;
	{
	  // header will be overwritten by find_sd_gpt_partition(), so locally
	  // scope it.
	  gpt_header* header = (gpt_header*) gpt_buf;
	  part_range = find_sd_gpt_partition(
	    header->partition_entries_lba,
	    header->num_partition_entries,
	    header->partition_entry_size,
	    &gpt_guid_sifive_fsbl,		// fsbl guid
	    gpt_buf
	  );
	}
	if (gpt_is_valid_partition_range(part_range)) print("found\r\n");
	else { print("fail\r\n"); return 1;	}

	print("copy sd data to RAM: ");
	error = sd_copy(
	  NULL,
	  part_range.first_lba,
	  part_range.last_lba + 1 - part_range.first_lba
	  //PAYLOAD_SIZE
	);
	if (error) print("done\r\n");
	else { print("fail\r\n"); return 1; }

	SPI->DATA = 0x10000000; //disable the SS line 0
	print("\njump to RAM now\r\n\n");

	return 0;
}


void irqCallback(){

}
