#include "d_lpc10.h"
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

/**********************************************************/
/* Synthesize a single pitch epoch

Inputs:
  VOICE  - Half frame voicing decisions
  PITCH  - Pitch
  RMS    - Energy
  RC     - Reflection coefficients
  LFRAME - Length of speech buffer
Outputs:
  IVUV   - Pitch epoch voicing decisions
  IPITI  - Pitch epoch length
  RMSI   - Pitch epoch energy
  RCI    - Pitch epoch RC's
  NOUT   - Number of pitch periods in this frame
  RATIO  - Previous to present energy ratio
Constants:
  ORDER  - Synthesis order (number of RC's) */

static FLOAT rco[ORDER];
static FLOAT yarc[ORDER];
static BOOL ivoico = 0;
static INDEX ipito = 0;
static INDEX jsamp;
static FLOAT rmso = 1;
static BOOL first = 1;

void pitsyn(BOOL voice[], INDEX pitch, FLOAT rms, FLOAT rc[], BOOL ivuv[],
	INDEX ipiti[], FLOAT rmsi[], FLOAT rci[][ORDER], INDEX * nout,
	FLOAT *ratio)
{
	INDEX i, j, lsamp, ip, istart;
	BOOL ivoice;
	INDEX jused, nl;
	FLOAT alrn, alro, prop;
	FLOAT slope, uvpit, vflag, xxy;

	if (rms < 1)
		rms = 1;

	if (rmso < 1)	/* $$$ quitar este if, no hace nada */
		rmso = 1;

	uvpit = 0.0;
	*ratio = (FLOAT)(rms / (rmso + 8.));

	if (first) {	/*1 */
		lsamp = 0;
		ivoice = voice[1];
		if (ivoice == 0) {
			pitch = LFRAME >> 2;  /* $$$ ponia  * 0.25 */
		}
		*nout = LFRAME / pitch;
		jsamp = LFRAME - *nout * pitch;
		for (i = 0; i < *nout; i++) {
			for (j = 0; j < ORDER; j++)
				rci[i][j] = rc[j];
			ivuv[i] = ivoice;
			ipiti[i] = pitch;
			rmsi[i] = rms;
		}
		first = 0;
	}	/*1 */
	else {	/*1 */
		vflag = 0;
		lsamp = LFRAME + jsamp;
/*      slope = (pitch - ipito) / (FLOAT)lsamp; */
		*nout = 0;
		jused = 0;
		istart = 1;
		if ((voice[0] == ivoico) && (voice[1] == voice[0])) {	/*2 */
			if (voice[1] == 0) {
/* SSUV - -   0  ,  0  ,  0 */
				pitch = LFRAME >> 2;  /* $$$ ponia  * 0.25 */
				ipito = pitch;
				if (*ratio > 8)
					rmso = rms;
			}
/* SSVC - -   1  ,  1  ,  1 */
			slope = (pitch - ipito) / (FLOAT)lsamp;
			ivoice = voice[1];
		}	/*2 */
		else {	/*2 */
			if (ivoico != 1) {
				if (ivoico == voice[0])
/* UV2VC2 - -  0  ,  0  ,  1    */
					nl = lsamp - (LFRAME >> 2);  /* $$$ ponia  * 0.25 */
				else
/* UV2VC1 - -  0  ,  1  ,  1    */
					nl = lsamp - 3 * (LFRAME >> 2);  /* $$$ ponia  * 0.25 */

				ipiti[0] = nl >> 1; /* $$$ ponia * 0.5; */
				ipiti[1] = nl - ipiti[0];
				ivuv[0] = 0;
				ivuv[1] = 0;
				rmsi[0] = rmso;
				rmsi[1] = rmso;
				for (i = 0; i < ORDER; i++) {
					rci[0][i] = rco[i];
					rci[1][i] = rco[i];
					rco[i] = rc[i];
				}
				slope = 0;
				*nout = 2;
				ipito = pitch;
				jused = nl;
				istart = nl + 1;
				ivoice = 1;
			}
			else {
				if (ivoico != voice[0])
/* VC2UV1 - -   1  ,  0  ,  0   */
					lsamp = (LFRAME >> 2) + jsamp;  /* $$$ ponia  * 0.25 */
				else
/* VC2UV2 - -   1  ,  1  ,  0   */
					lsamp = 3 * (LFRAME >> 2) + jsamp;  /* $$$ ponia  * 0.25 */

				for (i = 0; i < ORDER; i++) {
					yarc[i] = rc[i];
					rc[i] = rco[i];
				}
				ivoice = 1;
				slope = 0.;
				vflag = 1;
			}
		}	/*2 */

		while (1) {		/*3 */
			for (i = istart; i <= lsamp; i++) {		/*4 */
				ip = (INDEX)(ipito + slope * i + .5);  /* $$$ cast de float a index */
				if (uvpit != 0.0)
					ip = (INDEX)uvpit;   /* $$$ cast de float a index */
				if (ip <= i - jused) {	/*5 */
					if (*nout >= MAXPEP) {
						fprintf(stderr, "PITSYN: too many epochs\n");
						exit(1);
					}
					ipiti[*nout] = ip;
					pitch = ip;
					ivuv[*nout] = ivoice;
					jused += ip;
					prop = (FLOAT)(jused - ip * 0.5) / (FLOAT)lsamp;
					for (j = 0; j < ORDER; j++) {
						alro = (FLOAT)(log((1 + rco[j]) / (1 - rco[j])));
						alrn = (FLOAT)(log((1 + rc[j]) / (1 - rc[j])));	/* $$$ se sale de j?? */
						xxy = alro + prop * (alrn - alro);
						xxy = (FLOAT)(exp(xxy));
						rci[*nout][j] = (xxy - 1) / (xxy + 1);
					}
					rmsi[*nout] = (FLOAT)(log(rmso) + prop * (log(rms) - log(rmso)));
					rmsi[*nout] = (FLOAT)(exp(rmsi[*nout]));
					(*nout)++;
				}	/*5 */
			}	/*4 */
			if (vflag != 1)
				break;
			vflag = 0;
			istart = jused + 1;
			lsamp = LFRAME + jsamp;
			slope = 0;
			ivoice = 0;
			uvpit = (FLOAT)((lsamp - istart) * 0.5);
			if (uvpit > 90)
				uvpit *= 0.5;
			rmso = rms;
			for (i = 0; i < ORDER; i++) {
				rc[i] = yarc[i];
				rco[i] = yarc[i];
			}
		}	/*3 */
		jsamp = lsamp - jused;
	}	/*1 */
	if (*nout != 0) {
		ivoico = voice[1];
		ipito = pitch;
		rmso = rms;
		for (i = 0; i < ORDER; i++)
			rco[i] = rc[i];
	}

}

/**********************************************************/
