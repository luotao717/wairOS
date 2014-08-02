/*
 * Ralink MT7620 SoC specific setup
 *
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 * Parts of this file are based on Ralink's 2.6.21 BSP
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <asm/mipsregs.h>

#include <asm/mach-ralink/common.h>
#include <asm/mach-ralink/ramips_gpio.h>
#include <asm/mach-ralink/mt7620.h>
#include <asm/mach-ralink/mt7620_regs.h>

void __iomem * mt7620_sysc_base;
void __iomem * mt7620_memc_base;
enum mt762x_soc_type mt762x_soc;

void __init ramips_soc_prom_init(void)
{
	void __iomem *sysc = (void __iomem *) KSEG1ADDR(MT7620_SYSC_BASE);
	const char *name = "unknown";
	u32 n0;
	u32 n1;
	u32 id;

	n0 = __raw_readl(sysc + SYSC_REG_CHIP_NAME0);
	n1 = __raw_readl(sysc + SYSC_REG_CHIP_NAME1);

	if (n0 == MT7620_CHIP_NAME0 && n1 == MT7620_CHIP_NAME1) {
		mt762x_soc = MT762X_SOC_MT7620;
		name = "MT7620";
	} else {
		panic("mt7620: unknown SoC, n0:%08x n1:%08x\n", n0, n1);
	}

	id = __raw_readl(sysc + SYSC_REG_CHIP_ID);

	snprintf(ramips_sys_type, RAMIPS_SYS_TYPE_LEN,
		"Ralink %s Package ID : %s version number:%u eco number:%u \r\n",
		name,
		(id & CHIP_ID_PKG_ID ) ? "DRQFN" : "TFBGA",
		(id >> CHIP_ID_VER_ID_SHIFT) & CHIP_ID_ID_MASK,
		(id >> CHIP_ID_ECO_SHIFT ) & CHIP_ID_ID_MASK);

	ramips_mem_base = MT7620_SDRAM_BASE;

	if (soc_is_mt7620() ) {
		ramips_mem_size_min = MT7620_MEM_SIZE_MIN;
		ramips_mem_size_max = MT7620_MEM_SIZE_MAX;
	} else {
		BUG();
	}
}

static struct ramips_gpio_chip mt7620_gpio_chips[] = {
	{
		.chip = {
			.label	= "MT7620-GPIO0",
			.base	= 0,
			.ngpio	= 24,
		},
		.regs = {
			[RAMIPS_GPIO_REG_INT]	= 0x00,
			[RAMIPS_GPIO_REG_EDGE]	= 0x04,
			[RAMIPS_GPIO_REG_RENA]	= 0x08,
			[RAMIPS_GPIO_REG_FENA]	= 0x0c,
			[RAMIPS_GPIO_REG_DATA]	= 0x20,
			[RAMIPS_GPIO_REG_DIR]	= 0x24,
			[RAMIPS_GPIO_REG_POL]	= 0x28,
			[RAMIPS_GPIO_REG_SET]	= 0x2c,
			[RAMIPS_GPIO_REG_RESET]	= 0x30,
			[RAMIPS_GPIO_REG_TOGGLE] = 0x34,
		},
		.map_base = MT7620_PIO_BASE,
		.map_size = MT7620_PIO_SIZE,
	},
	{
		.chip = {
			.label	= "MT7620-GPIO1",
			.base	= 24,
			.ngpio	= 16,
		},
		.regs = {
			[RAMIPS_GPIO_REG_INT]	= 0x38,
			[RAMIPS_GPIO_REG_EDGE]	= 0x3c,
			[RAMIPS_GPIO_REG_RENA]	= 0x40,
			[RAMIPS_GPIO_REG_FENA]	= 0x44,
			[RAMIPS_GPIO_REG_DATA]	= 0x48,
			[RAMIPS_GPIO_REG_DIR]	= 0x4c,
			[RAMIPS_GPIO_REG_POL]	= 0x50,
			[RAMIPS_GPIO_REG_SET]	= 0x54,
			[RAMIPS_GPIO_REG_RESET]	= 0x58,
			[RAMIPS_GPIO_REG_TOGGLE] = 0x5c,
		},
		.map_base = MT7620_PIO_BASE,
		.map_size = MT7620_PIO_SIZE,
	},
	{
		.chip = {
			.label	= "MT7620-GPIO2",
			.base	= 40,
			.ngpio	= 32,
		},
		.regs = {
			[RAMIPS_GPIO_REG_INT]	= 0x60,
			[RAMIPS_GPIO_REG_EDGE]	= 0x64,
			[RAMIPS_GPIO_REG_RENA]	= 0x68,
			[RAMIPS_GPIO_REG_FENA]	= 0x6c,
			[RAMIPS_GPIO_REG_DATA]	= 0x70,
			[RAMIPS_GPIO_REG_DIR]	= 0x74,
			[RAMIPS_GPIO_REG_POL]	= 0x78,
			[RAMIPS_GPIO_REG_SET]	= 0x7c,
			[RAMIPS_GPIO_REG_RESET]	= 0x80,
			[RAMIPS_GPIO_REG_TOGGLE] = 0x84,
		},
		.map_base = MT7620_PIO_BASE,
		.map_size = MT7620_PIO_SIZE,
	},
	{
		.chip = {
			.label	= "MT7620-GPIO3",
			.base	= 72,
			.ngpio	= 1,
		},
		.regs = {
			[RAMIPS_GPIO_REG_INT]	= 0x88,
			[RAMIPS_GPIO_REG_EDGE]	= 0x8c,
			[RAMIPS_GPIO_REG_RENA]	= 0x90,
			[RAMIPS_GPIO_REG_FENA]	= 0x94,
			[RAMIPS_GPIO_REG_DATA]	= 0x98,
			[RAMIPS_GPIO_REG_DIR]	= 0x9c,
			[RAMIPS_GPIO_REG_POL]	= 0xa0,
			[RAMIPS_GPIO_REG_SET]	= 0xa4,
			[RAMIPS_GPIO_REG_RESET]	= 0xa8,
			[RAMIPS_GPIO_REG_TOGGLE] = 0xac,
		},
		.map_base = MT7620_PIO_BASE,
		.map_size = MT7620_PIO_SIZE,
	},
};

struct ramips_gpio_data mt7620_gpio_data = {
	.chips = mt7620_gpio_chips,
	.num_chips = ARRAY_SIZE(mt7620_gpio_chips),
};

static void mt7620_gpio_reserve(int first, int last)
{
	for (; first <= last; first++)
		gpio_request(first, "reserved");
}

void __init mt7620_gpio_init(u32 mode)
{
	u32 t;

	mt7620_sysc_wr(mode, SYSC_REG_GPIO_MODE);

	ramips_gpio_init(&mt7620_gpio_data);
#if 0
	if ((mode & MT7620_GPIO_MODE_I2C) == 0)
		mt7620_gpio_reserve(MT7620_GPIO_I2C_SD, MT7620_GPIO_I2C_SCLK);
#endif

	if ((mode & MT7620_GPIO_MODE_SPI) == 0)
		mt7620_gpio_reserve(MT7620_GPIO_SPI_EN, MT7620_GPIO_SPI_CLK);

	t = mode >> MT7620_GPIO_MODE_UART0_SHIFT;
	t &= MT7620_GPIO_MODE_UART0_MASK;
	switch (t) {
	case MT7620_GPIO_MODE_UARTF:
	case MT7620_GPIO_MODE_PCM_UARTF:
	case MT7620_GPIO_MODE_PCM_I2S:
	case MT7620_GPIO_MODE_I2S_UARTF:
		mt7620_gpio_reserve(MT7620_GPIO_7, MT7620_GPIO_14);
		break;
	case MT7620_GPIO_MODE_PCM_GPIO:
		mt7620_gpio_reserve(MT7620_GPIO_10, MT7620_GPIO_14);
		break;
	case MT7620_GPIO_MODE_GPIO_UARTF:
	case MT7620_GPIO_MODE_GPIO_I2S:
		mt7620_gpio_reserve(MT7620_GPIO_7, MT7620_GPIO_10);
		break;
	}

	if ((mode & MT7620_GPIO_MODE_UART1) == 0)
		mt7620_gpio_reserve(MT7620_GPIO_UART1_TXD,
				    MT7620_GPIO_UART1_RXD);
/*
	if ((mode & MT7620_GPIO_MODE_JTAG) == 0)
		mt7620_gpio_reserve(MT7620_GPIO_JTAG_TDO, MT7620_GPIO_JTAG_TDI);
*/

	if ((mode & (MT7620_GPIO_MODE_MDIO_MASK << MT7620_GPIO_MODE_MDIO_SHIFT )) != 2)
		mt7620_gpio_reserve(MT7620_GPIO_MDIO_MDC,
				    MT7620_GPIO_MDIO_MDIO);

/*
	if ((mode & MT7620_GPIO_MODE_SDRAM) == 0)
		mt7620_gpio_reserve(MT7620_GPIO_SDRAM_MD16,
				    MT7620_GPIO_SDRAM_MD31);
*/

	if ((mode & MT7620_GPIO_MODE_RGMII1) == 0)
		mt7620_gpio_reserve(MT7620_GPIO_GE0_TXD0,
				    MT7620_GPIO_GE0_RXCLK);
}
