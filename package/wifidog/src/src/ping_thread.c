/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/* $Id: ping_thread.c 1373 2008-09-30 09:27:40Z wichert $ */
/** @file ping_thread.c
    @brief Periodically checks in with the central auth server so the auth
    server knows the gateway is still up.  Note that this is NOT how the gateway
    detects that the central server is still up.
    @author Copyright (C) 2004 Alexandre Carmel-Veilleux <acv@miniguru.ca>
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#include "../config.h"
#include "safe.h"
#include "common.h"
#include "conf.h"
#include "debug.h"
#include "ping_thread.h"
#include "util.h"
#include "centralserver.h"
#include "firewall.h"
#include "fw_iptables.h"
#include "client_list.h"


static void ping(int * update_sec);
static void register_our(void);
static void bound_our(void);
static void authtime_control(void);




extern time_t started_time;

/** Launches a thread that periodically checks in with the wifidog auth server to perform heartbeat function.
@param arg NULL
@todo This thread loops infinitely, need a watchdog to verify that it is still running?
*/  
void
thread_ping(void *arg)
{
	pthread_cond_t		cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t		cond_mutex = PTHREAD_MUTEX_INITIALIZER;
	struct	timespec	timeout;
	int update_interval = config_get_config()->checkinterval;	
	
	
	while (1) {
		/* Make sure we check the servers at the very begining */
		debug(LOG_WARNING, "Running ping()");
		ping(&update_interval);
            register_our();
            //bound_our();
		
		/* Sleep for config.checkinterval seconds... */
		timeout.tv_sec = time(NULL) + update_interval;// config_get_config()->checkinterval;
		timeout.tv_nsec = 0;

		/* Mutex must be locked for pthread_cond_timedwait... */
		pthread_mutex_lock(&cond_mutex);
		
		/* Thread safe "sleep" */
		pthread_cond_timedwait(&cond, &cond_mutex, &timeout);

		/* No longer needs to be locked */
		pthread_mutex_unlock(&cond_mutex);
	}
}

void thread_authtime(void *arg)
{
	pthread_cond_t		cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t		cond_mutex = PTHREAD_MUTEX_INITIALIZER;
	struct	timespec	timeout;
	int update_interval = 60;	
	
	
	while (1) {
		/* Make sure we check the servers at the very begining */
		debug(LOG_WARNING, "Running authtime()");
		authtime_control();
		
		/* Sleep for config.checkinterval seconds... */
		timeout.tv_sec = time(NULL) + update_interval;// config_get_config()->checkinterval;
		timeout.tv_nsec = 0;

		/* Mutex must be locked for pthread_cond_timedwait... */
		pthread_mutex_lock(&cond_mutex);
		
		/* Thread safe "sleep" */
		pthread_cond_timedwait(&cond, &cond_mutex, &timeout);

		/* No longer needs to be locked */
		pthread_mutex_unlock(&cond_mutex);
	}
}


/*
 * get the xml value 
 * name length : 24
 * outlen: input : max of value
 * 	   output: lenghth of value
 */
static int get_xml_tval(char *origin_str, char* name, char *value, int * outlen)
{
	char tmp[32] = {0};
	int len = 0;
	char *phead=NULL, *pend=NULL;

	if ( strlen(name) > 24 ) {
		return -1;
	}

	// head <>
	len = snprintf(tmp, sizeof(tmp), "<%s>", name );
	phead = strstr( origin_str, tmp);
	if ( phead == NULL ) {
		return -1;
	}	
	phead += len;

	// end </>
	memset( tmp, 0, sizeof(tmp));
	len = snprintf(tmp, sizeof(tmp), "</%s>", name);
	pend = strstr( origin_str, tmp );
	if ( pend == NULL ) {
		return -1;
	}

	if ( phead > pend ) {
		return -1;
	}

	if ( phead == pend ) {
		*outlen = 0;
	}
	else {
		len = (( pend - phead ) < *outlen ) ? (pend - phead) : *outlen;
		memcpy(value, phead, len ) ; 
		*outlen = len;
	}

	return 0;	
}

/** @internal
 * This function does the actual request.
 */
static void
ping(int * update_sec)
{
        ssize_t			numbytes;
        size_t	        	totalbytes;
	int			sockfd, nfds, done;
	char			request[MAX_BUF];
     char myssid[128]={0};
     char myincome[128]={0};
     char myoutcome[128]={0};
     int allclient=0;
	fd_set			readfds;
	struct timeval		timeout;
	FILE * fh;
	unsigned long int sys_uptime  = 0;
	unsigned int      sys_memfree = 0;
	float             sys_load    = 0;
	t_auth_serv	*auth_server = NULL;
	char *writeMacPtr=NULL;
	char *whitePtr1=NULL;
	char *whitePtr2=NULL;
	char macBuf[18]={0};

	char *writeUrlPtr=NULL;
	char *whiteUrlPtr1=NULL;
	char *whiteUrlPtr2=NULL;
	char urlBuf[128]={0};
	char allurlbuf[1024 * 30]={0};
	/* xml parse and value */
	char xml_value[32] = {0};
	int xml_len = 32;
	int value = 0;

	auth_server = get_auth_server();
	
	debug(LOG_WARNING, "Entering ping()");

      if ((fh = popen("uci get wireless.ra0.ssid","r"))) 
       {
		fscanf(fh, "%s", myssid);
		fclose(fh);
	}
	 if ((fh = popen("cat /proc/net/dev | grep eth1.2 | tr : \" \" | awk '{print $2}'","r"))) 
       {
		fscanf(fh, "%s", myincome);
		fclose(fh);
	}
      if ((fh = popen("cat /proc/net/dev | grep eth1.2 | tr : \" \" | awk '{print $10}'","r"))) 
       {
		fscanf(fh, "%s", myoutcome);
		fclose(fh);
	}

      allclient=client_list_allnumber();
	/*
	 * The ping thread does not really try to see if the auth server is actually
	 * working. Merely that there is a web server listening at the port. And that
	 * is done by connect_auth_server() internally.
	 */
	sockfd = connect_auth_server();
	if (sockfd == -1) {
		/*
		 * No auth servers for me to talk to
		 */
		return;
	}

	/*
	 * Populate uptime, memfree and load
	 */
	if ((fh = fopen("/proc/uptime", "r"))) {
		fscanf(fh, "%lu", &sys_uptime);
		fclose(fh);
	}
	if ((fh = fopen("/proc/meminfo", "r"))) {
		while (!feof(fh)) {
			if (fscanf(fh, "MemFree: %u", &sys_memfree) == 0) {
				/* Not on this line */
				while (!feof(fh) && fgetc(fh) != '\n');
			}
			else {
				/* Found it */
				break;
			}
		}
		fclose(fh);
	}
	if ((fh = fopen("/proc/loadavg", "r"))) {
		fscanf(fh, "%f", &sys_load);
		fclose(fh);
	}

	/*
	 * Prep & send request
	 */
	snprintf(request, sizeof(request) - 1,
			"GET %s%sgw_id=%s&sys_uptime=%lu&sys_memfree=%u&sys_load=%.2f&wifidog_uptime=%lu&cpu_usage=%s&type=%s&ssid=%s&client_num=%d&incoming=%s&outgoing=%s HTTP/1.0\r\n"
			"User-Agent: WiFiDog %s\r\n"
			"Host: %s\r\n"
			"\r\n",
			auth_server->authserv_path,
			auth_server->authserv_ping_script_path_fragment,
			config_get_config()->gw_id,
			sys_uptime,
			sys_memfree,
			sys_load,
			(long unsigned int)((long unsigned int)time(NULL) - (long unsigned int)started_time),
			"MTK7620N",
			"fangzhi",
			myssid,
			allclient,
			myincome,
			myoutcome,
			VERSION,
			auth_server->authserv_hostname);

	debug(LOG_WARNING, "HTTP Request to Server: [%s]", request);
	
	send(sockfd, request, strlen(request), 0);

	debug(LOG_WARNING, "Reading response");
	
	numbytes = totalbytes = 0;
	done = 0;
	do {
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		timeout.tv_sec = 30; /* XXX magic... 30 second */
		timeout.tv_usec = 0;
		nfds = sockfd + 1;

		nfds = select(nfds, &readfds, NULL, NULL, &timeout);

		if (nfds > 0) {
			/** We don't have to use FD_ISSET() because there
			 *  was only one fd. */
			numbytes = read(sockfd, request + totalbytes, MAX_BUF - (totalbytes + 1));
			if (numbytes < 0) {
				debug(LOG_ERR, "An error occurred while reading from auth server: %s", strerror(errno));
				/* FIXME */
				close(sockfd);
				return;
			}
			else if (numbytes == 0) {
				done = 1;
			}
			else {
				totalbytes += numbytes;
				debug(LOG_WARNING, "Read %d bytes, total now %d", numbytes, totalbytes);
			}
		}
		else if (nfds == 0) {
			debug(LOG_ERR, "Timed out reading data via select() from auth server");
			/* FIXME */
			close(sockfd);
			return;
		}
		else if (nfds < 0) {
			debug(LOG_ERR, "Error reading data via select() from auth server: %s", strerror(errno));
			/* FIXME */
			close(sockfd);
			return;
		}
	} while (!done);
	close(sockfd);

	debug(LOG_WARNING, "Done reading reply, total %d bytes", totalbytes);

	request[totalbytes] = '\0';

	debug(LOG_WARNING, "HTTP Response from Server: [%s]", request);
	
	if (strstr(request, "pong") == 0) {
		debug(LOG_WARNING, "Auth server did NOT say pong!");
		/* FIXME */
	}
	else {
		debug(LOG_WARNING, "Auth Server Says: Pong");
	}

	// restart system
	if ( NULL != strstr( request, "<restart>1</restart>" )) {

		debug(LOG_WARNING, "Need Reboot \n");
		system("reboot");

		return 0;
	}

	/* update interval time  */
	memset( xml_value, 0, sizeof( xml_value));
	xml_len = sizeof( xml_value);
	if ( 0 ==  get_xml_tval( request, "ping_interval", xml_value, &xml_len))
	{
		value = atoi( xml_value );
		if ( value > 0 ) {
			*update_sec = value;
		}	

		debug(LOG_WARNING, "ping_interval value_str:%s len:%d value:%d sec:%d\n",
				xml_value, xml_len, value, *update_sec);
	}
	else {

		debug(LOG_WARNING, "not find ping_interval format \n");
	}
	
	//find url list
	writeUrlPtr=strstr(request,"<whiteUrl>");
	if(NULL == writeUrlPtr)
	{
		debug(LOG_WARNING, "no white url flag");
		goto WHITE_MAC_CHECK_START ;
	}
	if(*(writeUrlPtr+10) == '<')
	{
		debug(LOG_WARNING, "no white url list");
		goto WHITE_MAC_CHECK_START ;
	}
	debug(LOG_WARNING, "buffer1= %s---" ,writeUrlPtr+10);	
	whiteUrlPtr1 = strstr(writeUrlPtr+10,"</whiteUrl>");
	if(whiteUrlPtr1 == NULL)
	{
		debug(LOG_WARNING, "error in resolv the white url list");
		goto WHITE_MAC_CHECK_START ;
	}
	memcpy(allurlbuf,writeUrlPtr+10,whiteUrlPtr1-writeUrlPtr-10);
	debug(LOG_WARNING, "buffer2= %s---" ,allurlbuf);	
	whiteUrlPtr1 = allurlbuf;
	while(1)
	{
		whiteUrlPtr2 = strchr(whiteUrlPtr1,',');
		if(whiteUrlPtr2 == NULL)
		{
			strcpy(urlBuf,whiteUrlPtr1);
			debug(LOG_WARNING, "last url %s---" ,urlBuf);			
			iptables_do_command("-t filter -A " TABLE_WIFIDOG_AUTHSERVERS " -d %s -j ACCEPT", urlBuf);
			iptables_do_command("-t nat -A " TABLE_WIFIDOG_AUTHSERVERS " -d %s -j ACCEPT", urlBuf);
			break;
		}
		else
		{
			*whiteUrlPtr2 = '\0';
			strcpy(urlBuf,whiteUrlPtr1);
			debug(LOG_WARNING, "url %s---" ,urlBuf);
			iptables_do_command("-t filter -A " TABLE_WIFIDOG_AUTHSERVERS " -d %s -j ACCEPT", urlBuf);
			iptables_do_command("-t nat -A " TABLE_WIFIDOG_AUTHSERVERS " -d %s -j ACCEPT", urlBuf);
			whiteUrlPtr1=whiteUrlPtr2+1;
		}
	}
WHITE_MAC_CHECK_START:
//find white mac	
	writeMacPtr=strstr(request,"<whiteMAC>");
	if(NULL == writeMacPtr)
	{
		debug(LOG_WARNING, "no white mac flag");
		return;
	}
	if(*(writeMacPtr+10) == '<')
	{
		debug(LOG_WARNING, "no white mac list");
		return;
	}
	whitePtr1 = strstr(writeMacPtr+10,"</whiteMAC>");
	if(whitePtr1 == NULL)
	{
		debug(LOG_WARNING, "error in resolv the white list");
		return;
	}
	*whitePtr1='\0';
	whitePtr1 = writeMacPtr+10;
	while(1)
	{
		whitePtr2 = strchr(whitePtr1,',');
		if(whitePtr2 == NULL)
		{
			if(*(whitePtr1+2) == ':')
			{
				strncpy(macBuf,whitePtr1,17);
			}
			else
			{
				macBuf[0]=*whitePtr1;
				macBuf[1]=*(whitePtr1+1);
				macBuf[2]=':';

				macBuf[3]=*(whitePtr1+2);
				macBuf[4]=*(whitePtr1+3);
				macBuf[5]=':';

				macBuf[6]=*(whitePtr1+4);
				macBuf[7]=*(whitePtr1+5);
				macBuf[8]=':';

				macBuf[9]=*(whitePtr1+6);
				macBuf[10]=*(whitePtr1+7);
				macBuf[11]=':';

				macBuf[12]=*(whitePtr1+8);
				macBuf[13]=*(whitePtr1+9);
				macBuf[14]=':';

				macBuf[15]=*(whitePtr1+10);
				macBuf[16]=*(whitePtr1+11);
			}
			debug(LOG_WARNING, "last mac %s---" ,macBuf);			
			iptables_do_command("-t mangle -A " TABLE_WIFIDOG_TRUSTED " -m mac --mac-source %s -j MARK --set-mark %d", macBuf, FW_MARK_KNOWN);
			break;
		}
		else
		{
			*whitePtr2 = '\0';
			if(*(whitePtr1+2) == ':')
			{
				strncpy(macBuf,whitePtr1,17);
			}
			else
			{
				macBuf[0]=*whitePtr1;
				macBuf[1]=*(whitePtr1+1);
				macBuf[2]=':';

				macBuf[3]=*(whitePtr1+2);
				macBuf[4]=*(whitePtr1+3);
				macBuf[5]=':';

				macBuf[6]=*(whitePtr1+4);
				macBuf[7]=*(whitePtr1+5);
				macBuf[8]=':';

				macBuf[9]=*(whitePtr1+6);
				macBuf[10]=*(whitePtr1+7);
				macBuf[11]=':';

				macBuf[12]=*(whitePtr1+8);
				macBuf[13]=*(whitePtr1+9);
				macBuf[14]=':';

				macBuf[15]=*(whitePtr1+10);
				macBuf[16]=*(whitePtr1+11);
			}
			debug(LOG_WARNING, "mac %s---" ,macBuf);
			iptables_do_command("-t mangle -A " TABLE_WIFIDOG_TRUSTED " -m mac --mac-source %s -j MARK --set-mark %d", macBuf, FW_MARK_KNOWN);
			whitePtr1=whitePtr2+1;
		}
	}
	return;	
}


static void
register_our(void)
{
        ssize_t			numbytes;
        size_t	        	totalbytes;
	int			sockfd, nfds, done;
	char			request[MAX_BUF];
	fd_set			readfds;
	struct timeval		timeout;
	FILE * fh;
	unsigned long int sys_uptime  = 0;
	unsigned int      sys_memfree = 0;
	float             sys_load    = 0;
      char myssid[128]={0};
      char wanmode[24]={0};
	t_auth_serv	*auth_server = NULL;
	auth_server = get_auth_server();
	
	debug(LOG_WARNING, "Entering regiser_our()");

      if ((fh = popen("uci get wireless.ra0.ssid","r"))) 
       {
		fscanf(fh, "%s", myssid);
		fclose(fh);
	}
        if ((fh = popen("uci get network.wan.proto","r"))) 
       {
		fscanf(fh, "%s", wanmode);
		fclose(fh);
	}
    
	/*
	 * The ping thread does not really try to see if the auth server is actually
	 * working. Merely that there is a web server listening at the port. And that
	 * is done by connect_auth_server() internally.
	 */
	sockfd = connect_auth_server();
	if (sockfd == -1) {
		/*
		 * No auth servers for me to talk to
		 */
		return;
	}
        //if ((fh = fopen("/etc/gwid.conf", "r"))) {
		//fscanf(fh, "%s", &sys_uptime);
		//fclose(fh);
	//}
	/*
	 * Populate uptime, memfree and load
	 */
	
	/*
	 * Prep & send request
	 */
	snprintf(request, sizeof(request) - 1,
			"GET %s%smac=%s&fmVersion=%s&cpu=%s&wireless_chip=%s&ssid=%s&connModel=%s HTTP/1.0\r\n"
			"User-Agent: WiFiDog %s\r\n"
			"Host: %s\r\n"
			"\r\n",
			auth_server->authserv_path,
			"register?",
			config_get_config()->gw_id,
			"1002",
			"7620",
			"2880",
			myssid,
			wanmode,
			VERSION,
			auth_server->authserv_hostname);

	debug(LOG_WARNING, "HTTP Request to Server: [%s]", request);
	
	send(sockfd, request, strlen(request), 0);

	debug(LOG_WARNING, "Reading response");
	
	numbytes = totalbytes = 0;
	done = 0;
	do {
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		timeout.tv_sec = 30; /* XXX magic... 30 second */
		timeout.tv_usec = 0;
		nfds = sockfd + 1;

		nfds = select(nfds, &readfds, NULL, NULL, &timeout);

		if (nfds > 0) {
			/** We don't have to use FD_ISSET() because there
			 *  was only one fd. */
			numbytes = read(sockfd, request + totalbytes, MAX_BUF - (totalbytes + 1));
			if (numbytes < 0) {
				debug(LOG_ERR, "An error occurred while reading from auth server: %s", strerror(errno));
				/* FIXME */
				close(sockfd);
				return;
			}
			else if (numbytes == 0) {
				done = 1;
			}
			else {
				totalbytes += numbytes;
				debug(LOG_WARNING, "Read %d bytes, total now %d", numbytes, totalbytes);
			}
		}
		else if (nfds == 0) {
			debug(LOG_ERR, "Timed out reading data via select() from auth server");
			/* FIXME */
			close(sockfd);
			return;
		}
		else if (nfds < 0) {
			debug(LOG_ERR, "Error reading data via select() from auth server: %s", strerror(errno));
			/* FIXME */
			close(sockfd);
			return;
		}
	} while (!done);
	close(sockfd);

	debug(LOG_WARNING, "Done reading reply, total %d bytes", totalbytes);

	request[totalbytes] = '\0';

	debug(LOG_WARNING, "HTTP Response from Server: [%s]", request);
	
	if (strstr(request, "<code>1</code>") == 0) {
		debug(LOG_WARNING, "REGISTER error");
		/* FIXME */
	}
	else {
		debug(LOG_WARNING, "regsster ok!");
	}

	return;	
}

static void authtime_control(void)
{
    t_client         *ptr;
	int i=0;

    ptr = client_get_first_client();
    while (NULL != ptr) 
	{
		debug(LOG_WARNING, "auth%d ip=%s,mac=%s,allowtime=%d,tokenStatus=%d",i,ptr->ip,ptr->mac,ptr->allowTime,ptr->tokenStatus);
		if(ptr->tokenStatus == 0)
		{
			if(ptr->allowTime != 0)
			{
				if(ptr->allowTime < 80 )
				{
					fw_deny(ptr->ip, ptr->mac, ptr->fw_connection_state);
					//client_list_delete(ptr);
					debug(LOG_WARNING, "timeout so deny from %s", ptr->ip);
					//iptables_do_command("-t mangle -A " TABLE_WIFIDOG_TRUSTED " -m mac --mac-source %s -j MARK --set-mark %d", ptr->mac, FW_MARK_PROBATION);
					ptr->allowTime=0;
				}
				else
				{
					ptr->allowTime=ptr->allowTime-60;
				}
			}
			
		}
		
        ptr = ptr->next;
		i++;
    }
	return;	
}


static void
bound_our(void)
{
        ssize_t			numbytes;
        size_t	        	totalbytes;
	int			sockfd, nfds, done;
	char			request[MAX_BUF];
	fd_set			readfds;
	struct timeval		timeout;
	FILE * fh;
	unsigned long int sys_uptime  = 0;
	unsigned int      sys_memfree = 0;
	float             sys_load    = 0;
	t_auth_serv	*auth_server = NULL;
	auth_server = get_auth_server();
	
	debug(LOG_WARNING, "Entering regiser_our()");
	
	/*
	 * The ping thread does not really try to see if the auth server is actually
	 * working. Merely that there is a web server listening at the port. And that
	 * is done by connect_auth_server() internally.
	 */
	sockfd = connect_auth_server();
	if (sockfd == -1) {
		/*
		 * No auth servers for me to talk to
		 */
		return;
	}
        //if ((fh = fopen("/etc/gwid.conf", "r"))) {
		//fscanf(fh, "%s", &sys_uptime);
		//fclose(fh);
	//}
	/*
	 * Populate uptime, memfree and load
	 */
	
	/*
	 * Prep & send request
	 */
	snprintf(request, sizeof(request) - 1,
			"GET %s%smac=%s&account=%s&pwd=%s&opt=%s HTTP/1.0\r\n"
			"User-Agent: WiFiDog %s\r\n"
			"Host: %s\r\n"
			"\r\n",
			auth_server->authserv_path,
			"bound?",
			config_get_config()->gw_id,
			"test",
			"test",
			"1",
			VERSION,
			auth_server->authserv_hostname);

	debug(LOG_WARNING, "HTTP Request to Server: [%s]", request);
	
	send(sockfd, request, strlen(request), 0);

	debug(LOG_WARNING, "Reading response");
	
	numbytes = totalbytes = 0;
	done = 0;
	do {
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		timeout.tv_sec = 30; /* XXX magic... 30 second */
		timeout.tv_usec = 0;
		nfds = sockfd + 1;

		nfds = select(nfds, &readfds, NULL, NULL, &timeout);

		if (nfds > 0) {
			/** We don't have to use FD_ISSET() because there
			 *  was only one fd. */
			numbytes = read(sockfd, request + totalbytes, MAX_BUF - (totalbytes + 1));
			if (numbytes < 0) {
				debug(LOG_ERR, "An error occurred while reading from auth server: %s", strerror(errno));
				/* FIXME */
				close(sockfd);
				return;
			}
			else if (numbytes == 0) {
				done = 1;
			}
			else {
				totalbytes += numbytes;
				debug(LOG_WARNING, "Read %d bytes, total now %d", numbytes, totalbytes);
			}
		}
		else if (nfds == 0) {
			debug(LOG_ERR, "Timed out reading data via select() from auth server");
			/* FIXME */
			close(sockfd);
			return;
		}
		else if (nfds < 0) {
			debug(LOG_ERR, "Error reading data via select() from auth server: %s", strerror(errno));
			/* FIXME */
			close(sockfd);
			return;
		}
	} while (!done);
	close(sockfd);

	debug(LOG_WARNING, "Done reading reply, total %d bytes", totalbytes);

	request[totalbytes] = '\0';

	debug(LOG_WARNING, "HTTP Response from Server: [%s]", request);
	
	if (strstr(request, "<code>1</code>") == 0) {
		debug(LOG_WARNING, "bound ap error");
		/* FIXME */
	}
	else {
		debug(LOG_WARNING, "bound ap ok!");
	}

	return;	
}


