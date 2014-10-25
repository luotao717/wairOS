#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
  
struct clientInfo{
	char ip[16];
	char mac[20];
	char control;
	char always;     // 0 is  forever
	unsigned int passtime;
	unsigned int onlinetime;
	struct clientInfo *next;
};

struct clientInfo clientArr[254]={0};

static char *saved_pidfile;

static int get_mib(char *val, char *mib)
{
        FILE *fp;
         char buf[32];

        sprintf(buf, "nvram_get 2860 %s", mib);
            fp = popen(buf, "r");
        if (fp==NULL)
                return -1;

        if (NULL == fgets(buf, sizeof(buf),fp)) {
                pclose(fp);
                return -1;
        }

        //strcpy(val, strstr(buf, "\"")+1);
        strcpy(val, buf);
        val[strlen(val)-1] = '\0';
        pclose(fp);
        return 0;
}

static void pidfile_delete(void)
{
	if (saved_pidfile) unlink(saved_pidfile);
}

int pidfile_acquire(const char *pidfile)
{
	int pid_fd;
	if (!pidfile) return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) {
		printf("Unable to open pidfile %s: %m\n", pidfile);
	} else {
		lockf(pid_fd, F_LOCK, 0);
		if (!saved_pidfile)
			atexit(pidfile_delete);
		saved_pidfile = (char *) pidfile;
	}
	return pid_fd;
}

void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if (pid_fd < 0) return;

	if ((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

int main(int argc,char **argv)
{
	pid_t pid;
	int fd;
	int i=0,j=0;
	struct stat filestat;
	char cmdbuf[256]={0};
	char devMac[36]={0};
	char devSn[48]={0};
	char devUserEmail[64]={0};
  time_t timep;
  struct tm *nowtime=NULL;
  FILE *fh=NULL;
  char gwidmac[36]={0};
  char softVer[64]="1000";
  int upCount=30;
  int upgradeFlag=-1;
  int wgetCount=30;
  int retUpEnable=0;
  char retSoftVer[64]="1000";
  char retUrl[256]={0};
  char retMd5[65]={0};
  unsigned long retSize=0;
  int retIsFac=0;
  char retCmdid[65]={0};
  char wgetBuf[512]={0};
  struct stat statbuf;
	
	fd = pidfile_acquire("/var/autoUpgradedaemon.pid");
	pidfile_write_release(fd);
	while (1) 
	{
		time(&timep);
    nowtime=localtime(&timep);
    //printf("\r\n now time = %d:%d:%d",nowtime->tm_hour,nowtime->tm_min,nowtime->tm_sec );
    if(nowtime->tm_hour == 5)
    {
        if((nowtime->tm_min == 20) && (nowtime->tm_sec <= 51))
        {
        		if ((fh = fopen("/etc/gwid.conf", "r"))) 
        		{
							fscanf(fh, "%s", gwidmac);
							fclose(fh);
						}
						if ((fh = fopen("/usr/bin/softVersion", "r"))) 
        		{
							fscanf(fh, "%s", softVer);
							fclose(fh);
						}
						sprintf(cmdbuf,"autoUpdate -a fpUpCheck %s %s admin 1 &",gwidmac,softVer);
        		system(cmdbuf);
        		for(i=0;i<upCount;i++)
        		{
        				sleep(10);
						printf("\r\n start upgrade %d",i);
        				if ((fh = fopen("/etc/autoupflag", "r"))) 
        				{
									fscanf(fh, "%d", &upgradeFlag);
									fclose(fh);
									if(upgradeFlag != 1)
										continue;
									if ((fh = fopen("/tmp/upcheckResult", "r"))) 
									{
										fscanf(fh,"%d %s %s %lu %s %d %s", &retUpEnable,retSoftVer,retUrl,&retSize,retMd5,&retIsFac,retCmdid);
										fclose(fh);
										printf("\r\nuppara=%d %s %s %lu %s %d %s\r\n", retUpEnable,retSoftVer,retUrl,retSize,retMd5,retIsFac,retCmdid);
										if(0==retUpEnable)
										{
											break;
										}
										system("rm -f /tmp/newfireware");
										sprintf(wgetBuf,"wget %s -O /tmp/newfireware &",retUrl);
										system(wgetBuf);
										for(j=0;j<wgetCount;j++)
										{
											sleep(10);
											//printf("\r\nfdsfsadfkkk---%d\r\n",j);
											if(stat("/tmp/newfireware", &statbuf) == -1)
											{
												continue;
											}
											if(statbuf.st_size == retSize)
											{
												break;
											}
										}
										if(j<wgetCount)
										{
											//system("echo yyyy > /tmp/555555");
											printf("\r\ndebug start upgrade\r\n");
											if(retIsFac == 0)
											{
												system("sysupgrade -q -d 10 /tmp/newfireware &");
												//system("sysupgrade -n -q -d 10 /tmp/newfireware &");
											}
											else
											{
												system("sysupgrade -n -q -d 10 /tmp/newfireware &");
												
											}
											sleep(30);
											//system("sysupgrade -n -q -d 10 /tmp/newfireware &");
										}
										
									}
									else
									{
										break;
									}
								}
								else
								{
									continue;
								}
        		}
        }
    }
		sleep(10);
	}
	return 0;
}

