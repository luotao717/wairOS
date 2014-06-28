#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main()
{
	if(fork()==0)
		{
			unsigned char psbuf[5120],tmpbuf[32]; int gwswitch,x1 = 0  ;
			system("/bin/allow &");
			sleep(50);
			
			FILE * fgws = fopen("/etc/config/gwswitch","r");
			if(fgws == NULL)
			{
				tmpbuf[0]=1;
				fprintf(stdout,"Creating gwswitch and saving.\n");
				fgws = fopen("/etc/config/gwswitch","w+");
				fwrite(tmpbuf,1,1,fgws);
				fclose(fgws);
			}else
			{
				fread(tmpbuf,1,1,fgws);
				fprintf(stdout,"tmpbuf %d.\n",tmpbuf[0]);
				fclose(fgws);
			}
			gwswitch = tmpbuf[0];
			fprintf(stdout,"gwswitch %d.\n",gwswitch);
			if(gwswitch == 1)
				system("wifidog-init start");
			system("/bin/rjyq  >> /tmp/jyqlog.txt &");
			system("/bin/outr &");
			FILE * wifidogfile = NULL; int nread  = 0 ;
			while(1)
			{
				x1++;
				fprintf(stdout,"[RebootStatus] x1 value %d.\n",x1);
				if(x1 > 2880)
				{
					system("reboot &");
				}
				memset(psbuf,'\0',sizeof(psbuf));
				sleep(30);
				system("rm /tmp/gw-status.log");
				sleep(1);
				system("ps >> /tmp/gw-status.log");
				sleep(1);
				wifidogfile = fopen("/tmp/gw-status.log","r");
				if(wifidogfile == NULL)
					continue;
				nread = fread(psbuf,1,5120,wifidogfile);	
				if(nread <= 0)
				{	fclose(wifidogfile); continue;}
				else
				{
					if(strstr(psbuf,"wifidog"))
						{
							sleep(28);
							if( strstr(psbuf,"rjyq") == 0 )
							{
								system("/bin/rjyq &");  //rjyq 程序挂了，重启
							}
							continue;
						}
					else
						{
							fgws = fopen("/etc/config/gwswitch","r");
							fread(tmpbuf,1,1,fgws);
							fclose(fgws);
							gwswitch = tmpbuf[0];
							if(gwswitch == 1){
								fprintf(stdout,"Restart Gateway.\n");
								system("wifidog-init start");
							}else
							{
								fprintf(stdout,"Server Set Gateway Down...\n");
							}
							if( strstr(psbuf,"rjyq") == 0 )
							{
								system("/bin/rjyq &");  //rjyq 程序挂了，重启
							}
						}
				}	
			}
		}
} 