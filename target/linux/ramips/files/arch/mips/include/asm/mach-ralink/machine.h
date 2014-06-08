/*
 * Ralink machine types
 *
 * Copyright (C) 2010 Joonas Lahtinen <joonas.lahtinen@gmail.com>
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <asm/mips_machine.h>

enum ramips_mach_type {
	RAMIPS_MACH_GENERIC,
	RAMIPS_MACH_WIFI_ROUTER,/* Demo Wifi Router */
	RAMIPS_MACH_WIFI_ROUTER_POE,/* Demo Wifi Router POE */
};
