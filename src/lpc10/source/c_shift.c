#include "c_lpc10.h"

/**********************************************************/
/* shift buffers */

VOID shift( VOID )
{
	INDEX i, j;

/* Update (shift) sample buffers */
	for (i = SBUFL; i <= SBUFH-LFRAME; i++) {
		g_inbuf[i] = g_inbuf[LFRAME + i];
		g_pebuf[i] = g_pebuf[LFRAME + i];
	}
	for (i = LBUFL; i <= LBUFH-LFRAME; i++)
		g_lpbuf[i] = g_lpbuf[LFRAME + i];
	for (i = PWINL; i <= PWINH-LFRAME; i++)
		g_ivbuf[i] = g_ivbuf[LFRAME + i];

/* shift onset marks */
	for (i=0; i<g_osptr; i++)
		if (g_osbuf[i]>LFRAME)
			break;
	g_osptr -= i;
	for (j=0; j<g_osptr; j++)
		g_osbuf[j] = g_osbuf[i+j] - LFRAME;

/* shift other data */
	for (i = 0; i < AF-1; i++) {
		g_vwin[i][0] = g_vwin[i+1][0] - LFRAME;
		g_vwin[i][1] = g_vwin[i+1][1] - LFRAME;
		g_awin[i][0] = g_awin[i+1][0] - LFRAME;
		g_awin[i][1] = g_awin[i+1][1] - LFRAME;
		g_obound[i] = g_obound[i+1];
		g_voibuf[i][0] = g_voibuf[i+1][0];
		g_voibuf[i][1] = g_voibuf[i+1][1];
		g_rmsbuf[i] = g_rmsbuf[i+1];
		for (j = 0; j < ORDER; j++)
			g_rcbuf[i][j] = g_rcbuf[i+1][j];
	}
	g_voibuf[AF-1][0] = g_voibuf[AF][0];
	g_voibuf[AF-1][1] = g_voibuf[AF][1];
}

/**********************************************************/
