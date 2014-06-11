#include "config.h"

int make_connection(char * url,int rport)
{
		int sockfd, numbytes;
    struct hostent *he;
    struct sockaddr_in their_addr;
    unsigned int myport;

    
    if((he=gethostbyname(url))==NULL) {
    	LOG(LOG_DEBUG,"gethostbyname\n");
       return -1;
    }
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    	LOG(LOG_DEBUG,"socket\n");
       return -1;
    }
    their_addr.sin_family=PF_INET;
    their_addr.sin_port=htons(rport);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero),0);
    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
    	LOG(LOG_DEBUG,"connect\n");
       return -1;
    }
    return sockfd;
}


int send_package(int sockfd,unsigned char * sdata, int slen )
{
    int numbytes;
    if ((numbytes=send(sockfd, sdata, slen, 0)) == -1) {
    	LOG(LOG_DEBUG,"recv\n");
       return -1;
    }
  	return numbytes;
}

int read_package(int sockfd,unsigned char * dest_data,int maxlen)
{
	int j = 0,nread,retval = 0 ,ret ;
	fd_set fds;
	struct timeval tv;
	char rcv_buf[2048];
	memset(rcv_buf,'\0',sizeof(rcv_buf));
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	if( (ret = select(sockfd+1, &fds, NULL, NULL, &tv))== 0)
	{
		//LOG(LOG_DEBUG,"timeout\n");
		return -1;
	}else if(ret < 0 )  	//FILE DESC IS CLOSED
	{
		LOG(LOG_DEBUG,"SOCKET FILE DESC IS CLOSED\n");
		return -99;
	}
    nread = read(sockfd,dest_data,maxlen);
    return nread;
}
