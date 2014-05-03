/*
 *  Ralink MT7620 SoC platform device registration
 *
 *  Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _MT7620_ESW_PLATFORM_H
#define _MT7620_ESW_PLATFORM_H

enum {
	MT7620_ESW_VLAN_CONFIG_NONE = 0,
	MT7620_ESW_VLAN_CONFIG_LLLLLW,
	MT7620_ESW_VLAN_CONFIG_NLLLLW,
	MT7620_ESW_VLAN_CONFIG_WLLLLL,
	MT7620_ESW_VLAN_CONFIG_LLLLWN, /*port 4 wan , port 0-3 lan*/
};

/*for port 4 and port 5*/
enum {
	MT7620_PM_RGMII_TO_MAC = 0,
	MT7620_PM_MII_TO_MAC ,
	MT7620_PM_RMII_TO_MAC ,
	MT7620_PM_MAC_TO_PHY ,
	MT7620_PM_DISABLE,
};

enum {
	MT7620_ESW_RESET_ALL = 0,
	MT7620_ESW_RESET_SWITCH ,
	MT7620_ESW_RESET_EPHY ,
};
	
struct mt7620_esw_platform_data
{
	u8 vlan_config;
	u8 p4_phy_mode;
	u8 p5_phy_mode;
	u8 is_bga;
/*
	u32 reg_initval_fct2;
	u32 reg_initval_fpa2;
*/
	void (*reset_esw)(int mode);
};

#endif /* _MT7620_ESW_PLATFORM_H */
