#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
char allow_buf[102400],tmp[300],firebuf[300],dnstmp[250],dnshostbuf[10240];
char abuf[2048];
int  succ=0, i=0;

int allowlist()
{
	char userallowbuf[12048]={0},tmpallow[12048]={0},*allpoint =NULL;
	int ret,i,j,k,m,x1,x2;
	char tmpurl[64]={0};
	char * addr;   
	struct addrinfo * res, *pt;
	int totalnum = 0;
	FILE * allowlistfile = fopen("/etc/config/allow.list","r");
	if(allowlistfile!=NULL)
	{
		ret = fread(userallowbuf,1,12048,allowlistfile);
		fclose(allowlistfile);
		fprintf(stdout,"userallowbuf %s.\n",userallowbuf);
		memcpy(tmpallow,userallowbuf,12048);
		if(ret>0 )
			{
			allpoint = strtok(tmpallow,"#");
				while(allpoint)
				{
					struct hostent *he;
	    			struct sockaddr_in *sinp;
				    fprintf(stdout,"allpoint %s.\n",allpoint);
	
				    succ = getaddrinfo(allpoint, NULL, NULL, &res);
				    if(succ != 0)
				    {
				        printf("Can't get address info! error code = %d\n", succ);
				        allpoint = strtok(NULL,"#");
	                    continue;
				    }
				    for(pt=res, i=0; pt != NULL ; pt=pt->ai_next, i++){
				    memset(firebuf,'\0',sizeof(firebuf));	
				    sinp = (struct sockaddr_in *)pt->ai_addr;
				    addr = ( char *)inet_ntop(AF_INET, &sinp->sin_addr, abuf, 1024);
				    fprintf(stdout,"Addr [%s]\n",addr);
				    if( addr[0] < '1' && addr[0] > '9' )
				    	{
					    	printf("Error Dns,Next...\n");
					    	continue;
				    	}
				    snprintf(firebuf,sizeof(firebuf),"FirewallRule allow tcp to %s\n",addr);
				    snprintf(dnstmp,sizeof(dnstmp),"%s %s\n",allpoint,addr);
				    if(strstr(firebuf,"0.0.0.0"))
				    {
				    	fprintf(stdout,"Reject 0.0.0.0");
				    	continue;
				    }
				    strcat(dnshostbuf,dnstmp);
				    fprintf(stdout,"List[%s]",firebuf);
	
				    if(strstr(allow_buf,firebuf))
				    	continue;
				    strcat(allow_buf,firebuf);
				  	}
	
	                fprintf(stdout,"point %s.\n",allpoint);
	                allpoint = strtok(NULL,"#");
				}
			}
	}
}

int banlist()
{
	char userallowbuf[12048]={0},tmpallow[12048]={0},*allpoint =NULL;
	int ret,i,j,k,m,x1,x2;
	char tmpurl[64]={0};
	char * addr;   
	struct addrinfo * res, *pt;
	int totalnum = 0;
	FILE * allowlistfile = fopen("/etc/config/forbid.list","r");
	if(allowlistfile!=NULL)
	{
		ret = fread(userallowbuf,1,12048,allowlistfile);
		fclose(allowlistfile);
		fprintf(stdout,"forbid.list buf %s.\n",userallowbuf);
		memcpy(tmpallow,userallowbuf,12048);
		if(ret>0 )
			{
			allpoint = strtok(tmpallow,"#");
				while(allpoint)
				{
					struct hostent *he;
	    			struct sockaddr_in *sinp;
				    fprintf(stdout,"allpoint %s.\n",allpoint);
	
				    succ = getaddrinfo(allpoint, NULL, NULL, &res);
				    if(succ != 0)
				    {
				        printf("Can't get address info! error code = %d\n", succ);
				        allpoint = strtok(NULL,"#");
	                    continue;
				    }
				    for(pt=res, i=0; pt != NULL ; pt=pt->ai_next, i++){
				    memset(firebuf,'\0',sizeof(firebuf));	
				    sinp = (struct sockaddr_in *)pt->ai_addr;
				    addr = ( char *)inet_ntop(AF_INET, &sinp->sin_addr, abuf, 1024);
				    fprintf(stdout,"Addr [%s]\n",addr);
				    if( addr[0] < '1' && addr[0] > '9'  )
				    	{
					    	printf("Error Dns,Next...\n");
					    	continue;
				    	}
				    snprintf(firebuf,sizeof(firebuf),"FirewallRule block to %s\n",addr);
				    snprintf(dnstmp,sizeof(dnstmp),"%s %s\n",allpoint,addr);
				    if(strstr(firebuf,"0.0.0.0"))
				    {
				    	fprintf(stdout,"Reject 0.0.0.0");
				    	continue;
				    }
				    strcat(dnshostbuf,dnstmp);
				    fprintf(stdout,"List[%s]",firebuf);
	
				    if(strstr(allow_buf,firebuf))
				    	continue;
				    strcat(allow_buf,firebuf);
				  	}
	
	                fprintf(stdout,"point %s.\n",allpoint);
	                allpoint = strtok(NULL,"#");
				}
			}
	}
}

int main()
{
	//char allowurl[]="119.147.254.48\n119.147.254.48\n183.61.180.50\n119.147.254.50\n183.61.180.51\n183.61.217.51\n183.61.180.52\n183.61.217.52\n120.33.50.144\n113.105.73.144\n119.147.254.144\n113.105.73.145\n119.147.254.145\n119.147.254.146\n119.147.254.147\n183.61.180.22\n113.105.73.139\n113.105.73.140\n113.105.73.141\n113.105.73.142\n113.105.73.143\n119.147.254.143\n14.17.32.211\n182.140.167.44\n14.17.32.211\n183.60.15.153\n121.10.26.30\n120.33.50.39\n120.33.50.40\n124.228.42.42\n113.105.137.44\n120.33.50.145\n113.105.73.169\n119.147.254.176\n119.147.254.177\n220.169.154.179\n220.169.154.180\n113.17.185.180\n220.169.154.181\n113.17.185.181\n113.17.185.182\n121.10.26.29\n113.108.0.55\n113.108.0.99\n119.147.68.106\n183.61.32.185\n183.60.15.184\n123.151.148.77\n113.108.21.145\n183.60.11.195\n121.14.125.32\n14.17.41.164\n183.60.15.158\n113.108.7.233\n183.60.17.211\n119.147.192.10\n119.147.192.13\n119.147.192.14\n183.60.17.112\n183.60.17.189\n113.108.77.44\n113.108.91.44\n113.108.77.33\n119.147.146.206\n119.147.146.206\n14.17.41.156\n14.17.42.16\n119.147.254.19\n183.61.180.20\n119.147.254.20\n183.61.180.21\n183.60.217.14\n113.105.137.15\n183.60.217.15\n119.147.254.15\n113.105.137.16\n119.147.254.16\n119.147.254.17\n113.105.137.18\n183.61.180.18\n119.147.254.18\n113.105.137.19\n183.61.180.19\n119.147.2.30\n183.60.7.169\n113.108.16.109\n123.151.10.161\n115.238.249.253\n112.90.83.87\nqq.com\napp.weibo.com\nimg.t.sinajs.cn\napi.weibo.com\nweibo.cn\napi.weibo.cn\nm.weibo.cn\nwww.weibo.cn\nweibo.com\nwww.weibo.com\napp.weibo.com\nopen.weibo.com\nlogin.sina.com.cn\npub.idqqimg.com\nbadjs.qq.com\ncgi.connect.qq.com\nui.ptlogin2.qq.com\nsupport.qq.com\ntajs.qq.com\ngraph.qq.com\nopenapi.qzone.qq.com\nwww.qq.com\ncaptcha.qq.com\nconnect.qq.com\nface7.qun.qq.com\napi.pc120.com\np.api.pc120.com\nui.ptlogin2.qq.com\nptlogin2.qq.com\nimgcache.qq.com\npingtcss.qq.com\ncheck.ptlogin2.qq.com";
	//char allowurl[]="14.17.32.211\n182.140.167.44\n14.17.32.211\n183.60.15.153\n121.10.26.30\n120.33.50.39\n120.33.50.40\n124.228.42.42\n113.105.137.44\n120.33.50.145\n113.105.73.169\n119.147.254.176\n119.147.254.177\n220.169.154.179\n220.169.154.180\n113.17.185.180\n220.169.154.181\n113.17.185.181\n113.17.185.182\n121.10.26.29\n119.147.254.48\n183.61.180.50\n119.147.254.50\n183.61.180.51\n183.61.217.51\n183.61.180.52\n183.61.217.52\n120.33.50.144\n113.108.0.55\n113.108.0.99\n119.147.68.106\n183.61.32.185\n183.60.15.184\n123.151.148.77\n113.108.21.145\n183.60.11.195\n121.14.125.32\n14.17.41.164\n183.60.15.158\n113.108.7.233\n183.60.17.211\n119.147.192.10\n119.147.192.13\n119.147.192.14\n183.60.17.112\n183.60.17.189\n113.108.77.44\n113.108.91.44\n113.108.77.33\n119.147.146.206\n14.17.41.156\n14.17.42.16\n119.147.254.19\n183.61.180.20\n119.147.254.20\n183.61.180.21\n183.60.217.14\n113.105.137.15\n183.60.217.15\n119.147.254.15\n113.105.137.16\n119.147.254.16\n119.147.254.17\n113.105.137.18\n183.61.180.18\n119.147.254.18\n113.105.137.19\n183.61.180.19\n113.105.73.144\n119.147.254.144\n113.105.73.145\n119.147.254.145\n119.147.254.146\n119.147.254.147\n183.61.180.22\n113.105.73.139\n113.105.73.140\n113.105.73.141\n113.105.73.142\n113.105.73.143\n119.147.254.143\n119.147.2.30\n183.60.7.169\n113.108.16.109\n123.151.10.161\n218.30.114.85\n180.149.134.17\n180.149.135.230\n180.149.134.249\n180.149.138.237\n113.108.216.251\n180.149.153.216\n119.147.148.35\napi.weibo.com\nweibo.com\nwww.weibo.com\napp.weibo.com\nopen.weibo.com\nlogin.sina.com.cn\nweibo.cn\napi.weibo.cn\nm.weibo.cn\nu1.sinaimg.cn\nqq.com\npub.idqqimg.com\nbadjs.qq.com\ncgi.connect.qq.com\nui.ptlogin2.qq.com\nsupport.qq.com\ntajs.qq.com\ngraph.qq.com\nopenapi.qzone.qq.com\nwww.qq.com\ncaptcha.qq.com\nconnect.qq.com\nface7.qun.qq.com\napi.pc120.com\np.api.pc120.com\nui.ptlogin2.qq.com\nptlogin2.qq.com\nimgcache.qq.com\npingtcss.qq.com\ncheck.ptlogin2.qq.com";
	//char allowurl[]="14.17.32.211\n182.140.167.44\n163.177.65.160\n125.39.240.113\n120.209.130.115\n183.207.228.122\n101.226.129.158\n113.142.21.81\n180.96.86.192\n202.55.10.190\n101.226.103.106\n123.125.119.147\n140.206.160.207\n124.160.163.169\n111.30.132.101\n117.135.130.157\n120.198.201.156\n117.41.242.179\n182.131.30.42\n122.226.253.177\n120.33.50.40\n122.228.70.182\n122.226.253.178\n203.205.136.126\n60.172.80.51\n117.34.6.181\n119.84.72.44\n117.34.26.53\n182.131.30.43\n118.123.97.181\n61.155.220.177\n119.147.254.48\n122.193.23.186\n61.158.251.7\n119.167.141.44\n122.193.23.58\n27.221.21.188\n122.193.23.187\n218.60.55.46\n113.6.237.48\n163.177.153.56\n122.193.23.59\n119.190.4.51\n183.232.83.43\n112.25.58.172\n111.1.57.44\n120.192.85.170\n119.147.68.106\n113.108.0.55\n112.90.140.84\n113.108.0.99\n112.90.85.148\n120.209.130.62\n183.207.228.42\n183.61.32.185\n58.250.135.158\n120.198.189.104\n183.60.15.184\n123.151.148.77\n101.226.103.81\n114.134.85.210\n125.39.240.56\n140.206.160.218\n112.90.83.106\n117.135.171.220\n120.198.201.166\n113.108.21.145\n183.60.11.195\n112.95.241.205\n112.95.241.172\n120.209.130.120\n180.153.210.81\n121.14.125.32\n123.125.119.150\n14.17.41.164\n163.177.71.162\n120.198.203.166\n183.60.15.158\n112.90.83.87\n120.198.199.187\n113.142.21.75\n101.226.62.104\n103.7.30.39\n113.108.7.233\n123.138.162.75\n112.90.83.73\n120.196.210.86\n119.147.192.14\n119.147.192.13\n183.60.17.112\n119.147.192.10\n183.60.17.211\n103.7.28.62\n112.95.242.171\n112.90.143.119\n112.95.243.216\n120.198.198.37\n183.207.228.44\n120.209.130.63\n113.108.77.44\n113.108.77.33\n113.108.91.44\n112.90.138.26\n112.90.138.25\n119.147.146.126\n119.147.146.206\n119.188.46.61\n123.129.208.84\n221.130.162.69\n221.130.17.29\n101.226.76.31\n101.226.129.146\n203.186.47.206\n14.17.42.16\n123.151.10.167\n123.151.152.48\n140.206.160.203\n112.65.195.223\n125.39.247.166\n125.39.247.177\n163.177.71.159\n117.135.171.164\n111.30.135.152\n60.172.80.16\n122.228.70.146\n61.155.220.147\n182.131.30.13\n118.123.97.143\n183.136.158.138\n124.228.42.7\n117.34.6.138\n117.34.26.20\n112.117.220.20\n119.147.254.16\n182.131.30.16\n61.155.220.148\n203.186.47.217\n119.147.254.17\n119.167.195.24\n119.167.141.16\n182.118.124.144\n60.210.9.17\n218.60.55.13\n113.6.237.20\n122.193.23.13\n119.167.195.20\n122.193.23.139\n163.177.153.78\n122.193.23.140\n27.221.21.145\n119.190.4.15\n183.232.30.25\n112.25.58.142\n183.232.30.34\n120.192.85.145\n183.60.7.169\n113.108.70.177\n113.108.70.176\n119.147.2.30\n112.90.86.35\n112.90.141.232\n112.90.141.233\n183.207.233.71\n101.226.103.34\n113.108.16.109\n163.177.71.151\n123.151.10.161\n125.39.247.160\n140.206.160.219\n117.135.171.212\n120.198.203.153\n180.149.134.17\n114.134.80.161\n123.125.106.200\n183.207.233.65\n14.0.63.134\n175.41.12.101\n183.207.228.139\n221.130.162.68\n180.149.135.230\n114.134.80.166\n123.125.106.226\n221.179.193.227\n180.149.134.249\n123.125.106.232\n180.149.138.237\n123.125.106.178\n221.130.162.46\n203.90.242.124\n113.108.216.251\n202.108.7.198\n219.142.78.194\n180.149.153.216\n114.134.80.163\n221.179.190.208\n202.108.7.133\n123.125.105.231\n123.125.105.246\n111.13.88.248\n111.13.88.239\n111.13.88.237\n111.13.88.236\n119.147.148.35\n221.236.10.42\n221.236.10.41\n61.138.219.43\n222.73.28.54\n218.75.152.104\n222.73.28.97\n61.155.142.22\n117.34.15.32\n202.102.94.83\n221.236.30.71\n61.158.251.152\n111.161.68.68\n61.158.251.180\n36.250.64.50\n124.95.163.64\n218.9.147.195\n218.9.147.194\n111.161.68.67\n14.0.63.135\n175.43.20.80\n118.186.8.135\n111.1.46.136\n221.179.180.81\n211.136.10.24\n119.188.72.28\napi.weibo.com\nweibo.com\nwww.weibo.com\napp.weibo.com\nopen.weibo.com\nlogin.sina.com.cn\nweibo.cn\napi.weibo.cn\nm.weibo.cn\nu1.sinaimg.cn\nqq.com\npub.idqqimg.com\nbadjs.qq.com\ncgi.connect.qq.com\nui.ptlogin2.qq.com\nsupport.qq.com\ntajs.qq.com\ngraph.qq.com\nopenapi.qzone.qq.com\nwww.qq.com\ncaptcha.qq.com\nconnect.qq.com\nface7.qun.qq.com\napi.pc120.com\np.api.pc120.com\nui.ptlogin2.qq.com\nptlogin2.qq.com\nimgcache.qq.com\npingtcss.qq.com\ncheck.ptlogin2.qq.com";
//	char allowurl[]="180.149.135.230\n114.134.80.166\n123.125.106.226\n221.179.193.227\n180.149.134.17\n114.134.80.161\n123.125.106.200\n183.207.233.65\n218.75.152.108\n14.0.63.141\n183.207.228.154\n221.130.162.69\n180.149.134.249\n123.125.106.232\n221.130.162.46\n180.149.138.237\n123.125.106.178\n221.130.162.205\n113.108.216.251\n203.90.242.124\n202.108.7.198\n219.142.78.194\n180.149.153.216\n123.125.105.231\n202.108.7.133\n123.125.105.246\n111.13.88.248\n111.13.88.239\n111.13.88.236\n221.179.190.208\n114.134.80.163\n111.13.88.237\n61.244.110.164\n117.34.15.31\n221.236.10.41\n222.73.28.97\n221.236.30.67\n221.236.30.71\n222.73.28.54\n202.102.94.83\n61.155.142.21\n221.236.10.42\n119.147.148.35\n61.158.251.180\n61.158.251.152\n111.161.68.61\n175.43.20.80\n36.250.64.50\n111.161.68.67\n124.95.163.165\n218.9.147.194\n14.0.63.138\n218.9.147.195\n111.161.68.68\n14.0.63.134\n119.188.72.28\n221.179.180.81\n111.1.46.135\n14.17.32.211\n182.140.167.44\n163.177.65.160\n125.39.240.113\n183.207.228.28\n120.209.130.63\n113.142.21.81\n202.55.10.190\n183.60.15.153\n101.226.129.158\n180.96.86.192\n140.206.160.207\n123.125.119.147\n61.135.157.156\n124.160.163.169\n61.135.167.36\n117.135.130.157\n120.198.201.156\n111.30.132.101\n119.147.68.106\n113.108.0.99\n113.108.0.55\n112.90.140.84\n112.90.85.148\n183.207.228.123\n120.209.130.23\n183.61.32.185\n58.250.135.158\n120.198.189.104\n183.207.228.44\n123.151.148.77\n101.226.103.81\n114.134.85.210\n125.39.240.56\n140.206.160.218\n112.90.83.106\n117.135.171.220\n120.198.201.166\n113.108.21.145\n183.60.11.195\n112.95.241.205\n112.95.241.172\n120.209.130.27\n180.153.210.81\n121.14.125.32\n123.125.119.150\n120.209.130.117\n14.17.41.164\n163.177.71.162\n120.198.203.166\n183.60.15.158\n112.90.83.87\n120.198.199.187\n101.226.62.104\n123.138.162.75\n113.108.7.233\n103.7.30.39\n113.142.21.75\n112.90.83.73\n120.196.210.86\n119.147.192.13\n119.147.192.10\n183.60.17.211\n183.60.17.112\n103.7.28.62\n112.95.242.171\n112.90.143.118\n112.95.243.216\n112.90.143.119\n120.209.130.120\n120.198.198.39\n113.108.77.33\n112.90.138.25\n113.108.77.44\n113.108.91.44\n112.90.138.26\n120.209.130.118\n183.60.15.184\n14.17.42.16\n123.151.10.167\n101.226.76.31\n101.226.129.146\n203.186.47.209\n140.206.160.203\n125.39.247.166\n112.65.195.223\n163.177.71.159\n112.90.83.67\n125.39.247.177\n120.198.203.150\n117.135.171.164\n119.84.72.14\n61.177.126.78\n118.123.97.143\n118.123.97.148\n60.172.80.20\n183.61.180.21\n117.41.242.146\n202.100.73.149\n119.147.254.16\n60.172.80.19\n125.56.218.88\n112.117.220.17\n117.34.26.15\n61.177.126.81\n119.190.4.18\n119.188.94.19\n119.167.141.16\n163.177.153.78\n122.193.23.13\n218.60.55.15\n27.221.21.145\n122.193.23.139\n27.221.21.144\n113.6.237.18\n119.190.4.19\n61.182.131.15\n120.192.248.24\n120.192.85.145\n183.232.30.36\n220.194.199.20\n183.136.158.136\n117.34.6.144\n60.172.80.17\n113.105.73.142\n119.84.72.15\n112.117.220.15\n117.34.26.17\n119.190.4.17\n119.147.254.143\n117.41.242.145\n61.155.220.144\n183.136.158.135\n221.204.186.20\n119.190.4.15\n119.167.195.23\n122.193.23.140\n122.193.23.144\n218.60.55.14\n122.193.23.12\n113.6.237.15\n123.125.87.13\n183.232.30.25\n112.25.58.142\n120.192.85.148\n120.192.85.147\n119.84.72.13\n183.136.158.138\n112.117.220.20\n60.172.80.16\n61.155.220.145\n117.34.6.142\n182.131.30.14\n113.105.137.18\n119.84.72.16\n182.118.124.144\n119.188.94.17\n113.6.237.19\n60.210.9.15\n122.193.23.11\n27.221.21.146\n120.192.85.144\n112.25.58.141\n183.232.30.34\n113.108.70.176\n119.147.2.30\n113.108.70.177\n183.60.7.169\n112.90.141.233\n112.90.141.232\n112.90.86.35\n183.207.233.71\n120.209.130.62\n113.108.16.109\n101.226.103.34\n163.177.71.151\n123.151.10.161\n125.39.247.160\n140.206.160.219\n117.135.171.212\n120.198.203.153\n183.207.228.122\n111.30.131.126\n118.123.97.180\n222.73.132.186\n122.228.70.182\n203.205.136.126\n118.123.97.181\n120.33.50.39\n117.34.26.53\n60.172.80.51\n222.73.132.185\n61.155.220.177\n182.131.30.42\n117.34.6.180\n119.84.72.44\n117.41.242.177\n119.147.254.48\n119.188.94.51\n182.118.124.250\n122.193.23.58\n122.193.23.59\n27.221.21.188\n218.60.55.46\n122.193.23.186\n112.90.149.35\n113.6.237.49\n119.188.94.53\n120.204.205.163\n183.232.83.43\n112.25.58.172\n120.192.85.170\n120.209.130.115\n101.226.103.106\n117.41.242.179\n122.226.253.177\n120.33.50.40\n122.226.253.178\n117.34.6.181\n182.131.30.43\n61.158.251.7\n119.167.141.44\n122.193.23.187\n113.6.237.48\n163.177.153.56\n119.190.4.51\n111.1.57.44\n183.207.228.42\n119.147.192.14\n120.198.198.37\n119.147.146.126\n119.147.146.206\n119.188.46.61\n123.129.208.84\n221.130.17.29\n203.186.47.206\n123.151.152.48\n111.30.135.152\n122.228.70.146\n61.155.220.147\n182.131.30.13\n124.228.42.7\n117.34.6.138\n117.34.26.20\n182.131.30.16\n61.155.220.148\n203.186.47.217\n119.147.254.17\n119.167.195.24\n60.210.9.17\n218.60.55.13\n113.6.237.20\n119.167.195.20\n175.41.12.101\n183.207.228.139\n221.130.162.68\n61.138.219.43\n218.75.152.104\n61.155.142.22\n117.34.15.32\n124.95.163.64\n14.0.63.135\n118.186.8.135\n111.1.46.136\n211.136.10.24\n221.130.162.36\n61.155.142.24\n61.155.142.23\n202.102.94.25\n111.161.68.60\n119.188.72.29\n111.1.46.147\n222.73.28.76\n114.134.80.155\n175.43.124.200\n120.209.130.11\n112.117.220.52\n113.17.185.182\n222.73.3.35\n117.34.26.52\n61.155.220.176\n61.158.251.9\n27.221.21.189\n60.210.9.51\n111.10.27.154\n183.61.180.52\n119.147.254.176\n60.172.80.53\n60.172.80.52\n61.158.251.8\n112.90.149.36\n163.177.153.57\n120.192.248.43\na.dpool.weibo.com\nsa.dpool.weibo.com\napi.weibo.com\nweibo.com\nwww.weibo.com\napp.weibo.com\nopen.weibo.com\nlogin.sina.com.cn\nweibo.cn\napi.weibo.cn\nm.weibo.cn\nu1.sinaimg.cn\nweibo.grid.sinaedge.com\nu1sinaimg.gslb.sinaedge.com\nqq.com\nwww.qq.com\nbadjs.qq.com\ncgi.connect.qq.com\nui.ptlogin2.qq.com\nsupport.qq.com\ntajs.qq.com\ngraph.qq.com\nopenapi.qzone.qq.com\ncaptcha.qq.com\nconnect.qq.com\nface.qun.qq.com\nface7.qun.qq.com\nui.ptlogin2.qq.com\nptlogin2.qq.com\nimgcache.qq.com\nimgcache.tc.qq.com\nimgcache.tcdn.qq.com\npingtcss.qq.com\ncheck.ptlogin2.qq.com\ncgi.connect.qq.com\nui.ptlogin2.qq.com\npub.idqqimg.com\nqplus.tcdn.qq.com\nqplus.tc.qq.com\n";
	char allowurl[102400]={0};
	char  delim[]="\r\n";
	char * point = NULL;

	char  addr[10024];    struct addrinfo * res, *pt;
	struct hostent *he = NULL ;
	struct sockaddr_in *sinp  = NULL;
	FILE * fp = fopen("/etc/uallowurl.list","r");
	if(fp == NULL)
		return 0 ;
	fread(allowurl,1,sizeof(allowurl),fp);	
	fclose(fp);
	point = strtok(allowurl,delim);
	strcat(dnshostbuf,"127.0.0.1 localhost\n");
	while(point)
	{
		
	    fprintf(stdout,"Point %s.\n",point);
	
	    succ = getaddrinfo(point, NULL, NULL, &res);
	    if(succ != 0)
	    {
	        printf("Can't get address info! error code = %d\n", succ);
	        point = strtok(NULL,delim);
	        continue;
	    }
	    for(pt=res, i=0; pt != NULL ; pt=pt->ai_next, i++){
	    memset(tmp,'\0',sizeof(tmp));		
	    sinp = (struct sockaddr_in *)pt->ai_addr;
	    inet_ntop(AF_INET, &sinp->sin_addr, abuf, 16);
	    printf("abuf[0] %c\n",abuf[0]);
	    fprintf(stdout,"Addr [%s]\n",abuf);
	    if( addr[0] < '1' && addr[0] > '9' )
    	{
	    	printf("Error Dns,Next...\n");
	    	continue;
    	}
	    snprintf(tmp,sizeof(tmp),"FirewallRule allow tcp to %s\n",abuf);
	   // snprintf(dnstmp,sizeof(dnstmp),"%s %s\n",point,abuf);
	    if(strstr(tmp,"0.0.0.0"))
	    {
	    	fprintf(stdout,"Reject 0.0.0.0\n");
	    	continue;
	    }
	    //strcat(dnshostbuf,dnstmp);
	    fprintf(stdout,"[%s]",tmp);
	
	    if(strstr(allow_buf,tmp))
	    	continue;
	    strcat(allow_buf,tmp);
	  	}
 		point = strtok(NULL,delim);
	}
	fprintf(stdout,"Now Make Allowlist\n");
	allowlist();
	fprintf(stdout,"Now Make Forbidlist\n");
	banlist();
	//fprintf(stdout,"Allow buf %s",allow_buf);
	//fprintf(stdout,"Host buf %s",dnshostbuf);
	//FILE * hostfile = fopen("/etc/hosts","w+");
	//fwrite(dnshostbuf,1,strlen(dnshostbuf),hostfile);
	//fclose(hostfile);
	FILE * allowfile = fopen("/etc/allow.conf","w+");
	fwrite(allow_buf,1,strlen(allow_buf),allowfile);
	fclose(allowfile);
	system("rm /usr/local/etc/wifidog.conf");
	usleep(10000);
	system("/usr/bin/getip.sh");
	usleep(10000);
	system("/etc/makeconf.sh");
	usleep(20000);

}