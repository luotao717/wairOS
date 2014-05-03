/*
 * Ralink MT7620 SoC specific definitions
 *
 * Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * Parts of this file are based on Ralink's 2.6.21 BSP
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef _MT7620_H_
#define _MT7620_H_

#include <linux/init.h>
#include <linux/io.h>

enum mt762x_soc_type {
	MT762X_SOC_UNKNOWN = 0,
	MT762X_SOC_MT7620,
};

extern enum mt762x_soc_type mt762x_soc;

static inline int soc_is_mt7620(void)
{
	return mt762x_soc == MT762X_SOC_MT7620;
}

#define MT7620_MEM_SIZE_MIN (2 * 1024 * 1024)
#define MT7620_MEM_SIZE_MAX (64 * 1024 * 1024)

#define RT3352_MEM_SIZE_MIN (2 * 1024 * 1024)
#define RT3352_MEM_SIZE_MAX (256 * 1024 * 1024)

#define MT7620_CPU_IRQ_BASE	0
#define MT7620_INTC_IRQ_BASE	8
#define MT7620_INTC_IRQ_COUNT	32
#define MT7620_GPIO_IRQ_BASE	40

#define MT7620_CPU_IRQ_INTC	(MT7620_CPU_IRQ_BASE + 2)
#define MT7620_CPU_IRQ_FE	(MT7620_CPU_IRQ_BASE + 5)
#define MT7620_CPU_IRQ_WNIC	(MT7620_CPU_IRQ_BASE + 6)
#define MT7620_CPU_IRQ_COUNTER	(MT7620_CPU_IRQ_BASE + 7)

#define MT7620_INTC_IRQ_SYSCTL	(MT7620_INTC_IRQ_BASE + 0)
#define MT7620_INTC_IRQ_TIMER0	(MT7620_INTC_IRQ_BASE + 1)
#define MT7620_INTC_IRQ_TIMER1	(MT7620_INTC_IRQ_BASE + 2)
#define MT7620_INTC_IRQ_IA	(MT7620_INTC_IRQ_BASE + 3)
#define MT7620_INTC_IRQ_PCM	(MT7620_INTC_IRQ_BASE + 4)
#define MT7620_INTC_IRQ_UART0	(MT7620_INTC_IRQ_BASE + 5)
#define MT7620_INTC_IRQ_PIO	(MT7620_INTC_IRQ_BASE + 6)
#define MT7620_INTC_IRQ_DMA	(MT7620_INTC_IRQ_BASE + 7)
#define MT7620_INTC_IRQ_NAND	(MT7620_INTC_IRQ_BASE + 8)
#define MT7620_INTC_IRQ_PERFC	(MT7620_INTC_IRQ_BASE + 9)
#define MT7620_INTC_IRQ_I2S	(MT7620_INTC_IRQ_BASE + 10)
#define MT7620_INTC_IRQ_UART1	(MT7620_INTC_IRQ_BASE + 12)
#define MT7620_INTC_IRQ_ESW	(MT7620_INTC_IRQ_BASE + 17)
#define MT7620_INTC_IRQ_OTG	(MT7620_INTC_IRQ_BASE + 18)

extern void __iomem *mt7620_sysc_base;
extern void __iomem *mt7620_memc_base;

static inline void mt7620_sysc_wr(u32 val, unsigned reg)
{
	__raw_writel(val, mt7620_sysc_base + reg);
}

static inline u32 mt7620_sysc_rr(unsigned reg)
{
	return __raw_readl(mt7620_sysc_base + reg);
}

static inline void mt7620_memc_wr(u32 val, unsigned reg)
{
	__raw_writel(val, mt7620_memc_base + reg);
}

static inline u32 mt7620_memc_rr(unsigned reg)
{
	return __raw_readl(mt7620_memc_base + reg);
}

#define MT7620_GPIO_I2C_SD	1
#define MT7620_GPIO_I2C_SCLK	2
#define MT7620_GPIO_SPI_EN	3
#define MT7620_GPIO_SPI_CLK	4
#define MT7620_GPIO_SPI_DOUT	5
#define MT7620_GPIO_SPI_DIN	6
/* GPIO 7-14 is shared between UART0, PCM  and I2S interfaces */
#define MT7620_GPIO_7		7
#define MT7620_GPIO_8		8
#define MT7620_GPIO_9		9
#define MT7620_GPIO_10		10
#define MT7620_GPIO_11		11
#define MT7620_GPIO_12		12
#define MT7620_GPIO_13		13
#define MT7620_GPIO_14		14
#define MT7620_GPIO_UART1_TXD	15
#define MT7620_GPIO_UART1_RXD	16
#define MT7620_GPIO_JTAG_TDO	17
#define MT7620_GPIO_JTAG_TDI	18
#define MT7620_GPIO_JTAG_TMS	19
#define MT7620_GPIO_JTAG_TCLK	20
#define MT7620_GPIO_JTAG_TRST_N	21
#define MT7620_GPIO_MDIO_MDC	22
#define MT7620_GPIO_MDIO_MDIO	23
#define MT7620_GPIO_SDRAM_MD16	24
#define MT7620_GPIO_SDRAM_MD17	25
#define MT7620_GPIO_SDRAM_MD18	26
#define MT7620_GPIO_SDRAM_MD19	27
#define MT7620_GPIO_SDRAM_MD20	28
#define MT7620_GPIO_SDRAM_MD21	29
#define MT7620_GPIO_SDRAM_MD22	30
#define MT7620_GPIO_SDRAM_MD23	31
#define MT7620_GPIO_SDRAM_MD24	32
#define MT7620_GPIO_SDRAM_MD25	33
#define MT7620_GPIO_SDRAM_MD26	34
#define MT7620_GPIO_SDRAM_MD27	35
#define MT7620_GPIO_SDRAM_MD28	36
#define MT7620_GPIO_SDRAM_MD29	37
#define MT7620_GPIO_SDRAM_MD30	38
#define MT7620_GPIO_SDRAM_MD31	39
#define MT7620_GPIO_GE0_TXD0	40
#define MT7620_GPIO_GE0_TXD1	41
#define MT7620_GPIO_GE0_TXD2	42
#define MT7620_GPIO_GE0_TXD3	43
#define MT7620_GPIO_GE0_TXEN	44
#define MT7620_GPIO_GE0_TXCLK	45
#define MT7620_GPIO_GE0_RXD0	46
#define MT7620_GPIO_GE0_RXD1	47
#define MT7620_GPIO_GE0_RXD2	48
#define MT7620_GPIO_GE0_RXD3	49
#define MT7620_GPIO_GE0_RXDV	50
#define MT7620_GPIO_GE0_RXCLK	51

void mt7620_gpio_init(u32 mode);

#endif /* _MT7620_H_ */
