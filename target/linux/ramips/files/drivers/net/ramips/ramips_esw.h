/*version v0.2 phy modify*/
#include <linux/ioport.h>
#include <linux/phy.h>
#include <linux/switch.h>
#include <linux/mii.h>
#include <linux/jiffies.h>
#include <mt7620_regs.h>
#include <mt7620_esw_platform.h>

#ifndef _RAMIPS_ESW_H_
#define  _RAMIPS_ESW_H_

/* port 4 port 5 use extern phy , phy address is NET_ESW_EXTERN_PHY_BASE and NET_ESW_EXTERN_PHY_BASE + 1 */
#define MT7620_ESW_PHYADDR(n)		(((n) >= 4 ) ? (CONFIG_NET_ESW_EXTERN_PHY_BASE + (n) - 4 ) : (n))

#define MT7620_ESW_ATS_TIMEOUT		(5 * HZ)
#define MT7620_ESW_PHY_TIMEOUT		(5 * HZ)

#define MT7620_ESW_PORT0		0
#define MT7620_ESW_PORT1		1
#define MT7620_ESW_PORT2		2
#define MT7620_ESW_PORT3		3
#define MT7620_ESW_PORT4		4
#define MT7620_ESW_PORT5		5
#define MT7620_ESW_PORT6		6
#define MT7620_ESW_PORT7		7

#define MT7620_ESW_VLAN_NONE		0xfff

#define MT7620_ESW_PORTS_NONE		0 
#define MT7620_ESW_PMAP_LLLLLLL		0x7f 
#define MT7620_ESW_PMAP_LLLLLWL		0x5f
#define MT7620_ESW_PMAP_WLLLLLL		0x7e

#define MT7620_ESW_PMAP_WAN_LLLLLLL     0		
#define MT7620_ESW_PMAP_WAN_LLLLLWL     0x60		
#define MT7620_ESW_PMAP_WAN_WLLLLLL	0x41

/* port 4 wan, port 0-3 lan*/
#define MT7620_ESW_PMAP_LLLLWLL		0x6f
#define MT7620_ESW_PMAP_WAN_LLLLWLL     0x50		


#define MT7620_ESW_PORTS_INTERNAL					\
		(BIT(MT7620_ESW_PORT0) | BIT(MT7620_ESW_PORT1) |	\
		 BIT(MT7620_ESW_PORT2) | BIT(MT7620_ESW_PORT3) )

#define MT7620_ESW_PORTS_NOCPU						\
		(MT7620_ESW_PORTS_INTERNAL | BIT(MT7620_ESW_PORT4) | BIT(MT7620_ESW_PORT5))

#define MT7620_ESW_PORT_LAN	(MT7620_ESW_PORTS_INTERNAL | BIT(MT7620_ESW_PORT4))

#define MT7620_ESW_PORTS_CPU	BIT(MT7620_ESW_PORT6)

#define MT7620_ESW_PORTS_ALL						\
		(MT7620_ESW_PORTS_NOCPU | MT7620_ESW_PORTS_CPU)

#define MT7620_ESW_NUM_VLANS		16
#define MT7620_ESW_NUM_VIDS		4096
#define MT7620_ESW_NUM_PORTS		7
#define MT7620_ESW_NUM_LANWAN		6
#define MT7620_ESW_NUM_LEDS		6 /*5*/
/*
 * HW limitations for this switch:
 * - No large frame support (PKT_MAX_LEN at most 1536)
 * - Can't have untagged vlan and tagged vlan on one port at the same time,
 *   though this might be possible using the undocumented PPE.
 */

/* detail define the register of esw MT7620_SWITCH_BASE */
#define REG_ESW_WT_MAC_MFC              0x10
#define REG_ESW_WT_MAC_ATA1             0x74
#define REG_ESW_WT_MAC_ATA2             0x78
#define REG_ESW_WT_MAC_ATWD             0x7C
#define REG_ESW_WT_MAC_ATC              0x80 

/*ARL TABLE Access register*/
#define REG_ESW_TABLE_TSRA1		0x84
#define REG_ESW_TABLE_TSRA2		0x88
#define REG_ESW_TABLE_ATRD		0x8C

/*VLAN TABLE Access register*/
#define REG_ESW_VLAN_VTCR		0x90
#define REG_ESW_VLAN_VAWD1		0x94
#define REG_ESW_VLAN_VAWD2		0x98

/*VLAN ID Register*/
#define REG_ESW_VLAN_ID_BASE		0x100
#define REG_ESW_VLAN_ID(idx)		( REG_ESW_VLAN_ID_BASE + 4 * ((idx)/2) )
#define VLAN_ID_MASK			0xFFF
#define VLAN_ID_SHIFT(idx)		( 12 * ((idx) % 2) )

/*ESW Port Register*/
#define REG_ESW_PN_SSC_BASE		0x2000
#define REG_ESW_PN_SSC(n)		(REG_ESW_PN_SSC_BASE + 0x100 * (n))
#define REG_ESW_PN_PCR_BASE		0x2004	
#define REG_ESW_PN_PCR(n)		(REG_ESW_PN_PCR_BASE + 0x100 * (n))
#define REG_ESW_PN_PSC_BASE		0x20c	
#define REG_ESW_PN_PSC(n)		(REG_ESW_PN_PSC_BASE + 0x100 * (n))
#define REG_ESW_PN_PVC_BASE		0x2010	
#define REG_ESW_PN_PVC(n)		(REG_ESW_PN_PVC_BASE + 0x100 * (n))
#define REG_ESW_PN_PPBV1_BASE		0x2014	
#define REG_ESW_PN_PPBV1(n)		(REG_ESW_PN_PPBV1_BASE + 0x100 * (n))
#define REG_ESW_PN_PPBV2_BASE		0x2018	
#define REG_ESW_PN_PPBV2(n)		(REG_ESW_PN_PPBV2_BASE + 0x100 * (n))

/*ESW MAC Registers*/
#define REG_ESW_MN_PMCR_BASE		0x3000 /**/
#define REG_ESW_MN_PMCR(n)		(REG_ESW_MN_PMCR_BASE + 0x100 * (n))

#define REG_ESW_MN_PMEEECR_BASE		0x3004 /* Port n MAC EEE Control Register */
#define REG_ESW_MN_PMEEECR(n)		(REG_ESW_MN_PMEEECR_BASE + 0x100 * (n))

#define REG_ESW_MN_PMSR_BASE		0x3008 /* Port n MAC Status Register */
#define REG_ESW_MN_PMSR(n)		(REG_ESW_MN_PMSR_BASE + 0x100 * (n))

#define REG_ESW_SMACCR0			( 0x3fe4 )
#define REG_ESW_SMACCR1			( 0x3fe8 )
#define REG_ESW_CKGCR			( 0x3ff0 )

/* ESW MIB Registers */
#define REG_ESW_MIB_RBOC_BASE		0x4024 /*Rx Bad OCtet Counter of Port n*/
#define REG_ESW_MIB_RBOC(n)		(REG_ESW_MIB_RBOC_BASE + 0x100 * (n))
#define REG_ESW_MIB_RGOC_BASE		0x4028 /*Rx Good OCtet Counter of Port n*/
#define REG_ESW_MIB_RGOC(n)		(REG_ESW_MIB_RGOC_BASE + 0x100 * (n))

/*MDIO FOR ESW */
#define REG_ESW_MDIO_PPSC		0x7000 /*PHY Polling and SMI Master Control*/
#define REG_ESW_MDIO_PIAC		0x7004 /*PHY Indirect Access Control*/
#define REG_ESW_MDIO_IMR		0x7008 /*Interrupt Mask Register*/
#define REG_ESW_MDIO_ISR		0x700c /*Interrupt Status Register*/
#define REG_ESW_MDIO_CPC		0x7010 /*CPU Port Control*/
#define REG_ESW_MDIO_GPC1		0x7014 /*GIGA Port-I Control*/
#define REG_ESW_MDIO_DBGP		0x7018 /*Debug Probe Control*/
#define REG_ESW_MDIO_GPC2		0x701c /*GIGA Port-II Control*/
/*ESW MAX Register Number*/
#define REG_ESW_MAX			0x7FFFF

#define ATA1_BYTE_MASK			0xFF
/* REG_ESW_WT_MAC_ATA1 */
#define ATA1_BYTE_3			0 	/*MAC addr [23:16] or DIP addr [7:0]*/
#define ATA1_BYTE_2			8	/*MAC addr [31:23] or DIP addr [15:8]*/
#define ATA1_BYTE_1			16	/*MAC addr [39:32] or DIP addr [23:16]*/
#define ATA1_BYTE_0			24	/*MAC addr [47:40] or DIP addr [31:24]*/
/* REG_ESW_WT_MAC_ATA2 */
#define ATA2_BYTE_3			0 	/*SIP addr [7:0] or CVID[7:0]*/
#define ATA2_BYTE_2			8	/*SIP addr [15:8] or
						 b[15]: IVL
						 b[14:12] Filter ID[2:0]
						 b[11:8] CVID[11:8]
*/
#define ATA2_BYTE_1			16	/*MAC addr [7:0] or SIP addr [23:16]*/
#define ATA2_BYTE_0			24	/*MAC addr [15:8] or SIP addr [31:24]*/

/* REG_ESW_WT_MAC_ATWD */
#define ATWD_STATUS_MASK		0x3	/* Address Entry Live Status */
#define ATWD_STATUS_SHIFT		2
#define ATWD_PORT_MASK		0xFF	/* Destination Port Map */
#define ATWD_PORT_SHIFT		4
#define ATWD_LEAKY_EN			BIT(12) /*Leaky VLAN Enable */
#define ATWD_EG_TAG_MASK		0x7    /*Egress VLAN Tag Attribute*/
#define ATWD_EG_TAG_SHIFT		13
#define ATWD_USER_PRI_MASK		0x7	/*User Priority */
#define ATWD_USER_PRI_SHIFT		16
#define ATWD_SA_MIR_EN			BIT(19) /*Source Address Hit to Mirror Port */
#define ATWD_SA_PORT_FW_MASK		0x7    /* Source Address Hit Frame Port Forwarding */
#define ATWD_SA_PORT_FW_SHIFT 		20
#define ATWD_MY_MAC			BIT(23) /*MAC address is reserved for MY_MAC attribute */
#define ATWD_TIMER_MASK			0xFF	/*Age Timer */
#define ATWD_TIMER_SHIFT		24

/* REG_ESW_WT_MAC_ATC */
#define ATC_AC_CMD_MASK			0x7 /* Address Table Access Command */
				/*
 *				3'b000: read command;
 *				3'b001: write command;
 *				3'b010: clean commnad;
 *				3'b011: reserved
 *				3'b100: Start Search command
 *				3'b101: Next Search command
 *				3'b110~ Reserved
 * */
#define ATC_AC_CMD_SHIFT		0
#define ATC_AC_SAT_MASK			0x3 /* Address Table Single Access Target */
				/*
 * 				2'b00: Specified MAC address
 * 				2'b01: Specified DIP address entry
 * 				2'b10: Specified SIP address
 * 				2'b11: Specified address ( read only)
 * */
#define ATC_AC_SAT_SHIFT		4
#define ATC_AC_MAT_MASK			0xF /* Address Table Multiple Access Target */
				/*
 * 				4'b0000:  All MAC address entries
 * 				4'b0001:  All DIP/GA address
 * 				4'b0010:  All SIP address entries
 * 				4'b0011:  All valid address 
 * 				4'b0100:  All non-static MAC address
 * 				4'b0101:  All non-static DIP address
 * 				4'b0110:  All static MAC address
 * 				4'b0111:  All static DIP address
 * 				4'b1000:  All relative SIP address entried based
 * 				4'b1001:  All MAC Address entries with the customer VID specified in ATA2.CVID[11:0]
 * 				4'b1010:  All MAC Address entries with the Filter ID specified in ATA2.FID[2:0]
 * 				4'b1100:  All MAC Adress entries with the source ports specified in ATA1.PORT[7:0]
 * 				*/
#define ATC_AC_MAT_SHIFT		8
#define ATC_ADDR_INVLD			BIT(12) /* Address Entry is not Valid RO*/
#define ATC_SRCH_HIT			BIT(13) /* Linear Search Hit */
#define ATC_SRCH_END 			BIT(14) /* Linear Search End */
#define ATC_BUSY			BIT(15)	/* Address Table is Busy */
#define ATC_ADDR_MASK			0xFFF /*Address Table Access Index : debugging purposes  */
#define ATC_ADDR_SHIFT			16

/* #define REG_ESW_VLAN_VTCR		0x90 */
#define TBL_CTRL_FUNC_MASK		0x0F
#define TBL_CTRL_FUNC_SHIFT		12
#define TBL_CTRL_IDX_MASK		0xFFF
#define TBL_CTRL_IDX_SHIFT		0
#define TBL_CTRL_BUSY			BIT(31)
#define TBL_CTRL_IDX_INVALID		BIT(16)

/* #define REG_ESW_VLAN_VAWD1		0x94 */
#define VLAN_WD1_VALID			BIT(0)
#define VLAN_WD1_FID_MASK		0x7 /*Filtering Database*/
#define VLAN_WD1_FID_SHIFT		1 /*Filtering Database*/
#define VLAN_WD1_STAGI_MASK		0xFFF	/*Service Tag I*/
#define VLAN_WD1_STAGI_SHIFT		4	/*Service Tag I*/
#define VLAN_WD1_PORTM_MASK 		0xFF	/*VLAN member Port BIT MAPS*/
#define VLAN_WD1_PORTM_SHIFT		16	/*VLAN member Port BIT MAPS*/
#define VLAN_WD1_USER_PRI_MASK 		0x7	/*Service Tag(STAG) User Priority Value from VLAN Table*/
#define VLAN_WD1_USER_PRI_SHIFT		24	
#define VLAN_WD1_COPY_PRI		BIT(27) /*Copy User Priority Value from Customer Priority Tag for Stack VLAN */
#define VLAN_WD1_VTAG_EN		BIT(28) /*Per VLAN Egress Tag Control*/
#define VLAN_WD1_EG_CON			BIT(29) /*Egress Tag Consistent*/
#define VLAN_WD1_IVL_MAC		BIT(30) /*Independent VLAN learning*/
#define VLAN_WD1_PORT_STAG		BIT(31)	/*Port-based Service TAG*/

/* #define REG_ESW_VLAN_VAWD2		0x98 */
/* FOR VLAN ENTRY*/
#define VLAN_WD2_PTAG_MASK		0x3
#define VLAN_WD2_PTAG_BASE_SHIFT	0		 
#define VLAN_WD2_PTAG_SHIFT(n)		( VLAN_WD2_PTAG_BASE_SHIFT + 2 * (n) )
#define VLAN_WD2_S_TAG2_MASK		0xFF
#define VLAN_WD2_S_TAG2_SHIFT		16

/* REG_ESW_WT_MAC_MFC  */
#define REG_ESW_MIRROR_PORT_MASK	0x7 /*Mirror Port Number 3'h0 Port 0*/
#define REG_ESW_MIRROR_PORT_SHIFT	0
#define REG_ESW_MIRROR_EN		BIT(3) /*Mirror Port Enable*/
#define REG_ESW_CPU_PORT_MASK		0x7 /* CPU Port Number 3'h0: Port 0 */
#define REG_ESW_CPU_PORT_SHIFT		4
#define REG_ESW_CPU_EN			BIT(7) /*CPU Port Enable */

#define REG_ESW_UNU_FFP_MASK 		0xFF  /* Unknown Unicast Frame Flooding Ports :BIT MAPS */
#define REG_ESW_UNU_FFP_SHIFT		8

#define REG_ESW_UNM_FFP_MASK 		0xFF  /* Unknown Multicast Frame Flooding Ports :BIT MAPS */
#define REG_ESW_UNM_FFP_SHIFT		16

#define REG_ESW_BC_FFP_MASK 		0xFF  /* Broadcast Frame Flooding Ports :BIT MAPS */
#define REG_ESW_BC_FFP_SHIFT		24	

/* REG_ESW_PN_PCR */
#define PCR_PORT_VLAN_MASK		0x3 /* 2'b00:Port Matrix ; 2'b01:Fallback ; 2'b10:Check; 2'b11 Security Mode*/
#define PCR_PORT_VLAN_SHIFT		0
#define PCR_VLAN_MIS			BIT(2) /* VLAN Mismatch to Mirror Port */
#define PCR_MIS_PORT_FW_MASK		0x7  /*ACL Mismatch TO_CPU Forward */
#define PCR_MIS_PORT_FW_SHIFT		4
#define PCR_ACL_MIR			BIT(7) /*ACL Mismatch to Mirror Port */
#define PCR_PORT_RX_MIR			BIT(8) /*Port Rx Mirror Enable*/
#define PCR_PORT_TX_MIR			BIT(9) /*Port Tx Mirror Enable */
#define PCR_ACL_EN			BI(10)	/*Port-based ACL Enable*/
#define PCR_UP2TAG_EN			BIT(11) /*User Priority To tag Enable */
#define PCR_UP2DSCP_EN			BIT(12) /*User Priority To DSCP Enable */
#define PCR_PORT_MATRIX_MASK		0xFF /*Port Matrix Member egress portmap*/
#define PCR_PORT_MATRIX_SHIFT		16
#define PCR_PORT_PRI_MASK		0x7 /*port-based User Priority */
#define PCR_PORT_PRI_SHIFT		24
#define PCR_EG_TAG_MASK			0x3 /*Port-Based Egress VLAN Tag 2'b00 Untagged ; 2'b01 Swap ;2'b10 Tagged; 2'b11 Stack */
#define PCR_EG_TAG_SHIFT		28

/* REG_ESW_PN_PSC */
#define PSC_SA_DIS			BIT(4)

/* REG_ESW_PN_PVC */
#define PVC_ACC_FRM_MASK		0x3 /*Acceptable Frame Type : 2'b00 Admit All ;2'b01 Admin Only VLAN-tagged; 2'b10 Admin only untagged or priority ; 2'b11 reserve*/
#define PVC_ACC_FRM_SHIFT		0
#define PVC_UC_LKYV_EN			BIT(2)	/* Unicast Leaky VLAN Enable*/
#define PVC_MC_LKYV_EN			BIT(3)	/* Multicast Leaky VLAN Enable*/
#define PVC_BC_LKYV_EN			BIT(4)	/* Broadcast Leaky VLAN Enable*/
#define PVC_PORT_SPEC_TAG		BIT(5)	/* Special Tag Enable */
#define PVC_VLAN_ATTR_MASK		0x3	/* VLAN Port Attribute ,2'b00 User port ; 2'b01 Stack; 2'b10 Translation; 2'b11 Transparent */
#define PVC_VLAN_ATTR_SHIFT		6	
#define PVC_EG_TAG_MASK			0x7 /* Incoming Port Egress VLAN Tag Attribution */
					/*
 * 			3'b000: system default(disabled)
 * 			3'b001: Consistent
 * 			3'b010~3'b011: Reserved
 * 			3'b100: Untagged
 * 			3'b101: Swap
 * 			3'b110: Tagged
 * 			3'b111: Stack
 * */
#define PVC_EG_TAG_SHIFT		8
#define PVC_FORCE_PVID			BIT(14) /*Forces PVID on VLAN-tagged frames*/
#define PVC_DIS_PVID			BIT(15) /* PVID Disable for priority-tagged  */
#define PVC_STAG_VPID_MASK		0xFF /* Stack Tag VPID(VLAN Protocol ID) value*/
#define PVC_STAG_VPID_SHIFT		16


/* REG_ESW_PN_PPBV1  or REG_ESW_PN_PPBV2 */
#define PPBV_GN_PORT_VID_MASK		0xFFF /*pvid*/
#define PPBV_GN_PORT_PRI_MASK		0x7 /*port priority*/
#define PPBV_G0_PORT_VID_SHIFT		0
#define PPBV_G0_PORT_PRI_SHIFT		13
#define PPBV_G1_PORT_VID_SHIFT		16
#define PPBV_G1_PORT_PRI_SHIFT		29	
#define PPBV_G2_PORT_VID_SHIFT		0
#define PPBV_G2_PORT_PRI_SHIFT		13
#define PPBV_G3_PORT_VID_SHIFT		16
#define PPBV_G3_PORT_PRI_SHIFT		29	

/*  REG_ESW_MN_PMCR */
#define REG_ESW_FORCE_LINK		BIT(0) /* Port n Force MAC Link UP */
#define REG_ESW_FORCE_DPX		BIT(1) /* Port n Force duplex */
#define REG_ESW_FORCE_SPD_MASK		0x3 /* Port n Force Speed 2'b00:10Mbps 2'b01 100Mbps 2'b10 1000Mbps 2'b11 Invalid */
#define REG_ESW_FORCE_SPD_SHIFT		2
#define REG_ESW_FORCE_TX_FC		BIT(4) /* Port n Force Tx FC */
#define REG_ESW_FORCE_RX_FC		BIT(5) /* Port n Force RX FC */
#define REG_ESW_FORCE_EEE100		BIT(6) /* Port n Force LPI Mode For 100 Mbps */
#define REG_ESW_FORCE_EEE1G		BIT(7) /* Port n Force LPI Mode For 1000 Mbps */
#define REG_ESW_BACKPR_EN		BIT(8) /* Port n Back Pressure Enable */
#define REG_ESW_BKOFF_EN		BIT(9) /* Port n Backoff Enable */
#define REG_ESW_MAC_RX_EN		BIT(13) /* Port n Rx MAC Enable */
#define REG_ESW_MAC_TX_EN		BIT(14)	/* Port n Tx MAC Enable */
#define REG_ESW_FORCE_MODE		BIT(15) /* Port n Force Mode */
#define REG_ESW_IPG_CFG_MASK 		0x3 /* Port n Inter-Frame Gap(IFG) Shrink For CPU Port  */
#define REG_ESW_IPG_CFG_SHIFT 		18

/* REG_ESW_MN_PMSR */
#define PMSR_MAC_LINK_STS		BIT(0) /*Port n Link up Status : 1 linkup*/
#define PMSR_MAC_DPX_STS		BIT(1) /*Port n duplex Status 1 Full Duplex */
#define PMSR_MAC_SPD_STS_MASK		0x3    /*Port n Speed[1:0] Status 
					2'b00: 10Mbps
					2'b01: 100Mbps
					2'b10: 1000Mbps
					2'b11: Invalid 
*/
#define PMSR_MAC_SPD_STS_SHIFT		2
#define PMSR_TX_FC_STS			BIT(4) /*Port n Tx XFC Status */
#define PMSR_RX_FC_STS			BIT(5) /*Port n Rx XFC Status */
#define PMSR_EEE100_STS			BIT(6) /*Port n  LPI Mode Status for 100Mbps*/
#define PMSR_EEE1G_STS			BIT(7) /*Port n  LPI Mode Status for 1000Mbps*/

/* REG_ESW_MDIO_PIAC */
#define PIAC_MDIO_RW_DATA_MASK		0xFFFF /* MDIO Read/Write Data Field */
#define PIAC_MDIO_RW_DATA_SHIFT		0
#define PIAC MDIO_ST_MASK		0x3 /*2'b01 Start*/
#define PIAC_MDIO_ST_SHIFT		16
#define PIAC_MDIO_CMD_MASK		0x3 /* MDIO Command Field  2'b01 Write ; 2'b10 Read */
#define PIAC_MDIO_CMD_SHIFT		18
#define PIAC_MDIO_PHY_ADDR_MASK	0x1F
#define PIAC_MDIO_PHY_ADDR_SHIFT	20
#define PIAC_MDIO_REG_ADDR_MASK	0x1F
#define PIAC_MDIO_REG_ADDR_SHIFT	25
#define PIAC_PHY_ACS_ST		BIT(31)

/* REG_ESW_MDIO_GPC1 */
#define GPC1_PHY_BASE_MASK		0x1F
#define GPC1_PHY_BASE_SHIFT		16	


enum {
	/* Global attributes. */
	MT7620_ESW_ATTR_ENABLE_VLAN,
	MT7620_ESW_ATTR_ALT_VLAN_DISABLE,
	/* Port attributes. */
	MT7620_ESW_ATTR_PORT_DISABLE,
	MT7620_ESW_ATTR_PORT_DOUBLETAG,
	MT7620_ESW_ATTR_PORT_UNTAG,
	MT7620_ESW_ATTR_PORT_LED,
	MT7620_ESW_ATTR_PORT_LAN,
	MT7620_ESW_ATTR_PORT_RECV_BAD,
	MT7620_ESW_ATTR_PORT_RECV_GOOD,
};

struct mt7620_esw_port {
	bool	disable;
	bool	doubletag;
/*	bool	untag;*/
	u8	led;
	u16	pvid;
};

struct mt7620_esw_vlan_str {
	u8	ports;
	u8	ports_untag;
	u16	vid;
};
enum vtag_mode_enum {
	VLAN_TAG_EGRESS_DISABLE=0,
	VLAN_TAG_EGRESS_EG_CON, /*egress tag consistent*/
	VLAN_TAG_EGRESS_TAG_CONTROL,
}; 
#define VLAN_PRI_USER		BIT(0)
#define VLAN_COPY_PRI		BIT(1)

#define VLAN_TAG_EGRESS_UNTAG	0
#define VLAN_TAG_EGRESS_SWAP 	1
#define VLAN_TAG_EGRESS_TAG	2
#define VLAN_TAG_EGRESS_STACK	3
struct mt7620_esw_vlan_attr {
	u8 ivl_mac;
	enum vtag_mode_enum vtag_mode;
	u8 pri_mode;
	u8 usr_pri;
	u8 port_mem;
	u16 s_tag; /*service vlan id or tag index */
	u8 valid;	
	u8 egress_tag[MT7620_ESW_NUM_PORTS];			
};

/* table control : access table : read or write or flush */
enum tbl_access_cmd {
	TBL_FUNC_VLAN_READ 	= 0x00,
	TBL_FUNC_VLAN_WRITE 	= 0x01,
	TBL_FUNC_VLAN_INVALID	= 0x02,
	TBL_FUNC_VLAN_VALID	= 0x03,
	TBL_FUNC_ACL_READ	= 0x04,
	TBL_FUNC_ACL_WRITE	= 0x05,
	TBL_FUNC_ACL_TCM_READ	= 0x06,	
	TBL_FUNC_ACL_TCM_WRITE	= 0x07,	
	TBL_FUNC_ACL_MASK_READ	= 0x08,	
	TBL_FUNC_ACL_MASK_WRITE	= 0x09,	
	TBL_FUNC_ACL_RULE_READ	= 0x0a,	
	TBL_FUNC_ACL_RULE_WRITE	= 0x0b,	
	TBL_FUNC_ACL_RATE_READ	= 0x0c,	
	TBL_FUNC_ACL_RATE_WRITE	= 0x0d,	
	TBL_FUNC_RESERVE	= 0x0e,
};
#define TBL_VLAN_IDX_MIN	0
#define TBL_VLAN_IDX_MAX	0x0F
#define TBL_ACL_IDX_MIN		0
#define TBL_ACL_IDX_MAX		0x3F
#define TBL_ACL_MASK_IDX_MIN	0
#define TBL_ACL_MASK_IDX_MAX	0x1F
struct mt7620_esw_tbl_ctrl {
	u8 acc_ctrl;
	u16 index;
};

enum arl_tbl_acc_attr_multi {
	TBL_ARL_ATTR_M_ALL_MAC=0,
	TBL_ARL_ATTR_M_ALL_DIP_GA=1,
	TBL_ARL_ATTR_M_ALL_SIP=2,
	TBL_ARL_ATTR_M_ALL_VALID_ADDR=3,
	TBL_ARL_ATTR_M_ALL_N_STATIC_MAC =4,
	TBL_ARL_ATTR_M_ALL_N_STATIC_DIP=5,
	TBL_ARL_ATTR_M_ALL_STATIC_MAC=6,
	TBL_ARL_ATTR_M_ALL_STATIC_DIP=7,
	TBL_ARL_ATTR_M_ALL_RELATIVE_SIP_DIP=8,
	TBL_ARL_ATTR_M_ALL_RELATIVE_SIP_SIP=9,
	TBL_ARL_ATTR_M_ALL_MAC_CVID=0x0a,
	TBL_ARL_ATTR_M_ALL_MAC_FID=0x0b,
	TBL_ARL_ATTR_M_ALL_MAC_PORT=0x0c,
};
enum arl_tbl_acc_attr_single {
	TBL_ARL_ATTR_S_MAC=0,
	TBL_ARL_ATTR_S_DIP=1,
	TBL_ARL_ATTR_S_SIP=2,
	TBL_ARL_ATTR_S_RO_ADDR=3,
};
enum arl_tbl_acc_cmd {
	TBL_ARL_CMD_READ=0,
	TBL_ARL_CMD_WRITE=0x01,
	TBL_ARL_CMD_CLEAN=0x02,
	TBL_ARL_CMD_SCH=0x04,
	TBL_ARL_CMD_SCH_NXT=0x05,
}; 
#define TBL_ARL_ATTR_MASK	0x7F	
#define TBL_ARL_IS_MULTI	BIT(7)
struct mt7620_esw_arltbl_ctrl{
	u8 acc_attr;
	u8 acc_cmd;
};
struct mt7620_esw_arltbl_data {
	u8  valid;
	u8  is_static;
	u8  tag_attr; /*vlan tag attribute */
	u8  portmap;
	u16 fwd_attr; /*forwarding attribute*/
	u16 cvid;
	u16 time;
	u8 mac[6];
};

struct mt7620_esw {
	void __iomem		*base;
	struct mt7620_esw_platform_data *pdata;
	/* Protects against concurrent register rmw operations. */
	spinlock_t		reg_rw_lock;

	struct switch_dev	swdev;
	bool			global_vlan_enable;
	bool			alt_vlan_disable;
	struct mt7620_esw_vlan_str vlans[MT7620_ESW_NUM_VLANS];
	struct mt7620_esw_port ports[MT7620_ESW_NUM_PORTS];

};

static inline void
mt7620_esw_wr(struct mt7620_esw *esw, u32 val, unsigned reg)
{
	__raw_writel(val, esw->base + reg);
}

static inline u32
mt7620_esw_rr(struct mt7620_esw *esw, unsigned reg)
{
	return __raw_readl(esw->base + reg);
}

static inline void
mt7620_esw_rmw_raw(struct mt7620_esw *esw, unsigned reg, unsigned long mask,
		   unsigned long val)
{
	unsigned long t;

	t = __raw_readl(esw->base + reg) & ~mask;
	__raw_writel(t | val, esw->base + reg);
}


#endif /* _RAMIPS_ESW_H_*/
