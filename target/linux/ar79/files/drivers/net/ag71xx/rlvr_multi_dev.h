/*
 *  RLINK Mulit interface built-in ethernet mac driver
 *
 *  Copyright (C) 2012-2014 RogerWu<rogerwu81@gmail.com>
 *  Copyright (C) 2012 RogerWu<rogerwu81@gmail.com>
 *
 *  Based on Real internet driver
 *
 * this is use for router lan interface,there must need multi net driver for different lan switch port,
 * but there is only one hard netdriver for CPU
 *
 * modify log:
 * version 1 :
 * 	support Atheros' AG7100 driver
 * 
 */

#ifndef __RLVR_MULTI_DEV_H
#define __RLVR_MULTI_DEV_H


#ifdef __KERNEL__
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#endif /*__KERNEL__*/

#define RLVR_MULTI_SUPPORT_AG71XX
#undef RLVR_MULTI_SUPPORT_MT7620

#define RLVR_MULTI_VERSION "0.0.6"
#define RLVR_MULTI_NAME_SPLIT "_"

#ifdef  RLVR_MULTI_SUPPORT_MT7620
/*eth interface mac define: offset from*/
#define GMAC_BASE_OFFSET		0x04

#define GMAC_LANMAC_STEP		2
#define GMAC_WANMAC_STEP		1
/*#define GMAC_EOCMAC_STEP		3	*/

#endif/* RLVR_MULTI_SUPPORT_MT7620*/

/*suport the number of lan port's driver
 * set follow the hardware number 
 * */
#define RLVR_MULTI_WAN_NAME	"eth0"
#define RLVR_MULTI_LAN_NAME	"eth1"

#ifdef RLVR_MULTI_SUPPORT_MT7620
#ifdef CONFIG_MT7620_MACH_RL_S4005EF  
#define RLVR_MULTI_MIN_PORT 1
#define RLVR_MULTI_PORT  6
#define RLVR_MULTI_WAN_PORT 5
#define RLVR_MULTI_SPECIAL_PORT 100 /*for test*/
#define RLVR_MULTI_MIN_OFFSET 1
#else
#define RLVR_MULTI_MIN_PORT 0
#define RLVR_MULTI_PORT  6
#define RLVR_MULTI_WAN_PORT 0
#define RLVR_MULTI_SPECIAL_PORT 100 /*for test*/
#define RLVR_MULTI_MIN_OFFSET 1
#endif
#else
#ifdef CONFIG_ATH79_MACH_RL_ANS5001 
#define RLVR_MULTI_MIN_PORT 0
#define RLVR_MULTI_PORT  4
#define RLVR_MULTI_MIN_OFFSET 0
#else
#define RLVR_MULTI_MIN_PORT 0
#define RLVR_MULTI_PORT  4
#define RLVR_MULTI_MIN_OFFSET 0
#endif
#endif /*RLVR_MULTI_SUPPORT_MT7620*/

/*private data of AG71XX*/
#ifdef RLVR_MULTI_SUPPORT_AG71XX 
/*atheros header of package rx/tx */
/*
 * 2bytes head length	
 * bit 15:14 version
 * bit 13:12 priority
 * bit 11:8  type
 * bit 7     from cpu method
 * bit 6     from cpu or not
 * bit 5:0   cpu number
 * */
#ifdef CONFIG_ATH79_MACH_RL_ANS5001
#define AR_HEADER_VER	0x80
#else
#define AR_HEADER_VER	0x40
#endif /**/
#define AR_HEADER_VER_MASK 	0xC0

#define AR_HEADER_PRI	0x00
#define AR_HEADER_PRI_MASK 	0x30
#define AR_HEADER_PRI_OFF	4	

/* packet type:
 * 0 : normal packet	
 * 1 : reserved
 * 2 : MIB - auto-cast MIB frame
 * 3~4: reserverd 
 * 5 : READ_WRITE_REG :read or write register frame
 *  ----------------------------------------------
 *  8 bytes: command ,low bytes first
 *  4 bytes: data ,low bytes first
 *  2 bytes: header ,high bytes first
 *  0-12 bytes: data, low bytes first
 *  34-36 bytes: Padding
 *  4 bytes: CRC
 *  ---------------------------------------------
 *  6 : READ_WRITE_REG_ACK
 *  7~15 : reserved
 */
#define AR_HEADER_TYPE	0  /*normal packet*/
#define AR_HEADER_TYPE_MASK 0x0F

/*type default value*/
#define AR_HEADER_DEF_TYPE	(AR_HEADER_VER | AR_HEADER_PRI | AR_HEADER_TYPE )
/*from cpu attribute
 * 0 base port number and switch vlan table
 * 1 base port number
 * */
#define AR_HEADER_TX_ATTR	0
#define AR_HEADER_TX_ATTR_MASK  0x80
#define AR_HEADER_TX_ATTR_OFF	7


#define AR_HEADER_FROM_CPU	0x40
#define AR_HEADER_FROM_CPU_MASK	0x40
#define AR_HEADER_FROM_CPU_OFF  6

#define AR_HEADER_TO_CPU  ~(AR_HEADER_FROM_CPU)

#define AR_HEADER_PORT_ID	0
#define AR_HEADER_PORT_ID_MASK	0x0F

#define AR_HEADER_RESERVE1	0x30
struct rlvr_multi_ar_header{
	/*header mode*/
	unsigned char port;	
	unsigned char type;
}__attribute__((packed));
#define RLVR_MULTI_HEAD_DESC_VLAN_LEN sizeof(struct rlvr_multi_ar_header) 
#define RLVR_MULTI_HEAD_DESC_LEN sizeof(struct rlvr_multi_ar_header) 
#endif /*RLVR_MULTI_SUPPORT_AG71XX*/
#ifdef  RLVR_MULTI_SUPPORT_MT7620

#define RAETH_HEADER_TYPE	0  /*normal packet*/
#define RAETH_HEADER_TYPE_MASK 0x0F

#define RAETH_HEADER_PORT_MASK	0x0F
struct rlvr_multi_ar_header{
	/*header mode*/
	unsigned char port;
	unsigned char type;	
}__attribute__((packed));
#define RLVR_MULTI_HEAD_DESC_VLAN_LEN  (sizeof(struct rlvr_multi_ar_header) + 4)
#define RLVR_MULTI_HEAD_DESC_LEN  (sizeof(struct rlvr_multi_ar_header))
#endif /*RLVR_MULTI_SUPPORT_MT7620*/
struct rlvr_multi_ethhdr{
#ifdef RLVR_MULTI_SUPPORT_AG71XX
	struct rlvr_multi_ar_header hard_hdr ;
#endif /*RLVR_MULTI_SUPPORT_AG71XX*/
#ifdef RLVR_MULTI_SUPPORT_MT7620
	struct rlvr_multi_ar_header hard_hdr ;
#endif /*RLVR_MULTI_SUPPORT_MT7620*/
	unsigned char	h_dest[ETH_ALEN];
	unsigned char	h_source[ETH_ALEN];
#ifdef RLVR_MULTI_SUPPORT_MT7620
	__be16		h_vlan_proto;
	__be16		h_vlan_TCI;		
#endif /* RLVR_MULTI_SUPPORT_MT7620*/
	__be16		next_proto;	
}__attribute__((packed));
#define RLVR_MULTI_ETHERHDR_LEN 	sizeof(struct rlvr_multi_ethhdr) 

#define RLVR_MULTI_FLAGS_REB_HEADER 0x01 /*not contact to ip stack for this interface , not set it has some bug*/
#ifdef RLVR_MULTI_SUPPORT_MT7620
#define RLVR_MULTI_FLAGS_USE_VLAN 0x02 /*insert the vlan tag information */
#endif /* RLVR_MULTI_SUPPORT_MT7620*/
/*private data of multi interface */
struct rlvr_multi_dev_info{

	unsigned int portid;
	unsigned int flags;
	__be16	     h_vlan_proto;
	__be16	     h_vlan_TCI;
	struct net_device *real_dev;		
};


/*group of the multi virtual vlan interface */
struct rlvr_multi_dev_group {
	
	struct net_device 	*real_dev;/* The ethernet(like) device 
					   * the multi device is attached to 
					   */
	unsigned int 		nr_devs; /*number of multi device*/
	struct net_device *multi_dev_arrays[RLVR_MULTI_PORT+1];
	struct rcu_head 	rcu;/*??*/
};

#ifdef    RLVR_MULTI_SUPPORT_MT7620
static inline int rlvr_netmac_calc(unsigned char *mac, u_char step, u_char offset)
{
	unsigned int addval = 0 ;
	unsigned int maxval = 0xFF;
	
	if ( offset >= ETH_ALEN )
		return -1;
		
	if ( offset == 0 )
	{
		printk(KERN_WARNING "mac [0] can't set to multicast value \n")	;
		return -1;	
	}
	addval = mac[offset] + step ;
	while ( addval > maxval ) 
	{
		mac[offset] = addval % maxval; 
		step = addval / maxval;
		offset = offset - 1;
		
		if ( offset == 0 )
		    {
			printk(KERN_WARNING "mac [0] can't set to multicast value \n")	;
			return -1;
		    }
		
		addval = mac[offset] + step ;
	}

	mac[offset] = addval ;

	return 0;
}
#endif /*   RLVR_MULTI_SUPPORT_MT7620*/
#ifdef CONFIG_ATH79_MACH_RL_ANS5001
/*将head tag 从 SA 后面放到头部，兼容其他设备代码*/
static inline void rlvr_multi_head_recv_handle(struct sk_buff *skb) {
	unsigned int len = RLVR_MULTI_HEAD_DESC_LEN; 	
	struct  rlvr_multi_ar_header hdr;

	hdr.type = skb->data[12];
	hdr.port = skb->data[13];

	memmove(skb->data + len , skb->data, 12);

	skb->data[0] = hdr.port;
	skb->data[1] = hdr.type;
}
	
/*将head tag 从 head 后面放到SA ，兼容其他设备代码*/
static inline void rlvr_multi_head_send_handle(struct sk_buff *skb) {
	unsigned int len = RLVR_MULTI_HEAD_DESC_LEN; 	
	struct  rlvr_multi_ar_header hdr;

	hdr.port = skb->data[0];
	hdr.type = skb->data[1];
	memmove(skb->data, skb->data + len , 12);

	skb->data[12] = hdr.type;
	skb->data[13] = hdr.port;
}
#else
static inline void rlvr_multi_head_recv_handle(struct sk_buff *skb) {

}
static inline void rlvr_multi_head_send_handle(struct sk_buff *skb) {
}
#endif /* CONFIG_ATH79_MACH_RL_ANS5001*/ 
/*function declare*/
void rlvr_multi_link_adjust(struct net_device *real_dev,unsigned int portid,unsigned int status);
#ifdef   RLVR_MULTI_SUPPORT_MT7620
int mt7620_mtd_write_nm(char *name, loff_t to, size_t len, const u_char *buf);
int mt7620_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf);
struct sk_buff * rlvr_multi_parse_hdr_byport( struct sk_buff *skb,
			   	 struct rlvr_multi_ar_header *arheader);
struct sk_buff *rlvr_multi_put_tag_byport(struct sk_buff *skb,unsigned char portid) ;
#endif /*  RLVR_MULTI_SUPPORT_MT7620*/
int rlvr_multi_skb_recv( struct sk_buff *skb,struct net_device *dev);
int rlvr_multi_global_init(struct net_device *real_dev);
void rlvr_multi_global_uinit(struct net_device *real_dev);
#endif /*__RLVR_MULTI_DEV_H*/
