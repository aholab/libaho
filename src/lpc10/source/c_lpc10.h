#ifndef __C_LPC10_H
#define __C_LPC10_H

/**********************************************************/

#include "x_lpc10.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

#define MINWIN 90
#define MAXWIN 156
#define LTAU 60

#define NF 4
#define AF 3

#define SBUFL ((AF-2)*LFRAME+1)  /* (3-2)*180+1 = 181 */
#define SBUFH (NF*LFRAME)  /* 4*180 = 720 */
#define LBUFL ((AF-2)*LFRAME-MAXPIT+1)  /* (3-2)*180-156+1 = 25 */
#define LBUFH (NF*LFRAME)  /* 4*180 = 720 */
#define PWLEN (MAXPIT+MAXWIN)  /* 156 + 156 = 312 */
#define PWINH (AF*LFRAME)  /* 3 * 180 = 540 */
#define PWINL (PWINH-PWLEN+1)  /*  540 - 312 + 1 = 229 */

/* Default lower and upper limits of voicing window */
#define DVWINL (PWINH-PWLEN/2 - MAXWIN/2 +1)
#define DVWINH (DVWINL+MAXWIN-1)

/* Reference energy for initialization and {dither} threshold in voicin() */
#define REF 3000

/* onset buffer length */
#define OSLEN 10
/* onset: Width of the filter which computes the slope of FPC */
#define L2WID 16

/**********************************************************/
/* global data */

extern INDEX G_tau[LTAU];

/* analys */
extern FLOAT *g_inbuf, *g_pebuf, *g_lpbuf, *g_ivbuf;
extern FLOAT g_lparray[LBUFH-LBUFL+1], g_ivarray[PWINH-PWINL+1];
extern FLOAT g_pearray[SBUFH-SBUFL+1], g_inarray[SBUFH-SBUFL+1];
extern INDEX g_vwin[AF][2];
extern INDEX g_awin[AF][2];
extern BOOL g_voibuf[AF+1][2];
extern FLOAT g_rmsbuf[AF], g_rcbuf[AF][ORDER];
extern FLOAT g_bias;
extern INDEX g_obound[AF];
extern INDEX g_pitch;
/* onset */
extern INDEX g_osbuf[OSLEN];
extern INDEX g_osptr;
extern FLOAT g_n, g_d, g_fpc;
extern FLOAT g_l2buf[L2WID], g_l2sum1;
extern INDEX g_l2ptr1, g_l2ptr2, g_lasti;
extern BOOL g_hyst;

/* preenp */
extern FLOAT g_zpre[2];

/* voicin */
extern INDEX g_lbve, g_lbue, g_fbue, g_fbve, g_ofbue, g_olbue;
extern LINDEX g_sfbue, g_slbue;
extern FLOAT g_maxmin, g_snr, g_dither;
extern FLOAT g_voice[AF][2];

/* dyptrk */
extern FLOAT g_s[LTAU];
extern INDEX g_p[LTAU][2];
extern INDEX g_ipoint;
extern FLOAT g_alphax;

/* hp100 */
extern FLOAT g_hpa1, g_hpa2, g_hpb1, g_hpb2;

/**********************************************************/
/* coded parameters (actually quantized an transmitted) */

#define CODED_VOICE  g_voibuf[AF-2]
#define CODED_PITCH  g_pitch
#define CODED_RMS    g_rmsbuf[AF-3]
#define CODED_RC     g_rcbuf[AF-3]

/**********************************************************/

VOID analys( VOID );
VOID dcbias( INDEX len, FLOAT speech[], FLOAT sigout[] );
VOID difmag( FLOAT speech[], INDEX tau[], INDEX ltau, INDEX maxlag,
		FLOAT amdf[], INDEX* minptr, INDEX* maxptr );
VOID dyptrk( FLOAT amdf[LTAU], INDEX minptr, BOOL voice,
		INDEX* pitch, INDEX* midx );
FLOAT energy_sqrt( INDEX len, FLOAT speech[] );
VOID hp100( FLOAT speech[], FLOAT dest[] );
VOID hp100_i16( INT16 speech[], FLOAT dest[] );
VOID invert( FLOAT phi[ORDER][ORDER], FLOAT psi[ORDER], FLOAT rc[ORDER] );
VOID ivfilt( FLOAT lpbuf[], FLOAT ivbuf[], FLOAT ivrc[2] );
VOID coder_ini( VOID );
VOID lpfilt31( FLOAT inbuf[], FLOAT lpbuf[] );
VOID mload( INDEX awinf, FLOAT speech[],
		FLOAT phi[ORDER][ORDER], FLOAT psi[ORDER] );
VOID onset( FLOAT pebuf[], INDEX osbuf[OSLEN], INDEX* osptr );
VOID placea( INDEX ipitch, BOOL voibuf[AF+1][2], INDEX obound,
		INDEX vwin[AF][2], INDEX awin[AF][2], INDEX ewin[2] );
VOID placev( INDEX osbuf[OSLEN], INDEX osptr,
		INDEX* obound, INDEX vwin[AF][2] );
VOID preemp( FLOAT inbuf[], FLOAT pebuf[], INDEX len, FLOAT z[2] );
VOID rcchk( FLOAT rc1f[ORDER], FLOAT rc2f[ORDER] );
VOID tbdm( FLOAT speech[], INDEX tau[LTAU], FLOAT amdf[LTAU],
		INDEX* minptr, INDEX* maxptr, INDEX* mintau );
VOID voicin( INDEX vwin[AF][2], FLOAT inbuf[], FLOAT lpbuf[], INDEX half,
		FLOAT minamd, FLOAT maxamd, INDEX mintau, FLOAT ivrc[2],
		INDEX* obound, BOOL voibuf[AF+1][2] );
VOID vparms( INDEX vwin[AF][2], FLOAT inbuf[], FLOAT lpbuf[],
		INDEX half, FLOAT *dither, INDEX mintau, INDEX *zc,
		INDEX *lbe, INDEX *fbe, FLOAT *qs, FLOAT *rc1,
		FLOAT *ar_b, FLOAT *ar_f );
VOID shift( VOID );
VOID framein( FLOAT speech[] );
VOID framein_i16( INT16 speech[] );


VOID c_lpc10_ini( VOID );
VOID c_lpc10_code( FLOAT speech[] );
VOID c_lpc10_code_i16( INT16 speech[] );
VOID c_lpc10_peek( BOOL voice[2], INT *pitch,
		FLOAT *rms, FLOAT rc[ORDER] );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
