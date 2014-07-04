/*
 *  Asus WIFI_ROUTER board support
 *
 *  Copyright (C) 2014 Rogerwu<rogerwu81@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <asm/mach-ralink/machine.h>
#include <asm/mach-ralink/dev-gpio-buttons.h>
#include <asm/mach-ralink/dev-gpio-leds.h>
#include <asm/mach-ralink/mt7620.h>
#include <asm/mach-ralink/mt7620_regs.h>

#include "devices.h"

#define WIFI_ROUTER_GPIO_BUTTON_RESET	1
#define WIFI_ROUTER_GPIO_BUTTON_WPS	2
#define WIFI_ROUTER_GPIO_LED_SYTEM_GREEN 9
/*#define WIFI_ROUTER_GPIO_LED_3G_BLUE	9*/
#define WIFI_ROUTER_GPIO_LED_3G_RED	13
#define WIFI_ROUTER_GPIO_LED_POWER	11
#define WIFI_ROUTER_KEYS_POLL_INTERVAL	20
#define WIFI_ROUTER_KEYS_POLL_THRESHOLD	3
#define WIFI_ROUTER_KEYS_DEBOUNCE_INTERVAL (WIFI_ROUTER_KEYS_POLL_THRESHOLD * WIFI_ROUTER_KEYS_POLL_INTERVAL)
#if 0
const struct flash_platform_data wif_router_flash = {
/*	.type		= "mx25l3205d",*/
	.type		= "mx25l6406e",
};
#endif

struct spi_board_info wif_router_spi_slave_info[] __initdata = {
	{
		.modalias	= "m25p80",
		.platform_data	= NULL , /*&wif_router_flash,*/
		.irq		= -1,
		.max_speed_hz	= 10000000,
		.bus_num	= 0,
		.chip_select	= 0,
	},
};

static struct gpio_button wif_router_gpio_buttons[] __initdata = {
	{
		.desc		= "soft_reset",
		.type		= EV_KEY,
		.code		= BTN_0/*KEY_RESTART*/,
		/*.debounce_interval = WIFI_ROUTER_KEYS_DEBOUNCE_INTERVAL,*/
		.threshold	= WIFI_ROUTER_KEYS_POLL_THRESHOLD,		
		.gpio		= WIFI_ROUTER_GPIO_BUTTON_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= BTN_1 /*KEY_RESTART*/,
	/*	.debounce_interval = WIFI_ROUTER_KEYS_DEBOUNCE_INTERVAL,*/
		.threshold	= WIFI_ROUTER_KEYS_POLL_THRESHOLD,		
		.gpio		= WIFI_ROUTER_GPIO_BUTTON_WPS,
		.active_low	= 1,
	}
};

static struct gpio_led wif_router_leds_gpio[] __initdata = {
	{
		.name		= "demo:green:system",
		.gpio		= WIFI_ROUTER_GPIO_LED_SYTEM_GREEN,
		.active_low	= 1,
#if 0
	}, {
	{
		.name		= "demo:blue:3g",
		.gpio		= WIFI_ROUTER_GPIO_LED_3G_BLUE,
		.active_low	= 1,
	}, {
		.name		= "demo:red:3g",
		.gpio		= WIFI_ROUTER_GPIO_LED_3G_RED,
		.active_low	= 1,
	}, {
		.name		= "demo:blue:power",
		.gpio		= WIFI_ROUTER_GPIO_LED_POWER,
		.active_low	= 1,
#endif
	}
};

static void __init wifi_router_init(void)
{
	mt7620_gpio_init((MT7620_GPIO_MODE_GPIO << MT7620_GPIO_MODE_UART0_SHIFT) | MT7620_GPIO_MODE_I2C	);
	mt7620_register_spi(wif_router_spi_slave_info,
			    ARRAY_SIZE(wif_router_spi_slave_info));
	mt7620_esw_data.vlan_config = MT7620_ESW_VLAN_CONFIG_LLLLWN/*MT7620_ESW_VLAN_CONFIG_NLLLLW*/;

	mt7620_esw_data.p4_phy_mode = MT7620_PM_MAC_TO_PHY ; 
	mt7620_esw_data.p5_phy_mode = MT7620_PM_MAC_TO_PHY /*MT7620_PM_MII_TO_MAC*/; 


	mt7620_register_ethernet();

	ramips_register_gpio_leds(-1, ARRAY_SIZE(wif_router_leds_gpio),
				  wif_router_leds_gpio);

	ramips_register_gpio_buttons(-1, WIFI_ROUTER_KEYS_POLL_INTERVAL,
				     ARRAY_SIZE(wif_router_gpio_buttons),
				     wif_router_gpio_buttons);
	mt7620_register_wifi();
	mt7620_register_usb();
	mt7620_register_wdt();
}

static void __init wifi_router_v2_init(void)
{
	mt7620_gpio_init((MT7620_GPIO_MODE_GPIO << MT7620_GPIO_MODE_UART0_SHIFT) | MT7620_GPIO_MODE_I2C	);
	mt7620_register_spi(wif_router_spi_slave_info,
			    ARRAY_SIZE(wif_router_spi_slave_info));
	mt7620_esw_data.vlan_config = MT7620_ESW_VLAN_CONFIG_WLLLLL/*MT7620_ESW_VLAN_CONFIG_NLLLLW*/;

	mt7620_esw_data.p4_phy_mode = MT7620_PM_MAC_TO_PHY ; 
	mt7620_esw_data.p5_phy_mode = MT7620_PM_MAC_TO_PHY /*MT7620_PM_MII_TO_MAC*/; 


	mt7620_register_ethernet();

	ramips_register_gpio_leds(-1, ARRAY_SIZE(wif_router_leds_gpio),
				  wif_router_leds_gpio);

	ramips_register_gpio_buttons(-1, WIFI_ROUTER_KEYS_POLL_INTERVAL,
				     ARRAY_SIZE(wif_router_gpio_buttons),
				     wif_router_gpio_buttons);
	mt7620_register_wifi();
	mt7620_register_usb();
	mt7620_register_wdt();
}

MIPS_MACHINE(RAMIPS_MACH_WIFI_ROUTER, "WIFI-ROUTER", "FansupCM7620",
	     wifi_router_init);

MIPS_MACHINE(RAMIPS_MACH_WIFI_ROUTER_V2, "WIFI-ROUTER_V2", "FansupCM7620_V2",
	     wifi_router_v2_init);
