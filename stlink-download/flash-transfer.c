/* STM32F10x flash bulk write. */
/*
 * This file isn't directly used.  It is compiled to assembly code
 * which is then hand-tuned for use in stlink-download.c
 *
 * It appears that the STLink interface cannot directly generate the
 * 16 bit writes necessary to write the flash memory.  To do the write
 * we need to download and run a program.
 *
 * While it was a super PITA to figure this out, doing it this way is
 * a much faster approach than writing a word at a time through the STLink.
 * If flash write would have been available directly from STLink, I would
 * likely have spent even longer tuning that inheriently-slow method.
 */

/* FPEC flash controller interface, pm0063 manual */
#define FLASH_REGS_ADDR 0x40022000
#define FLASH_REGS_SIZE 0x28

#define FLASH_ACR (FLASH_REGS_ADDR + 0x00)
#define FLASH_KEYR (FLASH_REGS_ADDR + 0x04)
#define FLASH_SR (FLASH_REGS_ADDR + 0x0c)
#define FLASH_CR (FLASH_REGS_ADDR + 0x10)
#define FLASH_AR (FLASH_REGS_ADDR + 0x14)
#define FLASH_OBR (FLASH_REGS_ADDR + 0x1c)
#define FLASH_WRPR (FLASH_REGS_ADDR + 0x20)

short *dest;
int busy_cnt;

/* Attribute naked means the function prolog and postscript is
 * assumed to be present. */
int __attribute__((naked))
stm_flash_bulk_write(short *src, short *dest_in, volatile int count)
{
	int volatile  *flash_regs = (void*)FLASH_REGS_ADDR;
	int flash_sr;

	flash_regs[4] = 1;
	dest = dest_in;
	do {
		*dest++ = *src++;
		do busy_cnt++;
		while ((flash_sr = flash_regs[3]) & 0x01);
		if (flash_sr & 0x14)
			break;
	} while (count-=2);
	/* Halt. */
	return *src++;
}

#define F4_FLASH_REGS 0x40023C00

int __attribute__((naked))
stm_f4_flash_bulk_write(short *src, short *dest_in, volatile int count)
{
	int volatile  *flash_regs = (void*)F4_FLASH_REGS;
	int flash_sr;

	flash_regs[4] = 1;
	dest = dest_in;
	do {
		*dest++ = *src++;
		while ((flash_sr = flash_regs[3]) & 0x1000)
			;
		if (flash_sr & 0x14)
			break;
	} while (count-=2);
	/* Halt. */
	return *src++;
}

/*
 * Local variables:
 *  compile-command: "arm-none-eabi-gcc -O3 -fconserve-stack -fcaller-saves -mcpu=cortex-m3 -mthumb -S flash-transfer.c"
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
