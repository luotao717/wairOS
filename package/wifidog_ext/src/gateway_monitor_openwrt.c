#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main()
{
			unsigned char psbuf[5120],tmpbuf[128]; int gwswitch,x1 = 0  ;
			/*
			FILE *tempfp=NULL;

			if(tempfp=fopen("/etc/wifidog/bindstatus","r") == NULL)
			{
				printf("\r\n no bind so exit auth");
				return -1;
			}
			fgets(tmpbuf,sizeof(tmpbuf),tempfp);
			if(strstr(tmpbuf,"status:1")== NULL)
			{
				printf("\r\n bind error so exit auth");
				fclose(tempfp);
				return -1;
			}
			fclose(tempfp);
			*/
			system("/bin/allow &");
			sleep(20);
			system("wifidog-init start");
			
} 