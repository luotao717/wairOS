/*
 * Platform data definition for the mt762x driver
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#ifndef _MT762X_PLATFORM_H
#define _MT762X_PLATFORM_H

struct mt762x_platform_data {
	char *eeprom_file_name;
	const u8 *mac_address;

	int disable_2ghz;
	int disable_5ghz;
	int clk_is_20mhz;
};

#endif /* _MT762X_PLATFORM_H */
