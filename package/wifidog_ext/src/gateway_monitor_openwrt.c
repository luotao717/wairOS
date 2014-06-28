#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main()
{
			unsigned char psbuf[5120],tmpbuf[32]; int gwswitch,x1 = 0  ;
			system("/bin/allow &");
			sleep(50);
			system("wifidog-init start");
			
} 