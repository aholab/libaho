#include "c_lpc10.h"

/**********************************************************/
/* pitch values table */
INDEX G_tau[LTAU] = {
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,

	40, 42, 44, 46, 48,
	50, 52, 54, 56, 58,
	60, 62, 64, 66, 68,
	70, 72, 74, 76, 78,

	80, 84, 88, 92, 96,
	100, 104, 108, 112, 116,
	120, 124, 128, 132, 136,
	140, 144, 148, 152, 156
};

/**********************************************************/
/* analys */
FLOAT *g_inbuf, *g_pebuf, *g_lpbuf, *g_ivbuf;
FLOAT g_lparray[LBUFH-LBUFL+1], g_ivarray[PWINH-PWINL+1];
FLOAT g_pearray[SBUFH-SBUFL+1], g_inarray[SBUFH-SBUFL+1];
INDEX g_vwin[AF][2];
INDEX g_awin[AF][2];
BOOL g_voibuf[AF+1][2];
FLOAT g_rmsbuf[AF], g_rcbuf[AF][ORDER];
FLOAT g_bias;
INDEX g_obound[AF];
INDEX g_pitch;

/* onset */
INDEX g_osbuf[OSLEN];
INDEX g_osptr;
FLOAT g_n, g_d, g_fpc;
FLOAT g_l2buf[L2WID], g_l2sum1;
INDEX g_l2ptr1, g_l2ptr2, g_lasti;
BOOL g_hyst;

/* preenp */
FLOAT g_zpre[2];

/* voicin */
INDEX g_lbve, g_lbue, g_fbue, g_fbve, g_ofbue, g_olbue;
LINDEX g_sfbue, g_slbue;
FLOAT g_maxmin, g_snr, g_dither;
FLOAT g_voice[AF][2];

/* dyptrk */
FLOAT g_s[LTAU];
INDEX g_p[LTAU][2];
INDEX g_ipoint;
FLOAT g_alphax;

/* hp100 */
FLOAT g_hpa1, g_hpa2, g_hpb1, g_hpb2;

/**********************************************************/

