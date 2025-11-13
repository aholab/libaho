#ifndef __D_LPC10_H
#define __D_LPC10_H

/**********************************************************/

#include "x_lpc10.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

#define MAXPEP 11

/**********************************************************/
/* define VIEW_EXCIT to view just voiced/unvoiced source */

#define VIEW_EXCITxx

/**********************************************************/
/* global data */

/* synth */
extern INDEX ipiti[MAXPEP];
extern BOOL ivuv[MAXPEP];

extern FLOAT rci[MAXPEP][ORDER];
extern FLOAT rmsi[MAXPEP];
extern FLOAT pc[ORDER];
extern FLOAT noise[MAXPIT + ORDER];

/* bsynth */
extern FLOAT exc[MAXPIT + ORDER];
extern FLOAT exc2[MAXPIT + ORDER];

//extern INDEX G_tau[LTAU];

/**********************************************************/

VOID decoder_ini(VOID);
VOID deemp( FLOAT x[], INDEX n );
VOID irc2pc( FLOAT rc[], FLOAT pc[], FLOAT gprime, FLOAT *g2pass );
INT16 Rrandom(VOID);
VOID bsynz(FLOAT coef[], INDEX ip, BOOL iv, FLOAT sout[], FLOAT rms,
		FLOAT ratio, FLOAT g2pass);
VOID pitsyn( BOOL voice[], INDEX pitch, FLOAT rms, FLOAT rc[], BOOL ivuv[],
	INDEX ipiti[], FLOAT rmsi[], FLOAT rci[][ORDER], INDEX * nout,
	FLOAT *ratio);
VOID synths(BOOL voice[], INDEX pitch, FLOAT rms, FLOAT rc[], FLOAT speech[], INDEX * k);

VOID d_lpc10_ini( VOID );
INT d_lpc10_decode( BOOL voice[2], INT pitch, FLOAT rms,
	FLOAT rc[ORDER], FLOAT speech[] );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

#endif
