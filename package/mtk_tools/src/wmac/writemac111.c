
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

int set_usage()
{
	printf("\nUsage example: \n");
	printf("writemac 00e04c112230\n");
	return -1;
}

int main(int argc, char *argv[])
{
	char *cmd, *value;
	unsigned char mac_char[7]={0};
	unsigned char tmpbuf11[0x2000]={0};
	char str[20]={0};
	unsigned long burnmacval=0;
	unsigned char outputbuf[13]={0};
	unsigned int macBuf[7]={0};
	unsigned char tmpchar[3]={0};
	unsigned int  tttt=0;
	if (argc < 2)
	{
		set_usage();
		return -1;
	}
	value=argv[1];
	strncpy(str,value,12);
	sscanf(str+4,"%08x",&burnmacval);
	sprintf(outputbuf,"%c%c%c%c%08x",str[0],str[1],str[2],str[3],burnmacval);
	sscanf(outputbuf,"%02x%02x%02x%02x%02x%02x",&macBuf[0],&macBuf[1],&macBuf[2],&macBuf[3],&macBuf[4],&macBuf[5]);
	flash_write_wlan_mac(macBuf);
	#if 1
	burnmacval=burnmacval+1;
	sprintf(outputbuf,"%c%c%c%c%08x",str[0],str[1],str[2],str[3],burnmacval);
	sscanf(outputbuf,"%02x%02x%02x%02x%02x%02x",&macBuf[0],&macBuf[1],&macBuf[2],&macBuf[3],&macBuf[4],&macBuf[5]);
	flash_write_lan_mac(macBuf);
	flash_write_wan_mac(macBuf);
	#endif
	printf("\nwrite ok");	
	
	return 0;
}
