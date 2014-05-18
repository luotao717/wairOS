/*
 *  Ralink MT7620 SoC platform device registration
 *
 *  Copyright (C) 2009-2010 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/spi/spi.h>
#include <linux/mt762x_platform.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>

#include <asm/addrspace.h>

#include <asm/mach-ralink/mt7620.h>
#include <asm/mach-ralink/mt7620_regs.h>
#include "devices.h"

#include <ramips_eth_platform.h>
#include <mt7620_esw_platform.h>
#include <mt7620_ehci_platform.h>
#include <mt7620_ohci_platform.h>


static void mt7620_fe_reset(void)
{
	mt7620_sysc_wr(MT7620_RESET_FE, SYSC_REG_RESET_CTRL);
	mt7620_sysc_wr(0, SYSC_REG_RESET_CTRL);
}

static struct resource mt7620_eth_resources[] = {
	{
		.name 	= "mac_base",
		.start	= MT7620_FE_BASE,
		.end	= MT7620_FE_BASE + PAGE_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= MT7620_CPU_IRQ_FE,
		.end	= MT7620_CPU_IRQ_FE,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct ramips_eth_platform_data ramips_eth_data = {
	.mac = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 },
	.reset_fe = mt7620_fe_reset,
	.min_pkt_len = 64,
};

static struct platform_device mt7620_eth_device = {
	.name		= "ramips_eth",
	.resource	= mt7620_eth_resources,
	.num_resources	= ARRAY_SIZE(mt7620_eth_resources),
	.dev = {
		.platform_data = &ramips_eth_data,
	}
};

/*only support p4 p5
 * port : 4  
 * */

static int mt7620_esw_set_port_mode(u8 port , u8 mode)
{
	int ret = -1;
	u32 gpio_val = 0, mode_val = 0;
	u32 gpio_set_val=0,mode_set_shift = 0, mode_set_val = 0;

	if ( ( port >= 4 )  && ( port <= 5 ) ) 
	{
		gpio_val = mt7620_sysc_rr ( SYSC_REG_GPIO_MODE );	
		gpio_set_val = ( port == 4 ) ? MT7620_GPIO_MODE_RGMII2 : MT7620_GPIO_MODE_RGMII1 ;

		mode_val = mt7620_sysc_rr ( SYSC_REG_SYSTEM_CONFIG1 );
		mode_set_shift = ( port == 4 ) ? SYSCONFIG1_GE2_MODE_SHIFT : SYSCONFIG1_GE1_MODE_SHIFT;
		mode_set_val = SYSCONFIG1_GE_MODE_MASK << mode_set_shift;
		mode_val &= ~mode_set_val;

		if ( mode == MT7620_PM_RGMII_TO_MAC )
		{
			gpio_val &= ~gpio_set_val;
			/*RGMII MODE*/
		}
		else if ( mode == MT7620_PM_MII_TO_MAC ) 
		{
			gpio_val &= ~gpio_set_val;
			
			mode_val |= 0x01 << mode_set_shift;/*mii mode*/
		}
		else if ( mode == MT7620_PM_RMII_TO_MAC ) 
		{
			gpio_val &= ~gpio_set_val;
			mode_val |= 0x02 << mode_set_shift; /*RMII MODE*/
		}
		else if ( mode == MT7620_PM_MAC_TO_PHY ) 
		{
			gpio_val &= ~gpio_set_val;
			mode_val |= 0x03 << mode_set_shift; /*RJ-45 Mode: PHY4*/
			/*MAC to PHY MODE*/
		}
		else /*MT7620_PM_DISABLE*/ 
		{
			gpio_val |= gpio_set_val;
		
		}
		mt7620_sysc_wr ( gpio_val, SYSC_REG_GPIO_MODE );
		
		mt7620_sysc_wr ( mode_val, SYSC_REG_SYSTEM_CONFIG1 );
	/*	printk(KERN_INFO "%08x = %08x gpio %08x = %08x \n",
				SYSC_REG_SYSTEM_CONFIG1, mode_val, SYSC_REG_GPIO_MODE, gpio_val);
	*/
	

		ret  = 0;
	}

	return ret;
}

static void mt7620_esw_reset(int mode)
{
	u32 value  = 0;	
	if ( mode == MT7620_ESW_RESET_ALL ) {
		value = ( MT7620_RESET_ESW | MT7620_EPHY_RST );
	}else if ( mode == MT7620_ESW_RESET_SWITCH ) {
		value = MT7620_RESET_ESW ;
	}else /*if (mode == MT7620_ESW_RESET_EPHY*/ {
		value = MT7620_EPHY_RST ;
	}  
	
	mt7620_sysc_wr(value, SYSC_REG_RESET_CTRL);
	mt7620_sysc_wr(0, SYSC_REG_RESET_CTRL);
}
static struct resource mt7620_esw_resources[] = {
	{
		.name 	= "esw_base",
		.start	= MT7620_SWITCH_BASE,
		.end	= MT7620_SWITCH_BASE + PAGE_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

struct mt7620_esw_platform_data mt7620_esw_data = {
	/* All ports are LAN ports. */
	.vlan_config		= MT7620_ESW_VLAN_CONFIG_NONE,
	.reset_esw		= mt7620_esw_reset,
#if 0
	.reg_initval_fct2	= 0x00d6500c,
	/*
	 * ext phy base addr 31, enable port 5 polling, rx/tx clock skew 1,
	 * turbo mii off, rgmi 3.3v off
	 * port5: disabled
	 * port6: enabled, gige, full-duplex, rx/tx-flow-control
	 */
	.reg_initval_fpa2	= 0x3f502b28,
#endif
};

static struct platform_device mt7620_esw_device = {
	.name		= "mt7620-esw",
	.resource	= mt7620_esw_resources,
	.num_resources	= ARRAY_SIZE(mt7620_esw_resources),
	.dev = {
		.platform_data = &mt7620_esw_data,
	}
};

void __init mt7620_register_ethernet(void)
{
	struct clk *clk;
	u32 regdata=0;

	clk = clk_get(NULL, "sys");
	if (IS_ERR(clk))
		panic("unable to get SYS clock, err=%ld", PTR_ERR(clk));

	ramips_eth_data.sys_freq = clk_get_rate(clk);
	
	ramips_eth_data.speed = 1000;
	ramips_eth_data.duplex = 1;
 	ramips_eth_data.tx_fc = 1;	
 	ramips_eth_data.rx_fc = 1;	
	
	/*esw init*/
	mt7620_esw_set_port_mode ( 4, mt7620_esw_data.p4_phy_mode ) ;
	mt7620_esw_set_port_mode ( 5, mt7620_esw_data.p5_phy_mode ) ;
	mt7620_esw_data.is_bga =  ( mt7620_sysc_rr(SYSC_REG_CHIP_ID) & CHIP_ID_PKG_ID ) ? 1:0 ;

	/*pci to rc mode*/
	regdata = mt7620_sysc_rr( SYSC_REG_SYSTEM_CONFIG1 );	
	regdata |= BIT(8); /*pci*/
	mt7620_sysc_wr(regdata, SYSC_REG_SYSTEM_CONFIG1 );

	platform_device_register(&mt7620_esw_device);
	platform_device_register(&mt7620_eth_device);
}

static struct resource mt7620_wifi_resources[] = {
	{
		.start	= MT7620_WMAC_BASE,
		.end	= MT7620_WMAC_BASE + 0x3FFFF,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= MT7620_CPU_IRQ_WNIC,
		.end	= MT7620_CPU_IRQ_WNIC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct mt762x_platform_data mt7620_wifi_data;
static struct platform_device mt7620_wifi_device = {
	.name			= "mt7620_wmac",
	.resource		= mt7620_wifi_resources,
	.num_resources	= ARRAY_SIZE(mt7620_wifi_resources),
	.dev = {
		.platform_data = &mt7620_wifi_data,
	}
};

void __init mt7620_register_wifi(void)
{
	u32 t;
	mt7620_wifi_data.eeprom_file_name = "MT7620.eeprom";

	if ( soc_is_mt7620() ) {
		t = mt7620_sysc_rr(SYSC_REG_SYSTEM_CONFIG);
		t &= SYSCFG0_XTAL_SEL;
		if (!t)
			mt7620_wifi_data.clk_is_20mhz = 1;
	}
	platform_device_register(&mt7620_wifi_device);
}

static struct resource mt7620_wdt_resources[] = {
	{
		.start	= MT7620_TIMER_BASE,
		.end	= MT7620_TIMER_BASE + MT7620_TIMER_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device mt7620_wdt_device = {
	.name		= "ramips-wdt",
	.id		= -1,
	.resource	= mt7620_wdt_resources,
	.num_resources	= ARRAY_SIZE(mt7620_wdt_resources),
};

void __init mt7620_register_wdt(void)
{
#if 0
	u32 t;

	/* enable WDT reset output on pin SRAM_CS_N */
	t = mt7620_sysc_rr(SYSC_REG_SYSTEM_CONFIG);
	t |= MT7620_SYSCFG_SRAM_CS0_MODE_WDT <<
	     MT7620_SYSCFG_SRAM_CS0_MODE_SHIFT;
	mt7620_sysc_wr(t, SYSC_REG_SYSTEM_CONFIG);
#endif

	platform_device_register(&mt7620_wdt_device);
}

static struct resource mt7620_spi_resources[] = {
	{
		.flags	= IORESOURCE_MEM,
		.start	= MT7620_SPI_BASE,
		.end	= MT7620_SPI_BASE + MT7620_SPI_SIZE - 1,
	},
};

static struct platform_device mt7620_spi_device = {
	.name		= "ramips-spi",
	.id		= 0,
	.resource	= mt7620_spi_resources,
	.num_resources	= ARRAY_SIZE(mt7620_spi_resources),
};

void __init mt7620_register_spi(struct spi_board_info *info, int n)
{
	spi_register_board_info(info, n);
	platform_device_register(&mt7620_spi_device);
}

static struct resource mt7620_dwc_otg_resources[] = {
	{
		.start	= MT7620_OTG_BASE,
		.end	= MT7620_OTG_BASE + 0x3FFFF,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= MT7620_INTC_IRQ_OTG,
		.end	= MT7620_INTC_IRQ_OTG,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device mt7620_dwc_otg_device = {
	.name			= "dwc_otg",
	.resource		= mt7620_dwc_otg_resources,
	.num_resources	= ARRAY_SIZE(mt7620_dwc_otg_resources),
	.dev = {
		.platform_data = NULL,
	}
};

static atomic_t mt7620_usb_use_count = ATOMIC_INIT(0);

static void mt7620_usb_host_start(void)
{
	u32 t;

	if (atomic_inc_return(&mt7620_usb_use_count) != 1)
		return;

	t = mt7620_sysc_rr(SYSC_REG_USB_PHY_CFG);

	/* enable clock for port0's and port1's phys */
	t = mt7620_sysc_rr(SYSC_REG_CLKCFG1);
	t = t | CLKCFG1_UPHY0_CLK_EN | CLKCFG1_UPHY1_CLK_EN;
	mt7620_sysc_wr(t, SYSC_REG_CLKCFG1);
	mdelay(500);

	/* pull USBHOST and USBDEV out from reset */
	t = mt7620_sysc_rr(SYSC_REG_RESET_CTRL);
	t &= ~(MT7620_RESET_UHST | MT7620_UDEV_RST);
	mt7620_sysc_wr(t, SYSC_REG_RESET_CTRL);
	mdelay(500);

	/* enable host mode */
	t = mt7620_sysc_rr(SYSC_REG_SYSTEM_CONFIG1);
	t |= SYSCONFIG1_USB0_HOST_MODE;
	mt7620_sysc_wr(t, SYSC_REG_SYSTEM_CONFIG1);

	t = mt7620_sysc_rr(SYSC_REG_USB_PHY_CFG);
}

static void mt7620_usb_host_stop(void)
{
	u32 t;

	if (atomic_dec_return(&mt7620_usb_use_count) != 0)
		return;

	/* put USBHOST and USBDEV into reset */
	t = mt7620_sysc_rr(SYSC_REG_RESET_CTRL);
	t |= MT7620_RESET_UHST | MT7620_UDEV_RST;
	mt7620_sysc_wr(t, SYSC_REG_RESET_CTRL);
	udelay(10000);

	/* disable clock for port0's and port1's phys*/
	t = mt7620_sysc_rr(SYSC_REG_CLKCFG1);
	t &= ~(CLKCFG1_UPHY0_CLK_EN | CLKCFG1_UPHY1_CLK_EN);
	mt7620_sysc_wr(t, SYSC_REG_CLKCFG1);
	udelay(10000);
}

static struct mt7620_ehci_platform_data mt7620_ehci_data = {
	.start_hw	= mt7620_usb_host_start,
	.stop_hw	= mt7620_usb_host_stop,
};

static struct resource mt7620_ehci_resources[] = {
	{
		.start	= MT7620_EHCI_BASE,
		.end	= MT7620_EHCI_BASE + MT7620_EHCI_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= MT7620_INTC_IRQ_OTG,
		.end	= MT7620_INTC_IRQ_OTG,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 mt7620_ehci_dmamask = DMA_BIT_MASK(32);
static struct platform_device mt7620_ehci_device = {
	.name		= "mt7620-ehci",
	.id		= -1,
	.resource	= mt7620_ehci_resources,
	.num_resources	= ARRAY_SIZE(mt7620_ehci_resources),
	.dev            = {
		.dma_mask		= &mt7620_ehci_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &mt7620_ehci_data,
	},
};

static struct resource mt7620_ohci_resources[] = {
	{
		.start	= MT7620_OHCI_BASE,
		.end	= MT7620_OHCI_BASE + MT7620_OHCI_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= MT7620_INTC_IRQ_OTG,
		.end	= MT7620_INTC_IRQ_OTG,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct mt7620_ohci_platform_data mt7620_ohci_data = {
	.start_hw	= mt7620_usb_host_start,
	.stop_hw	= mt7620_usb_host_stop,
};

static u64 mt7620_ohci_dmamask = DMA_BIT_MASK(32);
static struct platform_device mt7620_ohci_device = {
	.name		= "mt7620-ohci",
	.id		= -1,
	.resource	= mt7620_ohci_resources,
	.num_resources	= ARRAY_SIZE(mt7620_ohci_resources),
	.dev            = {
		.dma_mask		= &mt7620_ohci_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &mt7620_ohci_data,
	},
};

void __init mt7620_register_usb(void)
{
	if ( soc_is_mt7620() ) {
		platform_device_register(&mt7620_ehci_device);
		platform_device_register(&mt7620_ohci_device);
	} else {
		BUG();
	}
}
