/*
 * Ralink MT7620 SoC specific platform device definitions
 *
 * Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef __MT7620_DEVICES_H
#define __MT7620_DEVICES_H

#include <asm/mach-ralink/mt7620_esw_platform.h>

struct physmap_flash_data;
struct spi_board_info;

extern struct physmap_flash_data mt7620_flash0_data;
extern struct physmap_flash_data mt7620_flash1_data;

extern struct mt7620_esw_platform_data mt7620_esw_data;

void mt7620_register_flash(unsigned int id);
void mt7620_register_ethernet(void);
void mt7620_register_wifi(void);
void mt7620_register_wdt(void);
void mt7620_register_spi(struct spi_board_info *info, int n);
void mt7620_register_usb(void);

#endif  /* __MT7620_DEVICES_H */

