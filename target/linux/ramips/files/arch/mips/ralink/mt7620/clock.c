/*
 *  Ralink MT7620 clock API
 *
 *  Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/clk.h>

#include <asm/mach-ralink/common.h>
#include <asm/mach-ralink/mt7620.h>
#include <asm/mach-ralink/mt7620_regs.h>
#include "common.h"

struct clk {
	unsigned long rate;
};

static struct clk mt7620_cpu_clk;
static struct clk mt7620_sys_clk;
static struct clk mt7620_wdt_clk;
static struct clk mt7620_uart_clk;

void __init mt7620_clocks_init(void)
{
	u32	cpll1val,cpll0val ;
	u32     clk_sel = 0;
	u32	is_swcfg = 0;
	u32 	multi_ratio = 0,div_ratio = 1;
	

	if (soc_is_mt7620()) {

		cpll1val = mt7620_sysc_rr(SYSC_REG_CPLL_CFG1);
		clk_sel = ( cpll1val & CPU_CLK_AUX0 ) ? 1 : 0 ; /*480Mhz or 600Mhz*/

		
		switch (clk_sel) {
		case 0:
			cpll0val = mt7620_sysc_rr(SYSC_REG_CPLL_CFG0);
			is_swcfg = cpll0val & CPLL_SW_CONFIG ; 

			if ( is_swcfg )
			{
			    mt7620_cpu_clk.rate = 600 * 1000 * 1000 ;
			}
			else
			{
			/* read the CPLL0 to determine real cpu clock */
			multi_ratio = ( cpll0val & CPLL_MULT_RATIO ) >> CPLL_MULT_RATIO_SHIFT ;
			div_ratio   = ( cpll0val & CPLL_DIV_RATIO ) >> CPLL_DIV_RATIO_SHIFT ;

			multi_ratio += 24 ;	/*begin from 24*/
			if ( div_ratio == 0 )   /*define from datasheet*/
				div_ratio = 2; 
			else if ( div_ratio == 1 )
				div_ratio = 3; 
			else if ( div_ratio == 2 )
				div_ratio = 4; 
			else if ( div_ratio == 3 )
				div_ratio = 8; 
			
			mt7620_cpu_clk.rate = (( BASE_CLOCK * multi_ratio ) / div_ratio ) * 1000 * 1000 ;
			}
			break;
		case 1:
			mt7620_cpu_clk.rate = 480 * 1000 * 1000;
			break;
		}
		mt7620_sys_clk.rate = mt7620_cpu_clk.rate / 4;
		mt7620_uart_clk.rate = 40 * 1000 * 1000;
		mt7620_wdt_clk.rate = mt7620_sys_clk.rate;
	} else {
		BUG();
	}
}

/*
 * Linux clock API
 */
struct clk *clk_get(struct device *dev, const char *id)
{
	if (!strcmp(id, "sys"))
		return &mt7620_sys_clk;

	if (!strcmp(id, "cpu"))
		return &mt7620_cpu_clk;

	if (!strcmp(id, "wdt"))
		return &mt7620_wdt_clk;

	if (!strcmp(id, "uart"))
		return &mt7620_uart_clk;

	return ERR_PTR(-ENOENT);
}
EXPORT_SYMBOL(clk_get);

int clk_enable(struct clk *clk)
{
	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	return clk->rate;
}
EXPORT_SYMBOL(clk_get_rate);

void clk_put(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_put);
