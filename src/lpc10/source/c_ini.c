#include "c_lpc10.h"

/**********************************************************/

VOID coder_ini( VOID )
{
	INDEX i, j;

/* onset */
	for (i = 0; i < L2WID; i++)
		g_l2buf[i] = (FLOAT)0.0;
	g_d = (FLOAT)1.0;
	g_n = g_fpc = g_l2sum1 = (FLOAT)0.0;
	g_l2ptr1 = 0;
	g_l2ptr2 = (L2WID/2);
	g_lasti = 0;
	g_hyst = FALSE;

/* dyptrk */
	g_ipoint = 0;
	g_alphax = (FLOAT)0.0;
	for (i=0; i<LTAU; i++) {
		g_p[i][0] = g_p[i][1] = 0;
		g_s[i]= (FLOAT)0.0;
	}

/* voicin */
	g_lbve = g_fbve = REF;
	g_ofbue = g_fbue = REF / 16;
	g_olbue = g_lbue = REF / 32;
	g_sfbue = g_slbue = 0;
	g_snr = 64 * ((FLOAT)g_fbve / (FLOAT)g_fbue);
	g_maxmin = (FLOAT)0.0;
	g_dither = (FLOAT)20.0;
	for (i = 0; i < AF; i++)
		g_voice[i][1] = g_voice[i][0] = (FLOAT)0.0;

/* preenf */
	g_zpre[0] = g_zpre[1] = (FLOAT)0.0;

/* analys */
	for (i = 0; i < OSLEN; i++)
		g_osbuf[i] = 0;
	g_osptr = 0;
/* Initialize arrays */
	for (i = 0; i < SBUFH - SBUFL + 1; i++)
		g_inarray[i] = g_pearray[i] = (FLOAT)0.0;
	for (i = 0; i < LBUFH - LBUFL + 1; i++)
		g_lparray[i] = (FLOAT)0.0;
	for (i = 0; i < PWINH - PWINL + 1; i++)
		g_ivarray[i] = (FLOAT)0.0;
/* align C arrays pointers for easy handling */
	g_inbuf = g_inarray - SBUFL;
	g_pebuf = g_pearray - SBUFL;
	g_lpbuf = g_lparray - LBUFL;
	g_ivbuf = g_ivarray - PWINL;

	g_vwin[AF - 1][0] = DVWINL;
	g_vwin[AF - 1][1] = DVWINH;
	g_vwin[AF - 2][0] = g_vwin[AF - 2][1] = 0;
	g_awin[AF - 1][0] = DVWINL;
	g_awin[AF - 1][1] = DVWINH;
	g_awin[AF - 2][0] = g_awin[AF - 2][1] = 0;
	for (j = 0; j < AF; j++)
		for (i = 0; i < ORDER; i++)
			g_rcbuf[j][i] = (FLOAT)0.0;
	for (i = 0; i < AF; i++)
		g_rmsbuf[i] = (FLOAT)0.0;
	for (i = 0; i <= AF; i++)
		g_voibuf[i][0]=g_voibuf[i][1] = FALSE;
	g_bias = (FLOAT)0.0;
	for (i = 0; i < AF; i++)
		g_obound[i] = 0;

/* hp100 */
	g_hpa1 = g_hpa2 = g_hpb1 = g_hpb2 = (FLOAT)0.0;
}

/**********************************************************/
