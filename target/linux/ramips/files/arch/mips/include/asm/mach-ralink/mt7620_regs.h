/*
 *  Ralink RT305 SoC register definitions
 *
 *  Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _MT7620_REGS_H_
#define _MT7620_REGS_H_

#include <linux/bitops.h>

#define MT7620_SDRAM_BASE		0x00000000
#define MT7620_SYSC_BASE		0x10000000
#define MT7620_TIMER_BASE		0x10000100
#define MT7620_INTC_BASE		0x10000200
#define MT7620_MEMC_BASE		0x10000300
#define MT7620_RBUS_MATRIXCTL_BASE	0x10000400
#define MT7620_UART0_BASE		0x10000500
#define MT7620_PIO_BASE			0x10000600
#define MT7620_NANDC_BASE		0x10000800
#define MT7620_I2C_BASE			0x10000900
#define MT7620_I2S_BASE			0x10000a00
#define MT7620_SPI_BASE			0x10000b00
#define MT7620_UART1_BASE		0x10000c00
#define MT7620_MIPS_CNT_BASE		0x10000d00
#define MT7620_PCM_BASE			0x10002000
#define MT7620_GDMA_BASE		0x10002800
#define MT7620_CRYPTO_ENGINE_BASE	0x10004000
#define MT7620_FE_BASE			0x10100000
#define MT7620_PPE_BASE			0x10100c00
#define MT7620_SWITCH_BASE		0x10110000
#define MT7620_USB_DEV_BASE		0x10120000
#define MT7620_PCI_BASE                 0x10140000
#define MT7620_WMAC_BASE		0x10180000
#define MT7620_OTG_BASE			0x101c0000

#define RALINK_MCNT_CFG			0x10000d00
#define RALINK_COMPARE			0x10000d04
#define RALINK_COUNT			0x10000d08

/*template*/
#define MT7620_ROM_BASE		0x00400000
#define MT7620_FLASH1_BASE	0x1b000000
#define MT7620_FLASH0_BASE	0x1f000000

#define MT7620_SYSC_SIZE	0x100
#define MT7620_TIMER_SIZE	0x100
#define MT7620_INTC_SIZE	0x100
#define MT7620_MEMC_SIZE	0x100
#define MT7620_UART0_SIZE	0x100
#define MT7620_PIO_SIZE		0x100
#define MT7620_UART1_SIZE	0x100
#define MT7620_SPI_SIZE		0x100
#define MT7620_FLASH1_SIZE	(16 * 1024 * 1024)
#define MT7620_FLASH0_SIZE	(8 * 1024 * 1024)

/*From the RT3352 edit*/
#define MT7620_EHCI_BASE	0x101c0000
#define MT7620_EHCI_SIZE	0x1000
#define MT7620_OHCI_BASE	0x101c1000
#define MT7620_OHCI_SIZE	0x1000

/********************************************************************************
 *
 *			MT7620_SYSC_BASE
 * 
 ********************************************************************************/
/* SYSC registers MT7620_SYSC_BASE */
#define SYSC_REG_CHIP_NAME0	0x000	/* Chip Name 0 */
#define SYSC_REG_CHIP_NAME1	0x004	/* Chip Name 1 */
#define SYSC_REG_CHIP_ID	0x00c	/* Chip Identification */
#define SYSC_REG_SYSTEM_CONFIG	0x010	/* System Configuration 0 */
#define SYSC_REG_SYSTEM_CONFIG1	0x014	/* System Configuration 1*/
#define SYSC_REG_CLKCFG0	0x02c	/* Clock Configuration Register 0*/
#define SYSC_REG_CLKCFG1	0x030	/* Clock Configuration Register 1*/
#define SYSC_REG_RESET_CTRL	0x034	/* Reset Control*/
#define SYSC_REG_RESET_STATUS	0x038	/* Reset Status*/
#define SYSC_REG_CPLL_CFG0	0x054	/* CPU PLL Configuration 0*/
#define SYSC_REG_CPLL_CFG1	0x058	/* CPU PLL Configuration 1*/
#define SYSC_REG_USB_PHY_CFG	0x05c	/* USB PHY controll*/
#define SYSC_REG_GPIO_MODE	0x060	/* GPIO Purpose Select */
#define SYSC_REG_IA_ADDRESS	0x310	/* Illegal Access Address */
#define SYSC_REG_IA_TYPE	0x314	/* Illegal Access Type */


/*SYSC_REG_CHIP_NAME0 && SYSC_REG_CHIP_NAME1*/
#define MT7620_CHIP_NAME0	0x3637544d
#define MT7620_CHIP_NAME1	0x20203032

/*RAMIPS_REVID SYSC_REG_CHIP_ID	*/
#define CHIP_ID_PKG_ID		BIT(16)
#define CHIP_ID_ID_MASK		0xf
#define CHIP_ID_VER_ID_SHIFT	8
#define CHIP_ID_ECO_SHIFT	0
#define CHIP_ID_REV_MASK	0xff

/* SYSC_REG_SYSTEM_CONFIG	*/
#define SYSCFG0_XTAL_SEL		BIT(6) /* 20 -> 40 Mhz*/

/* SYSC_REG_SYSTEM_CONFIG1 */
#define SYSCONFIG1_USB0_HOST_MODE	BIT(10)
#define SYSCONFIG1_GE_MODE_MASK		0x3UL
#define SYSCONFIG1_GE1_MODE_SHIFT	12
#define SYSCONFIG1_GE2_MODE_SHIFT	14

/* SYSC_REG_CLKCFG1 */
#define CLKCFG1_UPHY1_CLK_EN		BIT(22) /*UPHY 48M clock enable*/
#define CLKCFG1_UPHY0_CLK_EN		BIT(25) /*UPHY 12M clock enable*/
#define CLKCFG1_PCIE0_CLK_EN		BIT(26) /*PCIE0 clock enable*/
#define CLKCFG1_PCIE1_CLK_EN		BIT(27) /*PCIE1 clock enable*/

/* SYSC_REG_RESET_CTRL	*/
#define MT7620_RESET_SYSTEM	BIT(0)
#define MT7620_RESET_TIMER	BIT(8)
#define MT7620_RESET_INTC	BIT(9)
#define MT7620_RESET_MEMC	BIT(10)
#define MT7620_RESET_PCM	BIT(11)
#define MT7620_RESET_UART0	BIT(12)
#define MT7620_RESET_PIO	BIT(13)
#define MT7620_RESET_DMA	BIT(14)
#define MT7620_RESET_I2C	BIT(16)
#define MT7620_RESET_I2S	BIT(17)
#define MT7620_RESET_SPI	BIT(18)
#define MT7620_RESET_UART1	BIT(19)
/*#define MT7620_RESET_WNIC	BIT(20)*/
#define MT7620_RESET_FE		BIT(21)
#define MT7620_RESET_UHST	BIT(22) /*USB PHY Reset*/
#define MT7620_RESET_ESW	BIT(23)
#define MT7620_EPHY_RST		BIT(24)
#define MT7620_UDEV_RST		BIT(25)
#define MT7620_PCIE0_RST	BIT(26)
#define MT7620_PCIE1_RST	BIT(27)
#define MT7620_MIPS_CNT_RST	BIT(28)
#define MT7620_CRYPTO_RST	BIT(29)

/* SYSC_REG_CPLL_CFG0	*/
#define BASE_CLOCK              40      /* Mhz */

#define CPLL_DIV_RATIO_SHIFT    10
#define CPLL_DIV_RATIO          (0x3UL << CPLL_DIV_RATIO_SHIFT)
#define CPLL_MULT_RATIO_SHIFT   16
#define CPLL_MULT_RATIO         (0x7UL << CPLL_MULT_RATIO_SHIFT)
#define CPLL_SW_CONFIG		BIT(31)		/* CPU PLL Software Configuration bit1 by software */

/* SYSC_REG_CPLL_CFG1	*/
#define CPU_CLK_AUX0		BIT(24)		/*CPU Clock Auxiliary 0 Enable : bit 480Mhz*/

/* SYSC_REG_GPIO_MODE */
#define MT7620_GPIO_MODE_I2C		BIT(0)
#define MT7620_GPIO_MODE_UART0_SHIFT	2
#define MT7620_GPIO_MODE_UART0_MASK	0x7
#define MT7620_GPIO_MODE_UART0(x)	((x) << MT7620_GPIO_MODE_UART0_SHIFT)
#define MT7620_GPIO_MODE_UARTF		0x0
#define MT7620_GPIO_MODE_PCM_UARTF	0x1
#define MT7620_GPIO_MODE_PCM_I2S	0x2
#define MT7620_GPIO_MODE_I2S_UARTF	0x3
#define MT7620_GPIO_MODE_PCM_GPIO	0x4
#define MT7620_GPIO_MODE_GPIO_UARTF	0x5
#define MT7620_GPIO_MODE_GPIO_I2S	0x6
#define MT7620_GPIO_MODE_GPIO		0x7
#define MT7620_GPIO_MODE_UART1		BIT(5)
/*#define MT7620_GPIO_MODE_JTAG		BIT(6)*/
#define MT7620_GPIO_MODE_MDIO_SHIFT	7  /*2'b00 Normal ; 2'b10 GPIO*/
#define MT7620_GPIO_MODE_MDIO_MASK 	0x03UL	
/*#define MT7620_GPIO_MODE_MDIO		BIT(7)*/
/*#define MT7620_GPIO_MODE_SDRAM		BIT(8)*/
#define MT7620_GPIO_MODE_RGMII1		BIT(9)
#define MT7620_GPIO_MODE_RGMII2		BIT(10)
#define MT7620_GPIO_MODE_SPI		BIT(11)

/********************************************************************************
 *
 *			MT7620_INTC_BASE	
 * 
 ********************************************************************************/
#define INTC_REG_STATUS0	0x00
#define INTC_REG_STATUS1	0x04
#define INTC_REG_TYPE		0x20
#define INTC_REG_RAW_STATUS	0x30
#define INTC_REG_ENABLE		0x34
#define INTC_REG_DISABLE	0x38

/* all BIT of interrupt */
#define MT7620_INTC_INT_SYSCTL	BIT(0)
#define MT7620_INTC_INT_TIMER0	BIT(1)
#define MT7620_INTC_INT_TIMER1	BIT(2)
#define MT7620_INTC_INT_IA	BIT(3)
#define MT7620_INTC_INT_PCM	BIT(4)
#define MT7620_INTC_INT_UART0	BIT(5)
#define MT7620_INTC_INT_PIO	BIT(6)
#define MT7620_INTC_INT_DMA	BIT(7)
#define MT7620_INTC_INT_NAND	BIT(8)
#define MT7620_INTC_INT_PERFC	BIT(9)
#define MT7620_INTC_INT_I2S	BIT(10)
#define MT7620_INTC_INT_UART1	BIT(12)
#define MT7620_INTC_INT_ESW	BIT(17)
#define MT7620_INTC_INT_OTG	BIT(18)
#define MT7620_INTC_INT_GLOBAL	BIT(31)

/********************************************************************************
 *
 *			MT7620_MEMC_BASE
 * 
 ********************************************************************************/
/* MEMC registers */
#define MEMC_REG_SDRAM_CFG0	0x00
#define MEMC_REG_SDRAM_CFG1	0x04
/*
#define MEMC_REG_FLASH_CFG0	0x08
#define MEMC_REG_FLASH_CFG1	0x0c
*/
#define MEMC_REG_IA_ADDR	0x10
#define MEMC_REG_IA_TYPE	0x14
/*
#define FLASH_CFG_WIDTH_SHIFT	26
#define FLASH_CFG_WIDTH_MASK	0x3
#define FLASH_CFG_WIDTH_8BIT	0x0
#define FLASH_CFG_WIDTH_16BIT	0x1
#define FLASH_CFG_WIDTH_32BIT	0x2
*/

/* UART registers */
#define UART_REG_RX	0
#define UART_REG_TX	1
#define UART_REG_IER	2
#define UART_REG_IIR	3
#define UART_REG_FCR	4
#define UART_REG_LCR	5
#define UART_REG_MCR	6
#define UART_REG_LSR	7

#endif /* _MT7620_REGS_H_ */
