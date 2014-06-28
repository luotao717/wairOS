
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
	flash_read_size_eeprom_mtk(tmpbuf11,0x2000);
	//printf("\r\nstr=%s",str);
	//printf("\n%s",str+4);	
	sscanf(str+4,"%08x",&burnmacval);
	//printf("\nvaltt=%08x--%d",burnmacval,burnmacval);
	sprintf(outputbuf,"%c%c%c%c%08x",str[0],str[1],str[2],str[3],burnmacval);
	sscanf(outputbuf,"%02x%02x%02x%02x%02x%02x",&macBuf[0],&macBuf[1],&macBuf[2],&macBuf[3],&macBuf[4],&macBuf[5]);
	//printf("\r\noutput=%s--%s--%d--%02x",outputbuf,tmpchar,macBuf[0],tttt);
	//printf("\r\n%02x-%02x-%02x-%02x-%02x-%02x",macBuf[0],macBuf[1],macBuf[2],macBuf[3],macBuf[4],macBuf[5]);
	#if 1
	tmpbuf11[0x0004]=macBuf[0];
	tmpbuf11[0x0005]=macBuf[1];
	tmpbuf11[0x0006]=macBuf[2];
	tmpbuf11[0x0007]=macBuf[3];
	tmpbuf11[0x0008]=macBuf[4];
	tmpbuf11[0x0009]=macBuf[5];
	
	burnmacval=burnmacval+1;
	sprintf(outputbuf,"%c%c%c%c%08x",str[0],str[1],str[2],str[3],burnmacval);
	sscanf(outputbuf,"%02x%02x%02x%02x%02x%02x",&macBuf[0],&macBuf[1],&macBuf[2],&macBuf[3],&macBuf[4],&macBuf[5]);
	tmpbuf11[0x28]=macBuf[0];
	tmpbuf11[0x29]=macBuf[1];
	tmpbuf11[0x2a]=macBuf[2];
	tmpbuf11[0x2b]=macBuf[3];
	tmpbuf11[0x2c]=macBuf[4];
	tmpbuf11[0x2d]=macBuf[5];
	burnmacval++;
	sprintf(outputbuf,"%c%c%c%c%08x",str[0],str[1],str[2],str[3],burnmacval);
	sscanf(outputbuf,"%02x%02x%02x%02x%02x%02x",&macBuf[0],&macBuf[1],&macBuf[2],&macBuf[3],&macBuf[4],&macBuf[5]);
	tmpbuf11[0x2e]=macBuf[0];
	tmpbuf11[0x2f]=macBuf[1];
	tmpbuf11[0x30]=macBuf[2];
	tmpbuf11[0x31]=macBuf[3];
	tmpbuf11[0x32]=macBuf[4];
	tmpbuf11[0x33]=macBuf[5];
	#endif
	flash_write_all_mac_mtk(tmpbuf11);
	printf("\nwrite ok");
	
	return 0;
}