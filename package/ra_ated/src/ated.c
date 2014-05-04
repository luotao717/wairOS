#include <stdio.h>
#include <string.h>
#include <errno.h>  
#include <sys/socket.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <linux/wireless.h>
//#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include "ated.h"

#define SIGNAL

#ifdef SIGNAL
#include <signal.h>             /* signal */

void init_signals(void);
void sighup(int);
#endif // SIGNAL

#undef DBG
#ifdef DBG
#define DBGPRINT(fmt, args...)		printf(fmt, ## args)
#else
#define DBGPRINT(fmt, args...)
#endif

	
static void RaCfg_Agent(void);
static int OpenRaCfgSocket(void);
static void NetReceive(u8 *inpkt, int len);
static void SendRaCfgAckFrame(int len);


static const char broadcast_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static unsigned char packet[1536];
static int sock = -1;
static unsigned char buffer[2048];
static unsigned char my_eth_addr[6];
static unsigned char wlan_mac[6];
static int if_index;
static int do_fork = 1;
static sighup_flag = 1;

#define IFNAME_LEN 10
static char ifname[IFNAME_LEN + 1]={"br-lan"};
static void
usage(char *prog)
{
    printf("\nUsage:  %s [-i iface] [-a address] [-h]",
           prog);
    printf("\n  -i iface the max len of ifname is %d.\n \
	\tif non iface set, use iface eth1_0 as default.", IFNAME_LEN);
    printf("\n  -a xx:xx:xx:xx:xx:xx,  set the wireless MAC address.");	
    printf("\n  -h, \t\tdisplay this usage message.");	
    printf("\n\n");
    exit(1);	
}


#ifdef SIGNAL

void sighup(int dummy)
{
    sighup_flag = 0;
}


void init_signals(void)
{
	struct sigaction sa;

	sa.sa_flags = 0;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGABRT);
	

	sa.sa_handler = sighup;
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
}

#endif // SIGNAL

const char separator1 = ':'; 	//MacAddr string separator 
const char separator2 = '-'; 

unsigned char* MacAddrStringtoBytes(const char *pMACAddrString, 
	unsigned char* pMacAddrBytes)
{
	unsigned int i;
	
	for (i = 0; i < ETH_ALEN; ++i) {
		unsigned int number = 0;
		char ch;

		//Convert letter into lower case.
		ch = tolower (*pMACAddrString++);

		if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f')) {
			return NULL;
		}

		//Convert into number. 
		// a. If char is digit then ch - '0'
		// b. else (ch - 'a' + 10) it is done because addition of 10 takes correct value.
		number = isdigit (ch) ? (ch - '0') : (ch - 'a' + 10);
		ch = tolower (*pMACAddrString);

		if ((i < 5 && ch != separator1 && ch != separator2 ) || (i == 5 && ch != '\0' && !isspace (ch))) {
			++pMACAddrString;

			if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f')) {
				return NULL;
			}

			number <<= 4;
			number += isdigit (ch) ? (ch - '0') : (ch - 'a' + 10);
			ch = *pMACAddrString;

			if (i < 5 && ch != separator1 && ch != separator2) {
				return NULL;
			}
		}
		/* Store result.  */
		pMacAddrBytes[i] = (unsigned char) number;
		/* Skip separator.  */
		++pMACAddrString;
	}
	
	return pMacAddrBytes;
}

static void setMacAddress(void) 
{
	int i, s;
	struct iwreq pwrq;
	char tmpbuf[50] = {0};
	unsigned char lan_mac[6], wan_mac[6];
	unsigned long long baseMacAddr, lanMacAddr, wanMacAddr;
	int len = 0;
	
	/* set baseMacAddr = wlanMacAddr in network order bytes */
	baseMacAddr = (unsigned long long)wlan_mac[0] << 40 | (unsigned long long)wlan_mac[1] << 32 | \
			(unsigned long long)wlan_mac[2] << 24 | (unsigned long long)wlan_mac[3] << 16 | \
			(unsigned long long)wlan_mac[4] << 8 | (unsigned long long)wlan_mac[5]; 
	DBGPRINT("baseMacAddr = %llx, %llu \n", baseMacAddr, baseMacAddr);

	lanMacAddr = baseMacAddr + 4;
	DBGPRINT("lanMac = %04x-%04x-%04x \n", (unsigned short)(baseMacAddr >> 32), \
			(unsigned short)(baseMacAddr >> 16), (unsigned short)baseMacAddr);
	#if 0
	memcpy(lan_mac, wlan_mac, 6);
	lan_mac[5] = wlan_mac[5] + 4;
	//memcpy(wan_mac, wlan_mac, 6);
	//wan_mac[5] = wlan_mac[5] + 8;
	#endif
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		DBGPRINT("Socket error in IOCTL\n");
		return;
	}
	bzero(&pwrq, sizeof(pwrq));
	strncpy(pwrq.ifr_name, "ra0", IFNAMSIZ);

	/* set wlan address */
	for (i = 0; i < 3; i++) {
		bzero(tmpbuf, sizeof(tmpbuf));
		//len = sprintf(tmpbuf, "%x=%02x%02x", WMAC0_OFFSET + i*2, wlan_mac[1+i*2], wlan_mac[0+i*2]);
		len = sprintf(tmpbuf, "%x=%04x", WMAC0_OFFSET + i*2, ntohs(baseMacAddr >> (16*(2-i))));
		pwrq.u.data.pointer = (caddr_t) &tmpbuf;
		pwrq.u.data.length = len + 1;
		ioctl(s, RTPRIV_IOCTL_E2P, &pwrq);
	}

	/* set lan address */
	for (i = 0; i < 3; i++) {
		bzero(tmpbuf, sizeof(tmpbuf));
		//len = sprintf(tmpbuf, "%x=%02x%02x", GMAC0_OFFSET + i*2, lan_mac[1+i*2], lan_mac[0+i*2]);
		len = sprintf(tmpbuf, "%x=%04x", GMAC0_OFFSET + i*2, ntohs(lanMacAddr >> (16*(2-i))));
		//printf("i=%d, %s \n", i, tmpbuf);
		pwrq.u.data.pointer = (caddr_t) &tmpbuf;
		pwrq.u.data.length = len + 1;
		ioctl(s, RTPRIV_IOCTL_E2P, &pwrq);
	}

	#if 0 /* needn't to set WAN EEPROM VALUE for RL-R3000, JUSB base from LAN MAC ADDR */
	#endif
	
	close(s);
}
int main(int argc, char *argv[])
{
    pid_t pid;
    char mac[18] = {"0"};	

    if (argc > 5) {
	 usage(argv[0]);
    }
	
    for (int c; (c = getopt(argc, argv, "i:a:h")) != -1;) {
		switch (c) {
		case 'i':
		    if (strlen(optarg) > IFNAME_LEN) {
		        printf("the max len of ifname is %d\n", IFNAME_LEN);
	 		 return -1;			
		    }	
		    memcpy(ifname, optarg, strlen(optarg) + 1);
			DBGPRINT(" iface = %s\n", ifname);
		    break;
		case 'a':
		    if (strlen(optarg) > 17) {
		        printf("in valid mac address.\n");
	 		 return -1;			
		    }		
		    memcpy(mac, optarg, strlen(optarg) + 1);
		    MacAddrStringtoBytes(mac, wlan_mac);
			DBGPRINT(" mac address = %s, ---> %02x:%02x:%02x:%02x:%02x:%02x\n", mac, \
				wlan_mac[0], wlan_mac[1], wlan_mac[2], wlan_mac[3], wlan_mac[4], wlan_mac[5]);
		    setMacAddress();
		    break;
		case 'h':
		    usage(argv[0]);
		    break;
		default:
		    usage(argv[0]);
		    break;
		}
    	}
	
#ifdef SIGNAL
	init_signals();
#endif
	

    /* background ourself */
    if (do_fork) {
        pid = fork();
    } else {
        pid = getpid();
    }

    switch (pid) {
    case -1:
        /* error */
        perror("fork/getpid");
        return -1;
    case 0:
        /* child, success */
        break;
    default:
        /* parent, success */
        if (do_fork)
            return 0;
        break;
    }


	RaCfg_Agent();
	return 0;
}



static void RaCfg_Agent(void)
{
	int i = 0, n, count, s;
	struct timeval tv;
	fd_set readfds;
	unsigned short rcv_protocol;
	struct iwreq pwrq;
	unsigned short CmdId;
	unsigned int magic;

	if (OpenRaCfgSocket() != 0)
	{
		return;
	}
	
	/* Stop AP first */
	/* pass command to driver */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		DBGPRINT("Socket error in IOCTL\n");
		return;
	}
	bzero(&pwrq, sizeof(pwrq));
	CmdId = htons(RACFG_CMD_AP_STOP);
	memcpy(&packet[20], &CmdId, 2);
	magic = htonl(RACFG_MAGIC_NO);
	memcpy(&packet[14], &magic, 4);
	pwrq.u.data.pointer = (caddr_t) &packet[14];
	pwrq.u.data.length = 8;
	strncpy(pwrq.ifr_name, "ra0", IFNAMSIZ);

	ioctl(s, RTPRIV_IOCTL_ATE, &pwrq);
	close(s);
	
	
	/*
	 * Loop to recv cmd from host 
	 * to start up RT2880
	 */

	DBGPRINT("2880 ATE agent program start\n");

	tv.tv_sec=1;
	tv.tv_usec=0;
	
	
	while (sighup_flag) 
	{
		FD_ZERO(&readfds);
		FD_SET(sock,&readfds);
		
		count = select(sock+1,&readfds,NULL,NULL,&tv);
		
		if (count < 0)
		{
			DBGPRINT("socket select error\n");
			return;
		}
		else if (count == 0)
		{
			usleep(1000);
		}
		else
		{
			if ((n = recvfrom(sock, buffer, 2048, 0, NULL, NULL)) > 0)
			{
				memcpy(&rcv_protocol, buffer+12, 2);
				
				/* recv the protocol we are waiting */       
				if (rcv_protocol == ntohs(ETH_P_RACFG))
				{
					DBGPRINT("NetReceive\n");
					NetReceive(buffer, n);
				}
			}			
		}
		
	}

	/* Start AP */
	/* pass command to driver */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		DBGPRINT("Socket error in IOCTL\n");
		return;
	}
	bzero(&pwrq, sizeof(pwrq));
	CmdId = htons(RACFG_CMD_AP_START);
	memcpy(&packet[20], &CmdId, 2);
	magic = htonl(RACFG_MAGIC_NO);
	memcpy(&packet[14], &magic, 4);
	pwrq.u.data.pointer = (caddr_t) &packet[14];
	pwrq.u.data.length = 8;
	strncpy(pwrq.ifr_name, "ra0", IFNAMSIZ);

	ioctl(s, RTPRIV_IOCTL_ATE, &pwrq);
	close(s);

	close(sock);
	/* never happen */
}

static int OpenRaCfgSocket(void)
{
	struct ifreq ethreq;
	struct ifreq ifr;
	struct sockaddr_ll addr;
	struct in_addr	own_ip_addr;
	
	//if ((sock=socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RACFG)))<0) 
	if ((sock=socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))<0) // for testing
	{
		perror("socket");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	memcpy(ifr.ifr_name, ifname, strlen(ifname) + 1);
	DBGPRINT("ifname=%s\n", ifr.ifr_name);
	if (ioctl(sock, SIOCGIFINDEX, &ifr) != 0) {
		perror("ioctl(SIOCGIFINDEX)(eth_sock)");
		goto close;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;
	if_index = ifr.ifr_ifindex;
	
	DBGPRINT(" if_index=%d\n", if_index);

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) 
	{
		perror("bind");
		goto close;
	}

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) != 0) {
		perror("ioctl(SIOCGIFHWADDR)(eth_sock)");
		goto close;
	}
	
	/* john set promisc*/
#if 0
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0) {
		perror("ioctl(SIOCGIFFLAGS)(eth_sock)");
		//goto close;
	}
	if (ifr.ifr_flags & IFF_PROMISC) {
		DBGPRINT(" non promisc \n");
	}
	ifr.ifr_flags |= IFF_PROMISC;
#endif
	/* end */

	memcpy(my_eth_addr, ifr.ifr_hwaddr.sa_data, 6);

	return 0;

close:
	close(sock);
	sock = -1;
	return (-1);
}


/*
 * The Data filed of RaCfgAck Frame always is empty
 * during bootstrapping state
 */

static void SendRaCfgAckFrame(int len)
{
	struct ethhdr *p_ehead;
	struct sockaddr_ll socket_address;
	unsigned char *header, *data;
	int send_result = 0;
	
	
	header = &packet[0];
	data = &packet[14];
	p_ehead = (struct ethhdr *)&packet[0];

	socket_address.sll_family = PF_PACKET;
	socket_address.sll_protocol = htons(ETH_P_RACFG);
	socket_address.sll_ifindex = if_index;
	//socket_address.sll_pkttype = PACKET_OTHERHOST; // unicast
	socket_address.sll_pkttype = PACKET_BROADCAST;
	socket_address.sll_hatype = ARPHRD_ETHER;

	socket_address.sll_halen = ETH_ALEN;
	
	bzero(&socket_address.sll_addr[0], 8);

	//memcpy(&socket_address.sll_addr[0], p_ehead->h_dest, 6); // unicast
	memcpy(&socket_address.sll_addr[0], broadcast_addr, 6);
	
	send_result = sendto(sock, &packet[0], len, 0, (struct sockaddr *)&socket_address, sizeof(socket_address));
	DBGPRINT("response send bytes = %d\n", send_result);

}


static void NetReceive(u8 *inpkt, int len)
{
	int i;
	struct ethhdr		*p_ehead;
	struct racfghdr 	*p_racfgh;
	u16 				Command_Type;
	u16					Command_Id;
	u16					Sequence;
	u16 				Len;
	u8  				*ptr;
	ulong 				StartEntry; /* firmware start entry */
	struct 				iwreq pwrq;
	int    				s;
    pid_t               pid;


	/* 
	 * Check packet len 
	 */
	if (len < (ETH_HLEN + sizeof(struct racfghdr))) 
	{
		DBGPRINT("packet len is too short!\n");
		return;
	}

	p_ehead = (struct ethhdr *) inpkt;
	p_racfgh = (struct racfghdr *) &inpkt[ETH_HLEN];

	/*
	 * 1. Check if dest mac is my mac or broadcast mac
	 * 2. Ethernet Protocol ID == ETH_P_RACFG
	 * 3. RaCfg Frame Magic Number
	 */
	
	if ((p_ehead->h_proto == htons(ETH_P_RACFG)) && 
		((strncmp(my_eth_addr, p_ehead->h_dest, 6) == 0) || (strncmp(broadcast_addr, p_ehead->h_dest, 6) == 0))&&
		(le32_to_cpu(p_racfgh->magic_no) == RACFG_MAGIC_NO)) 
	{

		Command_Type = le16_to_cpu(p_racfgh->comand_type);
		if ((Command_Type & RACFG_CMD_TYPE_PASSIVE_MASK) != RACFG_CMD_TYPE_ETHREQ)
		{
			DBGPRINT("Command_Type error = %x\n", Command_Type);
			return;
		}
	} 
	else 
	{
		DBGPRINT("protocol or magic error\n");
		return;
	}


	Command_Id = le16_to_cpu(p_racfgh->comand_id);
	Sequence = le16_to_cpu(p_racfgh->sequence);
	Len	= le16_to_cpu(p_racfgh->length);

	/* Check for length and ID */
	if (((Command_Id == RACFG_CMD_AP_STOP) || (Command_Id == RACFG_CMD_AP_START)) && (Len == 0))
	{
		if (Command_Id == RACFG_CMD_AP_STOP)
		{
			DBGPRINT("Cmd:APStop\n");
		}
		else
		{
			DBGPRINT("Cmd:APStart\n");
		}
	}
	else if (((Command_Id == RACFG_CMD_AP_STOP) || (Command_Id == RACFG_CMD_AP_START)) && (Len != 0))
	{
			DBGPRINT("length field error, id = %x, Len = %d, len should be %d\n", Command_Id, Len, 0);
			return;
	}
	else if ((Command_Id <= SIZE_OF_CMD_ID_TABLE) && (cmd_id_len_tbl[Command_Id] != 0xffff))
	{
		if (Len != cmd_id_len_tbl[Command_Id])
		{
			DBGPRINT("length field or command id error, id = %x, Len = %d, len should be %d\n", Command_Id, Len, cmd_id_len_tbl[Command_Id]);
			return;
		}
	}
	// Len of RACFG_CMD_TX_START will be 0xffff or zero.
	else if ((Command_Id == RACFG_CMD_TX_START) && (Len != 0))
	{
		if (Len < 40) // TXWI:20 Count:2 Hlen:2 Header:2+2+6+6
		{
			DBGPRINT("Cmd:TxStart, length is too short, len = %d\n", Len);
			return;
		}
		
	}
	else if ((Command_Id == RACFG_CMD_TX_START) && (Len == 0))
	{
			DBGPRINT("Cmd:TxStart, length is zero, may be for Carrier test or Carrier Suppression\n");
	}
	else if (Command_Id == RACFG_CMD_E2PROM_WRITE_ALL)
	{
	}
	else
	{
	}

	/* pass command to driver */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		DBGPRINT("Socket error in IOCTL\n");
		return;
	}
	DBGPRINT("Command_Id = 0x%04x\n", Command_Id);
	bzero(&pwrq, sizeof(pwrq));
	bzero(&packet[0], 1536);
	//
	// Tell my pid to ra0 with RACFG_CMD_AP_START command.
	// Although the length field (Len) of this packet is zero,
	// it has 4-bytes content containing my pid.
	//
	if (Command_Id == RACFG_CMD_AP_START)
	{
		pid = getpid();
		// stuff my pid into the content
		memcpy(&p_racfgh->data[0], (u8 *)&pid, sizeof(pid));
		memcpy(&packet[14], p_racfgh, Len + 12 + sizeof(pid));
		pwrq.u.data.length = Len + 12 + sizeof(pid);
	}
	else
	{
		memcpy(&packet[14], p_racfgh, Len + 12);
		pwrq.u.data.length = Len + 12;
	}
	pwrq.u.data.pointer = (caddr_t) &packet[14];
	strncpy(pwrq.ifr_name, "ra0", IFNAMSIZ);

	ioctl(s, RTPRIV_IOCTL_ATE, &pwrq);
	close(s);
		
#ifdef DBG
	{
		u16 value;
		u16 *p;
		
		p_racfgh = (struct racfghdr *) pwrq.u.data.pointer;
		memcpy (&value, p_racfgh->data, 2);
		value = ntohs(value);
		if (value == 0)
		{
			int i;
			
			DBGPRINT("Response: Success\n");
			p = p_racfgh->data;
			for (i = 2; i < ntohs(p_racfgh->length); i++)
			{
				DBGPRINT("%2.2x ", (u16)*p);
				if (((i-1) % 8) == 0)
					DBGPRINT("\n");
				
				p++;
			}
			
		}
		else
		{
			DBGPRINT("Response: Failed\n");
		}
		
		
		DBGPRINT("return value = %x\n", value);
	}
#endif

	/* add ack bit to command type */
	p_racfgh = (struct racfghdr *)&packet[14];
	p_racfgh->comand_type = p_racfgh->comand_type | htons(~RACFG_CMD_TYPE_PASSIVE_MASK);

	/* prepare ethernet header */
	//memcpy(&packet[0], p_ehead->h_source, 6); // unicast
	memcpy(&packet[0], broadcast_addr, 6);
	p_ehead = (struct ethhdr *)&packet[0];
	memcpy(p_ehead->h_source, my_eth_addr, 6);
	p_ehead->h_proto = htons(ETH_P_RACFG);
	
	// determine the length to send and send Ack
	{
		u32 length;
		
		length = ntohs(p_racfgh->length) + 14 + 12;
		if (length < 60) 
		{
			length = 60;
		}
		else if (length > 1514)
		{
			DBGPRINT("response ethernet length is too long\n");
			return;
		}
		SendRaCfgAckFrame(length);
	}
	
}

