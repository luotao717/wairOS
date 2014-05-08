#include "ramips_esw.h"

static u8  raeth_esw_lan_portmaps = 0;
static u8  raeth_esw_wan_portmaps = 0;
 
extern void __iomem  *fe_mac_base ;

static void
mt7620_esw_rmw(struct mt7620_esw *esw, unsigned reg, unsigned long mask,
	       unsigned long val)
{
	unsigned long flags;

/*	printk("%s %d reg : 0x%x  mask: %x  data :0x%x \r\n",__FILE__,__LINE__,reg,mask,val);*/
	spin_lock_irqsave(&esw->reg_rw_lock, flags);
	mt7620_esw_rmw_raw(esw, reg, mask, val);
	spin_unlock_irqrestore(&esw->reg_rw_lock, flags);

}

static u32
mt7620_mii_read(struct mt7620_esw *esw, u32 phy_addr, u32 phy_register)
{
	u32 data = 0,read_data = 0; 
	unsigned long t_start = 0;
	int ret = 0;

/* printk("%s %d phy_addr: 0x%x  reg : 0x%x  data :0x%x \r\n",__FILE__,__LINE__,phy_addr,phy_register,*read_data);*/
	t_start = jiffies + MT7620_ESW_PHY_TIMEOUT ;	
	while (1) {
		if (!( mt7620_esw_rr(esw, REG_ESW_MDIO_PIAC ) &
		      PIAC_PHY_ACS_ST ))
			break;

		if (time_after(jiffies, t_start )) {
			ret = 1;
			goto out;
		}
	}

	data  =	(phy_register << PIAC_MDIO_REG_ADDR_SHIFT) |
	      (phy_addr << PIAC_MDIO_PHY_ADDR_SHIFT) | 
	      (0x02 << PIAC_MDIO_CMD_SHIFT ) | /*read*/
	      (0x01 << PIAC_MDIO_ST_SHIFT ); 	

	mt7620_esw_wr(esw, data, REG_ESW_MDIO_PIAC);

	data |= PIAC_PHY_ACS_ST;	
	mt7620_esw_wr(esw, data, REG_ESW_MDIO_PIAC);

	t_start = jiffies + MT7620_ESW_PHY_TIMEOUT;
	while (1) {
		if (!(( read_data = mt7620_esw_rr(esw, REG_ESW_MDIO_PIAC )) &
		    PIAC_PHY_ACS_ST ) )
			{
			read_data &= PIAC_MDIO_RW_DATA_MASK;
			break;
			}

		if (time_after(jiffies, t_start )) {
			ret = 1;
			break;
		}
	}
out:
	if (ret)
		printk(KERN_ERR "ramips_eth: MDIO timeout\n");

	return read_data;
}
static u32
mt7620_mii_write(struct mt7620_esw *esw, u32 phy_addr, u32 phy_register,
		 u32 write_data)
{
	u32 data = 0; 
	unsigned long t_start = 0;
	int ret = 0;

/* printk("%s %d phy_addr: 0x%x  reg : 0x%x  data :0x%x \r\n",__FILE__,__LINE__,phy_addr,phy_register,write_data);*/
	t_start = jiffies + MT7620_ESW_PHY_TIMEOUT ;	
	while (1) {
		if (!( mt7620_esw_rr(esw, REG_ESW_MDIO_PIAC ) &
		      PIAC_PHY_ACS_ST ))
			break;

		if (time_after(jiffies, t_start )) {
			ret = 1;
			goto out;
		}
	}

	write_data &= PIAC_MDIO_RW_DATA_MASK;
	data  =	(write_data << PIAC_MDIO_RW_DATA_SHIFT) |
	      (phy_register << PIAC_MDIO_REG_ADDR_SHIFT) |
	      (phy_addr << PIAC_MDIO_PHY_ADDR_SHIFT) | 
	      (0x01 << PIAC_MDIO_CMD_SHIFT ) | /*write */
	      (0x01 << PIAC_MDIO_ST_SHIFT ); 	

	mt7620_esw_wr(esw, data, REG_ESW_MDIO_PIAC);

	data |= PIAC_PHY_ACS_ST;	
	mt7620_esw_wr(esw, data, REG_ESW_MDIO_PIAC);

	t_start = jiffies + MT7620_ESW_PHY_TIMEOUT;
	while (1) {
		if (!( mt7620_esw_rr(esw, REG_ESW_MDIO_PIAC ) &
		    PIAC_PHY_ACS_ST ))
			break;

		if (time_after(jiffies, t_start )) {
			ret = 1;
			break;
		}
	}
out:
	if (ret)
		printk(KERN_ERR "ramips_eth: MDIO timeout\n");

	return ret;
}

static unsigned
mt7620_esw_get_vlan_id(struct mt7620_esw *esw, unsigned vlan)
{
	unsigned s;
	unsigned val;

	s = VLAN_ID_SHIFT(vlan);

	val = mt7620_esw_rr(esw, REG_ESW_VLAN_ID(vlan));
	val = (val >> s) & VLAN_ID_MASK;

	return val;
}

static void
mt7620_esw_set_vlan_id(struct mt7620_esw *esw, unsigned vlan, unsigned vid)
{
	unsigned s;

	s = VLAN_ID_SHIFT(vlan);

	mt7620_esw_rmw(esw,
		       REG_ESW_VLAN_ID(vlan), 
		       VLAN_ID_MASK << s,
		       ( vid & VLAN_ID_MASK ) << s);
}

static unsigned
mt7620_esw_get_pvid(struct mt7620_esw *esw, unsigned port)
{
	unsigned val,s;

	s = PPBV_G0_PORT_VID_SHIFT;
	val = mt7620_esw_rr(esw, REG_ESW_PN_PPBV1(port) );
	return ( val >> s ) & PPBV_GN_PORT_VID_MASK;
}

static void
mt7620_esw_set_pvid(struct mt7620_esw *esw, unsigned port, unsigned pvid)
{
	unsigned s;

/*	printk(KERN_INFO "pvid set port %d pvid: %d \n", port, pvid );*/
	s = PPBV_G0_PORT_VID_SHIFT ;
	mt7620_esw_rmw(esw,
		       REG_ESW_PN_PPBV1(port),
		       PPBV_GN_PORT_VID_MASK << s,
		       ( pvid & PPBV_GN_PORT_VID_MASK ) << s);
}

static int mt7620_esw_get_table(struct mt7620_esw *esw , 
				struct mt7620_esw_tbl_ctrl *tbl_ctrl,u32 *data1,u32 *data2 )
{
	int ret = 0;
	u32 r_data = 0;
	u32 w_data = 0;
	unsigned long is_start = 0;

	*data1 = 0;
	*data2 = 0;

	/*controll reg write*/
	w_data = ( tbl_ctrl->acc_ctrl & TBL_CTRL_FUNC_MASK ) << TBL_CTRL_FUNC_SHIFT;
	w_data |=( tbl_ctrl->index & TBL_CTRL_IDX_MASK ) << TBL_CTRL_IDX_SHIFT;
	/*set start flag*/
	w_data |= TBL_CTRL_BUSY;

	mt7620_esw_wr ( esw, w_data, REG_ESW_VLAN_VTCR ) ;	

	/*check the write*/
	is_start = jiffies + MT7620_ESW_ATS_TIMEOUT;
	while(1) {
		r_data = mt7620_esw_rr( esw, REG_ESW_VLAN_VTCR ) ;
		if (!( r_data & TBL_CTRL_BUSY ))
		    {
			break;
		    }
		if ( time_after( jiffies , is_start ))
		  {
			ret =  -1;
			break; 	
		  }
		}
	
	if ( ret == 0 )
	{
		*data1 = mt7620_esw_rr(esw,REG_ESW_VLAN_VAWD1);	
		*data2 = mt7620_esw_rr(esw,REG_ESW_VLAN_VAWD2);	
	}

/*	printk(KERN_INFO "get table:w_data: %X  %X  %X ret:%d\n",w_data,*data1,*data2,ret);*/

	return ret;
}
static int mt7620_esw_set_table(struct mt7620_esw *esw , 
				struct mt7620_esw_tbl_ctrl *tbl_ctrl,u32 *data1,u32 *data2 )
{
	int ret = 0 ;
	u32 w_data = 0;
	u32 is_busy = 0;
	unsigned long is_start = 0;

	/*write the data1 */	
	if ( NULL != data1 )
	{
		mt7620_esw_wr(esw,*data1,REG_ESW_VLAN_VAWD1);	
	}
	/*write the data2 */	
	if ( NULL != data2 )
	{
		mt7620_esw_wr(esw,*data2,REG_ESW_VLAN_VAWD2);
	}
	/*controll reg write*/
	w_data = ( tbl_ctrl->acc_ctrl & TBL_CTRL_FUNC_MASK ) << TBL_CTRL_FUNC_SHIFT;
	w_data |=( tbl_ctrl->index & TBL_CTRL_IDX_MASK ) << TBL_CTRL_IDX_SHIFT;
	/*set start flag*/
	w_data |= TBL_CTRL_BUSY;

	mt7620_esw_wr ( esw, w_data, REG_ESW_VLAN_VTCR ) ;	

	/*check the write*/
	is_start = jiffies + MT7620_ESW_ATS_TIMEOUT;
	while(1) {
		is_busy = mt7620_esw_rr( esw, REG_ESW_VLAN_VTCR ) & TBL_CTRL_BUSY;
		if (!is_busy )
		    {
			break;
		    }
		if ( time_after( jiffies , is_start  ))
		  {
			ret =  -1;
			break; 	
		  }
		}

	if ( ret != 0 )
	{
		printk(KERN_ERR "ramips_eth: access the hardware vlan&acl table timeout\n");
	}

/*	printk(KERN_INFO "set table:wdata: %X %X  %X ret:%d\n",w_data,*data1,*data2,ret);*/
	return ret ;	
}

static int mt7620_esw_get_vlan_tbl(struct mt7620_esw *esw ,
				   unsigned vlan,struct mt7620_esw_vlan_attr *vlan_attr)
{
	int ret = 0;
	int port = 0;
	u32	w_data1 = 0, w_data2 = 0 ;
	struct mt7620_esw_tbl_ctrl tbl_ctrl; 
		
	memset(vlan_attr,0, sizeof(*vlan_attr));
	/*VLAN Table Access read*/
	/*vlan read */
	memset(&tbl_ctrl, 0 ,sizeof(struct mt7620_esw_tbl_ctrl));

	tbl_ctrl.acc_ctrl = TBL_FUNC_VLAN_READ;	 
	tbl_ctrl.index = vlan;
	ret = mt7620_esw_get_table(esw,&tbl_ctrl,&w_data1,&w_data2);
	if ( ret != 0 )
	{
		printk(KERN_ERR "invalid entry %d read \r\n",vlan);
		return ret;
	}
	
	if ( !(w_data1 & VLAN_WD1_VALID) )
	{
		printk(KERN_ERR "invalid entry %d \r\n",vlan);
		return ret;
	}

	/*VLAN table data register 1 field*/		
	if ( w_data1 & VLAN_WD1_VALID ) 
	{
	    vlan_attr->ivl_mac = 1; /*enable*/
	}

	/*VTAG Information*/
	if ( w_data1 & VLAN_WD1_VTAG_EN ) 
	{
	    if ( w_data1 & VLAN_WD1_EG_CON ) 
		{
		vlan_attr->vtag_mode = VLAN_TAG_EGRESS_EG_CON; 
		}	
	    else{
		vlan_attr->vtag_mode = VLAN_TAG_EGRESS_TAG_CONTROL;
		for( port = 0 ;port < MT7620_ESW_NUM_PORTS; port++ )
		    {
			vlan_attr->egress_tag[port] = ( w_data2 >> VLAN_WD2_PTAG_SHIFT(port) ) & VLAN_WD2_PTAG_MASK; 
		    }
		}
	} 

	/* priority value get */
	if ( w_data1 & VLAN_COPY_PRI )
	{
	    vlan_attr->pri_mode |= VLAN_COPY_PRI;
	}
	
	vlan_attr->usr_pri = ( w_data1 >> VLAN_WD1_USER_PRI_SHIFT ) & VLAN_WD1_USER_PRI_MASK;

	vlan_attr->s_tag = ( w_data1 >> VLAN_WD1_STAGI_SHIFT ) & VLAN_WD1_STAGI_MASK ;	

	vlan_attr->port_mem =  ( w_data1 >> VLAN_WD1_PORTM_SHIFT ) & VLAN_WD1_PORTM_MASK;	

	vlan_attr->valid = 1;

	return ret;
} 
static int  
mt7620_esw_get_vmsc(struct mt7620_esw *esw, unsigned vlan,struct mt7620_esw_vlan_attr *vlan_attr)
{
	int ret = 0 ;
/*	struct mt7620_esw_vlan_attr vlan_attr;*/

	ret =  mt7620_esw_get_vlan_tbl(esw,vlan,vlan_attr);	

	return ret ;
}

static int mt7620_esw_set_vlan_tbl(struct mt7620_esw *esw ,
				   unsigned vlan,struct mt7620_esw_vlan_attr *vlan_attr)
{
	int ret = 0;
	int port = 0 ;
	u32	w_data1 = 0, w_data2 = 0 ;
	struct mt7620_esw_tbl_ctrl tbl_ctrl; 
		
	/*vlan write*/
	memset(&tbl_ctrl, 0 ,sizeof(struct mt7620_esw_tbl_ctrl));
	tbl_ctrl.index = vlan;

	/*VLAN Table Access */
	if ( vlan_attr->valid ) /*add or update the vlan table entry*/
	{
	    tbl_ctrl.acc_ctrl = TBL_FUNC_VLAN_WRITE ;	 
	    /*VLAN table data register 1 field*/		
	    if ( vlan_attr->ivl_mac )
		  {
			w_data1 |= VLAN_WD1_IVL_MAC;  
		  }	
	/*	printk(KERN_INFO "vlan set %d  data: 0x%08x \n",vlan,w_data1);*/
		if ( vlan_attr->vtag_mode == VLAN_TAG_EGRESS_EG_CON )
		{
		    w_data1 |= ( VLAN_WD1_VTAG_EN | VLAN_WD1_EG_CON );  
		}
		else if ( vlan_attr->vtag_mode == VLAN_TAG_EGRESS_TAG_CONTROL)
		{
		    w_data1 |= VLAN_WD1_VTAG_EN;
	
		    /*VLAN table data register 2 field*/	
		    for ( port = 0 ;port < MT7620_ESW_NUM_PORTS; port++ ) /*only port 0- port 6*/ 
		    {
		    	w_data2 |= (vlan_attr->egress_tag[port] << VLAN_WD2_PTAG_SHIFT(port));
		    }
		}
	
		/* priority value set */
		w_data1 |= ( vlan_attr->usr_pri & VLAN_WD1_USER_PRI_MASK ) << VLAN_WD1_USER_PRI_SHIFT;
		if ( vlan_attr->pri_mode & VLAN_COPY_PRI )
		{
		    w_data1 |= VLAN_WD1_COPY_PRI ;	
		}
		
		w_data1 |= (vlan_attr->s_tag & VLAN_WD1_STAGI_MASK) << VLAN_WD1_STAGI_SHIFT;
	
		w_data1 |= (vlan_attr->port_mem & VLAN_WD1_PORTM_MASK) << VLAN_WD1_PORTM_SHIFT;	
		w_data1 |= VLAN_WD1_VALID;
	}
	else { /*invalid the vlan table entry*/
	    tbl_ctrl.acc_ctrl = TBL_FUNC_VLAN_INVALID ;	 
	}
	ret = mt7620_esw_set_table(esw,&tbl_ctrl,&w_data1,&w_data2);

	return ret;
} 
static void
mt7620_esw_set_vmsc(struct mt7620_esw *esw, unsigned vlan, unsigned vmsc, unsigned vm_untags)
{
/*	unsigned s;*/
	unsigned i;
	struct mt7620_esw_vlan_attr vlan_attr;

	if ( vlan > TBL_VLAN_IDX_MAX )
	{
		printk ( KERN_ERR "ramip_eth: write vlan table, the vlan idex %d is out of rang:%d \r\n",
					vlan,TBL_VLAN_IDX_MAX);
		return ;
	} 

	memset(&vlan_attr, 0, sizeof(struct mt7620_esw_vlan_attr) );
	vlan_attr.port_mem = vmsc;
	if ( vlan_attr.port_mem != 0 )
	{
		vlan_attr.valid = 1;
	}
	/*untags set */
	vlan_attr.vtag_mode = VLAN_TAG_EGRESS_TAG_CONTROL;
	/*IVL vlan set learn by vlan */
	vlan_attr.ivl_mac = 1;

	for (i = 0; i < MT7620_ESW_NUM_PORTS; i++) {
		if ( BIT(i) &  vm_untags ) 
		{
			vlan_attr.egress_tag[i] = VLAN_TAG_EGRESS_UNTAG ;
		}			
		else
		{
			vlan_attr.egress_tag[i] = VLAN_TAG_EGRESS_TAG ;
		}
	
	}
	mt7620_esw_set_vlan_tbl( esw, vlan, &vlan_attr );
}

static unsigned
mt7620_esw_get_port_disable(struct mt7620_esw *esw)
{
	return 0;
}
static int mt7620_esw_get_l2l3_table(struct mt7620_esw *esw,
				struct mt7620_esw_arltbl_ctrl *ctrl, 
				u32 *data1, u32 *data2, u32 *data_fwd)
{
	int ret = 0 ;
	u32 w_data = 0;
	unsigned long is_start = 0;
	u32 is_busy = 0;
#if 0	
	if (data1 != NULL )
	{
	   mt7620_esw_wr(esw, *data1, REG_ESW_WT_MAC_ATA1);	
	}
	if (data2 != NULL )
	{
	   mt7620_esw_wr(esw, *data2, REG_ESW_WT_MAC_ATA2);	
	}
	if (data_fwd != NULL )
	{
	   mt7620_esw_wr(esw, *data_fwd, REG_ESW_WT_MAC_ATWD);	
	}
#endif

	if ( ctrl->acc_attr & TBL_ARL_IS_MULTI )
	{
		w_data |= ( ctrl->acc_attr & TBL_ARL_ATTR_MASK ) << ATC_AC_MAT_SHIFT; 
	}
	w_data |= ( ctrl->acc_cmd  << ATC_AC_CMD_SHIFT ) ;

	mt7620_esw_wr(esw, w_data, REG_ESW_WT_MAC_ATC );	

	w_data |=  ATC_BUSY;
	mt7620_esw_wr(esw, w_data, REG_ESW_WT_MAC_ATC );	

	/*check the write*/
	is_start = jiffies + MT7620_ESW_ATS_TIMEOUT;
	while(1) {
		is_busy = mt7620_esw_rr( esw, REG_ESW_WT_MAC_ATC) ;
		if (!(is_busy & ATC_BUSY) )
		    {
			if( is_busy & ATC_SRCH_END ){			 
				printk(KERN_INFO "ramips_eth: access the hardware l2/l3 table search end\n");
				ret = -1;
				break;
			}

			if ( is_busy & ATC_SRCH_HIT ){
				break;
				}
		    }

		if ( time_after( jiffies , is_start ))
		  {
			ret =  -1;
			break; 	
		  }
		}

	if ( ret != 0 )
	{
		printk(KERN_ERR "ramips_eth: access the hardware l2/l3 table timeout\n");
	}
	else{
		*data1 = mt7620_esw_rr(esw, REG_ESW_TABLE_TSRA1);
		*data2 = mt7620_esw_rr(esw, REG_ESW_TABLE_TSRA2);
		*data_fwd = mt7620_esw_rr(esw, REG_ESW_TABLE_ATRD);
	}

	printk(KERN_INFO "first search data: %08X - %08X - %08X  ret :%d\n",
				*data1, *data2, *data_fwd, ret);	
	return ret ;	
}
static int mt7620_esw_set_l2l3_table(struct mt7620_esw *esw,
				struct mt7620_esw_arltbl_ctrl *ctrl, 
				u32 *data1, u32 *data2, u32 *data_fwd)
{
	int ret = 0 ;
	u32 w_data = 0;
	unsigned long is_start = 0;
	u32 is_busy = 0;
	
	if (data1 != NULL )
	{
	   mt7620_esw_wr(esw, *data1, REG_ESW_WT_MAC_ATA1);	
	}
	if (data2 != NULL )
	{
	   mt7620_esw_wr(esw, *data2, REG_ESW_WT_MAC_ATA2);	
	}
	if (data_fwd != NULL )
	{
	   mt7620_esw_wr(esw, *data_fwd, REG_ESW_WT_MAC_ATWD);	
	}

	if ( ctrl->acc_attr & TBL_ARL_IS_MULTI )
	{
		w_data |= ( ctrl->acc_attr & TBL_ARL_ATTR_MASK ) << ATC_AC_MAT_SHIFT; 
	}
	w_data |= ( ctrl->acc_cmd  << ATC_AC_CMD_SHIFT ) ;

	mt7620_esw_wr(esw, w_data, REG_ESW_WT_MAC_ATC );	

	w_data |=  ATC_BUSY;
	mt7620_esw_wr(esw, w_data, REG_ESW_WT_MAC_ATC );	

	/*check the write*/
	is_start = jiffies + MT7620_ESW_ATS_TIMEOUT;
	while(1) {
		is_busy = mt7620_esw_rr( esw, REG_ESW_WT_MAC_ATC ) & ATC_BUSY ;
		if (!is_busy )
		    {
			break;
		    }
		if ( time_after( jiffies , is_start ))
		  {
			ret =  -1;
			break; 	
		  }
		}

	if ( ret != 0 )
	{
		printk(KERN_ERR "ramips_eth: access the hardware l2/l3 table timeout\n");
	}
	return ret ;	
}
#if 0
static int mt7620_esw_set_arl_table(struct mt7620_esw *esw, struct mt7620_esw_arltbl_data *data )
{
	int ret = 0;
	u32 data1= 0 , data2= 0, data_fwd = 0;	
	struct mt7620_esw_arltbl_ctrl ctrl_val ;
	
	memset(&ctrl_val, 0, sizeof(ctrl_val));	
	
	/*field data1*/
	/*field data2*/
	/*field data_fwd*/

	/**/
	ret =  mt7620_esw_set_l2l3_table(esw, &ctrl_val, &data1, &data2, &data_fwd);	
}
static int mt7620_esw_get_arl_table(struct mt7620_esw *esw,)
{
}
#endif

static int __mt7620_esw_search_arl_entry(struct mt7620_esw *esw, int acc_cmd, struct mt7620_esw_arltbl_data *ptr )
{
	int ret = 0;
	u32 data1, data2, data_fwd;
	struct mt7620_esw_arltbl_ctrl ctrl_val ;
	
	memset(&ctrl_val, 0, sizeof(ctrl_val));	

	ctrl_val.acc_attr =  TBL_ARL_IS_MULTI;
	ctrl_val.acc_attr |=  TBL_ARL_ATTR_M_ALL_MAC;

	ctrl_val.acc_cmd = acc_cmd;

	ret = mt7620_esw_get_l2l3_table(esw,&ctrl_val, &data1, &data2, &data_fwd);	
	if (ret == 0 )
	{
		memset(ptr, 0, sizeof(*ptr));

		ptr->mac[0] = (data1 >> 24) & 0xFF;
		ptr->mac[1] = (data1 >> 16) & 0xFF;
		ptr->mac[2] = (data1 >> 8) & 0xFF;
		ptr->mac[3] = (data1 >> 0) & 0xFF;
		ptr->mac[4] = (data2 >> 24) & 0xFF;
		ptr->mac[5] = (data2 >> 16) & 0xFF;

	/*	memcpy(ptr->mac, (u8*)&data1, 4);	
		memcpy(&ptr->mac[4],(u8*)&data2,2);*/
		ptr->cvid = ( data2 ) & (0xFFF);
		ptr->valid = !!(data_fwd & 0xc);
		ptr->is_static = !!(data_fwd & BIT(3)); 
		ptr->portmap = ( data_fwd >> 4) & 0xFF;

		printk(KERN_INFO "cmd: %d mac info: "
				"mac address: %02X:%02X:%02X:%02X:%02X:%02X "
				"vlanID : 0x%x portmap: 0x%x "
				"static : %d  valid: %d \n",
				acc_cmd,
				ptr->mac[0], ptr->mac[1], ptr->mac[2], ptr->mac[3], ptr->mac[4], ptr->mac[5],
				ptr->cvid, ptr->portmap, 
				ptr->is_static, ptr->valid);	
	}
	
	return ret;
}
static int mt7620_esw_search_arl_first_entry(struct mt7620_esw *esw, struct mt7620_esw_arltbl_data *ptr )
{
	int ret = 0;
	int acc_attr = TBL_ARL_CMD_SCH;

	ret =  __mt7620_esw_search_arl_entry(esw, acc_attr, ptr);

	return ret;
}
static int mt7620_esw_search_arl_next_entry(struct mt7620_esw *esw, struct mt7620_esw_arltbl_data *ptr )
{
	int ret = 0;
	int acc_attr = TBL_ARL_CMD_SCH_NXT;

	ret =  __mt7620_esw_search_arl_entry(esw, acc_attr, ptr);

	return ret;
}
static int mt7620_esw_clear_arl_table(struct mt7620_esw *esw,u8 clear_cmd)
{
	struct mt7620_esw_arltbl_ctrl ctrl_val ;
	
	memset(&ctrl_val, 0, sizeof(ctrl_val));	

	ctrl_val.acc_attr =  TBL_ARL_IS_MULTI;
	ctrl_val.acc_attr |= clear_cmd & TBL_ARL_ATTR_MASK;

	ctrl_val.acc_cmd = TBL_ARL_CMD_CLEAN;

	mt7620_esw_set_l2l3_table(esw,&ctrl_val, NULL, NULL, NULL );	
	return 0;
}
static void
mt7620_esw_set_port_disable(struct mt7620_esw *esw, unsigned disable_mask)
{
#if 0
	unsigned old_mask;
	unsigned enable_mask;
	unsigned changed;
	int i;

	old_mask = mt7620_esw_get_port_disable(esw);
	changed = old_mask ^ disable_mask;
	enable_mask = old_mask & disable_mask;

	/* enable before writing to MII */
	mt7620_esw_rmw(esw, MT7620_ESW_REG_POC0,
		       (MT7620_ESW_POC0_DIS_PORT_M <<
			MT7620_ESW_POC0_DIS_PORT_S),
		       enable_mask << MT7620_ESW_POC0_DIS_PORT_S);

	for (i = 0; i < MT7620_ESW_NUM_LEDS; i++) {
		if (!(changed & (1 << i)))
			continue;
		if (disable_mask & (1 << i)) {
			/* disable */
			mt7620_mii_write(esw, i, MII_BMCR,
					 BMCR_PDOWN);
		} else {
			/* enable */
			mt7620_mii_write(esw, i, MII_BMCR,
					 BMCR_FULLDPLX |
					 BMCR_ANENABLE |
					 BMCR_ANRESTART |
					 BMCR_SPEED100);
		}
	}

	/* disable after writing to MII */
	mt7620_esw_rmw(esw, MT7620_ESW_REG_POC0,
		       (MT7620_ESW_POC0_DIS_PORT_M <<
			MT7620_ESW_POC0_DIS_PORT_S),
		       disable_mask << MT7620_ESW_POC0_DIS_PORT_S);
#endif
}

static int
mt7620_esw_apply_config(struct switch_dev *dev);

static void mt7620_esw_phy_hw_init(struct mt7620_esw *esw)
{
	u8 is_bga = esw->pdata->is_bga;

	/* is BGA board */
#if 0	
    /*
    * Reg 31: Page Control
    * Bit 15     => PortPageSel, 1=local, 0=global
    * Bit 14:12  => PageSel, local:0~3, global:0~4
    *
    * Reg16~30:Local/Global registers
    *
    */
    /*correct  PHY  setting L3.0 BGA*/
    mt7620_mii_write(esw,1, 31, 0x4000); //global, page 4
  
    mt7620_mii_write(esw,1, 17, 0x7444);
    if(is_bga){
	mt7620_mii_write(esw,1, 19, 0x0114);
    }else{
	mt7620_mii_write(esw,1, 19, 0x0117);
    }

    mt7620_mii_write(esw,1, 22, 0x10cf);
    mt7620_mii_write(esw,1, 25, 0x6212);
    mt7620_mii_write(esw,1, 26, 0x0777);
    mt7620_mii_write(esw,1, 29, 0x4000);
    mt7620_mii_write(esw,1, 28, 0xc077);
    mt7620_mii_write(esw,1, 24, 0x0000);
    
    mt7620_mii_write(esw,1, 31, 0x3000); //global, page 3
    mt7620_mii_write(esw,1, 17, 0x4838);

    mt7620_mii_write(esw,1, 31, 0x2000); //global, page 2
    if(is_bga){
	mt7620_mii_write(esw,1, 21, 0x0515);
	mt7620_mii_write(esw,1, 22, 0x0053);
	mt7620_mii_write(esw,1, 23, 0x00bf);
	mt7620_mii_write(esw,1, 24, 0x0aaf);
	mt7620_mii_write(esw,1, 25, 0x0fad);
	mt7620_mii_write(esw,1, 26, 0x0fc1);
    }else{
	mt7620_mii_write(esw,1, 21, 0x0517);
	mt7620_mii_write(esw,1, 22, 0x0fd2);
	mt7620_mii_write(esw,1, 23, 0x00bf);
	mt7620_mii_write(esw,1, 24, 0x0aab);
	mt7620_mii_write(esw,1, 25, 0x00ae);
	mt7620_mii_write(esw,1, 26, 0x0fff);
    }
    mt7620_mii_write(esw,1, 31, 0x1000); //global, page 1
    mt7620_mii_write(esw,1, 17, 0xe7f8);
    
    mt7620_mii_write(esw,1, 31, 0x8000); //local, page 0
    mt7620_mii_write(esw,0, 30, 0xa000);
    mt7620_mii_write(esw,1, 30, 0xa000);
    mt7620_mii_write(esw,2, 30, 0xa000);
    mt7620_mii_write(esw,3, 30, 0xa000);
#if !defined (CONFIG_RAETH_HAS_PORT4)   
    mt7620_mii_write(esw,4, 30, 0xa000);
#endif

    mt7620_mii_write(esw,0, 4, 0x05e1);
    mt7620_mii_write(esw,1, 4, 0x05e1);
    mt7620_mii_write(esw,2, 4, 0x05e1);
    mt7620_mii_write(esw,3, 4, 0x05e1);
#if !defined (CONFIG_RAETH_HAS_PORT4)   
    mt7620_mii_write(esw,4, 4, 0x05e1);
#endif

    mt7620_mii_write(esw,1, 31, 0xa000); //local, page 2
    mt7620_mii_write(esw,0, 16, 0x1111);
    mt7620_mii_write(esw,1, 16, 0x1010);
    mt7620_mii_write(esw,2, 16, 0x1515);
    mt7620_mii_write(esw,3, 16, 0x0f0f);
#if !defined (CONFIG_RAETH_HAS_PORT4)   
    mt7620_mii_write(esw,4, 16, 0x1313);
#endif
	
#endif
}

static void
mt7620_esw_hw_init(struct mt7620_esw *esw)
{
	int i;
	u8 port_disable = 0;
	u32 value = 0 ,val_mask = 0;
	u32 w_data = 0;
	struct mt7620_esw_platform_data *pdata = esw->pdata;

	raeth_esw_lan_portmaps =  MT7620_ESW_PMAP_LLLLLLL;
	/*reset the switch */	
	pdata->reset_esw(MT7620_ESW_RESET_SWITCH);
	
	/*set the phy base_address*/
	value = 13 << GPC1_PHY_BASE_SHIFT;
	val_mask = GPC1_PHY_BASE_MASK << GPC1_PHY_BASE_SHIFT;
	mt7620_esw_rmw(esw, REG_ESW_MDIO_GPC1, val_mask, value);   	
	
	pdata->reset_esw ( MT7620_ESW_RESET_EPHY );

	/*init the phy register */
	mt7620_esw_phy_hw_init(esw);	

	/* CPU Port 6 Force Link 1G ,FC ON */
#if 0
	value  = REG_ESW_FORCE_LINK | REG_ESW_FORCE_DPX | 
		 (0x02 << REG_ESW_FORCE_SPD_SHIFT) | /*1000Mbps*/
		 REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
		 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		 REG_ESW_FORCE_MODE | 
		 (0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/
#endif
	value = 0x5e33b;
	mt7620_esw_wr( esw, value, REG_ESW_MN_PMCR(esw->swdev.cpu_port) );

	/* Set Port6 CPU Port */
	val_mask = ( REG_ESW_CPU_PORT_MASK << REG_ESW_CPU_PORT_SHIFT ) | REG_ESW_CPU_EN ;
	value  = (esw->swdev.cpu_port << REG_ESW_CPU_PORT_SHIFT) | REG_ESW_CPU_EN ;
	mt7620_esw_rmw(esw,REG_ESW_WT_MAC_MFC,val_mask, value);

	/* Port 5 */
	if ( esw->pdata->p5_phy_mode == MT7620_PM_RGMII_TO_MAC )
	{
		value  = REG_ESW_FORCE_LINK | REG_ESW_FORCE_DPX | 
			 (0x02 << REG_ESW_FORCE_SPD_SHIFT) | /*1000Mbps*/
			 REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 	 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
			 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		  	 REG_ESW_FORCE_MODE | 
		 	(0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/
		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(5) );
			
	}
	else if ( esw->pdata->p5_phy_mode == MT7620_PM_MII_TO_MAC )
	{
		value  = REG_ESW_FORCE_LINK | REG_ESW_FORCE_DPX | 
			 (0x01 << REG_ESW_FORCE_SPD_SHIFT) | /*100Mbps*/
			 REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 	 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
			 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		  	 REG_ESW_FORCE_MODE | 
		 	(0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/
		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(5) );
			
	}
	else if ( esw->pdata->p5_phy_mode == MT7620_PM_RMII_TO_MAC )
	{
		value  = REG_ESW_FORCE_LINK | REG_ESW_FORCE_DPX | 
			 (0x01 << REG_ESW_FORCE_SPD_SHIFT) | /*100Mbps*/
			 REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 	 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
			 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		  	 REG_ESW_FORCE_MODE | 
		 	(0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/
		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(5) );
	}
	else if ( esw->pdata->p5_phy_mode == MT7620_PM_MAC_TO_PHY )
	{
		/* support now */	
		printk(KERN_ERR "port 5 to phy is not change \r\n");
		value  = REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 	 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
			 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		 	(0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/

		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(5) );
	}
	else /*MT7620_PM_DISABLE*/
	{
		value  = REG_ESW_FORCE_MODE ; 
		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(5) );
	}

	/* Port 4 */
	if ( esw->pdata->p4_phy_mode == MT7620_PM_RGMII_TO_MAC )
	{
		value  = REG_ESW_FORCE_LINK | REG_ESW_FORCE_DPX | 
			 (0x02 << REG_ESW_FORCE_SPD_SHIFT) | /*1000Mbps*/
			 REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 	 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
			 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		  	 REG_ESW_FORCE_MODE | 
		 	(0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/
		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(4) );
			
	}
	else if ( esw->pdata->p4_phy_mode == MT7620_PM_MII_TO_MAC )
	{
		value  = REG_ESW_FORCE_LINK | REG_ESW_FORCE_DPX | 
			 (0x01 << REG_ESW_FORCE_SPD_SHIFT) | /*100Mbps*/
			 REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 	 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
			 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		  	 REG_ESW_FORCE_MODE | 
		 	(0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/
		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(4) );
			
	}
	else if ( esw->pdata->p4_phy_mode == MT7620_PM_RMII_TO_MAC )
	{
		value  = REG_ESW_FORCE_LINK | REG_ESW_FORCE_DPX | 
			 (0x01 << REG_ESW_FORCE_SPD_SHIFT) | /*100Mbps*/
			 REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 	 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
			 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		  	 REG_ESW_FORCE_MODE | 
		 	(0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/
		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(4) );
	}
	else if ( esw->pdata->p4_phy_mode == MT7620_PM_MAC_TO_PHY )
	{
		/* support now */	
		printk(KERN_ERR "port 4 to phy is not change \r\n");
		value  = REG_ESW_FORCE_TX_FC |  REG_ESW_FORCE_RX_FC |
		 	 REG_ESW_BACKPR_EN | REG_ESW_BKOFF_EN |
			 REG_ESW_MAC_RX_EN | REG_ESW_MAC_TX_EN |
		 	(0x01 << REG_ESW_IPG_CFG_SHIFT); /*Treansmit 96-bit IFG*/

		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(4) );
	}
	else /*MT7620_PM_DISABLE*/
	{
		value  = REG_ESW_FORCE_MODE ; 
		mt7620_esw_wr( esw,value,REG_ESW_MN_PMCR(4) );
	}
#if 0
	/* vodoo from original driver */
	mt7620_esw_wr(esw, 0xC8A07850, MT7620_ESW_REG_FCT0);
	mt7620_esw_wr(esw, 0x00000000, MT7620_ESW_REG_SGC2);

	/* Port priority 1 for all ports, vlan enabled. */
	mt7620_esw_wr(esw, 0x00005555 |
		      (MT7620_ESW_PORTS_ALL << MT7620_ESW_PFC1_EN_VLAN_S),
		      MT7620_ESW_REG_PFC1);
#endif

	/* Enable Back Pressure, and Flow Control default */
#if 0
	mt7620_esw_wr(esw,
		      ((MT7620_ESW_PORTS_ALL << MT7620_ESW_POC0_EN_BP_S) |
		       (MT7620_ESW_PORTS_ALL << MT7620_ESW_POC0_EN_FC_S)),
		      MT7620_ESW_REG_POC0);
#endif

	/* Enable Aging, and VLAN TAG removal default */
#if 0
	mt7620_esw_wr(esw,
		      ((MT7620_ESW_PORTS_ALL << MT7620_ESW_POC2_ENAGING_S) |
		       (MT7620_ESW_PORTS_NOCPU << MT7620_ESW_POC2_UNTAG_EN_S)),
		      MT7620_ESW_REG_POC2);
#endif
	/*mt7620_esw_wr(esw, esw->pdata->reg_initval_fct2, MT7620_ESW_REG_FCT2);*/

	/*
	 * 300s aging timer, max packet len 1536, broadcast storm prevention
	 * disabled, disable collision abort, mac xor48 hash, 10 packet back
	 * pressure jam, GMII disable was_transmit, back pressure disabled,
	 * 30ms led flash, unmatched IGMP as broadcast, rmc tb fault to all
	 * ports.
	 */
#if 0
	mt7620_esw_wr(esw, 0x0008a301, MT7620_ESW_REG_SGC);
#endif

	/* Setup SoC Port control register B/M/U frame default set */
#if 0
	mt7620_esw_wr(esw,
		      (MT7620_ESW_SOCPC_CRC_PADDING |
		       (MT7620_ESW_PORTS_CPU << MT7620_ESW_SOCPC_DISUN2CPU_S) |
		       (MT7620_ESW_PORTS_CPU << MT7620_ESW_SOCPC_DISMC2CPU_S) |
		       (MT7620_ESW_PORTS_CPU << MT7620_ESW_SOCPC_DISBC2CPU_S)),
		      MT7620_ESW_REG_SOCPC);
#endif
	/*mt7620_esw_wr(esw, esw->pdata->reg_initval_fpa2, MT7620_ESW_REG_FPA2);*/
	/*mt7620_esw_wr(esw, 0x00000000, MT7620_ESW_REG_FPA);*/

	/* Force Link/Activity on ports  set default */
#if 0
	/* Copy disabled port configuration from bootloader setup */
	port_disable = mt7620_esw_get_port_disable(esw);
	for (i = 0; i < 6; i++)
		esw->ports[i].disable = (port_disable & (1 << i)) != 0;

	mt7620_mii_write(esw, 0, 31, 0x8000);
	for (i = 0; i < 5; i++) {
		if (esw->ports[i].disable) {
			mt7620_mii_write(esw, i, MII_BMCR, BMCR_PDOWN);
		} else {
			mt7620_mii_write(esw, i, MII_BMCR,
					 BMCR_FULLDPLX |
					 BMCR_ANENABLE |
					 BMCR_SPEED100);
		}
#if 0
		/* TX10 waveform coefficient */
		mt7620_mii_write(esw, i, 26, 0x1601);
		/* TX100/TX10 AD/DA current bias */
		mt7620_mii_write(esw, i, 29, 0x7058);
		/* TX100 slew rate control */
		mt7620_mii_write(esw, i, 30, 0x0018);
#endif
	}
#endif

#if 0
	/* PHY IOT */
	/* select global register */
	mt7620_mii_write(esw, 0, 31, 0x0);
	/* tune TP_IDL tail and head waveform */
	mt7620_mii_write(esw, 0, 22, 0x052f);
	/* set TX10 signal amplitude threshold to minimum */
	mt7620_mii_write(esw, 0, 17, 0x0fe0);
	/* set squelch amplitude to higher threshold */
	mt7620_mii_write(esw, 0, 18, 0x40ba);
	/* longer TP_IDL tail length */
	mt7620_mii_write(esw, 0, 14, 0x65);
	/* select local register */
	mt7620_mii_write(esw, 0, 31, 0x8000);
#endif

	switch (esw->pdata->vlan_config) {
	case MT7620_ESW_VLAN_CONFIG_NONE:
		raeth_esw_lan_portmaps = MT7620_ESW_PMAP_LLLLLLL;
		raeth_esw_wan_portmaps =  MT7620_ESW_PMAP_WAN_LLLLLLL;
		break;
	case MT7620_ESW_VLAN_CONFIG_NLLLLW:
	case MT7620_ESW_VLAN_CONFIG_LLLLLW:
		raeth_esw_lan_portmaps = MT7620_ESW_PMAP_LLLLLWL;
		raeth_esw_wan_portmaps = MT7620_ESW_PMAP_WAN_LLLLLWL;
		break;
	case MT7620_ESW_VLAN_CONFIG_WLLLLL:
		raeth_esw_lan_portmaps = MT7620_ESW_PMAP_WLLLLLL;
		raeth_esw_wan_portmaps = MT7620_ESW_PMAP_WAN_WLLLLLL;
		break;
	case MT7620_ESW_VLAN_CONFIG_LLLLWN:
		raeth_esw_lan_portmaps = MT7620_ESW_PMAP_LLLLWLL;
		raeth_esw_wan_portmaps = MT7620_ESW_PMAP_WAN_LLLLWLL;
		break;
	default:
		BUG();
	}

#if 0
	/*
	 * Unused HW feature, but still nice to be consistent here...
	 * This is also exported to userspace ('lan' attribute) so it's
	 * conveniently usable to decide which ports go into the wan vlan by
	 * default.
	 */
	mt7620_esw_rmw(esw, MT7620_ESW_REG_SGC2,
		       MT7620_ESW_SGC2_LAN_PMAP_M << MT7620_ESW_SGC2_LAN_PMAP_S,
		       raeth_esw_lan_portmaps << MT7620_ESW_SGC2_LAN_PMAP_S);
#endif
	
	for ( i = 0; i < MT7620_ESW_NUM_PORTS; i++ )
	{
	  if ( BIT(i) & MT7620_ESW_PORTS_NOCPU )
	  {
	     	if ( BIT(i) & raeth_esw_lan_portmaps ) /*lan ports*/
	     	{
			/* portmap for lan forwarding */
	         	/*w_data =( 0x3UL<< PCR_PORT_VLAN_SHIFT) |  ( 0xFF << PCR_PORT_MATRIX_SHIFT ) ;*/
	         	w_data = ( raeth_esw_lan_portmaps << PCR_PORT_MATRIX_SHIFT ) ;
	         	mt7620_esw_wr(esw, w_data, REG_ESW_PN_PCR(i));

	        	/* LAN ports set as transparent port */
	        	w_data = (0x3UL << PVC_VLAN_ATTR_SHIFT ) | (0x8100 << PVC_STAG_VPID_SHIFT );
	         	mt7620_esw_wr(esw, w_data, REG_ESW_PN_PVC(i));
	     	}
	    	else if ( BIT(i) & raeth_esw_wan_portmaps ) /*wan ports*/
		{
			 /* portmap for wan forwarding */
	         	/*w_data =( 0x3UL<< PCR_PORT_VLAN_SHIFT) |  ( 0xFF << PCR_PORT_MATRIX_SHIFT ) ;*/
	         	w_data = ( raeth_esw_wan_portmaps << PCR_PORT_MATRIX_SHIFT ) ;
	         	mt7620_esw_wr(esw, w_data, REG_ESW_PN_PCR(i));

	        	/* WAN ports set as transparent port */
	         	w_data = (0x3UL << PVC_VLAN_ATTR_SHIFT ) | (0x8100 << PVC_STAG_VPID_SHIFT );
	         	mt7620_esw_wr(esw, w_data, REG_ESW_PN_PVC(i));

			/*close the wan port learn mac*/
/*			mt7620_esw_rmw(esw, REG_ESW_PN_PSC(i), PSC_SA_DIS, PSC_SA_DIS); */
		}
	    }
	    else /*cpu and p7*/
	     {
		 /* portmap for cpu forwarding */
	         w_data = ( 0xFF << PCR_PORT_MATRIX_SHIFT ) /*| ( 0x2UL<< PCR_EG_TAG_SHIFT ) */;
	         mt7620_esw_wr(esw, w_data, REG_ESW_PN_PCR(i));

	         /* ports set as user port */
	         w_data = ( 0x00UL << PVC_VLAN_ATTR_SHIFT ) | (0x8100 << PVC_STAG_VPID_SHIFT );
	         mt7620_esw_wr(esw, w_data, REG_ESW_PN_PVC(i));
		/*default the cpu port vid = 1*/
/*		 mt7620_esw_set_pvid(esw, i, 1);*/
	     }
	} 
		
/*	esw->global_vlan_enable = 1;
	esw->alt_vlan_disable = 0;	
*/
	/* Apply the empty config. */
	mt7620_esw_apply_config(&esw->swdev);
}

static int
mt7620_esw_apply_config(struct switch_dev *dev)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	int i;
	u8 disable = 0;
	u8 doubletag = 0;
	u8 en_vlan = 0;
/*	u8 untag = 0;*/
	u32 w_data = 0;

	for (i = 0; i < MT7620_ESW_NUM_VLANS; i++) {
		u32 vid, vmsc, vm_untags;
		if (esw->global_vlan_enable) {
			vid = esw->vlans[i].vid;
			vmsc = esw->vlans[i].ports;
			vm_untags = esw->vlans[i].ports_untag;
		} else {
			vid = MT7620_ESW_VLAN_NONE;
			vmsc = MT7620_ESW_PORTS_NONE;
			vm_untags = MT7620_ESW_PORTS_NONE;
		}
	/*	printk("set index: %d vlan: %d map %x \n", i, vid, vmsc);*/
		mt7620_esw_set_vlan_id(esw, i, vid);
		mt7620_esw_set_vmsc(esw, i, vmsc, vm_untags);
	}

	for (i = 0; i < MT7620_ESW_NUM_PORTS; i++) {
		u32 pvid;
		disable |= esw->ports[i].disable << i;
		if (esw->global_vlan_enable) {
			doubletag |= esw->ports[i].doubletag << i;
			en_vlan   |= 1                       << i;
	/*		untag     |= esw->ports[i].untag     << i;*/
			pvid       = esw->ports[i].pvid;
		} else {
			int x = esw->alt_vlan_disable ? 0 : 1;
			doubletag |= x << i;
			en_vlan   |= x << i;
	/*		untag     |= x << i;*/
			pvid       = 0;
		}
		mt7620_esw_set_pvid(esw, i, pvid);
#if 0
		if (i < MT7620_ESW_NUM_LEDS)
			mt7620_esw_wr(esw, esw->ports[i].led,
				      MT7620_ESW_REG_P0LED + 4*i);
#endif
	}

	mt7620_esw_set_port_disable(esw, disable);

	for ( i = 0; i < MT7620_ESW_NUM_PORTS; i++ )
	{
	  if ( en_vlan & (0x1UL << i ))
	  {
	     if ( i < MT7620_ESW_NUM_LANWAN )
	     {
	         /*set the port vlan security lan/wan */ 
	         w_data =( 0x3UL<< PCR_PORT_VLAN_SHIFT) |  ( 0xFF << PCR_PORT_MATRIX_SHIFT ) ;
	         mt7620_esw_wr(esw, w_data, REG_ESW_PN_PCR(i));

	         /* LAN/WAN ports set as transparent port */
	         w_data = (0x3UL << PVC_VLAN_ATTR_SHIFT ) | (0x8100 << PVC_STAG_VPID_SHIFT );
	         mt7620_esw_wr(esw, w_data, REG_ESW_PN_PVC(i));
	     }
	    else /*cpu and p7*/
	     {
	         /*set the port vlan security and taged */ 
	         w_data =( 0x3UL<< PCR_PORT_VLAN_SHIFT) |  ( 0xFF << PCR_PORT_MATRIX_SHIFT )
				 | ( 0x2UL<< PCR_EG_TAG_SHIFT ) ;
	         mt7620_esw_wr(esw, w_data, REG_ESW_PN_PCR(i));

	         /* ports set as user port */
	         w_data = ( 0x00UL << PVC_VLAN_ATTR_SHIFT ) | (0x8100 << PVC_STAG_VPID_SHIFT );
	         mt7620_esw_wr(esw, w_data, REG_ESW_PN_PVC(i));

		/*default the cpu port vid = 1*/
		 mt7620_esw_set_pvid(esw, i, 1);
		
	     }
	  }      
	} 
#if 0
	mt7620_esw_rmw(esw, MT7620_ESW_REG_SGC2,
		       (MT7620_ESW_SGC2_DOUBLE_TAG_M <<
			MT7620_ESW_SGC2_DOUBLE_TAG_S),
		       doubletag << MT7620_ESW_SGC2_DOUBLE_TAG_S);
	mt7620_esw_rmw(esw, MT7620_ESW_REG_PFC1,
		       MT7620_ESW_PFC1_EN_VLAN_M << MT7620_ESW_PFC1_EN_VLAN_S,
		       en_vlan << MT7620_ESW_PFC1_EN_VLAN_S);
	mt7620_esw_rmw(esw, MT7620_ESW_REG_POC2,
		       MT7620_ESW_POC2_UNTAG_EN_M << MT7620_ESW_POC2_UNTAG_EN_S,
		       untag << MT7620_ESW_POC2_UNTAG_EN_S);
#endif

	if (!esw->global_vlan_enable) {
		/*
		 * Still need to put all ports into vlan 0 or they'll be
		 * isolated.
		 * NOTE: vlan 0 is special, no vlan tag is prepended
		 */
		mt7620_esw_set_vlan_id(esw, 0, 0);
		mt7620_esw_set_vmsc(esw, 0, MT7620_ESW_PORTS_ALL, MT7620_ESW_PORTS_ALL);
	}

	/*clear the arl table */	
	mt7620_esw_clear_arl_table(esw, TBL_ARL_ATTR_M_ALL_MAC); 

	return 0;
}

static int
mt7620_esw_reset_switch(struct switch_dev *dev)
{
	u32 rdata1,rdata0;
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	esw->global_vlan_enable = 0;
	memset(esw->ports, 0, sizeof(esw->ports));
	memset(esw->vlans, 0, sizeof(esw->vlans));

	/*reserv the mac address*/
	rdata1 = mt7620_esw_rr(esw, REG_ESW_SMACCR1);
	rdata0 = mt7620_esw_rr(esw, REG_ESW_SMACCR0);

	mt7620_esw_hw_init(esw);

	mt7620_esw_wr(esw, rdata1, REG_ESW_SMACCR1);
	mt7620_esw_wr(esw, rdata0, REG_ESW_SMACCR0);
	return 0;
}

static int
mt7620_esw_get_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);

	val->value.i = esw->global_vlan_enable;

	return 0;
}

static int
mt7620_esw_set_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);

	esw->global_vlan_enable = val->value.i != 0;

	return 0;
}

static int
mt7620_esw_get_alt_vlan_disable(struct switch_dev *dev,
				const struct switch_attr *attr,
				struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);

	val->value.i = esw->alt_vlan_disable;

	return 0;
}

static int
mt7620_esw_set_alt_vlan_disable(struct switch_dev *dev,
				const struct switch_attr *attr,
				struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);

	esw->alt_vlan_disable = val->value.i != 0;

	return 0;
}

#if 0 
static int
mt7620_esw_get_port_link(struct switch_dev *dev,
			 int port,
			 struct switch_port_link *link)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	u32 speed, poa;

	if (port < 0 || port >= MT7620_ESW_NUM_PORTS)
		return -EINVAL;

	poa = mt7620_esw_rr(esw, REG_ESW_MN_PMCR(port)) ;

	link->link = (poa & PMSR_MAC_LINK_STS) ? 1 : 0 ;
	link->duplex = (poa & PMSR_MAC_DPX_STS) ? 1 : 0 ;

	speed = ( poa >> PMSR_MAC_SPD_STS_SHIFT ) & PMSR_MAC_SPD_STS_MASK ;
	switch (speed) {
	case 0:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case 1:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case 2: /* forced gige speed can be 2 or 3 */
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	default:
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}

	return 0;
}
#endif
static int
mt7620_esw_get_port_bool(struct switch_dev *dev,
			 const struct switch_attr *attr,
			 struct switch_val *val)
{
#if 0
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	int idx = val->port_vlan;
	u32 x, reg, shift;

	if (idx < 0 || idx >= MT7620_ESW_NUM_PORTS)
		return -EINVAL;

	switch (attr->id) {
	case MT7620_ESW_ATTR_PORT_DISABLE:
		reg = MT7620_ESW_REG_POC0;
		shift = MT7620_ESW_POC0_DIS_PORT_S;
		break;
	case MT7620_ESW_ATTR_PORT_DOUBLETAG:
		reg = MT7620_ESW_REG_SGC2;
		shift = MT7620_ESW_SGC2_DOUBLE_TAG_S;
		break;
	case MT7620_ESW_ATTR_PORT_UNTAG:
		reg = MT7620_ESW_REG_POC2;
		shift = MT7620_ESW_POC2_UNTAG_EN_S;
		break;
	case MT7620_ESW_ATTR_PORT_LAN:
		reg = MT7620_ESW_REG_SGC2;
		shift = MT7620_ESW_SGC2_LAN_PMAP_S;
		if (idx >= MT7620_ESW_NUM_LANWAN)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	x = mt7620_esw_rr(esw, reg);
	val->value.i = (x >> (idx + shift)) & 1;

#endif
	return 0;
}

static int
mt7620_esw_set_port_bool(struct switch_dev *dev,
			 const struct switch_attr *attr,
			 struct switch_val *val)
{
#if 0
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	int idx = val->port_vlan;

	if (idx < 0 || idx >= MT7620_ESW_NUM_PORTS ||
	    val->value.i < 0 || val->value.i > 1)
		return -EINVAL;

	switch (attr->id) {
	case MT7620_ESW_ATTR_PORT_DISABLE:
		esw->ports[idx].disable = val->value.i;
		break;
	case MT7620_ESW_ATTR_PORT_DOUBLETAG:
		esw->ports[idx].doubletag = val->value.i;
		break;
	case MT7620_ESW_ATTR_PORT_UNTAG:
		esw->ports[idx].untag = val->value.i;
		break;
	default:
		return -EINVAL;
	}
#endif

	return 0;
}

static int
mt7620_esw_get_port_recv_badgood(struct switch_dev *dev,
				 const struct switch_attr *attr,
				 struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	int idx = val->port_vlan;
	u32 reg =  ( attr->id == MT7620_ESW_ATTR_PORT_RECV_GOOD) ? REG_ESW_MIB_RGOC(idx) : REG_ESW_MIB_RBOC(idx) ;
	u32 r_data = 0;

	if (idx < 0 || idx >= MT7620_ESW_NUM_LANWAN)
		return -EINVAL;

	r_data = mt7620_esw_rr( esw, reg );
	val->value.i = r_data ;

	return 0;
}

static int
mt7620_esw_get_port_led(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
#if 0
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	int idx = val->port_vlan;

	if (idx < 0 || idx >= MT7620_ESW_NUM_PORTS ||
	    idx >= MT7620_ESW_NUM_LEDS)
		return -EINVAL;

	val->value.i = mt7620_esw_rr(esw, MT7620_ESW_REG_P0LED + 4*idx);
#endif
	return 0;
}

static int
mt7620_esw_set_port_led(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
#if 0
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	int idx = val->port_vlan;

	if (idx < 0 || idx >= MT7620_ESW_NUM_LEDS)
		return -EINVAL;

	esw->ports[idx].led = val->value.i;
#endif
	return 0;
}

static int
mt7620_esw_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);

	if (port >= MT7620_ESW_NUM_PORTS)
		return -EINVAL;

	*val = mt7620_esw_get_pvid(esw, port);

	return 0;
}

static int
mt7620_esw_set_port_pvid(struct switch_dev *dev, int port, int val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);

	if (port >= MT7620_ESW_NUM_PORTS)
		return -EINVAL;

	esw->ports[port].pvid = val;

	return 0;
}

static int
mt7620_esw_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	u32 vmsc;
	int vlan_idx = -1;
	int i;
	struct mt7620_esw_vlan_attr vlan_attr;

	val->len = 0;

	if (val->port_vlan < 0 || val->port_vlan >= MT7620_ESW_NUM_VIDS)
		return -EINVAL;

	memset(&vlan_attr, 0, sizeof(vlan_attr));
	/* valid vlan? */
	for (i = 0; i < MT7620_ESW_NUM_VLANS; i++) {
		if (mt7620_esw_get_vlan_id(esw, i) == val->port_vlan &&
		    mt7620_esw_get_vmsc(esw, i,&vlan_attr) == 0 ) {
			vlan_idx = i;
			break;
		}
	}

	if (vlan_idx == -1)
		return -EINVAL;

	/*mt7620_esw_get_vmsc(esw, vlan_idx);*/

	vmsc = vlan_attr.port_mem;
	for (i = 0; i < MT7620_ESW_NUM_PORTS; i++) {
		struct switch_port *p;
		int port_mask = 1 << i;

		if (!(vmsc & port_mask))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		p->flags = vlan_attr.egress_tag[i];
	}

	return 0;
}

static int
mt7620_esw_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	int ports;
	int ports_untag;
	int vlan_idx = -1;
	int i;

	if (val->port_vlan < 0 || val->port_vlan >= MT7620_ESW_NUM_VIDS ||
	    val->len > MT7620_ESW_NUM_PORTS)
		return -EINVAL;

	/* one of the already defined vlans? */
	for (i = 0; i < MT7620_ESW_NUM_VLANS; i++) {
		if (esw->vlans[i].vid == val->port_vlan &&
		    esw->vlans[i].ports != MT7620_ESW_PORTS_NONE) {
			vlan_idx = i;
			break;
		}
	}

	/* select a free slot */
	for (i = 0; vlan_idx == -1 && i < MT7620_ESW_NUM_VLANS; i++) {
		if (esw->vlans[i].ports == MT7620_ESW_PORTS_NONE)
			vlan_idx = i;
	}

	/* bail if all slots are in use */
	if (vlan_idx == -1)
		return -EINVAL;

	ports = MT7620_ESW_PORTS_NONE;
	ports_untag = MT7620_ESW_PORTS_NONE;
	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];
		int port_mask = 1 << p->id;
		bool untagged = !(p->flags & (1 << SWITCH_PORT_FLAG_TAGGED));

		if (p->id >= MT7620_ESW_NUM_PORTS)
			return -EINVAL;

		ports |= port_mask;
		if ( untagged ) 
		{
			ports_untag |= 0x1L << p->id;
		}
	/*	esw->ports[p->id].untag = untagged;*/
	}
	esw->vlans[vlan_idx].ports = ports;
	if (ports == MT7620_ESW_PORTS_NONE)
		esw->vlans[vlan_idx].vid = MT7620_ESW_VLAN_NONE;
	else
		esw->vlans[vlan_idx].vid = val->port_vlan;
	esw->vlans[vlan_idx].ports_untag = ports_untag;

	return 0;
}

static int
mt7620_set_switch_reg(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	int is_fe = 0;
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	struct switch_reg *regrw = val->value.regrw;
	
	is_fe =( regrw->addr & 0x100000 ) ? 1 : 0 ;

	/*write the switch register*/	
	printk(KERN_ALERT"the switch register write: register: 0x%.08X  value: 0x%.08X fe reg: %s \r\n",	
			regrw->addr,regrw->data,is_fe ? "Yes":"No");	

	if ( is_fe == 1 ) {
		__raw_writel(regrw->data, fe_mac_base + (regrw->addr & 0xFFFF));
	}
	else {
		mt7620_esw_wr(esw,regrw->data, (regrw->addr & 0xFFFF));
	}
	
	return 0;
}

static int
mt7620_get_switch_reg(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	int is_fe = 0;
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	struct switch_reg *regrw = val->value.regrw;
	
	is_fe =( regrw->addr & 0x100000 ) ? 1 : 0 ;

	/*read the switch register*/	
	if ( is_fe == 1 ) {
		regrw->data = __raw_readl(fe_mac_base + (regrw->addr & 0xFFFF));
	}
	else{
		regrw->data = mt7620_esw_rr(esw, (regrw->addr & 0xFFFF));
	}

	printk(KERN_ALERT"the switch register read: register: 0x%.08X  value: 0x%.08X fe_reg: %s\r\n",
			regrw->addr,regrw->data,is_fe? "Yes":"No");	
	return 0;
}

static int
mt7620_set_switch_phy_reg(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	unsigned phy_addr = val->port_vlan;	
	struct switch_reg *regrw = val->value.regrw;
	
	/*write the switch register*/	
	printk(KERN_ALERT"the switch phy register write: register: phy %u  0x%.08X  value: 0x%.08X \r\n",	
			phy_addr,regrw->addr,regrw->data);	

	mt7620_mii_write(esw,phy_addr, MT7620_ESW_PHYADDR(regrw->addr),regrw->data & 0xFFFF );
	
	return 0;
}

static int
mt7620_get_switch_phy_reg(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct mt7620_esw *esw = container_of(dev, struct mt7620_esw, swdev);
	unsigned phy_addr = val->port_vlan;	
	struct switch_reg *regrw = val->value.regrw;
	
	/*read the switch register*/
	regrw->data = mt7620_mii_read(esw, MT7620_ESW_PHYADDR(phy_addr), regrw->addr);

	printk(KERN_ALERT"the switch phy register read: phy: %u register: 0x%.08X  value: 0x%.08X \r\n",
			phy_addr,regrw->addr,regrw->data);	
#if 1
	{
	 int ii = 0;
	 int ret = 0;
	 struct mt7620_esw_arltbl_data entry; 

	 ret = mt7620_esw_search_arl_first_entry(esw, &entry);
	 if ( (ret == 0) && entry.valid)
	    {
	 	for( ii = 0; ii < 1000; ii++)
		{
			ret = mt7620_esw_search_arl_next_entry(esw,&entry);	
	 		if (!((ret == 0) && entry.valid))
			{
				break;
			}
		}
	    }
	}
#endif
	return 0;
}
static const struct switch_attr mt7620_esw_global[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "VLAN mode (1:enabled)",
		.max = 1,
		.id = MT7620_ESW_ATTR_ENABLE_VLAN,
		.get = mt7620_esw_get_vlan_enable,
		.set = mt7620_esw_set_vlan_enable,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "alternate_vlan_disable",
		.description = "Use en_vlan instead of doubletag to disable"
				" VLAN mode",
		.max = 1,
		.id = MT7620_ESW_ATTR_ALT_VLAN_DISABLE,
		.get = mt7620_esw_get_alt_vlan_disable,
		.set = mt7620_esw_set_alt_vlan_disable,
	},
	{
		.type = SWITCH_TYPE_REGISTER,
		.name = "switch_reg",
		.description = "read/write switch register",
		.set = mt7620_set_switch_reg,
		.get = mt7620_get_switch_reg,
	},
};

static const struct switch_attr mt7620_esw_port[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "disable",
		.description = "Port state (1:disabled)",
		.max = 1,
		.id = MT7620_ESW_ATTR_PORT_DISABLE,
		.get = mt7620_esw_get_port_bool,
		.set = mt7620_esw_set_port_bool,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "doubletag",
		.description = "Double tagging for incoming vlan packets "
				"(1:enabled)",
		.max = 1,
		.id = MT7620_ESW_ATTR_PORT_DOUBLETAG,
		.get = mt7620_esw_get_port_bool,
		.set = mt7620_esw_set_port_bool,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "untag",
		.description = "Untag (1:strip outgoing vlan tag)",
		.max = 1,
		.id = MT7620_ESW_ATTR_PORT_UNTAG,
		.get = mt7620_esw_get_port_bool,
		.set = mt7620_esw_set_port_bool,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "led",
		.description = "LED mode (0:link, 1:100m, 2:duplex, 3:activity,"
				" 4:collision, 5:linkact, 6:duplcoll, 7:10mact,"
				" 8:100mact, 10:blink, 12:on)",
		.max = 15,
		.id = MT7620_ESW_ATTR_PORT_LED,
		.get = mt7620_esw_get_port_led,
		.set = mt7620_esw_set_port_led,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "lan",
		.description = "HW port group (0:wan, 1:lan)",
		.max = 1,
		.id = MT7620_ESW_ATTR_PORT_LAN,
		.get = mt7620_esw_get_port_bool,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "recv_bad",
		.description = "Receive bad packet counter",
		.id = MT7620_ESW_ATTR_PORT_RECV_BAD,
		.get = mt7620_esw_get_port_recv_badgood,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "recv_good",
		.description = "Receive good packet counter",
		.id = MT7620_ESW_ATTR_PORT_RECV_GOOD,
		.get = mt7620_esw_get_port_recv_badgood,
	},
	{
		.type = SWITCH_TYPE_REGISTER,
		.name = "switch_phy_reg",
		.description = "read/write switch phy register",
		.set = mt7620_set_switch_phy_reg,
		.get = mt7620_get_switch_phy_reg,
	},
};

static const struct switch_attr mt7620_esw_vlan[] = {
};

static const struct switch_dev_ops mt7620_esw_ops = {
	.attr_global = {
		.attr = mt7620_esw_global,
		.n_attr = ARRAY_SIZE(mt7620_esw_global),
	},
	.attr_port = {
		.attr = mt7620_esw_port,
		.n_attr = ARRAY_SIZE(mt7620_esw_port),
	},
	.attr_vlan = {
		.attr = mt7620_esw_vlan,
		.n_attr = ARRAY_SIZE(mt7620_esw_vlan),
	},
	.get_vlan_ports = mt7620_esw_get_vlan_ports,
	.set_vlan_ports = mt7620_esw_set_vlan_ports,
	.get_port_pvid = mt7620_esw_get_port_pvid,
	.set_port_pvid = mt7620_esw_set_port_pvid,
/*
	.get_port_link = mt7620_esw_get_port_link,
*/
	.apply_config = mt7620_esw_apply_config,
	.reset_switch = mt7620_esw_reset_switch,
};

static int
mt7620_esw_probe(struct platform_device *pdev)
{
	struct mt7620_esw_platform_data *pdata;
	struct mt7620_esw *esw;
	struct switch_dev *swdev;
	struct resource *res;
	int err;

	pdata = pdev->dev.platform_data;
	if (!pdata)
		return -EINVAL;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "esw_base");
	if (!res) {
		dev_err(&pdev->dev, "no memory resource found\n");
		return -ENOMEM;
	}

	esw = kzalloc(sizeof(struct mt7620_esw), GFP_KERNEL);
	if (!esw) {
		dev_err(&pdev->dev, "no memory for private data\n");
		return -ENOMEM;
	}

	esw->base = ioremap_nocache(res->start, resource_size(res));
	if (!esw->base) {
		dev_err(&pdev->dev, "ioremap failed\n");
		err = -ENOMEM;
		goto free_esw;
	}

	swdev = &esw->swdev;
	swdev->name = "mt7620-esw";
	swdev->alias = "mt7620";
	swdev->cpu_port = MT7620_ESW_PORT6;
	swdev->ports = MT7620_ESW_NUM_PORTS;
	swdev->vlans = MT7620_ESW_NUM_VLANS/*MT7620_ESW_NUM_VIDS*/;
	swdev->ops = &mt7620_esw_ops;

	err = register_switch(swdev, NULL);
	if (err < 0) {
		dev_err(&pdev->dev, "register_switch failed\n");
		goto unmap_base;
	}

	pr_info("%s: Found and Register!\n",swdev->name);        	

	platform_set_drvdata(pdev, esw);

	esw->pdata = pdata;
	spin_lock_init(&esw->reg_rw_lock);
	mt7620_esw_hw_init(esw);

	return 0;

unmap_base:
	iounmap(esw->base);
free_esw:
	kfree(esw);
	return err;
}

static int
mt7620_esw_remove(struct platform_device *pdev)
{
	struct mt7620_esw *esw;

	esw = platform_get_drvdata(pdev);
	if (esw) {
		unregister_switch(&esw->swdev);
		platform_set_drvdata(pdev, NULL);
		iounmap(esw->base);
		kfree(esw);
	}

	return 0;
}

static struct platform_driver mt7620_esw_driver = {
	.probe = mt7620_esw_probe,
	.remove = mt7620_esw_remove,
	.driver = {
		.name = "mt7620-esw",
		.owner = THIS_MODULE,
	},
};

static int __init
mt7620_esw_init(void)
{
	return platform_driver_register(&mt7620_esw_driver);
}

static void
mt7620_esw_exit(void)
{
	platform_driver_unregister(&mt7620_esw_driver);
}
