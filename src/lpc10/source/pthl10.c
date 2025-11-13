#include <stdio.h>
#include <stdlib.h>

#include "c_lpc10.h"
#include "lpc10.h"

/**********************************************************/

BOOL g_used = FALSE;

INDEX panalys( VOID );

/**********************************************************/

PTHL10 *pthl10_construct( VOID )
{
	if (g_used) {
		fprintf(stderr,"Por ahora no soporto mas de una estructura PTHL10!");
		exit(1);
	}

	g_used = TRUE;
	pthl10_reset((PTHL10*)1);
	return (PTHL10*)1;
}

/**********************************************************/

VOID pthl10_destruct( PTHL10 *p )
{
	(void)p;  /* avoid "unused parameter" warning */
	g_used = FALSE;
}

/**********************************************************/

VOID pthl10_reset( PTHL10 *p )
{
	(void)p;  /* avoid "unused parameter" warning */
	coder_ini();
}

/**********************************************************/

INT pthl10_get_i16( PTHL10 *p, INT16 speech[LPC10_LFRAME] )
{
	(void)p;  /* avoid "unused parameter" warning */
	shift();
	framein_i16(speech);
	return panalys();
}

/**********************************************************/

INDEX panalys( VOID )
{
	static FLOAT amdf[LTAU];
	INDEX midx, minptr, maxptr, mintau;
	FLOAT ivrc[2];

	preemp(g_inbuf+(SBUFH-LFRAME+1), g_pebuf+(SBUFH-LFRAME+1), LFRAME, g_zpre);
	onset(g_pebuf, g_osbuf, &g_osptr);
	placev(g_osbuf, g_osptr, &g_obound[AF-1], g_vwin);

	lpfilt31(g_inbuf+LBUFH-PWLEN-1, g_lpbuf+LBUFH-PWLEN+1);
	ivfilt(g_lpbuf+PWINL, g_ivbuf+PWINL, ivrc);
	tbdm(g_ivbuf+PWINL, G_tau, amdf, &minptr, &maxptr, &mintau);

	voicin(g_vwin, g_inbuf, g_lpbuf, 0, amdf[minptr], amdf[maxptr], mintau, ivrc, g_obound, g_voibuf);
	voicin(g_vwin, g_inbuf, g_lpbuf, 1, amdf[minptr], amdf[maxptr], mintau, ivrc, g_obound, g_voibuf);
	dyptrk(amdf, minptr, g_voibuf[AF][1], &g_pitch, &midx);

	if (CODED_VOICE[0]||CODED_VOICE[1])
		return G_tau[CODED_PITCH];
	else
		return 0;
}

/**********************************************************/
