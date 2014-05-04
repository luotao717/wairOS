/*
 *  Atheros 933x : richerlink  RL_ANS5001 board support
 *
 *  Copyright (C) 2012-2013 Roger Wu<wuxiaobo@richerlink.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-nfc.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include <asm/mach-ar79/ar71xx_regs.h>
#include <asm/mach-ar79/ath79.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#define RL_ANS5001_GPIO_LED_WAN 	18 /*Reserve 15: not find real*/	
#define RL_ANS5001_GPIO_LED_WLAN 	13	
#define RL_ANS5001_GPIO_LED_USB		11 /*USB gpio*/
#define RL_ANS5001_GPIO_LED_SYSTEM	14 /*27*/	

#define RL_ANS5001_GPIO_BTN_JUMPSTART	11
#define RL_ANS5001_GPIO_BTN_RESET		17

#define RL_ANS5001_KEYS_POLL_INTERVAL	20	/* msecs */
#define RL_ANS5001_KEYS_POLL_THRESHOLD	3
#define RL_ANS5001_KEYS_DEBOUNCE_INTERVAL	( RL_ANS5001_KEYS_POLL_THRESHOLD * RL_ANS5001_KEYS_POLL_INTERVAL)

#define RL_ANS5001_MAC0_OFFSET		0x0000
#define RL_ANS5001_MAC1_OFFSET		0x0006
#define RL_ANS5001_CALDATA_OFFSET		0x1000
#define RL_ANS5001_WMAC_MAC_OFFSET		0x1002

static struct gpio_led rl_ans5001_leds_gpio[] __initdata = {
	{
		.name		= "rl_ans5001:green:usb",
		.gpio		= RL_ANS5001_GPIO_LED_USB,
		.active_low	= 1,
	},
	{
		.name		= "rl_ans5001:green:wlan",
		.gpio		= RL_ANS5001_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "rl_ans5001:green:wan",
		.gpio		= RL_ANS5001_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "rl_ans5001:red:system",
		.gpio		= RL_ANS5001_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_button rl_ans5001_gpio_buttons[] __initdata = {
	{
		.desc		= "soft_reset",
		.type		= EV_KEY,
		.code		= BTN_0/*KEY_RESTART*/,
	/*	.debounce_interval = RL_ANS5001_KEYS_DEBOUNCE_INTERVAL,*/
		.threshold	= RL_ANS5001_KEYS_POLL_THRESHOLD,		
		.gpio		= RL_ANS5001_GPIO_BTN_RESET,
		.active_low	= 1,
	},
#if 0
	{
		.desc		= "jumpstart",
		.type		= EV_KEY,
		.code		= BTN_1/*KEY_WPS_BUTTON*/,
		/*.debounce_interval = RL_ANS5001_KEYS_DEBOUNCE_INTERVAL,*/
		.threshold	= RL_ANS5001_KEYS_POLL_THRESHOLD,		
		.gpio		= RL_ANS5001_GPIO_BTN_JUMPSTART,
		.active_low	= 1,
	}
#endif
};
#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition rl_ans5001_partitions[] = {
	{
		.name		= "u-boot",
		.offset		= 0,
		.size		= 0x020000,
		.mask_flags	= MTD_WRITEABLE,
	} , {
		.name		= "kernel",
		.offset		= 0x020000,
		.size		= 0xe0000,
	} , {
		.name		= "rootfs",
		.offset		= 0x100000,
		.size		= 0x6f0000,
	} , {
		.name		= "config",
		.offset		= 0x7f0000,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	} , {
		.name		= "firmware",
		.offset		= 0x020000,
		.size		= 0x7d0000,
	}
};
#endif /* CONFIG_MTD_PARTITIONS */

static struct flash_platform_data rl_ans5001_flash_data = {
#ifdef CONFIG_MTD_PARTITIONS
        .parts          = rl_ans5001_partitions,
        .nr_parts       = ARRAY_SIZE(rl_ans5001_partitions),
#endif
};
static void __init rl_ans5001_common_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);
	/*ath79_register_m25p80(&rl_ans5001_flash_data );*/
	/*ath79_register_wmac(art + RL_ANS5001_CALDATA_OFFSET,
			    art + RL_ANS5001_WMAC_MAC_OFFSET);
	*/

	ath79_init_mac(ath79_eth0_data.mac_addr, art + RL_ANS5001_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + RL_ANS5001_MAC1_OFFSET, 0);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);/*swap phy4-ph0*/

/*	ath79_register_mdio(0, 0x0);*/
	ath79_register_mdio(1, 0x0);

	/* WAN port */
        /* GMAC0 is connected to the PHY0 of the internal switch */
        ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(0);
        ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
        ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev; 
	ath79_register_eth(0);

	/* LAN ports */
        ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
        ath79_eth1_data.speed = SPEED_1000;
        ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_register_eth(1);
}

static void __init rl_ans5001_setup(void)
{
	u32 data = 0;	
	
	rl_ans5001_common_setup();

	/*set the GPIO27 for GPIO function*/
	data = ath79_reset_rr(AR934X_RESET_REG_BOOTSTRAP);
	data |= BIT(18); 
	ath79_reset_wr(AR934X_RESET_REG_BOOTSTRAP, data);
		
	ath79_register_leds_gpio(-1, ARRAY_SIZE(rl_ans5001_leds_gpio),
				 rl_ans5001_leds_gpio);

	ath79_register_gpio_buttons(-1, RL_ANS5001_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(rl_ans5001_gpio_buttons),
					rl_ans5001_gpio_buttons);
	ath79_register_usb();

	ath79_register_nfc();
}

MIPS_MACHINE(ATH79_MACH_RL_ANS5001, "RL-ANS5001", "Richerlink RL-ANS5001 reference board",
	     rl_ans5001_setup);
