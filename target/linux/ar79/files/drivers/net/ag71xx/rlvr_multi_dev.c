/*
 * FILE		RICHERLINK MULIT NET DRIVER 
 * 		special vitual device handling
 *
 * Authors:	Roger Wu <RogerWu81@gmail.com>
 *
 * Fixes:
 * 		Dec 26 2012: Roger Wu <rogerwu81@gmail.com>
 * 		  - add the file
 * 		  - support the atheros 71xx chip handle 
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <net/arp.h>
#include <linux/mtd/mtd.h>

#include "rlvr_multi_dev.h"

/**************************VARABLE****************************/
static struct rlvr_multi_dev_group *rlvr_group = NULL;
/***************************FUNCTION****************************/
/*revert the port*/
#ifdef  RLVR_MULTI_SUPPORT_AG71XX
/*#define RLVR_GET_DEV_ID(port,num)  ((num)-1 - (port)%(num))*/
#define RLVR_GET_DEV_ID(port,num)  (port)
#else
#define RLVR_GET_DEV_ID(port,num)  (port)
#endif /**/


#ifdef RLVR_MULTI_SUPPORT_MT7620
int mt7620_mtd_write_nm(char *name, loff_t to, size_t len, const u_char *buf)
{
	int ret = -1;
	size_t rdlen, wrlen;
	struct mtd_info *mtd;
	struct erase_info ei;
	u_char *bak = NULL;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return (int)mtd;
	if (len > mtd->erasesize) {
		put_mtd_device(mtd);
		return -E2BIG;
	}

	bak = kmalloc(mtd->erasesize, GFP_KERNEL);
	if (bak == NULL) {
		put_mtd_device(mtd);
		return -ENOMEM;
	}

	ret = mtd->read(mtd, 0, mtd->erasesize, &rdlen, bak);
	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}
	if (rdlen != mtd->erasesize)
		printk(KERN_WARNING "warning: ra_mtd_write: rdlen is not equal to erasesize\n");

	memcpy(bak + to, buf, len);

	ei.mtd = mtd;
	ei.callback = NULL;
	ei.addr = 0;
	ei.len = mtd->erasesize;
	ei.priv = 0;
	ret = mtd->erase(mtd, &ei);
	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}

	ret = mtd->write(mtd, 0, mtd->erasesize, &wrlen, bak);

	put_mtd_device(mtd);
	kfree(bak);
	return ret;
}

int mt7620_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf)
{
	int ret;
	size_t rdlen;
	struct mtd_info *mtd;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return (int)mtd;

	ret = mtd->read(mtd, from, len, &rdlen, buf);
	if (rdlen != len)
		printk("warning: ra_mtd_read_nm: rdlen is not equal to len\n");

	put_mtd_device(mtd);
	return ret;
}
#endif/*RLVR_MULTI_SUPPORT_MT7620*/

static inline struct rlvr_multi_dev_group *rlvr_multi_group_alloc(struct net_device *real_dev)
{
	struct rlvr_multi_dev_group *rg = NULL;

	rg =  kzalloc(sizeof(*rg),GFP_KERNEL);
	if ( !rg)
		{
		return NULL;
		}

	rg->real_dev = real_dev;
	return rg;
}
static inline struct net_device *rlvr_multi_get_device(struct rlvr_multi_dev_group *rg,
							unsigned int port_id)
{
	struct net_device *dev= NULL;
	u32 index = 0;

	index = RLVR_GET_DEV_ID(port_id,RLVR_MULTI_PORT);
/*	dev = rg->multi_dev_arrays[port_id%RLVR_MULTI_PORT];*/
	dev = rg->multi_dev_arrays[index];

	return dev ? dev : NULL;
}
static void rlvr_multi_set_device(struct rlvr_multi_dev_group *rg,
						unsigned int port_id ,
						struct net_device *dev)
{
	struct net_device **rgdev=NULL;
	u32 index = 0;	

	if (rg == NULL)
		return ;
	
	index = RLVR_GET_DEV_ID(port_id,RLVR_MULTI_PORT); 	
	rgdev = &rg->multi_dev_arrays[index];
/*	rgdev = &rg->multi_dev_arrays[port_id%RLVR_MULTI_PORT];*/

	*rgdev = dev;	
}
static void rlvr_multi_clear_device(struct rlvr_multi_dev_group *rg,
						unsigned int port_id) 
{
	struct net_device **rgdev=NULL;

	if (rg == NULL)
		return ;

	rgdev = &rg->multi_dev_arrays[port_id%RLVR_MULTI_PORT];
	*rgdev = NULL;	
	
}
static struct rlvr_multi_dev_group * rlvr_multi_find_group(struct net_device *real_dev)
{
	real_dev = real_dev;
	return rlvr_group;
} 
static struct net_device * rlvr_multi_find_dev(struct net_device *real_dev,unsigned int portid)
{
	struct rlvr_multi_dev_group *rg = NULL;
	
	rg = rlvr_multi_find_group(real_dev);
	if(rg == NULL)
		{
		return NULL;
		} 

	return rlvr_multi_get_device(rg,portid);
}
static int rlvr_check_real_dev(struct net_device *real_dev,unsigned int portid)
{
	struct net_device *dev = NULL;

	if ((dev = rlvr_multi_find_dev(real_dev,portid)) != NULL)
	{
		
		printk("%s %d dev address :%p \r\n",__FILE__,__LINE__,dev);
		return -EEXIST;
	}

	return 0;
}
/*ethtool*/
static int rlvr_multi_ethtool_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	const struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);

	return dev_ethtool_get_settings(dev_pri->real_dev, cmd);
} 
static void rlvr_multi_ethtool_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *drvinfo)
{
	const struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev;

	if ( real_dev->ethtool_ops && real_dev->ethtool_ops->get_drvinfo)
	{
		real_dev->ethtool_ops->get_drvinfo(dev ,drvinfo);
	}
}

static u32 rlvr_multi_ethtool_get_rx_csum(struct net_device *dev)
{
	const struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);

	return dev_ethtool_get_rx_csum(dev_pri->real_dev);	
}
static u32 rlvr_multi_ethtool_get_flags(struct net_device *dev)
{
	const struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	
	return dev_ethtool_get_flags(dev_pri->real_dev);
} 

#ifdef RLVR_MULTI_SUPPORT_AG71XX
static void rlvr_multi_build_hdr_ag71xx(struct net_device *dev ,struct rlvr_multi_ar_header *arhdr)
{
	unsigned char tx_attr = AR_HEADER_TX_ATTR;
	unsigned char from_cpu = AR_HEADER_FROM_CPU;
	unsigned char portid = 0 ;
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev); 

	arhdr->type = AR_HEADER_DEF_TYPE;			
	
	portid = (dev_pri->portid + 1) & AR_HEADER_PORT_ID_MASK;
	
	arhdr->port = tx_attr | from_cpu | AR_HEADER_RESERVE1| portid ; 
	
}
static int rlvr_multi_parse_hdr_ag71xx(struct rlvr_multi_ar_header *arhdr ,
					struct rlvr_multi_dev_info *dev_pri)
{
	unsigned int portid = 0;

	portid = arhdr->port & AR_HEADER_PORT_ID_MASK;
	
	dev_pri->portid = portid - 1 ;

	return 0;
} 

#endif /*RLVR_MULTI_SUPPORT_AG71XX*/
#ifdef RLVR_MULTI_SUPPORT_MT7620
static void rlvr_multi_build_hdr_raeth(struct net_device *dev ,struct rlvr_multi_ethhdr *rlvrhdr)
{
	unsigned char portid = 0 ;
	struct rlvr_multi_ar_header *arhdr = &rlvrhdr->hard_hdr; 
	unsigned char *pdata = NULL;

	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev); 

	arhdr->type = RAETH_HEADER_TYPE;
	
	portid = ( dev_pri->portid + 1 ) & RAETH_HEADER_PORT_MASK;
	
	arhdr->port = portid ; 

	/*add vlan message*/
	pdata = rlvrhdr->h_dest;
	memmove(pdata, pdata + 4, 12);

	if ( dev_pri->flags & RLVR_MULTI_FLAGS_USE_VLAN )
	{
		rlvrhdr->h_vlan_proto = htons(dev_pri->h_vlan_proto);
		rlvrhdr->h_vlan_TCI = htons(dev_pri->h_vlan_TCI); 
	}
}
static int rlvr_multi_parse_hdr_raeth(struct rlvr_multi_ar_header *arhdr ,
					struct rlvr_multi_dev_info *dev_pri)
{
	unsigned int portid = 0;

	portid = arhdr->port & RAETH_HEADER_PORT_MASK;
	
	dev_pri->portid = portid - 1 ;

	return 0;
} 

#endif /*RLVR_MULTI_SUPPORT_MT7620*/
static void rlvr_multi_build_hdr(struct net_device *dev ,struct rlvr_multi_ethhdr *rlvrhdr)
{
#ifdef RLVR_MULTI_SUPPORT_AG71XX
	rlvr_multi_build_hdr_ag71xx(dev, &rlvrhdr->hard_hdr);
#endif /*RLVR_MULTI_SUPPORT_AG71XX*/
#ifdef RLVR_MULTI_SUPPORT_MT7620
	rlvr_multi_build_hdr_raeth(dev, rlvrhdr);
#endif /*RLVR_MULTI_SUPPORT_MT7620*/
}

/**/
static struct net_device * rlvr_multi_parse_hdr( struct net_device *real_dev,
			   	 struct rlvr_multi_ar_header *arheader)
{
	struct  rlvr_multi_dev_info parsdev;
	struct net_device *dev = NULL;
#ifdef RLVR_MULTI_SUPPORT_AG71XX
	rlvr_multi_parse_hdr_ag71xx(arheader,&parsdev);	
#endif /*RLVR_MULTI_SUPPORT_AG71XX*/
#ifdef RLVR_MULTI_SUPPORT_MT7620
	rlvr_multi_parse_hdr_raeth(arheader,&parsdev);	
#endif /*RLVR_MULTI_SUPPORT_MT7620*/

	/*find the dev*/
	dev =  rlvr_multi_find_dev(real_dev,parsdev.portid);	
	return dev;
} 
#ifdef  RLVR_MULTI_SUPPORT_MT7620
struct sk_buff * rlvr_multi_parse_hdr_byport( struct sk_buff *skb,
			   	 struct rlvr_multi_ar_header *arheader)
{
	struct rlvr_multi_ar_header *phrd = NULL;

	phrd = (struct rlvr_multi_ar_header *)skb->data;

	skb_pull(skb,RLVR_MULTI_HEAD_DESC_LEN);

	*arheader = *phrd ;	

	return skb;
}
#endif /* RLVR_MULTI_SUPPORT_MT7620*/
static struct sk_buff *_rlvr_multi_put_tag(struct sk_buff *skb,struct rlvr_multi_dev_info *dev_pri) 
{
	struct rlvr_multi_ethhdr *rlvrhdr;
	
	if ( skb_cow_head(skb,RLVR_MULTI_HEAD_DESC_VLAN_LEN) < 0)
	{
		kfree_skb(skb);
		return NULL;
	}
		
	rlvrhdr = (struct rlvr_multi_ethhdr *) skb_push(skb,RLVR_MULTI_HEAD_DESC_VLAN_LEN);

	/*Move the mac address*/
/*	memmove(skb->data,skb->data + RLVR_MULTI_ETHERHDR_LEN ,2* ETH_ALEN);*/

	skb->mac_header -= RLVR_MULTI_HEAD_DESC_VLAN_LEN;

	/*build header of special chip*/
	rlvr_multi_build_hdr(skb->dev,rlvrhdr);	

	rlvr_multi_head_send_handle(skb);
	return skb; 
	
}
#ifdef  RLVR_MULTI_SUPPORT_MT7620
struct sk_buff *rlvr_multi_put_tag_byport(struct sk_buff *skb,unsigned char portid) 
{
	struct rlvr_multi_ar_header *phrd = NULL;

	phrd = (struct rlvr_multi_ar_header *) skb_push(skb,RLVR_MULTI_HEAD_DESC_LEN);
	phrd->type = RAETH_HEADER_TYPE;   
	phrd->port = portid + 1;   
	
	return skb;
}
#endif /* RLVR_MULTI_SUPPORT_MT7620*/

#if 0
static __be16 rlvr_multi_eth_type_trans(struct sk_buff *skb,
					struct net_device *dev,
					struct rlvr_multi_ethhdr *rlvrhdr )
{
	unsigned char *rawp;

	skb->dev = dev;

	if (unlikely( is_multicast_ether_addr(rlvrhdr->h_dest)))
	{
		if ( !compare_ether_addr_64bits(rlvrhdr->h_dest , dev->broadcast))
		{
			skb->pkt_type = PACKET_BROADCAST;	
		}
		else
		{
			skb->pkt_type = PACKET_MULTICAST;	
		}
	}	
	else if ( 1 /*dev->flags & IFF_PROMISC*/) 
	{
		if (unlikely(compare_ether_addr_64bits(rlvrhdr->h_dest,dev->dev_addr)))
		{
			skb->pkt_type = PACKET_OTHERHOST;
		}
	}

	if (netdev_uses_dsa_tags(dev))
	{
		return htons(ETH_P_DSA);
	}
	if ( netdev_uses_trailer_tags(dev))
	{
		return htons(ETH_P_TRAILER);
	}

	if ( ntohs(rlvrhdr->next_proto) >= 1536)
	{
		return rlvrhdr->next_proto;
	}
	
	rawp = skb->data;
	
	if ( *(unsigned short *)rawp == 0xFFFF)
	{
		return htons(ETH_P_802_3);
	}

	return htons(ETH_P_802_2);
}
#endif
/*recv*/
int rlvr_multi_skb_recv( struct sk_buff *skb,struct net_device *dev)
{
	struct rlvr_multi_ar_header *arheader = NULL;
#if 0	
	skb = skb_share_check(skb,GFP_ATOMIC);
	if (skb == NULL)
	{
		goto err_free;
	}		
#endif

	if ( unlikely(!pskb_may_pull(skb,RLVR_MULTI_HEAD_DESC_LEN)))
	{
		goto err_free;
	}

	rcu_read_lock();

 	rlvr_multi_head_recv_handle(skb);

	arheader = (struct rlvr_multi_ar_header *)skb->data;

/*	skb_reset_mac_header(skb);*/

	skb_pull(skb,RLVR_MULTI_HEAD_DESC_LEN);

	skb->dev = rlvr_multi_parse_hdr(dev,arheader);
	if ( skb->dev == NULL)
	{
		printk("%s %d recv error ! not find the dev: of real dev: %s port:%u\r\n",
			__FILE__,__LINE__,dev->name, arheader->port);
		goto  err_unlock;
	}

	if (!netif_carrier_ok(skb->dev) )
	{
		goto  err_unlock;
	}
	
	skb->dev->last_rx = jiffies;
	skb->dev->stats.rx_packets++;
	skb->dev->stats.rx_bytes += skb->len +  RLVR_MULTI_HEAD_DESC_LEN;
#if 0
	printk("%s %d receive pkt of dev: %s \r\n",__FILE__,__LINE__,skb->dev->name);
		{
		int ii;
		printk("%s %d %s: recv data : length : %d\r\n",__FILE__,__LINE__,dev->name,skb->len);
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
#endif
/*	skb->protocol = rlvr_multi_eth_type_trans(skb,skb->dev,rlvrhdr);*/
	skb->ip_summed = CHECKSUM_NONE;
	skb->protocol = eth_type_trans(skb, skb->dev);

	netif_receive_skb(skb);	

	rcu_read_unlock();
	return 0;
err_unlock:
	rcu_read_unlock();
err_free:
	if ( skb->dev ){
		skb->dev->stats.rx_errors++ ;
	}
	kfree_skb(skb);
	return -ENOSPC;
}
/*net header ops*/
static int rlvr_multi_dev_hard_header(struct sk_buff *skb, struct net_device *dev,
					unsigned short type, const void *daddr ,
					const void * saddr ,unsigned len )
{
	struct rlvr_multi_ethhdr *rlvrhdr = NULL;
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device * real_dev = NULL;
	int add_len = 0;
	int rc = 0;

	/* Now make the lower layer header*/	
	if (WARN_ON(skb_headroom(skb) < dev->hard_header_len))
	{
		return -ENOSPC;
	}

	/*mac address build*/
	if (saddr == NULL)
	{
		saddr = dev->dev_addr;
	}

        /* Now make the underlying real hard header */
        real_dev = dev_pri->real_dev;
        rc = dev_hard_header(skb, real_dev, type, daddr, saddr, len + add_len); 
        if (rc > 0)
                rc += add_len;

	/*put the tag of dev */
	if (!( dev_pri->flags & RLVR_MULTI_FLAGS_REB_HEADER)) 
	{
		skb = _rlvr_multi_put_tag(skb,dev_pri);
		if(skb == NULL){
			return -RLVR_MULTI_HEAD_DESC_VLAN_LEN  ;
		}

		rc += RLVR_MULTI_HEAD_DESC_VLAN_LEN ;	
	}

	return rc  ;
} 
static int rlvr_multi_dev_rebuild_header(struct sk_buff *skb ) 
{
	struct net_device *dev = skb->dev;
	struct rlvr_multi_ethhdr *rlvrhdr = (struct rlvr_multi_ethhdr *)(skb->data);
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);

		switch (rlvrhdr->next_proto) {
#ifdef CONFIG_INET
		case htons(ETH_P_IP):
			return arp_find(rlvrhdr->h_dest ,skb);	
#endif
		default:
	            pr_debug("%s: unable to resolve type %X addresses.\n",
       	                  dev->name, ntohs(rlvrhdr->next_proto));

			memcpy(rlvrhdr->h_source,dev->dev_addr,ETH_ALEN);
			break;
		}
	return 0;
}

void rlvr_multi_link_adjust(struct net_device *real_dev,unsigned int portid,unsigned int status)
{
	struct net_device *dev;

	if ( portid >= RLVR_MULTI_PORT )
	{
		return;
	} 	

	dev = rlvr_multi_find_dev(real_dev,portid);
	if ( dev == NULL)
	{
		return;
	}

	/*Link up*/
	if ( status )	
	{
		if (!netif_carrier_ok(dev) )
		{
			printk(KERN_INFO"%s: link up\n",dev->name);
			netif_carrier_on(dev);
		}
	}
	else
	{
		if ( netif_carrier_ok(dev) )
		{
			printk(KERN_INFO"%s: link down\n",dev->name);
			netif_carrier_off(dev);
		}
	}
}
static const struct header_ops rlvr_multi_header_ops = {
	.create		= rlvr_multi_dev_hard_header,
	.rebuild	= rlvr_multi_dev_rebuild_header,
	.parse		= eth_header_parse,
};
/*net_device ops*/
static int rlvr_multi_dev_init(struct net_device *dev)
{
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev;

	netif_carrier_off(dev);

	dev->flags	= real_dev->flags & ~(IFF_UP | IFF_PROMISC | IFF_ALLMULTI);
	dev->iflink	= real_dev->ifindex;
	dev->state	= (real_dev->state & ((1<<__LINK_STATE_NOCARRIER) |
					  (1<<__LINK_STATE_DORMANT))) |
		      (1<<__LINK_STATE_PRESENT);
	
	dev->features	|= real_dev->features;
	dev->gso_max_size = real_dev->gso_max_size;

	dev->dev_id 	=  real_dev->dev_id;

	if ( is_zero_ether_addr(dev->dev_addr))
	{
		memcpy(dev->dev_addr,real_dev->dev_addr,dev->addr_len);		
	}
	if ( is_zero_ether_addr(dev->broadcast))
	{
		memcpy(dev->broadcast ,real_dev->broadcast ,dev->addr_len);
	}

	/*header ops*/
	dev->header_ops		= &rlvr_multi_header_ops;
	dev->hard_header_len	= RLVR_MULTI_ETHERHDR_LEN;/*real_dev->hard_header_len + RLVR_MULTI_HEAD_LEN;*/
/*	dev->netdev_ops		= &rlvr_multi_device_ops;*/

	return 0;
}
static void rlvr_multi_dev_uninit(struct net_device *dev)
{
	/*UN DO*/	
}
static int rlvr_multi_dev_open(struct net_device *dev) 
{
	int ret  = 0;
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev;	


	if ( !( real_dev->flags & IFF_UP ))
	{
		return -ENETDOWN;
	}
	
	if ( compare_ether_addr(dev->dev_addr,real_dev->dev_addr ))
	{
		ret  =  dev_unicast_add ( real_dev ,dev->dev_addr);
		if ( ret < 0)
		{
			goto out ;
		}
	}
	if ( dev->flags & IFF_ALLMULTI) 
	{
		ret =  dev_set_allmulti(real_dev ,1);
		if ( ret < 0)
		{
			goto del_unicast ;
		}
	}
	
	if (dev->flags & IFF_PROMISC)
	{
		ret = dev_set_promiscuity(real_dev,1);
		if ( ret < 0)
		{
			goto clear_allmulti ;
		}
	}
	

/*	netif_carrier_on(dev);*/
	netif_carrier_off(dev);
	return 0;

clear_allmulti:
	if (dev->flags & IFF_ALLMULTI)
	{
		dev_set_allmulti(real_dev,-1);
	}
del_unicast:
	if (compare_ether_addr(dev->dev_addr,real_dev->dev_addr))
	{
		dev_unicast_delete(real_dev,dev->dev_addr);
	}
out:
/*	netif_carrier_off(dev);*/
	return ret;
}

static int rlvr_multi_dev_stop(struct net_device *dev)
{
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev; 
	
	dev_mc_unsync(real_dev,dev);
	dev_unicast_unsync(real_dev,dev);

	if (dev->flags & IFF_ALLMULTI)
	{
		dev_set_allmulti(real_dev ,-1);	
	}
	if (dev->flags & IFF_PROMISC)
	{
		dev_set_promiscuity(real_dev,-1);
	} 	
	if (compare_ether_addr(dev->dev_addr,real_dev->dev_addr))
	{
		dev_unicast_delete(real_dev,dev->dev_addr);
	}

	netif_carrier_off(dev);
	return 0;
}
static netdev_tx_t rlvr_multi_hard_start_xmit(struct sk_buff * skb ,
							struct net_device *dev)
{
	int ret = 0;	
	int len = 0;
	int queue = skb_get_queue_mapping(skb);
	struct netdev_queue *txq = netdev_get_tx_queue(dev,queue);
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev;	
	
	if ( dev_pri->flags & RLVR_MULTI_FLAGS_REB_HEADER )
	{
		skb = _rlvr_multi_put_tag(skb,dev_pri);
		if (skb == NULL)
		{
			txq->tx_dropped++;
			dev_kfree_skb(skb);
			return NETDEV_TX_OK;
		}	
	}

#if 0
	printk("%s %d send dev: %s \r\n",__FILE__,__LINE__,dev->name);
		{
		int ii;
		printk("%s %d : ifac: %s: ====== send data : length : %d \r\n",
			__FILE__,__LINE__,dev->name,skb->len);
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
#endif
	/*queue packect to skb*/
	skb->dev =  real_dev;
	len = skb->len;
	ret = dev_queue_xmit(skb);
	
	if (likely(ret == NET_XMIT_SUCCESS))
	{
		txq->tx_packets++;
		txq->tx_bytes += len;
	}
	else
	{
		txq->tx_dropped++;
	}
	return NETDEV_TX_OK;
}
static int rlvr_multi_dev_validate_addr(struct net_device *dev)
{
	if ( !is_valid_ether_addr(dev->dev_addr))
	{
		return -EADDRNOTAVAIL;
	}

	return 0;
}

static int rlvr_multi_dev_set_mac_address(struct net_device *dev ,void *p)
{
	int ret = 0;
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev;
	struct sockaddr *addr = p;

	if ( is_valid_ether_addr(addr->sa_data))
	{
		return -EADDRNOTAVAIL;
	}		

	if ( !( dev->flags & IFF_UP ))
	{
		goto out ;
	}

	
	if ( compare_ether_addr(addr->sa_data ,real_dev->dev_addr ))
	{
		ret = dev_unicast_add(real_dev,addr->sa_data);
		if (ret < 0)
		{
			return ret;	
		}
	}

	if ( compare_ether_addr(dev->dev_addr, real_dev->dev_addr))
	{
		dev_unicast_delete (real_dev,dev->dev_addr);
	}

out:
	memcpy(dev->dev_addr,addr->sa_data,ETH_ALEN);	
	return 0;
}
static void rlvr_multi_dev_set_multicast_list( struct net_device *dev)
{
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);

	dev_mc_sync(dev_pri->real_dev, dev);
	dev_unicast_sync(dev_pri->real_dev, dev);
}
static int rlvr_multi_dev_change_mtu(struct net_device *dev, int new_mtu)
{
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	
	if ( dev_pri->real_dev->mtu < new_mtu )
	{
		return -ERANGE;
	}

	dev->mtu = new_mtu;

	return 0;
}
static int rlvr_multi_dev_ioctl(struct net_device *dev ,
				struct ifreq *ifr ,int cmd)
{
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev;
	const struct net_device_ops * real_ops = real_dev->netdev_ops;
	struct ifreq ifr_r ;
	int ret = -EOPNOTSUPP;

	strncpy(ifr_r.ifr_name , real_dev->name,IFNAMSIZ);	
	ifr_r.ifr_ifru = ifr->ifr_ifru;

	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		if ( netif_device_present(real_dev) && real_ops->ndo_do_ioctl )
		{
			ret = real_ops->ndo_do_ioctl(real_dev,&ifr_r,cmd);
		}
		break; 
	}

	if ( !ret )
	{
		ifr->ifr_ifru = ifr_r.ifr_ifru;
	}
	
	return ret;
}

/*****************************ops define****************************/
static const struct net_device_ops rlvr_multi_device_ops = {
	.ndo_init		= rlvr_multi_dev_init,
	.ndo_uninit		= rlvr_multi_dev_uninit,
	.ndo_open		= rlvr_multi_dev_open,
	.ndo_stop		= rlvr_multi_dev_stop,
	.ndo_start_xmit		= rlvr_multi_hard_start_xmit,
	.ndo_validate_addr	= rlvr_multi_dev_validate_addr,
	.ndo_set_mac_address	= rlvr_multi_dev_set_mac_address,
	.ndo_set_multicast_list	= rlvr_multi_dev_set_multicast_list,
	.ndo_change_mtu		= rlvr_multi_dev_change_mtu,
	.ndo_do_ioctl		= rlvr_multi_dev_ioctl,	
}; 
static const struct ethtool_ops rlvr_multi_ethtool_ops = {
	.get_settings		= rlvr_multi_ethtool_get_settings,
	.get_drvinfo		= rlvr_multi_ethtool_get_drvinfo,
	.get_link		= ethtool_op_get_link,
	.get_rx_csum		= rlvr_multi_ethtool_get_rx_csum,
	.get_flags		= rlvr_multi_ethtool_get_flags,
	
};
/**************************************************************/
static void rlvr_multi_setup( struct net_device *dev)
{
	ether_setup(dev);

	dev->tx_queue_len = 0;
	
	/*ops init*/
	dev->netdev_ops		= &rlvr_multi_device_ops;
/*	dev->destructor		= free_netdev;*/
	dev->ethtool_ops	= &rlvr_multi_ethtool_ops;
}
static int _rlvr_register_multi_dev(struct net_device *dev)
{
	int ret = 0;
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev;	
	unsigned int portid = dev_pri->portid;
	struct rlvr_multi_dev_group *grp= NULL;

	grp = rlvr_multi_find_group(real_dev);
	if ( grp == NULL)
	{
		grp = rlvr_multi_group_alloc(real_dev);
		if ( grp == NULL )
		{
			return -ENOBUFS;		
		}
		rlvr_group = grp;	
	}
	
	ret = register_netdev(dev);
	if ( ret < 0 )
	{
		return ret;	
	}	
	/*Account for reference in struct dev */
	dev_hold(real_dev);
	
	rlvr_multi_set_device(grp,portid,dev);
	grp->nr_devs++;

	/*enable the port header register to hard net_driver */

	return 0;
}
static void _rlvr_unregister_multi_dev(struct net_device *dev)
{
	struct rlvr_multi_dev_info *dev_pri = netdev_priv(dev);
	struct net_device *real_dev = dev_pri->real_dev; 
	struct rlvr_multi_dev_group *grp = NULL;
	unsigned int portid = dev_pri->portid;
	
	grp = rlvr_multi_find_group(real_dev); 
	if (grp != NULL)
	{
		if ( rlvr_multi_find_dev (real_dev,portid))
		{ 	
			rlvr_multi_clear_device(grp,portid);	
			grp->nr_devs--;
			if ( grp->nr_devs == 0 )
			{
				kzfree(grp);	
			}
		}
	}
	
	unregister_netdev(dev);

	dev_put(real_dev);	
	
}
static int rlvr_register_multi_dev(struct net_device *real_dev,unsigned int portid)
{
	int ret = 0;
	struct net_device *new_dev = NULL;
	struct rlvr_multi_dev_info *dev_pri = NULL;
	char name[IFNAMSIZ];
#ifdef RLVR_MULTI_SUPPORT_MT7620
	u_char read_mac[ETH_ALEN] ={0};
	u_char zero_mac0[ETH_ALEN] ={0x00,0x00,0x00,0x00,0x00,0x00}; 	
	u_char zero_mac1[ETH_ALEN] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#endif /* RLVR_MULTI_SUPPORT_MT7620*/
	
	if ( portid >= RLVR_MULTI_PORT )
		return -ERANGE;

	ret  = rlvr_check_real_dev(real_dev,portid);	
	if ( ret < 0)
	{
		return ret;
	}
#ifdef  RLVR_MULTI_SUPPORT_MT7620	
	if ( portid == RLVR_MULTI_WAN_PORT )	
	{
		strcpy(name, RLVR_MULTI_WAN_NAME );
	}
	else
#endif /* RLVR_MULTI_SUPPORT_MT7620*/
	{
#ifdef  RLVR_MULTI_SUPPORT_MT7620
		if ( portid == RLVR_MULTI_SPECIAL_PORT ){
			snprintf(name,IFNAMSIZ,"%s"RLVR_MULTI_NAME_SPLIT"%u",RLVR_MULTI_LAN_NAME, RLVR_GET_DEV_ID( 1-RLVR_MULTI_MIN_OFFSET, RLVR_MULTI_PORT));
		}else 
#endif /* RLVR_MULTI_SUPPORT_MT7620*/
		{
			snprintf(name,IFNAMSIZ,"%s"RLVR_MULTI_NAME_SPLIT"%u",RLVR_MULTI_LAN_NAME, RLVR_GET_DEV_ID( portid-RLVR_MULTI_MIN_OFFSET, RLVR_MULTI_PORT));
		}
	}
	
	/*alloc the net_device*/
	new_dev = alloc_netdev_mq(sizeof(*dev_pri),name,rlvr_multi_setup,real_dev->num_tx_queues);
	
	if ( new_dev == NULL)
		return -ENOBUFS;

	new_dev->real_num_tx_queues = real_dev->real_num_tx_queues;

	dev_pri = netdev_priv(new_dev);
	dev_pri->portid = portid;
	dev_pri->real_dev = real_dev;
/*
	if ( portid == RLVR_MULTI_WAN_PORT ){
		dev_pri->flags = 0;
	}else
*/
	{	
		dev_pri->flags = RLVR_MULTI_FLAGS_REB_HEADER ;
	}
#ifdef   RLVR_MULTI_SUPPORT_MT7620
	dev_pri->flags |= RLVR_MULTI_FLAGS_USE_VLAN;
	dev_pri->h_vlan_proto = ETH_P_8021Q;
	if ( portid == RLVR_MULTI_WAN_PORT)
	{
		dev_pri->h_vlan_TCI = 2;
	} 
	else
	{
		dev_pri->h_vlan_TCI = 1;
	}
	/*set dev address */
	memset(read_mac, 0 , ETH_ALEN);
	if ( portid == RLVR_MULTI_WAN_PORT )
	{
 		ret = mt7620_mtd_read_nm("Factory", GMAC_BASE_OFFSET, ETH_ALEN, read_mac);
	}
#if 0
	else
	{
 		ret = mt7620_mtd_read_nm("Factory", GMAC0_OFFSET + ((portid-RLVR_MULTI_MIN_PORT)*GMAC0_OFFSET_STEP), ETH_ALEN, read_mac);
	}
#endif
	
	if ( (ret == 0) && 
	     ( memcmp(read_mac, zero_mac0, ETH_ALEN) != 0 ) && 
	     ( memcmp(read_mac, zero_mac1, ETH_ALEN) != 0 ) ) 
	{
		ret = rlvr_netmac_calc(read_mac, GMAC_WANMAC_STEP, ETH_ALEN-1);
		if ( ret == 0)
		{
			memcpy(new_dev->dev_addr, read_mac, ETH_ALEN);
		}
	}

#endif /*RLVR_MULTI_SUPPORT_MT7620*/

	ret = _rlvr_register_multi_dev(new_dev);
	if ( ret < 0)
		goto out_free_newdev;

	return 0;

out_free_newdev:
	free_netdev(new_dev);
	return ret;

}
static void rlvr_unregister_multi_dev(struct net_device *real_dev,unsigned int portid)
{
	struct net_device *dev = NULL;

	dev = rlvr_multi_find_dev(real_dev,portid);
	if (dev == NULL)
	{
		return ;
	}
	
	_rlvr_unregister_multi_dev(dev);
	/*unregister the device*/
	free_netdev(dev);
}
int rlvr_multi_global_init(struct net_device *real_dev)
{
	int ret = 0;
	unsigned int portid = 0;
	unsigned int currid = 0;

	for ( portid = RLVR_MULTI_MIN_PORT; portid < RLVR_MULTI_PORT; portid++)
	{
		ret = rlvr_register_multi_dev(real_dev,portid);	
		if ( ret < 0 )
		{
		printk(KERN_ERR"%s: add sub ethernet driver %u error %d\r\n",real_dev->name,portid,ret);
/*		return ret;*/
		currid = portid;
		goto err_uninit_rlvr_global;
		} 
	}	
	return 0;

 err_uninit_rlvr_global:
	for(portid = RLVR_MULTI_MIN_PORT; portid < currid; portid++)
	{
		rlvr_unregister_multi_dev(real_dev,portid);	
	} 	
	return ret;
}
void rlvr_multi_global_uinit(struct net_device *real_dev)
{
	unsigned int portid = 0;
	unsigned int currid = RLVR_MULTI_PORT;
	
	for(portid = RLVR_MULTI_MIN_PORT; portid < currid; portid++)
	{
		rlvr_unregister_multi_dev(real_dev,portid);	
	} 	
}  
