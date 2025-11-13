#include "d_lpc10.h"
#include <math.h>
#include <stdlib.h>

/**********************************************************/
/* Synthesize One Pitch Epoch

Inputs:
 coef  - Predictor coefficients
 ip    - Pitch period (number of samples to synthesize)
 iv    - Voicing for the current epoch
 rms   - Energy for the current epoch
 ratio - Energy slope for plosives
 g2pass- Sharpening factor for 2 pass synthesis
Outputs:
 sout  - Synthesized speech
Constants:
 ORDER - Synthesizer filter order (number of PC's) */

#ifdef VIEW_EXCIT
/* define VIEW_EXCIT to view just voiced/unvoiced source */
FLOAT kexc[25] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 728, 0, 0, 0, 0,
	0, 0, 0, 0, 0
};
#else
FLOAT kexc[25] = {
	8, -16, 26, -48, 86, -162, 294, -502, 718, -728,
	184, 672, -610, -672, 184, 728, 718, 502, 294, 162,
	86, 48, 26, 16, 8
};
#endif


static INDEX ipo = 0;

#ifdef VIEW_EXCIT
/* define VIEW_EXCIT to view just voiced/unvoiced source */
static FLOAT a0 = .125, a1 = 0, a2 = 0, b0 = 0, b1 = 0, b2 = 0;
#else
static FLOAT a0 = .125, a1 = .75, a2 = .125, b0 = -.125, b1 = .25, b2 = -.125;
#endif

static FLOAT rmso = 0.0, lpi1 = 0.0, lpi2 = 0.0, hpi1 = 0.0, hpi2 = 0.0;

VOID bsynz(FLOAT coef[], INDEX ip, BOOL iv, FLOAT sout[], FLOAT rms,
	FLOAT ratio, FLOAT g2pass)
{
	INDEX px, i, j, k;
	FLOAT lpi0, hpi0;
	FLOAT pulse, sscale, xssq, sum, sum2, ssq, gain, xy;

/*  Calculate history scale factor XY and scale filter state    */
	xy = (FLOAT)(MMIN((rmso / (rms + .000001)), 8.0));
	rmso = rms;
	for (i = 0; i < ORDER; i++)
		exc2[i] = exc2[ipo + i] * xy;

	ipo = ip;

	if (iv == 0) {

/*  Generate white noise for unvoiced   */

		for (i = 0; i < ip; i++)
			exc[ORDER + i] = (FLOAT)(Rrandom() >> 6);

/*  Impulse doublet excitation for plosives */

/*  $$$ en algun momento, poner esta
	 px = ((UINT16)Rrandom()*(ip-1) >> 16) + ORDER + 1;
 */
		px = (INDEX) ((Rrandom() + 32768L) * (ip - 1) >> 16) + ORDER + 1;
		pulse = (FLOAT)(ratio * 85.5);
		if (pulse > 2000)
			pulse = 2000;
		exc[px - 1] += pulse;
		exc[px] -= pulse;
	}
	else {	/*  Load voiced excitation  */
		sscale = (FLOAT)(sqrt((FLOAT)ip) * 0.144341801);
		for (i = 0; i < ip; i++) {
			lpi0 = (i < 25) ? sscale * kexc[i] : 0;
			hpi0 = (FLOAT)(Rrandom() >> 6);
			exc[ORDER + i] = a0 * lpi0 + a1 * lpi1 + a2 * lpi2 +
				b0 * hpi0 + b1 * hpi1 + b2 * hpi2;
			lpi2 = lpi1;
			lpi1 = lpi0;
			hpi2 = hpi1;
			hpi1 = hpi0;
		}
	}

/*   Synthesis filters: */

	xssq = 0;
	for (i = 0; i < ip; i++) {
		k = ORDER + i;
/*    Modify the excitation with all-zero filter  1 + G*SUM   */
/*   Synthesize using the all pole filter  1 / (1 - SUM)    */
		sum = 0;
		sum2 = 0;
		for (j = 0; j < ORDER; j++) {
			sum += coef[j] * exc[k - j - 1];
			sum2 += coef[j] * exc2[k - j - 1];
		}

#ifdef VIEW_EXCIT
/* define VIEW_EXCIT to view just voiced/unvoiced source */
		exc2[k] = exc[k];
		(void)g2pass;  /* remove warning: arg not used */
#else
		exc2[k] = sum * g2pass + exc[k] + sum2;
#endif

		xssq = xssq + exc2[k] * exc2[k];
	}
/*  Save filter history for next epoch  */
	if (ip>MAXPIT) exit(1);
	for (i = 0; i < ORDER; i++) {
		exc[i] = exc[ip + i];
/*      exc2[i] = exc2[ip + i]; */
	}

/*  Apply gain to match RMS */

	ssq = rms * rms * ip;
	gain = (FLOAT)(sqrt(ssq / xssq));

#ifdef VIEW_EXCIT
/* define VIEW_EXCIT to view just voiced/unvoiced source */
	gain *= 0.5;
#endif

	for (i = 0; i < ip; i++)
		sout[i] = gain * exc2[ORDER + i];
}

/**********************************************************/
