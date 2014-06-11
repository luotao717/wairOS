#include "config.h"
//00:26:b9:1a:dd:ff

unsigned char current_bcd_timer[7];    //当前时间的BCD码，不能直接使用，必须调用函数，获取最新的BCD码才行。返回的是BYTE类型的。
extern int make_connection(char * url,int rport);
extern int send_package(int sockfd,unsigned char * sdata, int slen );
extern int read_package(int sockfd,unsigned char * dest_data,int maxlen);
// 0   1  2  3  4  5 6  7  8  9  10  11 12 13    14       15 16
//68  30 30 30 33 37 46 31 44 36 30 30 31  01    01       00 00              E6    16
//BC:D1:77:E5:4E:1B
unsigned char wan_mac[]={'B','C','D','1','7','7','E','5','4','E','1','B'};
int init_asypack(struct asy_package * p,unsigned char ccode, unsigned char iclass, unsigned int ilength,unsigned char eflag, unsigned char * idata);
int combine_data(unsigned char type,unsigned char cmdcode,unsigned char infocode,unsigned char * dest_data,int maxlength,unsigned char * idata,unsigned int datalen);
int main();


unsigned char * current_time_convert_bcd_code()
{
         int year = 0 ,month = 0 , day = 0 ,hour = 0,minute=0,second=0;
         time_t att_time =  time(NULL);
	     struct tm * atttm = localtime(&att_time);
	     year = atttm->tm_year+1900 ;
	     month = atttm->tm_mon+1  ;
	     day = atttm->tm_mday  ;
	     hour = atttm->tm_hour  ;
	     minute = atttm->tm_min;
	     second = atttm->tm_sec ;
         current_bcd_timer[0] = (year / 1000)*16 + ( year % 1000 )/100 ; //第一个byte
         current_bcd_timer[1] = ((year % 100)/10)*16 + ( year % 10 ) ; //第二个byte
         current_bcd_timer[2] =  month < 0x0A? month : 0x10+( month )-10;  //第三个byte
         current_bcd_timer[3] =  day < 0x0A ? day: day<0x14 ? day+6 : day<0x1E ? day+12: day == 0x1F? 0x31:0x30;
         current_bcd_timer[4] =  hour < 0x0A ? hour: hour<0x14 ? hour+6 : hour<0x1E ? hour+12: 0 ;
         current_bcd_timer[5] =  minute < 0x0A ? minute: minute<0x14 ? minute+6 : minute<0x1E ?
                                 minute+12: minute < 0x28 ? minute+18:minute <0x32?minute + 24 :minute< 0x3C ?minute + 30 : 0;
         current_bcd_timer[6] =  second < 0x0A ? second: second<0x14 ? second+6 : second<0x1E ?
                                 second+12: second < 0x28 ? second+18:second <0x32?second + 24 :second< 0x3C ?second + 30 : 0;
         wip_debug("Current BCD Date:%0X%0X%0X%0X%0X%0X%0X\n",current_bcd_timer[0],current_bcd_timer[1],current_bcd_timer[2],current_bcd_timer[3],
                         current_bcd_timer[4],current_bcd_timer[5],current_bcd_timer[6]);
         return  current_bcd_timer;
}


int init_asypack(struct asy_package * p,unsigned char ccode, unsigned char iclass, unsigned int ilength,unsigned char eflag, unsigned char * idata)
{
	p->header = 0x68;
	memcpy(p->macaddr,wan_mac,sizeof(p->macaddr));
	p->cmdcode = ccode;
	p->info_class = iclass;
	p->info_length[0] = ilength/255;
	p->info_length[1] = ilength%255;
	p->end_flag = eflag;
	p->info_data = idata;
	return 0 ;
}


int combine_data(unsigned char type,unsigned char cmdcode,unsigned char infocode,unsigned char * dest_data,int maxlength,unsigned char * idata,unsigned int datalen)
{
	int i ;
	struct asy_package * apack = (struct asy_package *)malloc(sizeof(struct asy_package));
	init_asypack(apack , cmdcode , infocode, datalen, 0x16, idata);
	apack->crc = 0xE6;
	dest_data[0] = apack->header ;
	memcpy(dest_data+1,apack->macaddr,sizeof(apack->macaddr));
	dest_data[13] = apack->cmdcode;
	dest_data[14] = apack->info_class;
	dest_data[15] = apack->info_length[0];
	dest_data[16] = apack->info_length[1];
	memcpy(dest_data+17,apack->info_data,datalen);
	//if(datalen>0)
	//	fprintf(stdout,"InfoData %d.\n",idata[0]);
	dest_data[17+datalen] = apack->crc;
	dest_data[18+datalen] = apack->end_flag;
	LOG(LOG_DEBUG,"combine_data %d\n",datalen);
	for(i = 0 ; i < 19+datalen; i++)
		printf("%0X ",dest_data[i]);
	printf("\n");
	free(apack);
	return 0 ;
}

int analysis_pack(unsigned char * src_data,int len,int sockfd)
{
	unsigned char *dest_data = NULL, *get_data =NULL,* urlp=NULL,*upgradecmd = NULL,*firmwareversion=NULL,*forbidip =NULL;  int ret=0,i,banflag ;
	unsigned char tmpbuf[128]={0};
	unsigned char cmdcode = src_data[13];
	unsigned char infocode = src_data[14];
	unsigned int datalen = src_data[15]*255+ src_data[16];
	if( cmdcode == 0x1)
	{
		switch (infocode)
		{
			case 0x4:   //reboot cmd
				dest_data = (unsigned char * ) malloc(100);
				printf("reboot cmd\n");
				tmpbuf[0]=1;
				combine_data(1,1,4,dest_data,100,tmpbuf,1);
				dest_data[13] = 0;
				for(i = 0 ; i < 19+1 ; i++)
					printf("%0X ",dest_data[i]);
				printf("\n");
				printf("Server Need Device Rebooting\n");
				ret = send_package(sockfd,dest_data, 19+1);
				system("reboot");
				break;
			default:
				break;
		}
	}
	if( cmdcode == 0x4)
	{
		switch (infocode)
		{
			case 0x1:	//动态设置认证
				dest_data = (unsigned char * ) malloc(100);
				int switchdata = src_data[17];
				FILE * fgws = fopen("/etc/config/gwswitch","r");
				if(fgws == NULL)
				{
					fgws = fopen("/etc/config/gwswitch","w+");
					tmpbuf[0] = 1;
					fwrite(tmpbuf,1,1,fgws);
					fclose(fgws);
				}else
				{
					fread(tmpbuf,1,1,fgws);
					fclose(fgws);
				}
				if(switchdata == 1 && tmpbuf[0] == 1)  //存储的,都是1
				{

				}else
				if(switchdata == 1 && tmpbuf[0] == 2)	//开启,原来存储的是关闭
				{
					system("wifidog-init start");
					fgws = fopen("/etc/config/gwswitch","w+");
					tmpbuf[0] = 1;
					fwrite(tmpbuf,1,1,fgws);
					fclose(fgws);
				}else
				if(switchdata == 2  && tmpbuf[0] == 1)		//服务器要求关闭，存储的是开启的
				{
					fgws = fopen("/etc/config/gwswitch","w+");
					system("wdctl stop");
					tmpbuf[0] = 2;
					fwrite(tmpbuf,1,1,fgws);
					fclose(fgws);
				}else
				if(switchdata == 2  && tmpbuf[0] == 2)		//服务器要求关闭，存储的是关闭的
				{
				}
				combine_data(1,4,1,dest_data,100,tmpbuf,1);
				for(i = 0 ; i < 19+1 ; i++)
					printf("%0X ",dest_data[i]);
				printf("\n");
				printf("Server Need %d\n",switchdata);
				ret = send_package(sockfd,dest_data, 19+1);
				break;
				//平台要求远程升级
			case 0x3:
				urlp = (unsigned char * ) malloc(128);
				upgradecmd = (unsigned char * ) malloc(512);
				firmwareversion = (unsigned char * ) malloc(32);
				//do with upgrade firmware
				memcpy(urlp,src_data+17,128);
				memcpy(firmwareversion,src_data+17+128,32);
				LOG(LOG_DEBUG,"URL %s,Version %s.\n",urlp,firmwareversion);
				system("echo NewUpdated >> /etc/config/updatefile");
				system("rm /etc/config/firmwareversion");
				sleep(1);
				snprintf(tmpbuf,sizeof(tmpbuf),"wget -P /tmp %s",urlp);
				system(tmpbuf);
				sleep(30);
				snprintf(tmpbuf,sizeof(tmpbuf),"echo %s >> /etc/config/firmwareversion",firmwareversion);
				system(tmpbuf);
				snprintf(upgradecmd,512,"/sbin/sysupgrade /tmp/*.bin");
				LOG(LOG_DEBUG,"upgradecmd %s\n",upgradecmd);
				system(upgradecmd);
				dest_data = (unsigned char * ) malloc(100);
				combine_data(1,4,3,dest_data,100,"\0x1",1);
				for(i = 0 ; i < 19+1 ; i++)
					printf("%0X ",dest_data[i]);
				printf("\n");
				ret = send_package(sockfd,dest_data, 19+1);
				free(urlp);free(firmwareversion);
				break;
			case 0x5:
				forbidip = (unsigned char * ) malloc(5);
				dest_data = (unsigned char * ) malloc(100);
				banflag = src_data[17];
				unsigned char urldata[64],totalbuf[69]={0};
				memcpy(urldata,src_data+22,64);
				memcpy(totalbuf,src_data+17,69);
				forbidip[0] = src_data[17];forbidip[1] = src_data[18];
				forbidip[2] = src_data[19];forbidip[3] = src_data[20];forbidip[4] = src_data[21];
				combine_data(1,4,5,dest_data,100,totalbuf,69);
				for(i = 0 ; i < 19+69 ; i++)
					printf("%0X ",dest_data[i]);
				printf("\n");
				printf("List Url %s. forbidip[0]  %d\n",urldata,forbidip[0] );
				ret = send_package(sockfd,dest_data, 19+69);
				char ipbuf[128]={0};
				char allbuf[2048]={0};
				if(forbidip[0] == 1)  //白名单
				{
				    printf("allow ip.\n");
					if(forbidip[1] != 0)	// IP不为空。
					{
						 int outfd = open("/etc/config/allow.list",O_APPEND|O_RDWR);
	    			     if(outfd<=0)
	    			     {
    			    	 	snprintf(tmpbuf,sizeof(tmpbuf),"%d.%d.%d.%d#",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
    			     		outfd = open("/etc/config/allow.list",O_CREAT |O_RDWR );
    			     		write(outfd,tmpbuf,strlen(tmpbuf));
    			     		close(outfd);
	    			     }
	    			     else
	    			     {
	    			    	snprintf(tmpbuf,sizeof(tmpbuf),"%d.%d.%d.%d#",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
	    			     	write(outfd,tmpbuf,strlen(tmpbuf));
	    			     	close(outfd);
	    			    }
						/*
						FILE * listfile = fopen("/etc/config/allow.list","r");
						if(listfile!=NULL)
						{
							fread(allbuf,1,2048,listfile);
							snprintf(tmpbuf,sizeof(tmpbuf),"%d.%d.%d.%d",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
							if(strstr(allbuf,tmpbuf) == 0 )
							{
								snprintf(ipbuf,sizeof(ipbuf),"echo %d.%d.%d.%d >> /etc/config/allow.list",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
								system(ipbuf);
							}
						}else
						{
							snprintf(ipbuf,sizeof(ipbuf),"echo %d.%d.%d.%d >> /etc/config/allow.list",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
							system(ipbuf);
						}
						*/
					}
					if(urldata[0] != 0)		//域名不为空
					{
					    printf("allow url.\n");
					    int outfd = open("/etc/config/allow.list",O_APPEND|O_RDWR);
	    			     if(outfd<=0)
	    			     {
    			    	 	snprintf(tmpbuf,sizeof(tmpbuf),"%s#",urldata);
    			     		outfd = open("/etc/config/allow.list",O_CREAT |O_RDWR );
    			     		write(outfd,tmpbuf,strlen(tmpbuf));
    			     		close(outfd);
	    			     }
	    			     else
	    			     {
	    			    	snprintf(tmpbuf,sizeof(tmpbuf),"%s#",urldata);
	    			     	write(outfd,tmpbuf,strlen(tmpbuf));
	    			     	close(outfd);
	    			    }
					    /*
						FILE * listfile = fopen("/etc/config/allow.list","w");
						if(listfile!=NULL)
						{
							fread(allbuf,1,2048,listfile);
							snprintf(tmpbuf,sizeof(tmpbuf),"%d.%d.%d.%d",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
							if(strstr(allbuf,tmpbuf) == 0 )
							{
								//snprintf(ipbuf,sizeof(ipbuf),"echo %s >> /etc/config/allow.list",urldata);
								//system(ipbuf);
								fwrite("");
							}
						}else
						{
							snprintf(ipbuf,sizeof(ipbuf),"echo %s >> /etc/config/allow.list",urldata);
							system(ipbuf);
						}
						*/
					}
					printf("execute cmd.\n");
					system("/bin/allow &");
					sleep(30);
					system("wdctl stop");
				}else
				{
					if(forbidip[1] != 0)	// IP不为空。
					{
						FILE * listfile = fopen("/etc/config/forbid.list","r");
						if(listfile!=NULL)
						{
							fread(allbuf,1,2048,listfile);
							snprintf(tmpbuf,sizeof(tmpbuf),"%d.%d.%d.%d",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
							if(strstr(allbuf,tmpbuf) == 0 )
							{
								snprintf(ipbuf,sizeof(ipbuf),"echo %d.%d.%d.%d >> /etc/config/forbid.list",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
								system(ipbuf);
							}
						}else
						{
							snprintf(ipbuf,sizeof(ipbuf),"echo %d.%d.%d.%d >> /etc/config/forbid.list",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
							system(ipbuf);
						}
					}
					if(urldata[0] != 0)		//域名不为空
					{
						FILE * listfile = fopen("/etc/config/forbid.list","r");
						if(listfile!=NULL)
						{
							fread(allbuf,1,2048,listfile);
							snprintf(tmpbuf,sizeof(tmpbuf),"%d.%d.%d.%d",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
							if(strstr(allbuf,tmpbuf) == 0 )
							{
								snprintf(ipbuf,sizeof(ipbuf),"echo %s >> /etc/config/forbid.list",urldata);
								system(ipbuf);
							}
						}else
						{
							snprintf(ipbuf,sizeof(ipbuf),"echo %s >> /etc/config/forbid.list",urldata);
							system(ipbuf);
						}
					}
					/*
					if(forbidip[1] != 0)	// IP不为空。
					{
						snprintf(ipbuf,sizeof(ipbuf),"echo %d.%d.%d.%d >> /etc/config/forbid.list",forbidip[1],forbidip[2],forbidip[3],forbidip[4]);
						system(ipbuf);
					}
					if(urldata[0] != 0)		//域名不为空
					{
						snprintf(ipbuf,sizeof(ipbuf),"echo %s >> /etc/config/forbid.list",urldata);
						system(ipbuf);
					}
					*/
					printf("execute cmd.\n");
					system("/bin/allow &");
					sleep(30);
					system("wdctl stop");
				}
				break;
			default: break;
		}
	}
	if( cmdcode == 0x2)
	{
		switch (infocode)
		{
			case 0x1:	//绑定操作
				fprintf(stdout,"Binding Operate : src_data[17] %d.\n",src_data[17]);
				if( src_data[17] == 0x1 )
					return 1;
				else if( src_data[17] == 0x2 )
					return -1;
				break;
			default:
				return -99; //错误的包
				break;
		}
	}
	if( cmdcode == 0x5)
	{
		int fnread,ret,i; unsigned char tmptimebuf[7]={0},tmpversionbuf[32]={0},totalbuf[45]={0};
		FILE * firmwarev = NULL,*firmwareupdatetime = NULL;
		switch (infocode)
		{
			case 0x1:	//查询版本号
				dest_data = (unsigned char * ) malloc(100);
				firmwarev = fopen("/etc/config/firmwareversion","r");
				if(firmwarev!=NULL)
				{fnread = fread(tmpversionbuf,1,32,firmwarev);	//读版本号
				fclose(firmwarev);}
				if(tmpversionbuf[fnread-1] == '\n')
					tmpversionbuf[fnread-1] = '\0';
				firmwareupdatetime = fopen("/etc/config/firmtime","r");
				if(firmwarev!=NULL)
				{fread(tmptimebuf,1,7,firmwareupdatetime);  //读时间
				fclose(firmwareupdatetime);}
				printf("read file finished\n");
				memcpy(totalbuf,tmpversionbuf,32);
				printf("cp data tmpversionbuf finished\n");
				memcpy(totalbuf+32,tmptimebuf,7);
				printf("totalbuf data finished\n");
				combine_data(1,5,1,dest_data,100,totalbuf,39);
				printf("cp data finished\n");
				for(i = 0 ; i < 19+39 ; i++)
					printf("%0X ",dest_data[i]);
				printf("\n");
				printf("Version %s.\n",tmpversionbuf);
				ret = send_package(sockfd,dest_data, 19+39);
				free(dest_data);
				return ret ;
			default:
				return -99; //错误的包
				break;
		}
	}
	get_data = (unsigned char * ) malloc(100);

	ret = read_package(sockfd,get_data,100);
	if(ret <=0)
	{
		LOG(LOG_DEBUG,"Read error.\n");
		free(dest_data);
		free(get_data);
		return -1;
	}
	LOG(LOG_DEBUG,"read data length %d.\n",ret);
	for(i = 0 ; i < ret; i++)
	printf("%0X ",get_data[i]);
	printf("\n");
	free(dest_data);
	free(get_data);
	return 0;
}

#include <sys/stat.h>

unsigned long get_file_size(const char *path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	if(stat(path, &statbuff) < 0){
		return filesize;
	}else{
		filesize = statbuff.st_size;
	}
	return filesize;
}

char macdata[20];
int main()
{
	int i ,sockfd,ret,timeout_count = 0; time_t lastheart_time ;
	unsigned char * dest_data = NULL;
	unsigned char * get_data = NULL;
	unsigned char tmpbuf[512]={0},log_buf[512]={0};
	FILE * macfile = popen("ifconfig br-lan","r");
	fread(tmpbuf,1,sizeof(tmpbuf)-1,macfile);
	pclose(macfile);
	char * point = strstr(tmpbuf,"HWaddr ");
	memcpy(macdata,point+7,17);
	if(macdata[strlen(macdata)-1] == '\n')
		macdata[strlen(macdata)-1] = '\0';
	wan_mac[0] = macdata[0];wan_mac[1] = macdata[1];
	wan_mac[2] = macdata[3];wan_mac[3] = macdata[4];
	wan_mac[4] = macdata[6];wan_mac[5] = macdata[7];
	wan_mac[6] = macdata[9];wan_mac[7] = macdata[10];
	wan_mac[8] = macdata[12];wan_mac[9] = macdata[13];
	wan_mac[10] = macdata[15];wan_mac[11] = macdata[16];
	LOG(LOG_DEBUG,"WanMac %s.\n",wan_mac);
	//unsigned char * bcd_code = current_time_convert_bcd_code();
	//FILE * firmwareupdatetime = fopen("/etc/config/firmtime","w+");
	//fwrite(current_bcd_timer,1,7,firmwareupdatetime);
	//fclose(firmwareupdatetime);
#ifdef _TEST_CASE
	//unsigned char buf[]={0x68 ,0x42  ,0x44 ,0x44 ,0x31 ,0x37 ,0x37 ,0x45 ,0x35 ,0x34 ,0x45 ,0x31 ,0x42 ,0x5 ,0x1 ,0x0 ,0x0 ,0x3D ,0x16 };
	//unsigned char buf[]={0x68 ,0x42,0x44,0x44,0x31,0x37 ,0x37 ,0x45 ,0x35 ,0x34 ,0x45 ,0x31,0x42,0x4,0x1,0x0,0x1,0x0,0xE6,0x16};
	//unsigned char buf[]={0x68 ,0x42, 0x44 ,0x44 ,0x31 ,0x37 ,0x37 ,0x45, 0x35, 0x34, 0x45 ,0x31 ,0x42, 0x0, 0x1 ,0x0 ,0x1 ,0x1, 0x3A ,0x16};
	//unsigned char buf[]={0x68,0x62,0x63,0x64,0x31,0x37,0x37,0x65,0x35,0x34,0x65,0x31,0x62,0x1,0x4,0x0,0x0,0xFB,0x16 };
	unsigned char buf[]={0x68 ,0x62 ,0x63 ,0x64 ,0x31 ,0x37 ,0x37 ,0x65 ,0x35 ,0x34 ,0x65 ,0x31 ,0x62 ,0x4 ,0x5 ,0x0 ,0x45 ,0x1 ,0x0 ,0x0 ,0x0 ,0x0 ,0x77,0x77 ,0x77 ,0x2E ,0x62 ,0x61 ,0x69 ,0x64 ,0x75 ,0x2E ,0x63 ,0x6F ,0x6D ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x4B ,0x16};
	analysis_pack(buf,88,0);
#endif
	char serveripdata[64]="121.15.200.241";
	char portbuf[16]="8087";
	int sport = atoi("8087");
	char username[128]="wawifi";
	char passwd[128]="123456";
	char authserverdata[128]="oau.wawifi.cn";
	FILE * serveripfile  = fopen("/etc/config/serverip.conf","r");
	if(serveripfile!=NULL)
	{
		memset(serveripdata,'\0',sizeof(serveripdata));
		fread(serveripdata,1,sizeof(serveripdata),serveripfile);
		fclose(serveripfile);
	}
	FILE * aufile  = fopen("/etc/config/authserver.conf","r");
	if(aufile!=NULL)
	{
		memset(authserverdata,'\0',sizeof(authserverdata));
		fread(authserverdata,1,sizeof(authserverdata),aufile);
		fclose(aufile);
	}
	FILE * pfile  = fopen("/etc/config/serverport.conf","r");
	if(aufile!=NULL)
	{
		memset(portbuf,'\0',sizeof(portbuf));
		fread(portbuf,1,sizeof(portbuf),pfile);
		fclose(pfile);
	}
	FILE * unamefile  = fopen("/etc/config/username.conf","r");
	if(unamefile!=NULL)
	{
		memset(username,'\0',sizeof(username));
		fread(username,1,sizeof(username),unamefile);
		fclose(unamefile);
	}	
	FILE * pwdfile  = fopen("/etc/config/userpwd.conf","r");
	if(pwdfile!=NULL)
	{
		memset(passwd,'\0',sizeof(passwd));
		fread(passwd,1,sizeof(passwd),pwdfile);
		fclose(pwdfile);
	}	
	LOG(LOG_DEBUG,"Load %s  %s  %s  %s  %s.\n",serveripdata,portbuf,username,passwd,authserverdata);
	
	START:
	dest_data = (unsigned char * ) malloc(100);
	get_data = (unsigned char * ) malloc(100);
	combine_data(1,1,1,dest_data,100,"",0);
	for(i = 0 ; i < 19 ; i++)
	printf("%0X ",dest_data[i]);
	printf("\n");
	sleep(10);
	printf("Start Connection.\n");
	lastheart_time = time(NULL);
	sockfd = make_connection(serveripdata,atoi(portbuf));
	if(sockfd <=0)
		{perror("Connection error.\n");close(sockfd);goto START;}
	ret = send_package(sockfd,dest_data, 19+0);
	if(ret <=0)
		{perror("Send error.\n");close(sockfd);goto START;}
	
	LOG(LOG_DEBUG,"send data length %d.\n",ret);
	ret = read_package(sockfd,get_data,100);
	if(ret <=0)
		{perror("Read error out timeout.\n");close(sockfd);goto START;}
	lastheart_time = time(NULL);
	LOG(LOG_DEBUG,"read data length %d.\n",ret);
	for(i = 0 ; i < ret; i++)
	printf("%0X ",get_data[i]);
	printf("\n");
	free(dest_data);
	free(get_data);
	LOG(LOG_DEBUG,"If is Already Updated.\n");
	FILE * upgradeconfig = fopen("/etc/config/updatefile","r");
	if(upgradeconfig!= NULL)
	{
		int fnread;
		LOG(LOG_DEBUG,"Upload Data To Server.\n");
		unsigned char * bcd_code = current_time_convert_bcd_code();
		unsigned char tmptimebuf[64]={0};
		tmptimebuf[0] = 1;
		FILE * firmwarev = fopen("/etc/mrbaron_version","r");
		if(firmwarev == NULL)
			snprintf(tmptimebuf+1,30,"version not exist");
		else	
			fnread = fread(tmptimebuf+1,1,30,firmwarev);
		fclose(firmwarev);
		if(tmptimebuf[1+fnread] == '\n')
			tmptimebuf[1+fnread] = '\0';
		memcpy(tmptimebuf+33,current_bcd_timer,7);
		FILE * firmwareupdatetime = fopen("/etc/config/firmtime","w+");
		fwrite(current_bcd_timer,1,7,firmwareupdatetime);
		fclose(firmwareupdatetime);
		dest_data = (unsigned char * ) malloc(100);
		combine_data(1,3,3,dest_data,100,tmptimebuf,40);
		for(i = 0 ; i < 19+40 ; i++)
			printf("%0X ",dest_data[i]);
		printf("\n");
		ret = send_package(sockfd,dest_data, 19+40);
		free(dest_data);
		system("rm /etc/config/updatefile");
		fclose(upgradeconfig);
	}else
	{
		LOG(LOG_DEBUG,"Update File Not Exist.\n");
	}
  timeout_count = 0 ;
	do{
		if(get_file_size("/etc/socket.log") > 1000000)
		{	system("rm /etc/socket.log");
			exit(0);
		}
		timeout_count ++;
		if(timeout_count > 30)
		{
				LOG(LOG_DEBUG,"Socket Is Timeout Need Reconnect.\n");
				close(sockfd);
				goto START;
		}
		dest_data = (unsigned char * ) malloc(800);
		get_data = (unsigned char * ) malloc(800);
		fprintf(stdout,"timeout_count %d,HeartBeat:%d\n",timeout_count,(int)(time(NULL) - lastheart_time));
		FILE * fillreg = fopen("/tmp/reg.atonce","r");
		if( fillreg != NULL )
		{
			fclose(fillreg);
			sleep(1);
			system("rm /tmp/reg.atonce");
			dest_data = (unsigned char * ) malloc(200);
			get_data = (unsigned char * ) malloc(200);
			ret = read_package(sockfd,get_data,100);
			if(ret <=0)
			{LOG(LOG_DEBUG,"Read Time Out.\n");}
			
			for(i = 0 ; i < ret; i++)
			printf("%0X ",get_data[i]);
			printf("\n");
			unsigned char * buftotal = (unsigned char * ) malloc(96); int rg1 = 0;
			unsigned char acc[128];
			unsigned char pwd[128];
			unsigned char version[128]= "1.0.0";
			memcpy(acc,username,128);
			memcpy(pwd,passwd,128);
			memset(buftotal,'\0',96);
			for(rg1 = 0 ; rg1 < strlen(acc); rg1++)
				buftotal[rg1] = acc[rg1];
			for(rg1 = 0 ; rg1 < strlen(pwd); rg1++)
				buftotal[32+rg1] = pwd[rg1];
			for(rg1 = 0 ; rg1 < strlen(version); rg1++)
				buftotal[64+rg1] = version[rg1];
			combine_data(1,2,1,dest_data,200,buftotal,96);
			LOG(LOG_DEBUG,"Send Reg Code.\n");
			for(i = 0 ; i < 19+96; i++)
				printf("0x%0X ",dest_data[i]);
			printf("\n");
			ret = send_package(sockfd,dest_data, 19+96);
			ret = read_package(sockfd,get_data,200);
			
			LOG(LOG_DEBUG,"Read Reg BackData. %d.\n",ret);
			for(i = 0 ; i < ret; i++)
				printf("0x%0X ",get_data[i]);
			printf("\n");
			char savebuf[16]={0};
			if(analysis_pack(get_data,ret,sockfd) == 1)
				{
					savebuf[0] = 1;
					FILE * fs = fopen("/etc/config/bindstatus.conf","w+");
					fwrite(savebuf,1,1,fs);
					fclose(fs);
					LOG(LOG_DEBUG,"analysis_pack success,binding user success.\n");
				}
			else
				{
					savebuf[0] = 2;
					FILE * fs = fopen("/etc/config/bindstatus.conf","w+");
					fwrite(savebuf,1,1,fs);
					fclose(fs);
					LOG(LOG_DEBUG,"binding user failed.\n");
				}
			free(dest_data);
			free(get_data);
			continue;
		}
		// 新增
		FILE * logonfp = fopen("/tmp/logontime.data","r");
		if(logonfp!=NULL)
		{
			char logonbuf[1024]={0};
			fread(logonbuf,1,sizeof(logonbuf),logonfp);
			fclose(logonfp);
			int m1,m2,m3,m4,ip1,ip2,ip3,ip4;
			char delim[]="\r\n",*point = NULL , time[16]={0}, upmac[32]={0},upip[32]={0},tmpipb[5]; unsigned char totaldest[128]={0},respbuf[800]={0};
			point = strtok(logonbuf,delim);
			while(point!=NULL)
			{
				for( m1=0 ; m1 < 10 ; m1++ )
				{
					time[m1] = point[m1] ;
				}
				//c8:3a:35:c6:4d:e7
				upmac[0] = point[11];
				upmac[1] = point[12];
				upmac[2] = point[14];
				upmac[3] = point[15];
				upmac[4] = point[17];
				upmac[5] = point[18];
				upmac[6] = point[20];
				upmac[7] = point[21];
				upmac[8] = point[23];
				upmac[9] = point[24];
				upmac[10] = point[26];
				upmac[11] = point[27];
				
				for( m2=0 ; point[29+m2] != '\0' &&  m2 < 15 ; m2++ )
				{

					upip[m2] = point[29+m2] ;
				}
				for(m3 = 0 ,m4 =0; m4 < 5 && upip[m3]!='.' ; m3++,m4++ )
				{
					tmpipb[m4] = upip[m3];
				}
				ip1 = atoi(tmpipb);
				memset(tmpipb,'\0',sizeof(tmpipb));m3++ ;
				for(m4= 0; m4 < 5 && upip[m3]!='.' ; m3++,m4++ )
				{
					tmpipb[m4] = upip[m3];
				}
				ip2 = atoi(tmpipb);
				memset(tmpipb,'\0',sizeof(tmpipb));
				for(m3++ ,m4= 0; m4 < 5 && upip[m3]!='.' ; m3++,m4++ )
				{
					tmpipb[m4] = upip[m3];
				}
				ip3 = atoi(tmpipb);
				memset(tmpipb,'\0',sizeof(tmpipb));
				for(m3++ ,m4= 0; m4 < 5 && upip[m3]!='.' ; m3++ ,m4++)
				{
					tmpipb[m4] = upip[m3];
				}
				ip4 = atoi(tmpipb);
				memcpy(totaldest,upmac,12);
				totaldest[12]= ip1;totaldest[13]= ip2;totaldest[14]= ip3;totaldest[15]= ip4;totaldest[16]=1;
				memcpy(totaldest+17,time,10);
				
				LOG(LOG_DEBUG,"Send Logon Data.\n");
				combine_data(1,3,1,dest_data,800,totaldest,45);
				ret = send_package(sockfd,dest_data, 19+45);
				ret = read_package(sockfd,respbuf,800);
				if(ret > 0  )
					LOG(LOG_DEBUG,"read data length %d.\n",ret);	
				for(i = 0 ; i < ret; i++)
				printf("%0X ",respbuf[i]);
				printf("\n");
				point = strtok(NULL,delim);
			}
			system("rm /tmp/logontime.data");
			usleep(100000);
		}
		
		// 新增发送下线指令
		FILE * logoutfp = fopen("/tmp/logouttime.data","r");
		if(logoutfp!=NULL)
		{
			char logonbuf[1024]={0};
			fread(logonbuf,1,sizeof(logonbuf),logoutfp);
			fclose(logoutfp);
			int m1,m2,m3,m4,ip1,ip2,ip3,ip4;
			char delim[]="\r\n",*point = NULL , time[16]={0}, upmac[32]={0},upip[32]={0},tmpipb[5]; unsigned char totaldest[128]={0},respbuf[800]={0};
			point = strtok(logonbuf,delim);
			while(point!=NULL)
			{
				for( m1=0 ; m1 < 10 ; m1++ )
				{
					time[m1] = point[m1] ;
				}
				//c8:3a:35:c6:4d:e7
				upmac[0] = point[11];
				upmac[1] = point[12];
				upmac[2] = point[14];
				upmac[3] = point[15];
				upmac[4] = point[17];
				upmac[5] = point[18];
				upmac[6] = point[20];
				upmac[7] = point[21];
				upmac[8] = point[23];
				upmac[9] = point[24];
				upmac[10] = point[26];
				upmac[11] = point[27];
				
				for( m2=0 ; point[29+m2] != '\0' &&  m2 < 15 ; m2++ )
				{
					upip[m2] = point[29+m2] ;
				}
				for(m3 = 0 ,m4 =0; m4 < 5 && upip[m3]!='.' ; m3++,m4++ )
				{
					tmpipb[m4] = upip[m3];
				}
				ip1 = atoi(tmpipb);
				memset(tmpipb,'\0',sizeof(tmpipb));m3++ ;
				for(m4= 0; m4 < 5 && upip[m3]!='.' ; m3++,m4++ )
				{
					tmpipb[m4] = upip[m3];
				}
				ip2 = atoi(tmpipb);
				memset(tmpipb,'\0',sizeof(tmpipb));
				for(m3++ ,m4= 0; m4 < 5 && upip[m3]!='.' ; m3++,m4++ )
				{
					tmpipb[m4] = upip[m3];
				}
				ip3 = atoi(tmpipb);
				memset(tmpipb,'\0',sizeof(tmpipb));
				for(m3++ ,m4= 0; m4 < 5 && upip[m3]!='.' ; m3++ ,m4++)
				{
					tmpipb[m4] = upip[m3];
				}
				ip4 = atoi(tmpipb);
				memcpy(totaldest,upmac,12);
				totaldest[12]= ip1;totaldest[13]= ip2;totaldest[14]= ip3;totaldest[15]= ip4;totaldest[16]=2;
				memcpy(totaldest+31,time,10);
				
				LOG(LOG_DEBUG,"Send Logout Data.\n");
				combine_data(1,3,1,dest_data,800,totaldest,45);
				ret = send_package(sockfd,dest_data, 19+45);
				ret = read_package(sockfd,respbuf,800);
				if(ret > 0  )
					LOG(LOG_DEBUG,"read data length %d.\n",ret);	
				for(i = 0 ; i < ret; i++)
				printf("%0X ",respbuf[i]);
				printf("\n");
				point = strtok(NULL,delim);
			}
			system("rm /tmp/logouttime.data");
			usleep(100000);
		}
		
		if( time(NULL) - lastheart_time > 60  )
		{
			LOG(LOG_DEBUG,"Send HeartBeat Data.\n");
			combine_data(1,1,3,dest_data,800,"",0);
			ret = send_package(sockfd,dest_data, 19);
			ret = read_package(sockfd,get_data,800);
			if(ret == -99 || ret == 0 )//critical
			{
				close(sockfd);
				free(get_data);
				free(dest_data);
				goto START;
			}
			else if(ret == -1)  //timeout
			{
				LOG(LOG_DEBUG,"Read HeartBeat timeout.\n");
				free(get_data);
				free(dest_data);continue;
			}
			LOG(LOG_DEBUG,"Read HeartBeat bak.\n");
			timeout_count = 0 ;
			lastheart_time = time(NULL);
			free(get_data);
			free(dest_data);
			continue;
		}
		ret = read_package(sockfd,get_data,800);
		if(ret == -99 || ret == 0 )//critical
		{	close(sockfd);
			free(get_data);
			free(dest_data);
			goto START;
		}
		else if(ret == -1)  //timeout
			{//LOG(LOG_DEBUG,"Read timeout.\n");
			free(get_data);
			free(dest_data);continue;
			}
		LOG(LOG_DEBUG,"read data length %d.\n",ret);
		for(i = 0 ; i < ret; i++)
		printf("%0X ",get_data[i]);
		printf("\n");
		lastheart_time = time(NULL);
		analysis_pack(get_data,ret,sockfd);
		free(get_data);
		free(dest_data);
	}while(1);
}
