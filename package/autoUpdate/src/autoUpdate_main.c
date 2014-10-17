/*==============================================================================
//
// Project:
//		userapp-ddns module
//
//------------------------------------------------------------------------------
// Description:
//
//           main of the ddns part
==============================================================================*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "ddns.h"

int get_mib(char *val, char *mib)
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


int SYS_wan_ip;
static int powerStatus=0;
static int linkServer=0;
static int linkServerCount=0;

int main(int argc,char *argv[]) 
{
	struct userInfo_cxy info;
	char devsn[64]={0};
	char email[64]={0};
	char line[20]={0};
	int ret;
	FILE *getFp=NULL;
		
	if(argc!=7 && argc!=11 && argc!=8 && argc!=4)	{
		usage();
		return -1;
	}

	if(strcmp(argv[1],"-s") && strcmp(argv[1],"-m")&& strcmp(argv[1],"-o")&& strcmp(argv[1],"-a")) {
		usage();
		return -1;
	}
	sprintf(line, "%d\n", getpid());
	if ((getFp = fopen("/var/run/autoUpdate.pid", "w")) != NULL)
	{
		fwrite(line, strlen(line), 1, getFp);
		fclose(getFp);
	}
	info.service = (struct service_cxy *)find_service((char *)argv[2]);
	if(!strcmp(argv[1],"-s")) 
	{
		strcpy(info.mx, "0");
		strcpy(info.backmx,"0");
		info.updated_time=0;
		info.trytime=0;
		info.ip=inet_addr(argv[6]);
		SYS_wan_ip=inet_addr(argv[6]);
	}
	else if(!strcmp(argv[1],"-o")) 
	{
              get_mib(email,"BandUserEmail");
        	get_mib(devsn,"devsn");
        	powerStatus=atoi(argv[3]);
        	strcpy(info.host, email);
        	strcpy(info.usrname, "admin");
        	strcpy(info.usrpwd, "admin");
		strcpy(info.backmx, devsn);
	}
        else if(!strcmp(argv[1],"-a")) 
	{
             //printf("\r\ndsadas");
              strcpy(info.host, argv[3]);
              strcpy(info.usrname, argv[4]);
        	strcpy(info.usrpwd, argv[5]);
              info.wildcard = atoi(argv[6]);
                //printf("\r\nmac=%s--user=%s--pwd=%s--%d--%s\n",info.host,info.usrname,info.usrpwd,info.wildcard,info.service->name);
	}
	else {
		strcpy(info.mx, argv[7]);
		strcpy(info.backmx,argv[8]);
		info.updated_time = atoi(argv[9]);
		info.trytime = atoi(argv[10]);
	}

	if(info.ip)
		info.status = UPDATERES_OK;
	else
		info.status = UPDATERES_ERROR;


       sleep(1);
	ret = info.status = info.service->update_entry(&info);
       if(info.wildcard == 1)
       {
           if(ret == 0)
           {
                system("echo 1 > /etc/autoupflag");
           }
           else
           {
                system("echo 0 > /etc/autoupflag");
           }
       }
       else
       {
            
            if(ret == 0)
            {
                 system("echo 1 > /etc/autoupgradeOkflag");
            }
            else
            {
                 system("echo 0 > /etc/autoupgradeOkflag");
            }
       }
	return ret;
}



