/*
 * Platform data definition for built-in OHCI controller of the
 * Ralink MT7620 SoCs
 *
 * Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef _MT7620_OHCI_PLATFORM_H
#define _MT7620_OHCI_PLATFORM_H

struct mt7620_ohci_platform_data {
	void	(*start_hw)(void);
	void	(*stop_hw)(void);
};

#endif /*  _MT7620_OHCI_PLATFORM_H */
