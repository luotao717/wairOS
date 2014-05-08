/*
 * Ralink MT7620 SoC specific setup
 *
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * Parts of this file are based on Ralink's 2.6.21 BSP
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/clk.h>

#include <asm/mips_machine.h>
#include <asm/reboot.h>
#include <asm/time.h>

#include <asm/mach-ralink/common.h>
#include <asm/mach-ralink/mt7620.h>
#include <asm/mach-ralink/mt7620_regs.h>
#include "common.h"

static void mt7620_restart(char *command)
{
	mt7620_sysc_wr(MT7620_RESET_SYSTEM, SYSC_REG_RESET_CTRL);
	while (1)
		if (cpu_wait)
			cpu_wait();
}

static void mt7620_halt(void)
{
	while (1)
		if (cpu_wait)
			cpu_wait();
}

unsigned int __cpuinit get_c0_compare_irq(void)
{
	return CP0_LEGACY_COMPARE_IRQ;
}

void __init ramips_soc_setup(void)
{
	struct clk *clk;

	mt7620_sysc_base = ioremap_nocache(MT7620_SYSC_BASE, PAGE_SIZE);
	mt7620_memc_base = ioremap_nocache(MT7620_MEMC_BASE, PAGE_SIZE);

	mt7620_clocks_init();

	clk = clk_get(NULL, "cpu");
	if (IS_ERR(clk))
		panic("unable to get CPU clock, err=%ld", PTR_ERR(clk));

	printk(KERN_INFO "%s running at %lu.%02lu MHz\n", ramips_sys_type,
		clk_get_rate(clk) / 1000000,
		(clk_get_rate(clk) % 1000000) * 100 / 1000000);

	_machine_restart = mt7620_restart;
	_machine_halt = mt7620_halt;
	pm_power_off = mt7620_halt;

	clk = clk_get(NULL, "uart");
	if (IS_ERR(clk))
		panic("unable to get UART clock, err=%ld", PTR_ERR(clk));

	ramips_early_serial_setup(0, MT7620_UART0_BASE, clk_get_rate(clk),
				  MT7620_INTC_IRQ_UART0);

	ramips_early_serial_setup(1, MT7620_UART1_BASE, clk_get_rate(clk),
				  MT7620_INTC_IRQ_UART1);
}

void __init plat_time_init(void)
{
	struct clk *clk;

	clk = clk_get(NULL, "cpu");
	if (IS_ERR(clk))
		panic("unable to get CPU clock, err=%ld", PTR_ERR(clk));

	mips_hpt_frequency = clk_get_rate(clk) / 2;
}
