/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   based on Ralink SDK3.3
 *   Copyright (C) 2009 John Crispin <blogic@openwrt.org>
 */

#ifndef RAMIPS_ETH_H
#define RAMIPS_ETH_H

#include <linux/mii.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h> 
#include <linux/dma-mapping.h>
#include <linux/workqueue.h>


#define RAMIPS_DRV_NAME		"ramips_eth"
#define RAMIPS_DRV_VERSION	"1.0.0"

#define NUM_RX_DESC     256
#define NUM_TX_DESC     256

#define RAMIPS_NAPI_WEIGHT      64
#define RAMIPS_OOM_REFILL       (1 + HZ/10)   

#define RAMIPS_TX_MTU_LEN	1540
#define RAMIPS_RX_PKT_SIZE	\
	(ETH_FRAME_LEN + ETH_FCS_LEN + VLAN_HLEN)
#define RAMIPS_RX_BUF_SIZE (RAMIPS_RX_PKT_SIZE + NET_SKB_PAD + NET_IP_ALIGN)

/* Frame Engine Registers */
/* Global Registers */
#define REG_FE_GLO_CFG          	0x00
#define REG_FE_RST_GL           	0x04
#define REG_FE_FOE_TS_T            	0x10
/* CDMA Registers */
#define REG_FE_CDMA_RELATED			0x0400

#define REG_FE_CDMA_CSG_CFG			( REG_FE_CDMA_RELATED + 0x00 )

/* PSE Registers */
#define REG_FE_PSE_RELATED         0x0500
#define REG_FE_PSE_FQFC_CFG        ( REG_FE_PSE_RELATED + 0x00 )
#define REG_FE_PSE_IQ_CFG          ( REG_FE_PSE_RELATED + 0x04 )
#define REG_FE_PSE_QUE_STA         ( REG_FE_PSE_RELATED + 0x08 )

/* GDMA Registers */
#define REG_FE_GDMA1_RELATED       0x0600
#define REG_FE_GDMA1_FWD_CFG       ( REG_FE_GDMA1_RELATED + 0x00 )
#define REG_FE_GDMA1_SHPR_CFG      ( REG_FE_GDMA1_RELATED + 0x04 )
#define REG_FE_GDMA1_MAC_ADRL      ( REG_FE_GDMA1_RELATED + 0x08 )
#define REG_FE_GDMA1_MAC_ADRH      ( REG_FE_GDMA1_RELATED + 0x0C )

/* Old FE with New PDMA */
#define REG_FE_PDMA_RELATED            0x0800
/* 1. PDMA */
#define REG_FE_TX_BASE_PTR0            ( REG_FE_PDMA_RELATED+0x000 )
#define REG_FE_TX_MAX_CNT0             ( REG_FE_PDMA_RELATED+0x004 )
#define REG_FE_TX_CTX_IDX0             ( REG_FE_PDMA_RELATED+0x008 )
#define REG_FE_TX_DTX_IDX0             ( REG_FE_PDMA_RELATED+0x00C )

#define REG_FE_TX_BASE_PTR1            ( REG_FE_PDMA_RELATED+0x010 )
#define REG_FE_TX_MAX_CNT1             ( REG_FE_PDMA_RELATED+0x014 )
#define REG_FE_TX_CTX_IDX1             ( REG_FE_PDMA_RELATED+0x018 )
#define REG_FE_TX_DTX_IDX1             ( REG_FE_PDMA_RELATED+0x01C )

#define REG_FE_TX_BASE_PTR2            ( REG_FE_PDMA_RELATED+0x020 )
#define REG_FE_TX_MAX_CNT2             ( REG_FE_PDMA_RELATED+0x024 )
#define REG_FE_TX_CTX_IDX2             ( REG_FE_PDMA_RELATED+0x028 )
#define REG_FE_TX_DTX_IDX2             ( REG_FE_PDMA_RELATED+0x02C )

#define REG_FE_TX_BASE_PTR3            ( REG_FE_PDMA_RELATED+0x030 )
#define REG_FE_TX_MAX_CNT3             ( REG_FE_PDMA_RELATED+0x034 )
#define REG_FE_TX_CTX_IDX3             ( REG_FE_PDMA_RELATED+0x038 )
#define REG_FE_TX_DTX_IDX3             ( REG_FE_PDMA_RELATED+0x03C )

#define REG_FE_RX_BASE_PTR0            ( REG_FE_PDMA_RELATED+0x100 )
#define REG_FE_RX_MAX_CNT0             ( REG_FE_PDMA_RELATED+0x104 )
#define REG_FE_RX_CALC_IDX0            ( REG_FE_PDMA_RELATED+0x108 )
#define REG_FE_RX_DRX_IDX0             ( REG_FE_PDMA_RELATED+0x10C )

#define REG_FE_RX_BASE_PTR1            ( REG_FE_PDMA_RELATED+0x110 )
#define REG_FE_RX_MAX_CNT1             ( REG_FE_PDMA_RELATED+0x114 )
#define REG_FE_RX_CALC_IDX1            ( REG_FE_PDMA_RELATED+0x118 )
#define REG_FE_RX_DRX_IDX1             ( REG_FE_PDMA_RELATED+0x11C )

#define REG_FE_PDMA_INFO               ( REG_FE_PDMA_RELATED+0x200 )
#define REG_FE_PDMA_GLO_CFG            ( REG_FE_PDMA_RELATED+0x204 )
#define REG_FE_PDMA_RST_IDX            ( REG_FE_PDMA_RELATED+0x208 )
#define REG_FE_PDMA_RST_CFG            (REG_FE_PDMA_RST_IDX)
#define REG_FE_DLY_INT_CFG             ( REG_FE_PDMA_RELATED+0x20C )
#define REG_FE_FREEQ_THRES             ( REG_FE_PDMA_RELATED+0x210 )
#define REG_FE_INT_STATUS              ( REG_FE_PDMA_RELATED+0x220 )
/*#define REG_FE_INT_STATUS		( REG_FE_INT_STATUS)*/ 
#define REG_FE_INT_MASK                ( REG_FE_PDMA_RELATED+0x228 )
#define REG_FE_INT_ENABLE		( REG_FE_INT_MASK )
#define REG_FE_SCH_Q01_CFG		(REG_FE_PDMA_OFFSET+0x280 )
#define REG_FE_SCH_Q23_CFG		(REG_FE_PDMA_OFFSET+0x284 )

/* Register BIT Descriptions */

/* REG_FE_CDMA_CSG_CFG		*/
#define CSG_TCS_GEN_EN			BIT(0) /*Enable TCP checksum generation*/
#define CSG_UCS_GEN_EN			BIT(0) /*Enable UDP checksum generation*/
#define CSG_ICS_GEN_EN			BIT(0) /*Enable IPv4 header checksum generation*/
	/*source port to pdma ring selection*/
#define CSG_SP_RING_WM(value,ring)	ramips_bit_field_mask((value),(ring<<8),(0xFFUL<<8))
#define CSG_INS_VLAN_WM(value,ins_vlan)	ramips_bit_field_mask((value),(ins_vlan<<16),(0xFFFF<<8))

/* REG_FE_GDMA1_FWD_CFG       */
#define GDMA1_FWD_GDM_FRC_P_WM(value,dstport) ramips_bit_field_mask((value),(dstport),(0x7))
#define GDMA1_FWD_GDM_STRPCRC		BIT(16)
#define GDMA1_GDM_JMB_EN		BIT(19)
#define GDMA1_GDM_UCS_EN		BIT(20)
#define GDMA1_GDM_TCS_EN		BIT(21)
#define GDMA1_GDM_ICS_EN		BIT(22)
#define GDMA1_GDM_DROP_256B		BIT(23)
#define GDMA1_GDM_TCI_81XX		BIT(24)
#define GDMA1_GDM_20USTICK_SLT		BIT(25)
#define GDMA1_GDM_JMB_LEN_WM(value,jmblen)	ramips_bit_field_mask((value),(jmblen<<28),(0x0FUL<<28))

/* dma ring REG_FE_PDMA_RST_CFG */
#define PDMA_RST_DRX_IDX1		BIT(17)
#define PDMA_RST_DRX_IDX0		BIT(16)
#define PDMA_RST_DTX_IDX3		BIT(3)
#define PDMA_RST_DTX_IDX2		BIT(2)
#define PDMA_RST_DTX_IDX1		BIT(1)
#define PDMA_RST_DTX_IDX0		BIT(0)

/* REG_FE_DLY_INT_CFG             */
#define DLY_INT_DELAY_EN_INT             0x80
#define DLY_INT_DELAY_MAX_INT            0x04
#define DLY_INT_DELAY_MAX_TOUT           0x04
#define DLY_INT_DELAY_CHAN               (((DLY_INT_DELAY_EN_INT | DLY_INT_DELAY_MAX_INT) << 8) | DLY_INT_DELAY_MAX_TOUT)
#define DLY_INT_DELAY_INIT               ((DLY_INT_DELAY_CHAN << 16) | DLY_INT_DELAY_CHAN)

#define DLY_INT_PSE_FQFC_CFG_INIT        0x80504000


			
/* REG_FE_PDMA_GLO_CFG            */
#define GLO_TX_DMA_EN			BIT(0) /*Tx DMA Enable */
#define GLO_TX_DMA_BUSY			BIT(1) /*RO Indicates whether TX_DMA is busy*/
#define GLO_RX_DMA_EN			BIT(2) /*RX DMA Enable*/
#define GLO_RX_DMA_BUSY			BIT(3) /*RO Indicates whether RX_DMA is busy*/
	/*PDMA Burst size 0:4w; 1 8w ; 2:16w; 3:32w */
#define GLO_PDMA_BIT_SIZE_WM(value,size)	\
			ramips_bit_field_mask((value),((size)<<4),(0x3UL<<4))
#define GLO_PDMA_BIT_4DWORD		0
#define GLO_PDMA_BIT_8DWORD		1
#define GLO_PDMA_BIT_16DWORD		2
#define GLO_PDMA_BIT_32DWORD		3

#define GLO_TX_WB_DDONE			BIT(6)	/*Tx Write Back DDONE*/
#define GLO_BIG_ENDIAN			BIT(7)	/*Selects the Endian mode for PDMA */
#define GLO_DESC_32B			BIT(8)	/*Support 32Byte Descriptor*/
#define GLO_BYTE_SWAP			BIT(29)	/*Byte Swap*/
#define GLO_CSR_CLKGATE			BIT(30)	/*Enable Control Status Register Clock Gate */
#define GLO_2B_OFFSET			BIT(31)	/*Rx 2 Byte Offset */

#define GLO_INIT_MASK			0xFF
  
/* REG_FE_INT_ENABLE	*/
#define INT_EN_TX_DONE_INT_MASK0	BIT(0) /* TX Queue 0 Done Interrupt Mask */
#define INT_EN_TX_DONE_INT_MASK1	BIT(1) /* TX Queue 1 Done Interrupt Mask */
#define INT_EN_TX_DONE_INT_MASK2	BIT(2) /* TX Queue 2 Done Interrupt Mask */
#define INT_EN_TX_DONE_INT_MASK3	BIT(3) /* TX Queue 3 Done Interrupt Mask */
#define INT_EN_RX_DONE_INT_MASK0	BIT(16) /* RX Queue 0 Done Interrupt Mask */
#define INT_EN_RX_DONE_INT_MASK1	BIT(17) /* RX Queue 1 Done Interrupt Mask */
#define INT_EN_TX_DLY_INT_MASK		BIT(28) /* Tx Delay Interrupt Mask */
#define TX_DLY_INT			INT_EN_TX_DLY_INT_MASK
#define INT_EN_TX_COHERENT_INT_MASK	BIT(29) /* Tx Coherent Interrupt Mask */
#define INT_EN_RX_DLY_INT_MASK		BIT(30) /* Rx Delay Interrupt Mask */
#define RX_DLY_INT 			INT_EN_RX_DLY_INT_MASK
#define INT_EN_RX_COHERENT_INT_MASK	BIT(31) /* Rx Coherent Interrupt Mask */

#define INT_POLLING	(INT_EN_TX_DONE_INT_MASK0 | INT_EN_TX_DONE_INT_MASK1 |INT_EN_TX_DONE_INT_MASK2 | \
			INT_EN_TX_DONE_INT_MASK3 | INT_EN_RX_DONE_INT_MASK0 | INT_EN_RX_DONE_INT_MASK1 | \
			TX_DLY_INT | RX_DLY_INT ) 

/***************************************************************************************************/

/* RX DMA Description */
#define RX_DMA_PLEN0(_x)		(((_x) >> 16) & 0x3fff)
#define RX_DMA_LSO			BIT(30)
#define RX_DMA_DONE			BIT(31)

/* TX DMA Description */
#define TX_DMA_PLEN0_MASK		((0x3fff) << 16)
#define TX_DMA_PLEN0(_x)		(((_x) & 0x3fff) << 16)
#define TX_DMA_LSO			BIT(30)
#define TX_DMA_DONE			BIT(31)
#define TX_DMA_QN(_x)			((_x) << 16)
#define TX_DMA_PN(_x)			((_x) << 24)
#define TX_DMA_QN_MASK			TX_DMA_QN(0x7)
#define TX_DMA_PN_MASK			TX_DMA_PN(0x7)

struct ramips_dma_desc {
	unsigned int d1;
	unsigned int d2;
	unsigned int d3;
	unsigned int d4;
} __packed __aligned(4);

struct raeth_buf {
	struct sk_buff		*skb;
	struct ramips_dma_desc	*desc;
	dma_addr_t		dma_addr;
	unsigned int		pad;
};

struct raeth_ring {
	struct raeth_buf	*buf; 
	u8 			*descs_cpu;
	dma_addr_t		descs_dma;
	unsigned int		desc_size;
	unsigned int		curr;
	unsigned int		dirty;
	unsigned int		size;
};

struct raeth_int_stats {
	unsigned long		rx_delayed;
	unsigned long		tx_delayed;
	unsigned long		rx_done0;
	unsigned long		tx_done0;
	unsigned long		tx_done1;
	unsigned long		tx_done2;
	unsigned long		tx_done3;
	unsigned long		rx_coherent;
	unsigned long		tx_coherent;

	unsigned long		pse_fq_empty;
	unsigned long		pse_p0_fc;
	unsigned long		pse_p1_fc;
	unsigned long		pse_p2_fc;
	unsigned long		pse_buf_drop;

	unsigned long		total;
};

struct raeth_debug {
	struct dentry		*debugfs_dir;

	struct raeth_int_stats	int_stats;
};

struct raeth_priv
{
	void __iomem		*mac_base;

	spinlock_t		page_lock;
	struct platform_device	*pdev;
	struct net_device	*dev;
	struct napi_struct	napi;
	u32 			msg_enable;

	struct raeth_ring	rx_ring;	
	struct raeth_ring	tx_ring;	
	
	int			link;
	int			speed;
	int			duplex;
	int			tx_fc;
	int			rx_fc;

	u32			phylinks;
	u32			linkcount;
	struct mt7620_esw	*esw; 
	struct mii_bus		*mii_bus;
	int			mii_irq[PHY_MAX_ADDR];
	struct phy_device	*phy_dev;
	spinlock_t		phy_lock;

	struct work_struct	restart_work;
	struct delayed_work	link_work;
	struct timer_list	oom_timer;

#ifdef CONFIG_NET_RAMIPS_DEBUG_FS
	struct raeth_debug	debug;
#endif
};

#define raeth_assert(_cond)						\
do {									\
	if (_cond)							\
		break;							\
	printk("%s,%d: assertion failed\n", __FILE__, __LINE__);	\
	BUG();								\
} while (0)

static inline struct ramips_eth_platform_data *raeth_get_pdata(struct raeth_priv *rg)
{
	return rg->pdev->dev.platform_data;
}

static inline int raeth_desc_valid(struct ramips_dma_desc *desc)
{
	return (desc->d2 & RX_DMA_DONE) != 0;
}

static inline int raeth_desc_pktlen(struct ramips_dma_desc *desc)
{
	return RX_DMA_PLEN0(desc->d2);
}

static inline void ramips_bit_field_mask(u32 value,u32 data,u32 mask)
{
	value &= ~mask;
	value |= (data & mask);
}
#ifdef CONFIG_NET_RAMIPS_DEBUG_FS
int raeth_debugfs_root_init(void);
void raeth_debugfs_root_exit(void);
int raeth_debugfs_init(struct raeth_priv *re);
void raeth_debugfs_exit(struct raeth_priv *re);
void raeth_debugfs_update_int_stats(struct raeth_priv *re, u32 status);
#else
static inline int raeth_debugfs_root_init(void) { return 0; }
static inline void raeth_debugfs_root_exit(void) {}
static inline int raeth_debugfs_init(struct raeth_priv *re) { return 0; }
static inline void raeth_debugfs_exit(struct raeth_priv *re) {}
static inline void raeth_debugfs_update_int_stats(struct raeth_priv *re,
						  u32 status) {}
#endif /* CONFIG_NET_RAMIPS_DEBUG_FS */

#endif /* RAMIPS_ETH_H */
