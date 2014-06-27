/* */
/*
 *	wairosNetwork.c -- wairOS network Settings
 *
 *	Copyright (c) WAIROS Corporation All Rights Reserved.
 *
 *	$Id: leo Luo
 */

#include	<stdlib.h>
#include	<sys/ioctl.h>
#include	<net/if.h>
#include	<net/route.h>
#include    <string.h>
#include    <dirent.h>
#include	<uci.h>
#include	"wairosNetwork.h"
#include	"webs.h"


static int getLanInfoJson(int eid, webs_t wp, int argc, char_t **argv);
static void setLanInfo(webs_t wp, char_t *path, char_t *query);




void formDefineNetwork(void)
{
	websAspDefine(T("getLanInfoJson"), getLanInfoJson);

	websFormDefine(T("setLanInfo"), setLanInfo);
}



/*
 * description: write lan info as json format
 */
static int getLanInfoJson(int eid, webs_t wp, int argc, char_t **argv)
{
	struct uci_context *uciContent;
	struct uci_ptr uciPoint;
	//struct uci_package * pkg = NULL;  
	char *lanip = strdup("network.lan.ipaddr");
	char *lannetmask = strdup("network.lan.netmask");
	uciContent = uci_alloc_context ();
	if (uci_lookup_ptr (uciContent, &uciPoint, lanip, 1) != UCI_OK)
    {
      uci_perror (uciContent, "XXX");
      return 1;
    }
	else
	{
		websWrite(wp, T("lanip:'%s',\n"), uciPoint.o->v.string);
	}
	if (uci_lookup_ptr (uciContent, &uciPoint,lannetmask, 1) != UCI_OK)
    {
      uci_perror (uciContent, "XXX");
      return 1;
    }
	else
	{
		websWrite(wp, T("lannetmask:'%s'\n"), uciPoint.o->v.string);
	}
	uci_free_context (uciContent);
	free (lanip);
	free (lannetmask);
	return 0;
}

/* goform/setLanInfo */
static void setLanInfo(webs_t wp, char_t *path, char_t *query)
{
	
	char_t	*ip, *nm;

	struct uci_context *uciContent;
	struct uci_ptr uciPoint;

	char tmpBuf[64]={0};
	
	ip = websGetVar(wp, T("lanip"), T(""));
	nm = websGetVar(wp, T("lannetmask"), T(""));

	sprintf(tmpBuf,"network.lan.ipaddr=%s",ip);

	uciContent = uci_alloc_context ();
	if (uci_lookup_ptr (uciContent, &uciPoint, tmpBuf, 1) != UCI_OK)
    {
      uci_perror (uciContent, "XXX");
      return 1;
    }
	uci_set(uciContent,&uciPoint);

	sprintf(tmpBuf,"network.lan.netmask=%s",nm);
	if (uci_lookup_ptr (uciContent, &uciPoint, tmpBuf, 1) != UCI_OK)
    {
      uci_perror (uciContent, "XXX");
      return 1;
    }
	uci_set(uciContent,&uciPoint);
	uci_commit(uciContent, &uciPoint.p, 0);
	uci_free_context (uciContent);
	printf("\r\ndsadasd\r\n");
	websWrite(wp, T("HTTP/1.0 200 OK\n"));
	websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);
	websWrite(wp, T("Pragma: no-cache\n"));
	websWrite(wp, T("Cache-control: no-cache\n"));
	websWrite(wp, T("Content-Type: text/html\n"));
	websWrite(wp, T("\n"));
	websWrite(wp,T("1"));
	websDone(wp, 200);   
	//websRedirect(wp, "/network/lan.htm");
	//return;
}



