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

char auth_log[1024]="GET /logout.aspx?mac=%s&gw_mac=%s&ip=%s&token=%s  HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; Tablet PC 2.0; .NET4.0C; .NET4.0E; InfoPath.2)\r\nAccept-Encoding: gzip, deflate\r\nHost: oau.wawifi.cn:80\r\nConnection: Keep-Alive\r\n\r\n";
char heartbeat[1024]="POST /login.aspx HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/4.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; Tablet PC 2.0; .NET4.0C; .NET4.0E; InfoPath.2)\r\nContent-Length: %d\r\nHost: 115.28.41.113:80\r\nConnection: Keep-Alive\r\n\r\n%s";

char send_buf[2048];
char buf[1024];
char mac[64],ip[64];
char saved_log[100][64];
int save_sequence = 0;

int send_http(char * mac,char *gw_mac,char * uip,char* token)
{
   int sockfd, numbytes , j = 0,k,l,m, flag = 0 ,fnread = 0;char *point ;  
    struct hostent *he;
    struct sockaddr_in their_addr;
    unsigned int myport;
    char clientbuf[5120],tmpdata[256],gwid[256];
    myport = 80;
    
	if((he=gethostbyname("oau.wawifi.cn"))==NULL) {
	    herror("gethostbyname");
	    return -1;
	}
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
       perror("socket");
       return -1;
	}

		    their_addr.sin_family=PF_INET;
		    their_addr.sin_port=htons(myport);
		    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
		    bzero(&(their_addr.sin_zero),0);
			
		    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
		        perror("connect");
		        return -1;
		    }
		   	snprintf(send_buf,sizeof(send_buf),auth_log,mac,gw_mac,uip,token);	
		    if ((numbytes=send(sockfd, send_buf, strlen(send_buf), 0)) == -1) {
		        perror("send");
		        close(sockfd);
		        return -1;
		    }
			fprintf(stdout,"send buf %s\n",send_buf);
		    if ((numbytes=recv(sockfd, buf, 1024, 0)) == -1) {
		        perror("recv");
		        close(sockfd);
		        return -1;
		    }
		    buf[numbytes] = 0;
		    printf("Received: %s\n",buf);
		    close(sockfd);
		    return 1;
}

int main()
{
	char  outfilebuf[10240],*point=NULL;int i=0,j=0;
	char usermac[20]={0},userip[20]={0},usertoken[128]={0},wanmac[20]={0};
	//if(fork()==0)
	{
		while(1)
		{
			sleep(30);
			//starttime,client->mac,client->ip,client->token);
			//1
			//1383629466#34:23:ba:27:0e:ef#192.168.2.128#c0b5af0ce6c180a8fc95afdb1406c401
			FILE * fout = fopen("/tmp/logouttime.data","r");
			if(fout == NULL)
			{
				fprintf(stdout,"File Open Failed /tmp/logouttime.data\n");
				continue;
			}
			fread(outfilebuf,1,sizeof(outfilebuf),fout);
			char delim[]="\n";
			point = strtok(outfilebuf,delim);
			while(point!=NULL)
			{
				
				memcpy(usermac,point+11,17);
				for(i = 29; i < 29+15 && point[i] != '#' ;i++)
				{
					userip[i-29] = point[i];
				}
				i++;
				for( j = i ; i < strlen(point)&& point[i] != '#' ;i++)
				{
					usertoken[i-j] = point[i];
				}
				i++;
				for( j = i ; i < strlen(point)&& point[i] != '#' ;i++)
				{
					wanmac[i-j] = point[i];
				}
				point = strtok(NULL,delim);
				fprintf(stdout,"Logout %s %s %s %s.\n",usermac,userip,usertoken,wanmac);
				if(send_http(usermac,wanmac,userip,usertoken)== 1)
					fprintf(stdout,"Send Logout OK.\n");
				else
					fprintf(stdout,"Send Logout Failed.\n");
			}
			system("rm /tmp/logouttime.data");
		}
	}
}



