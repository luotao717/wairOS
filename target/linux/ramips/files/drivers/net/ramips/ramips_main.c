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
 *   Copyright (C) 2009 John Crispin <blogic@openwrt.org>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>
#include <linux/phy.h>

#include <ramips_eth_platform.h>
#include "ramips_eth.h"

#undef  DO_MAC_READ

#define TX_TIMEOUT (20 * HZ / 100)
#define	MAX_RX_LENGTH	1600

#ifdef CONFIG_SOC_MT7620
#include <mt7620.h>
#include "ramips_esw.c"
#else
static inline int mt7620_esw_init(void) { return 0; }
static inline void mt7620_esw_exit(void) { }
#endif

void __iomem  *fe_mac_base ;

#define phys_to_bus(a)  (a & 0x1FFFFFFF)

#ifdef CONFIG_NET_RAMIPS_DEBUG
#define RADEBUG(fmt, args...)	printk(KERN_DEBUG fmt, ## args)
#else
#define RADEBUG(fmt, args...)	do {} while (0)
#endif

#define RAETH_DEFAULT_MSG_ENABLE	\
	(NETIF_MSG_DRV			\
	| NETIF_MSG_PROBE		\
	| NETIF_MSG_LINK		\
	| NETIF_MSG_TIMER		\
	| NETIF_MSG_IFDOWN		\
	| NETIF_MSG_IFUP		\
	| NETIF_MSG_RX_ERR		\
	| NETIF_MSG_TX_ERR)

static int raeth_msg_level = -1;

enum raeth_reg {
	RAETH_REG_PDMA_GLO_CFG = 0,
	RAETH_REG_PDMA_RST_CFG,
	RAETH_REG_DLY_INT_CFG,
	RAETH_REG_TX_BASE_PTR0,
	RAETH_REG_TX_MAX_CNT0,
	RAETH_REG_TX_CTX_IDX0,
	RAETH_REG_RX_BASE_PTR0,
	RAETH_REG_RX_MAX_CNT0,
	RAETH_REG_RX_CALC_IDX0,
	RAETH_REG_INT_ENABLE,
	RAETH_REG_INT_STATUS,
	RAETH_REG_GDMA1_FWD_CFG,
	RAETH_REG_CDMA_CSG_CFG,
	RAETH_REG_RST_GL,
	RAETH_REG_COUNT
};

static const u32 ramips_reg_table[RAETH_REG_COUNT] = {
	[RAETH_REG_PDMA_GLO_CFG] = REG_FE_PDMA_GLO_CFG,
	[RAETH_REG_PDMA_RST_CFG] = REG_FE_PDMA_RST_CFG,
	[RAETH_REG_DLY_INT_CFG] = REG_FE_DLY_INT_CFG,
	[RAETH_REG_TX_BASE_PTR0] = REG_FE_TX_BASE_PTR0,
	[RAETH_REG_TX_MAX_CNT0] = REG_FE_TX_MAX_CNT0,
	[RAETH_REG_TX_CTX_IDX0] = REG_FE_TX_CTX_IDX0,
	[RAETH_REG_RX_BASE_PTR0] = REG_FE_RX_BASE_PTR0,
	[RAETH_REG_RX_MAX_CNT0] = REG_FE_RX_MAX_CNT0,
	[RAETH_REG_RX_CALC_IDX0] = REG_FE_RX_CALC_IDX0,
	[RAETH_REG_INT_ENABLE] = REG_FE_INT_ENABLE,
	[RAETH_REG_INT_STATUS] = REG_FE_INT_STATUS,
	[RAETH_REG_GDMA1_FWD_CFG] = REG_FE_GDMA1_FWD_CFG,
	[RAETH_REG_CDMA_CSG_CFG] = REG_FE_CDMA_CSG_CFG,
	[RAETH_REG_RST_GL] = REG_FE_RST_GL,
};

extern int rl_mtd_read(
	char *name,
	loff_t from,
	size_t len,
	u_char *buf);

static struct net_device * ramips_dev;
static void __iomem *ramips_fe_base = 0;

static inline u32 get_reg_offset(enum raeth_reg reg)
{
	const u32 *table;

	table = ramips_reg_table;

	return table[reg];
}

static inline void
ramips_fe_wr(struct raeth_priv *rg, u32 val, unsigned reg)
{
	__raw_writel(val, rg->mac_base + reg);
}

static inline u32
ramips_fe_rr(struct raeth_priv *rg, unsigned reg)
{
	return __raw_readl(rg->mac_base + reg);
}

static inline void ramips_fe_twr(struct raeth_priv *rg, u32 val, enum raeth_reg reg)
{
	ramips_fe_wr(rg, val, get_reg_offset(reg));
}

static inline u32 ramips_fe_trr(struct raeth_priv *rg, enum raeth_reg reg)
{
	return ramips_fe_rr(rg, get_reg_offset(reg));
}

static inline void ramips_fe_int_disable(struct raeth_priv *rg, u32 mask)
{
	ramips_fe_twr(rg, ramips_fe_trr(rg, RAETH_REG_INT_ENABLE) & ~mask,
		     RAETH_REG_INT_ENABLE);
	/* flush write */
	ramips_fe_trr(rg, RAETH_REG_INT_ENABLE);
}

static inline void ramips_fe_int_enable(struct raeth_priv *rg, u32 mask)
{
	ramips_fe_twr(rg, ramips_fe_trr(rg, RAETH_REG_INT_ENABLE) | mask,
		     RAETH_REG_INT_ENABLE);
	/* flush write */
	ramips_fe_trr(rg, RAETH_REG_INT_ENABLE);
}

static unsigned char *raeth_speed_str(struct raeth_priv *rg)
{
	switch (rg->speed) {
	case SPEED_1000:
		return "1000";
	case SPEED_100:
		return "100";
	case SPEED_10:
		return "10";
	}

	return "?";
}

static inline void raeth_hw_set_macaddr(struct raeth_priv *rg, unsigned char *mac)
{
	u32 data = 0 ;

	if ( rg->esw == NULL )
	{
	   printk(KERN_WARNING "raeth set macaddress : find switch0 error \n");
	   return ;
	}

	printk(KERN_INFO "mac address : %02X.%02X.%02X.%02X.%02X.%02X \n",
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	data = mac[0]<<8 | mac[1] ;
	mt7620_esw_wr(rg->esw, data, REG_ESW_SMACCR1);

	data = ( mac[2] << 24 ) | ( mac[3] << 16 ) | ( mac[4] << 8 ) | mac[5] ;
	mt7620_esw_wr(rg->esw, data, REG_ESW_SMACCR0);
}

static void raeth_ring_free(struct raeth_ring *ring)
{
	kfree(ring->buf);

	if (ring->descs_cpu)
		dma_free_coherent(NULL, ring->size * ring->desc_size,
				  ring->descs_cpu, ring->descs_dma);
}

static int raeth_ring_alloc(struct raeth_ring *ring)
{
	int err;
	int i;

	ring->desc_size = sizeof(struct ramips_dma_desc);
#if 0
	if (ring->desc_size % cache_line_size()) {
		RADEBUG("mt7620: ring %p, desc size %u rounded to %u\n",
			ring, ring->desc_size,
			roundup(ring->desc_size, cache_line_size()));
		ring->desc_size = roundup(ring->desc_size, cache_line_size());
	}
#endif

	ring->descs_cpu = dma_alloc_coherent(NULL, ring->size * ring->desc_size,
					     &ring->descs_dma, GFP_ATOMIC);
	if (!ring->descs_cpu) {
		err = -ENOMEM;
		goto err;
	}


	ring->buf = kzalloc(ring->size * sizeof(*ring->buf), GFP_KERNEL);
	if (!ring->buf) {
		err = -ENOMEM;
		goto err;
	}

	for (i = 0; i < ring->size; i++) {
		int idx = i * ring->desc_size;
		ring->buf[i].desc = (struct ramips_dma_desc *)&ring->descs_cpu[idx];
		RADEBUG("ramips net: ring %p, desc %d at %p\n",
			ring, i, ring->buf[i].desc);
	}

	return 0;

err:
	return err;
}

static void raeth_ring_tx_clean(struct raeth_priv *rg)
{
	struct raeth_ring *ring = &rg->tx_ring;
	struct net_device *dev = rg->dev;
	u32 bytes_compl = 0, pkts_compl = 0;

	while (ring->curr != ring->dirty) {
		u32 i = ring->dirty % ring->size;

		if (!raeth_desc_valid(ring->buf[i].desc)) {
			ring->buf[i].desc->d2 = TX_DMA_LSO | TX_DMA_PLEN0(ring->buf[i].skb->len);
			dev->stats.tx_errors++;
		}

		if (ring->buf[i].skb) {
			bytes_compl += ring->buf[i].skb->len;
			pkts_compl++;
			dev_kfree_skb_any(ring->buf[i].skb);
		}
		ring->buf[i].skb = NULL;
		ring->dirty++;
	}

	/* flush descriptors */
	wmb();

/*	netdev_completed_queue(dev, pkts_compl, bytes_compl);*/
}

static void raeth_ring_tx_init(struct raeth_priv *rg)
{
	int i;
	struct raeth_ring *ring = &rg->tx_ring;
	struct ramips_dma_desc *txd= NULL;

	for ( i = 0; i < ring->size; i++) {
		txd = ring->buf[i].desc;	

		txd->d4 = TX_DMA_QN(3) | TX_DMA_PN(1);
		txd->d2 = TX_DMA_LSO | TX_DMA_DONE;
	}

	ring->curr = 0;
	ring->dirty = 0;

	/* flush descriptors */
	wmb();
}

static void raeth_ring_rx_clean(struct raeth_priv *rg)
{
	struct raeth_ring *ring = &rg->rx_ring;
	int i;

	if (!ring->buf)
		return;

	for (i = 0; i < ring->size; i++)
	/*	if (ring->buf[i].rx_buf) {*/
		if (ring->buf[i].skb) {
			dma_unmap_single(&rg->dev->dev, ring->buf[i].dma_addr,
					 RAMIPS_RX_BUF_SIZE, DMA_FROM_DEVICE);
		/*	kfree(ring->buf[i].rx_buf);*/
			kfree(ring->buf[i].skb);
		}
}

static int raeth_buffer_offset(struct raeth_priv *rg)
{
	int offset = NET_SKB_PAD;

	/*
	 */
	return offset + NET_IP_ALIGN;
}
static bool raeth_fill_rx_buf(struct raeth_priv *rg, struct raeth_buf *buf,
			       int offset)
{
	struct sk_buff *skb = NULL;
#if 0
	void *data;
	data = kmalloc(RAETH_RX_BUF_SIZE +
		       SKB_DATA_ALIGN(sizeof(struct skb_shared_info)),
		       GFP_ATOMIC);
	
	if (!data)
		return false;

#endif
	skb = dev_alloc_skb(RAMIPS_RX_BUF_SIZE + 
			SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) );
	if ( !skb )
	{
		return false;
	}
	skb->dev = rg->dev; 

/*	buf->rx_buf = data;*/
	buf->skb = skb;
	
	buf->dma_addr = dma_map_single(&rg->dev->dev, skb->data,
				       RAMIPS_RX_BUF_SIZE, DMA_FROM_DEVICE);

	buf->desc->d1 = (u32) buf->dma_addr + offset;
	return true;
}
static int raeth_ring_rx_init(struct raeth_priv *rg)
{
	struct raeth_ring *ring = &rg->rx_ring;
	unsigned int i;
	int ret;
	int offset = raeth_buffer_offset(rg);

	ret = 0;
#if 0
	for (i = 0; i < ring->size; i++) {
		ring->buf[i].desc->next = (u32) (ring->descs_dma +
			ring->desc_size * ((i + 1) % ring->size));

		RADEBUG("mt7620: RX desc at %p, next is %08x\n",
			ring->buf[i].desc,
			ring->buf[i].desc->next);
	}
#endif 

	for (i = 0; i < ring->size; i++) {
		if (!raeth_fill_rx_buf(rg, &ring->buf[i], offset)) {
			ret = -ENOMEM;
			break;
		}
 
/*		ring->buf[i].desc->d2 = RX_DMA_LSO;*/
		ring->buf[i].desc->d2 = RAMIPS_RX_PKT_SIZE << 16 ;
	}

	/* flush descriptors */
	wmb();

	ring->curr = 0;
	ring->dirty = 0;

	return ret;
}

static int raeth_ring_rx_refill(struct raeth_priv *rg)
{
	struct raeth_ring *ring = &rg->rx_ring;
	unsigned int count;
	int offset = raeth_buffer_offset(rg);

	count = 0;
	for (; ring->curr - ring->dirty > 0; ring->dirty++) {
		unsigned int i;

		i = ring->dirty % ring->size;

	/*	if (!ring->buf[i].rx_buf &&*/
		if (!ring->buf[i].skb &&
		    !raeth_fill_rx_buf(rg, &ring->buf[i], offset))
			break;

/*		ring->buf[i].desc->d2 = RX_DMA_LSO;*/
		ring->buf[i].desc->d2 = RAMIPS_RX_PKT_SIZE << 16 ;
		ramips_fe_twr(rg, i, RAETH_REG_RX_CALC_IDX0);

		count++;
	}

	/* flush descriptors */
	wmb();

	RADEBUG("%s: %u rx descriptors refilled\n", rg->dev->name, count);

	return count;
}

static int raeth_rings_init(struct raeth_priv *rg)
{
	int ret;

	ret = raeth_ring_alloc(&rg->tx_ring);
	if (ret)
		return ret;

	raeth_ring_tx_init(rg);
/*	ramips_ring_setup(rg);*/

	ret = raeth_ring_alloc(&rg->rx_ring);
	if (ret)
		return ret;

	ret = raeth_ring_rx_init(rg);
	return ret;
}

static void raeth_rings_cleanup(struct raeth_priv *rg)
{
	raeth_ring_rx_clean(rg);
	raeth_ring_free(&rg->rx_ring);

	raeth_ring_tx_clean(rg);
/*	netdev_reset_queue(rg->dev);*/
	raeth_ring_free(&rg->tx_ring);
}

static inline int ramips_mdio_init(struct raeth_priv *re)
{
	return 0;
}

static inline void ramips_mdio_cleanup(struct raeth_priv *re)
{
}

static inline int ramips_phy_connect(struct raeth_priv *rg)
{
	/*use for link switch dev */
	struct switch_dev *dev = NULL;
	struct mt7620_esw *esw = NULL;

	dev = swconfig_get_dev_bydevname("switch0");
	if ( dev == NULL )
	{
	   printk(KERN_WARNING "raeth set macaddress : find switch0 error \n");
	   return -1;
	}

	esw = container_of(dev, struct mt7620_esw, swdev);

	rg->esw = esw ;	

	return 0;
}

static inline void ramips_phy_disconnect(struct raeth_priv *rg)
{
	rg->esw = NULL;
}

static inline void ramips_phy_start(struct raeth_priv *re)
{
}

static inline void ramips_phy_stop(struct raeth_priv *re)
{
}



static void raeth_setup_dma(struct raeth_priv *rg)
{
	ramips_fe_twr(rg, rg->tx_ring.descs_dma, RAETH_REG_TX_BASE_PTR0);
	ramips_fe_twr(rg, rg->tx_ring.size, RAETH_REG_TX_MAX_CNT0);
	ramips_fe_twr(rg, 0, RAETH_REG_TX_CTX_IDX0);
	ramips_fe_twr(rg, PDMA_RST_DTX_IDX0, RAETH_REG_PDMA_RST_CFG);

	ramips_fe_twr(rg, rg->rx_ring.descs_dma, RAETH_REG_RX_BASE_PTR0);
	ramips_fe_twr(rg, rg->rx_ring.size, RAETH_REG_RX_MAX_CNT0);
	ramips_fe_twr(rg, (rg->rx_ring.size - 1), RAETH_REG_RX_CALC_IDX0);
	ramips_fe_twr(rg, PDMA_RST_DRX_IDX0, RAETH_REG_PDMA_RST_CFG);
}

static int raeth_tx_packets(struct raeth_priv *rg)
{
	struct raeth_ring *ring = &rg->tx_ring;
/*	struct ramips_eth_platform_data *pdata = raeth_get_pdata(rg);*/
	int sent = 0;
	int bytes_compl = 0;

	RADEBUG("%s: processing TX ring\n", rg->dev->name);

	while (ring->dirty != ring->curr) {
		unsigned int i = ring->dirty % ring->size;
		struct ramips_dma_desc *desc = ring->buf[i].desc;
		struct sk_buff *skb = ring->buf[i].skb;

		if (!raeth_desc_valid(desc)) {
#if 0
			if (pdata->is_ar7240 &&
			    raeth_check_dma_stuck(rg, ring->buf[i].timestamp))
				schedule_work(&rg->restart_work);
#endif
/*			printk(KERN_WARNING "ring is not valid: dirty: %d curr: %d size: %d desc: %x %x %x %x \n",
				ring->curr,ring->dirty,ring->size, 
				desc->d1, desc->d2, desc->d3, desc->d4);
*/
			break;
		}

		bytes_compl += skb->len;
		rg->dev->stats.tx_bytes += skb->len;
		rg->dev->stats.tx_packets++;

		dev_kfree_skb_any(skb);
		ring->buf[i].skb = NULL;

		ring->dirty++;
		sent++;
	}

	RADEBUG("%s: %d packets sent out\n", rg->dev->name, sent);

/*	netdev_completed_queue(rg->dev, sent, bytes_compl);*/
	if ((ring->curr - ring->dirty) < (ring->size * 3) / 4)
		netif_wake_queue(rg->dev);

	return sent;
}
static int raeth_rx_packets(struct raeth_priv *rg, int limit)
{
	struct net_device *dev = rg->dev;
	struct raeth_ring *ring = &rg->rx_ring;
	int offset = raeth_buffer_offset(rg);
	int done = 0;

	RADEBUG("%s: rx packets, limit=%d, curr=%u, dirty=%u\n",
			dev->name, limit, ring->curr, ring->dirty);

/*	printk(KERN_WARNING "%s: rx packets, limit=%d, curr=%u, dirty=%u\n",
			dev->name, limit, ring->curr, ring->dirty);
*/

	while (done < limit) {
		unsigned int i = ring->curr % ring->size;
		struct ramips_dma_desc *desc = ring->buf[i].desc;
		struct sk_buff *skb=NULL;
		int pktlen;
		int err = 0;
		int portid = 0;

/*		printk(KERN_WARNING "desc value: %.08X %.08X %.08X %.08X \n",desc->d1, desc->d2, desc->d3, desc->d4);
*/
		if (!raeth_desc_valid(desc))
			break;

		if ((ring->dirty + ring->size) == ring->curr) {
			raeth_assert(0);
			break;
		}

		pktlen = raeth_desc_pktlen(desc);
	/*	pktlen -= ETH_FCS_LEN; */

		dma_unmap_single(&dev->dev, ring->buf[i].dma_addr,
				 RAMIPS_RX_BUF_SIZE, DMA_FROM_DEVICE);

		dev->last_rx = jiffies;
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += pktlen;

/*		skb = build_skb(ring->buf[i].rx_buf, 0);*/
		skb =  ring->buf[i].skb;
		if (!skb) {
	/*		kfree(ring->buf[i].rx_buf);*/
			kfree(ring->buf[i].skb);
			goto next;
		}

		skb_reserve(skb, offset);
		skb_put(skb, pktlen);
#if 0
		{
		int ii;
		printk("%s: recv data : length : %d offset: %d\r\n",dev->name,pktlen,offset);
		for ( ii = 0 ;ii< pktlen; ii ++ )
		{
			if (( ii != 0 ) && (ii % 16 == 0))
			{
				printk("\r\n");
			}
			printk("%02X ",skb->data[ii]);
		}
		printk("\r\n end printk the data \r\n");
		
		}
#endif
		skb->dev = dev;
		skb->ip_summed = CHECKSUM_NONE;
		skb->protocol = eth_type_trans(skb, dev);
		netif_receive_skb(skb);

next:
	/*	ring->buf[i].rx_buf = NULL;*/
		ring->buf[i].skb = NULL;
		done++;
#if 0	
		desc->d2 = RX_DMA_LSO;
		ramips_fe_twr(rg, i, RAETH_REG_RX_CALC_IDX0);
#endif
		ring->curr++;
	}

	raeth_ring_rx_refill(rg);

	RADEBUG("%s: rx finish, curr=%u, dirty=%u, done=%d\n",
		dev->name, ring->curr, ring->dirty, done);
/*	printk(KERN_WARNING "%s: rx finish, curr=%u, dirty=%u, done=%d\n",
		dev->name, ring->curr, ring->dirty, done);
*/

	return done;
}

static int raeth_hw_init(struct net_device *dev)
{
	u32 reg_data = 0;
	struct raeth_priv *rg = netdev_priv(dev);
	int err;

	err = raeth_rings_init(rg);
	if (err)
		goto err_free_irq;

	raeth_hw_set_macaddr(rg, dev->dev_addr);

	raeth_setup_dma(rg);

/*	ramips_fe_twr(rg, DLY_INT_DELAY_INIT, RAETH_REG_DLY_INT_CFG);
	ramips_fe_twr(rg, INT_EN_TX_DLY_INT_MASK | INT_EN_RX_DLY_INT_MASK, RAETH_REG_INT_ENABLE);*/
	ramips_fe_twr(rg, INT_POLLING, RAETH_REG_INT_ENABLE);

	reg_data = ramips_fe_trr(rg, RAETH_REG_GDMA1_FWD_CFG) ;
	reg_data &= ~(0xFFFF | GDMA1_GDM_UCS_EN | GDMA1_GDM_TCS_EN | GDMA1_GDM_ICS_EN ) ;
	ramips_fe_twr(rg, reg_data, RAETH_REG_GDMA1_FWD_CFG);

	reg_data = ramips_fe_trr(rg, RAETH_REG_CDMA_CSG_CFG) ;
	reg_data &= ~(CSG_ICS_GEN_EN | CSG_TCS_GEN_EN | CSG_UCS_GEN_EN );
	ramips_fe_twr(rg, reg_data, RAETH_REG_CDMA_CSG_CFG);

/*	ramips_fe_wr(rg, RAMIPS_PSE_FQFC_CFG_INIT, RAETH_REG_PSE_FQ_CFG);*/

	ramips_fe_twr(rg, 1, RAETH_REG_RST_GL);
	ramips_fe_twr(rg, 0, RAETH_REG_RST_GL);

	return 0;
err_free_irq:
/*
	free_irq(dev->irq, dev);
*/
	return err;
}

void raeth_link_adjust(struct raeth_priv *rg)
{
	if ( !rg->link ) {
		netif_carrier_off(rg->dev);
		if (netif_msg_link(rg))
			pr_info("%s: link down\n", rg->dev->name);
		return;
	}

	netif_carrier_on(rg->dev);
	if (netif_msg_link(rg))
		pr_info("%s: link up (%sMbps/%s duplex)\n",
			rg->dev->name,
			raeth_speed_str(rg),
			(DUPLEX_FULL == rg->duplex) ? "Full" : "Half");
}
static void link_function(struct work_struct *work) {
	struct raeth_priv *rg = container_of(work, struct raeth_priv, link_work.work);
	struct mt7620_esw *esw = rg->esw;
	unsigned long flags;
	u8 mask;
	int i;
	int status = 0;
	u32 phylinks=0;

	/*mask = ~esw->swdata->phy_poll_mask;*/
	
	phylinks = rg->phylinks;

	for (i = 0; i < MT7620_ESW_NUM_LANWAN; i++) {
		int link;

		link = mt7620_mii_read(rg->esw, MT7620_ESW_PHYADDR(i), MII_BMSR);
		if ((link & BMSR_LSTATUS) && ( link != 0xFFFF) ) {
			status = 1;
		/*	break;*/
	/*		if ( (phylinks & BIT(i)) == 0 )*/
			{
			/*	link_phy_led_update(esw,i,1);*/
				phylinks |= BIT(i);
			}
		}
		else
		{
/*			if ( phylinks & BIT(i) )*/
			{
			/*	link_phy_led_update(esw,i,0);*/
				phylinks &= ~BIT(i);
			}
		}
	}

	spin_lock_irqsave(&rg->page_lock, flags);
	rg->phylinks = phylinks; 
	if (status != rg->link) {
		rg->link = status;
		raeth_link_adjust(rg);
	}
	rg->linkcount++;
	spin_unlock_irqrestore(&rg->page_lock, flags);

#if 0
/*	if ((ews->linkcount % 2) == 0)*/
	{
		link_led_bright_blink(esw);	
	}
#endif

	schedule_delayed_work(&rg->link_work, HZ / 2);
}
static int __init raeth_probe(struct net_device *dev)
{
	struct raeth_priv *rg = netdev_priv(dev);
	struct ramips_eth_platform_data *pdata = raeth_get_pdata(rg);
	int err;
	u_char read_mac[ETH_ALEN] ={0};
	u_char zero_mac0[ETH_ALEN] ={0x00,0x00,0x00,0x00,0x00,0x00}; 	
	u_char zero_mac1[ETH_ALEN] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	BUG_ON(!pdata->reset_fe);
	pdata->reset_fe();

	memcpy(dev->dev_addr, pdata->mac, ETH_ALEN);
	//ddsadfsdfsd;
#if 1 //by luo for read mac from flash
	err = rl_mtd_read("Factory", 0x0028, 6, read_mac);
	printk(KERN_INFO "read mac: ret: %d  mac %02X.%02X.%02X.%02X.%02X.%02X \n",err,
			read_mac[0],read_mac[1],read_mac[2],read_mac[3],read_mac[4],read_mac[5]);
       if ( err == 0)
	{
		memcpy(dev->dev_addr, read_mac, ETH_ALEN);
	}
#endif
#ifdef   DO_MAC_READ
/*	net_srandom(jiffies);*/
 	err = mt7620_mtd_read_nm("Factory", GMAC_BASE_OFFSET, ETH_ALEN, read_mac);
	printk(KERN_INFO "read mac: ret: %d  mac %02X.%02X.%02X.%02X.%02X.%02X \n",err,
			read_mac[0],read_mac[1],read_mac[2],read_mac[3],read_mac[4],read_mac[5]);

	if ( (err == 0) && 
	     ( memcmp(read_mac, zero_mac0, ETH_ALEN) != 0 ) && 
	     ( memcmp(read_mac, zero_mac1, ETH_ALEN) != 0 ) ) 
	{
		err = rlvr_netmac_calc(read_mac, GMAC_LANMAC_STEP, ETH_ALEN-1);
		if ( err == 0)
		{
			memcpy(dev->dev_addr, read_mac, ETH_ALEN);
		}
	}

#endif	/*DO_MAC_READ*/

	ether_setup(dev);
	dev->mtu = 1500;
/*	dev->watchdog_timeo = TX_TIMEOUT;*/
/*	spin_lock_init(&rg->page_lock);*/

	spin_lock_init(&rg->phy_lock);

	err = ramips_mdio_init(rg);
	if (err)
		return err;

	err = ramips_phy_connect(rg);
	if (err)
		goto err_mdio_cleanup;

	err = raeth_debugfs_init(rg);
	if (err)
		goto err_phy_disconnect;

	err = raeth_hw_init(dev);
	if (err)
		goto err_debugfs;

	return 0;

err_debugfs:
	raeth_debugfs_exit(rg);
err_phy_disconnect:
	ramips_phy_disconnect(rg);
err_mdio_cleanup:
	ramips_mdio_cleanup(rg);
	return err;
}

static void raeth_uninit(struct net_device *dev)
{
	struct raeth_priv *rg = netdev_priv(dev);

	raeth_debugfs_exit(rg);
	ramips_phy_disconnect(rg);
	ramips_mdio_cleanup(rg);
	ramips_fe_twr(rg, 0, RAETH_REG_INT_ENABLE);

	raeth_rings_cleanup(rg);
}

static int raeth_open(struct net_device *dev)
{
	u32 glo_cfg = 0 ;
	struct raeth_priv *rg = netdev_priv(dev);

	napi_enable(&rg->napi);

	netif_carrier_off(dev);
	ramips_phy_start(rg);

	glo_cfg = ramips_fe_trr(rg, RAETH_REG_PDMA_GLO_CFG);

	glo_cfg &= GLO_INIT_MASK;
	glo_cfg = GLO_TX_WB_DDONE | GLO_TX_DMA_EN | GLO_RX_DMA_EN;

	glo_cfg |= GLO_2B_OFFSET;

	GLO_PDMA_BIT_SIZE_WM(glo_cfg,GLO_PDMA_BIT_16DWORD); /*16dwords*/
	
	ramips_fe_twr(rg, glo_cfg, RAETH_REG_PDMA_GLO_CFG);
	
	netif_start_queue(dev);
/*	rg->link = 1; 
	raeth_link_adjust(rg);
*/
	/*init the phylink state */
	rg->phylinks = 0;
	schedule_delayed_work(&rg->link_work, HZ / 10);
	return 0;
}

static int raeth_stop(struct net_device *dev)
{
	u32 glo_cfg = 0;
	unsigned long flags = 0 ;
	struct raeth_priv *rg = netdev_priv(dev);

	netif_carrier_off(dev);
	/*raeth_phy_stop(rg);*/
	cancel_delayed_work_sync(&rg->link_work);

	spin_lock_irqsave(&rg->page_lock, flags);

	netif_stop_queue(dev);

	glo_cfg = ramips_fe_trr(rg, RAETH_REG_PDMA_GLO_CFG);

	glo_cfg &= ~(GLO_TX_WB_DDONE | GLO_TX_DMA_EN | GLO_RX_DMA_EN);

	ramips_fe_twr(rg, glo_cfg, RAETH_REG_PDMA_GLO_CFG);

	napi_disable(&rg->napi);
	del_timer_sync(&rg->oom_timer);

	spin_unlock_irqrestore(&rg->page_lock, flags);

/*	raeth_rings_cleanup(rg);*/

	return 0;
}

static netdev_tx_t raeth_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct raeth_priv *rg = netdev_priv(dev);
	struct ramips_eth_platform_data *pdata = raeth_get_pdata(rg);
	struct raeth_ring *ring = &rg->tx_ring;
	struct ramips_dma_desc *desc/*, *desc_nxt*/;
	dma_addr_t dma_addr;
	int i;
	unsigned long tx_cpu_owner_idx0 = 0;

#if 0
		{
		int ii;
		printk("%s: ====== send data : length : %d \r\n",dev->name,skb->len);
		for ( ii = 0 ;ii< skb->len; ii ++ )
		{
			if (( ii != 0 ) && (ii % 16 == 0))
			{
				printk("\r\n");
			}
			printk("%02X ",skb->data[ii]);
		}
		printk("\r\n end printk the data \r\n");
		}
	printk(KERN_INFO "start ring curr: %d size: %d dirty: %d \n",ring->curr,ring->size,ring->dirty);
#endif

	
	i = ring->curr % ring->size;
	tx_cpu_owner_idx0 = (i+1) % ring->size;

	desc = ring->buf[i].desc;
/*	desc_nxt = ring->buf[tx_cpu_owner_idx0].desc;*/

	if((ring->buf[i].skb != NULL ) || (ring->buf[tx_cpu_owner_idx0].skb != NULL)){
		goto err_drop;
	}

	if (pdata->min_pkt_len) {
		if (skb->len < pdata->min_pkt_len) {
			if (skb_padto(skb, pdata->min_pkt_len)) {
				printk(KERN_ERR
				       "raeth: skb_padto failed\n");
				goto err_drop;
			}
			skb_put(skb, pdata->min_pkt_len - skb->len);
		}
	}

	dev->trans_start = jiffies;
	dma_addr = dma_map_single(&dev->dev, skb->data, skb->len,
				     DMA_TO_DEVICE);

/*	netdev_sent_queue(dev, skb->len);*/
	ring->buf[i].skb = skb;
#if 0
	ring->buf[i].timestamp = jiffies;
#endif
	desc->d1 = (u32) dma_addr;
	desc->d4 = 0;
	desc->d2 = TX_DMA_LSO | TX_DMA_PLEN0(skb->len);
	

	/* flush descriptor */
	wmb();

	ring->curr++;
	if (ring->curr == (ring->dirty + ring->size)) {
		RADEBUG("%s: tx queue full\n", rg->dev->name);
		netif_stop_queue(dev);
	}

	RADEBUG("%s: packet injected into TX queue\n", rg->dev->name);
/*	while(!raeth_desc_valid(desc_nxt)) {
		printk(KERN_WARNING "tx queue %lu is not empty now \r\n",tx_cpu_owner_idx0);		
	}
*/
	tx_cpu_owner_idx0 = (tx_cpu_owner_idx0 + 1) % ring->size;
/*	desc_nxt = ring->buf[tx_cpu_owner_idx0].desc;*/
/*	if  (!raeth_desc_valid(desc_nxt))*/
	if ( ring->buf[tx_cpu_owner_idx0].skb != NULL )
	{
		netif_stop_queue(dev);
		dev->stats.tx_errors++;
	}

	spin_lock(&rg->page_lock);
	ramips_fe_twr(rg, tx_cpu_owner_idx0, RAETH_REG_TX_CTX_IDX0);
	/*netdev_sent_queue(dev, skb->len);*/
	spin_unlock(&rg->page_lock);
#if 0
	printk(KERN_INFO "finished desc: %x %x %x %x \n",desc->d1, desc->d2, desc->d3, desc->d4);
	printk(KERN_INFO "finished ring curr: %d size: %d dirty: %d \n",ring->curr,ring->size,ring->dirty);
#endif
	
	return NETDEV_TX_OK;
err_drop:
	dev->stats.tx_dropped++;

	dev_kfree_skb(skb);

	return NETDEV_TX_OK;
}


#if 0
static void
raeth_tx_housekeeping(unsigned long ptr)
{
	struct net_device *dev = (struct net_device*)ptr;
	struct raeth_priv *re = netdev_priv(dev);
	unsigned int bytes_compl = 0, pkts_compl = 0;

	spin_lock(&re->page_lock);
	while (1) {
		struct raeth_tx_info *txi;
		struct ramips_tx_dma *txd;

		txi = &re->tx_info[re->skb_free_idx];
		txd = txi->tx_desc;

		if (!(txd->txd2 & TX_DMA_DONE) || !(txi->tx_skb))
			break;

		pkts_compl++;
		bytes_compl += txi->tx_skb->len;

		dev_kfree_skb_irq(txi->tx_skb);
		txi->tx_skb = NULL;
		re->skb_free_idx++;
		if (re->skb_free_idx >= NUM_TX_DESC)
			re->skb_free_idx = 0;
	}
/*	netdev_completed_queue(dev, pkts_compl, bytes_compl);*/
	spin_unlock(&re->page_lock);

	ramips_fe_int_enable(TX_DLY_INT);
}
#endif

static int raeth_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct raeth_priv *rg = netdev_priv(dev);
	int ret;

	switch (cmd) {
	case SIOCETHTOOL:

		if (rg->phy_dev == NULL)
			break;

		spin_lock_irq(&rg->page_lock);
		ret = phy_ethtool_ioctl(rg->phy_dev, (void *) ifr->ifr_data);
		spin_unlock_irq(&rg->page_lock);
		return ret;

	case SIOCSIFHWADDR:
		if (copy_from_user
			(dev->dev_addr, ifr->ifr_data, sizeof(dev->dev_addr)))
			return -EFAULT;
		return 0;

	case SIOCGIFHWADDR:
		if (copy_to_user
			(ifr->ifr_data, dev->dev_addr, sizeof(dev->dev_addr)))
			return -EFAULT;
		return 0;

	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		if (rg->phy_dev == NULL)
			break;

#if 0
		return phy_mii_ioctl(rg->phy_dev, ifr, cmd);
#endif
		return 0;

	default:
		break;
	}

	return -EOPNOTSUPP;
}
static void raeth_oom_timer_handler(unsigned long data)
{
	struct net_device *dev = (struct net_device *) data;
	struct raeth_priv *rg = netdev_priv(dev);

	napi_schedule(&rg->napi);
}
static void
raeth_timeout(struct net_device *dev)
{
	struct raeth_priv *rg = netdev_priv(dev);

	/*tasklet_schedule(&re->tx_housekeeping_tasklet);*/
	schedule_work(&rg->restart_work);
}

static void raeth_restart_work_func(struct work_struct *work)
{
	struct raeth_priv *rg = container_of(work, struct raeth_priv, restart_work);

#if 0
	if (ag71xx_get_pdata(rg)->is_ar724x) {
		rg->link = 0;
		ag71xx_link_adjust(rg);
		return;
	}
#endif

	raeth_stop(rg->dev);
	raeth_uninit(rg->dev);
	raeth_probe(rg->dev);
	raeth_open(rg->dev);

}

static int raeth_poll(struct napi_struct *napi, int limit)
{
	struct raeth_priv *rg = container_of(napi, struct raeth_priv, napi);
	struct ramips_eth_platform_data *pdata = raeth_get_pdata(rg);
	struct net_device *dev = rg->dev;
	struct raeth_ring *rx_ring;
	unsigned long flags;
	int tx_done = 0;
	int rx_done  = 0;

	tx_done = raeth_tx_packets(rg);

	RADEBUG("%s: processing RX ring\n", dev->name);
	rx_done = raeth_rx_packets(rg, limit);

/*	ramips_debugfs_update_napi_stats(rg, rx_done, tx_done);*/

	rx_ring = &rg->rx_ring;

	/*if (rx_ring->buf[rx_ring->dirty % rx_ring->size].rx_buf == NULL)*/
	if (rx_ring->buf[rx_ring->dirty % rx_ring->size].skb == NULL)
		goto oom;

	if ((rx_done < limit) || (!netif_running(dev))) {

		RADEBUG("%s: disable polling mode, rx=%d, tx=%d,limit=%d\n",
			dev->name, rx_done, tx_done, limit);

		napi_complete(napi);

		/* enable interrupts */
		spin_lock_irqsave(&rg->page_lock, flags);
		ramips_fe_int_enable(rg,INT_POLLING);
		spin_unlock_irqrestore(&rg->page_lock, flags);

		return rx_done;
	}

/*more:*/
	RADEBUG("%s: stay in polling mode, rx=%d, tx=%d, limit=%d\n",
			dev->name, rx_done, tx_done, limit);
	return rx_done;

oom:
	if (netif_msg_rx_err(rg))
		pr_info("%s: out of memory\n", dev->name);

	mod_timer(&rg->oom_timer, jiffies + RAMIPS_OOM_REFILL);
	napi_complete(napi);

	return 0;
}

static irqreturn_t raeth_irq(int irq, void *dev)
{
	struct raeth_priv *rg = netdev_priv(dev);
	unsigned int status;

	status = ramips_fe_trr(rg, RAETH_REG_INT_STATUS);
	status &= ramips_fe_trr(rg, RAETH_REG_INT_ENABLE);

	if (!status)
		return IRQ_NONE;

	ramips_fe_twr(rg, status, RAETH_REG_INT_STATUS);

	if (likely( status & INT_POLLING ))	
	{
		ramips_fe_int_disable(rg, INT_POLLING);
		napi_schedule(&rg->napi);	
	}
		
#if 0
	if (status & RX_DLY_INT) {
		ramips_fe_int_disable(rg, RX_DLY_INT);
	/*	tasklet_schedule(&rg->rx_tasklet);*/
	}

	if (status & TX_DLY_INT) {
		ramips_fe_int_disable(rg, TX_DLY_INT);
	/*	tasklet_schedule(&rg->tx_housekeeping_tasklet);*/
	}
#endif
	raeth_debugfs_update_int_stats(rg, status);

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void raeth_netpoll(struct net_device *dev)
{
	disable_irq(dev->irq);
	raeth_irq(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif


static const struct net_device_ops raeth_netdev_ops = {
	.ndo_init		= raeth_probe,
	.ndo_uninit		= raeth_uninit,
	.ndo_open		= raeth_open,
	.ndo_stop		= raeth_stop,
	.ndo_start_xmit		= raeth_hard_start_xmit,
	.ndo_do_ioctl		= raeth_do_ioctl,
	.ndo_tx_timeout		= raeth_timeout,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= raeth_netpoll,
#endif
};

static int __devinit raeth_pdev_probe(struct platform_device *pdev)
{
	struct net_device *dev = NULL;
	struct resource *res;
	struct raeth_priv *rg;

	struct ramips_eth_platform_data *pdata = NULL; 
	int err = 0;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data specified\n");
		err = -ENXIO;
		goto err_out;
	}

	dev = alloc_etherdev(sizeof(*rg));
	if (!dev) {
		dev_err(&pdev->dev, "alloc_etherdev failed\n");
		err = -ENOMEM;
		goto err_out;
	}

	SET_NETDEV_DEV(dev, &pdev->dev);

	rg = netdev_priv(dev);
	rg->pdev = pdev;
	rg->dev = dev;
	rg->msg_enable = netif_msg_init(raeth_msg_level,
					RAETH_DEFAULT_MSG_ENABLE);

	spin_lock_init(&rg->page_lock);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "mac_base");
	if (!res) {
		dev_err(&pdev->dev, "no mac_base resource found\n");
		err = -ENXIO;
		goto err_free_dev;
	}

	rg->mac_base = ioremap_nocache(res->start, resource_size(res));
	if (!rg->mac_base)
	{
		dev_err(&pdev->dev, "unable to ioremap mac_base\n");
		err = -ENOMEM;
		goto err_free_dev;
	}
	fe_mac_base = rg->mac_base;
	
	strcpy(dev->name, "eth1");

	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource found\n");
		err = -ENXIO;
		goto err_unmap_base;
	}

	err = request_irq(dev->irq,  raeth_irq,
			  IRQF_DISABLED,
			  dev->name, dev);
	if (err) {
		dev_err(&pdev->dev, "unable to request IRQ %d\n", dev->irq);
		goto err_unmap_base;
	}

	dev->base_addr = (unsigned long)rg->mac_base;
	dev->netdev_ops = &raeth_netdev_ops;
/*	dev->ethtool_ops = &ag71xx_ethtool_ops;*/

	/*ramips_dev->addr_len = ETH_ALEN;*/
	/*ramips_dev->netdev_ops = &raeth_netdev_ops;*/

	INIT_WORK(&rg->restart_work, raeth_restart_work_func);

	init_timer(&rg->oom_timer);
	rg->oom_timer.data = (unsigned long) dev;
	rg->oom_timer.function = raeth_oom_timer_handler;

	rg->speed = pdata->speed;
	rg->duplex = pdata->duplex;
	rg->rx_fc = pdata->rx_fc;
	rg->tx_fc = pdata->tx_fc;

	rg->tx_ring.size = NUM_TX_DESC ;
	rg->rx_ring.size = NUM_RX_DESC ;

	netif_napi_add(dev, &rg->napi, raeth_poll, RAMIPS_NAPI_WEIGHT);

	err = register_netdev(dev);
	if (err) {
		dev_err(&pdev->dev, "error bringing up device\n");
		goto err_free_desc;
	}

	pr_info("%s: Ralink MT7620 at 0x%08lx, irq %d\n",
			 dev->name, dev->base_addr, dev->irq);

	platform_set_drvdata(pdev, dev);

	INIT_DELAYED_WORK(&rg->link_work, link_function);
	return 0;

 err_free_netdrv:
	unregister_netdev(dev);
 err_free_desc:
/* err_free_irq:*/
	free_irq(dev->irq, dev);
 err_unmap_base:
	iounmap(rg->mac_base);
 err_free_dev:
	kfree(dev);
 err_out:
	platform_set_drvdata(pdev, NULL);
	return err;
}

static int __devexit raeth_pdev_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);

	if ( dev ) {
		struct raeth_priv *rg = netdev_priv(dev);	

		unregister_netdev(dev);
		free_irq(dev->irq,dev);
		iounmap(rg->mac_base);
		kfree(dev);
		platform_set_drvdata(pdev, NULL);
	}
	RADEBUG("raeth: unloaded\n");
	return 0;
}

static struct platform_driver raeth_driver = {
	.probe = raeth_pdev_probe,
	.remove = __exit_p(raeth_pdev_remove),
	.driver = {
		.name =  RAMIPS_DRV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init
raeth_init(void)
{
	int ret;

	ret = raeth_debugfs_root_init();
	if (ret)
		goto err_out;

	ret = mt7620_esw_init();
	if (ret)
		goto err_debugfs_exit;

	ret = platform_driver_register(&raeth_driver);
	if (ret) {
		printk(KERN_ERR
		       "raeth: Error registering platfom driver!\n");
		goto esw_cleanup;
	}

	return 0;

esw_cleanup:
	mt7620_esw_exit();
err_debugfs_exit:
	raeth_debugfs_root_exit();
err_out:
	return ret;
}

static void __exit
raeth_cleanup(void)
{
	platform_driver_unregister(&raeth_driver);
	mt7620_esw_exit();
	raeth_debugfs_root_exit();
}

module_init(raeth_init);
module_exit(raeth_cleanup);

MODULE_VERSION(RAMIPS_DRV_VERSION);
MODULE_AUTHOR("RogerWu <wuxiaobo@richerlink.com>");
MODULE_DESCRIPTION("ethernet driver for ramips boards");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" RAMIPS_DRV_NAME);
