#include <time.h>    //获取当前时间，用于系统时间。
#include <memory.h>
#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix标准函数定义*/
#include <sys/types.h>  /**/
#include <sys/stat.h>   /**/
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>
#include <termios.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdarg.h>
#include <net/if.h>

#define LOG_DEBUG "DEBUG"
#define LOG_TCP "TCP"
#define LOG_ERROR "ERROR"
#define LOG_PROCESS  "PROCESS"
#define LOG_AT_RESPONSE  "AT_RESPONSE"

#define wip_debug printf

#define LOG(level, format, ...) \
    do { \
        fprintf(stderr, "[%s|%s@%s,%d] " format "", \
            level, __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
    } while (0)


int main()
{
	unsigned char buf[128]={0};
	unsigned char stbuf[16]={0};
	FILE * fp = fopen("/etc/config/bindstatus.conf","r");
	if(fp == NULL)
	{
		FILE * notbindfp = fopen("/etc/notbind","r");
		fread(buf,1,128,notbindfp);
		fclose(notbindfp);
	}
	else
	{
		FILE * bindfp =NULL;
		fread(stbuf,1,1,fp);
		fclose(fp);
		if(stbuf[0]==1)  //suc 
		{
			bindfp = fopen("/etc/bindsuc","r");
		}
		else
		{
			bindfp = fopen("/etc/bindfail","r");
		}
		fread(buf,1,128,fp);
		fclose(bindfp);
	}
	printf("%s", buf);
}
