#include <math.h>
#include "c_lpc10.h"

/**********************************************************/
/* Calculate voicing parameters:

Inputs:
.  vwin[AF][2] - Voicing window limits (vwin[AF-1][0] to vwin[AF-1][1])
.  inbuf[] - Raw input speech (with DC bias removed each frame)
.  lpbuf[] - Low-pass filtered speech buffer
.  half - Present analysis half frame number
.  mintau - Lag corresponding to minimum AMDF value (pitch estimate)
In/Ouputs:
.  dither - Zero crossing threshold
Outputs:
.  zc - Zero crossing rate
.  lbe - Low band energy (sum of magnitudes - SM)
.  fbe - Full band energy (SM)
.  qs - Ratio of 6 dB/oct preemphasized energy to full band energy
.  rc1 - First reflection coefficient
.  ar_b - Product of the causal forward and reverse pitch prediction gains
.  ar_f - Product of the noncausal forward and reverse pitch prediction gains
Internal:
.  oldsgn - Previous sign of dithered signal (TRUE for negative)
.  vlen - Length of voicing window
.  start - Lower address of current half of voicing window
.  stop - Upper address of current half of voicing window
.  e_0 - Energy of LPF speech (sum of squares - SS)
.  e_b - Energy of LPF speech backward one pitch period (SS)
.  e_f - Energy of LPF speech forward one pitch period (SS)
.  r_b - Autocovariance of LPF speech backward one pitch period
.  r_f - Autocovariance of LPF speech forward one pitch period
.  lp_rms - Energy of LPF speech (sum of magnitudes - SM)
.  ap_rms - Energy of all-pass speech (SM)
.  e_pre - Energy of 6dB preemphasized speech (SM)
.  e0ap - Energy of all-pass speech (SS) */

VOID vparms( INDEX vwin[AF][2], FLOAT inbuf[], FLOAT lpbuf[],
		INDEX half, FLOAT *dither, INDEX mintau, INDEX *zc,
		INDEX *lbe, INDEX *fbe, FLOAT *qs, FLOAT *rc1,
		FLOAT *ar_b, FLOAT *ar_f )
{
	INDEX i, vlen, start, stop;
	BOOL oldsgn, sign;
	FLOAT lp_rms, ap_rms, e_pre, e0ap, e_0, e_b, r_b, e_f, r_f, tmp;
	FLOAT *ptr1, *ptr2;

/* Calculate zero crossings ({zc}) and several energy and correlation
measures on low band and full band speech. Each measure is taken
over either the first or the second half of the voicing window,
depending on the variable {half}. */
	lp_rms = ap_rms = e_pre = e0ap = (FLOAT)0.0;
	e_0 = e_b = e_f = r_f = r_b = (FLOAT)0.0;
	*rc1 = (FLOAT)0.0;
	*zc = 0;

	vlen = vwin[AF-1][1] - vwin[AF-1][0] + 1;
	start = vwin[AF-1][0] + half * (vlen >> 1) + 1;
	stop = start + (vlen >> 1);
	oldsgn = (inbuf[start-1] - (*dither) < 0);
	ptr1 = lpbuf + start;
	ptr2 = inbuf + start;
	for (i = start; i < stop; i++) {
		lp_rms += (FLOAT)fabs(*ptr1);
		ap_rms += (FLOAT)fabs(*ptr2);
		e_pre += (FLOAT)fabs((*ptr2) - (*(ptr2-1)));
		e0ap += (*ptr2) * (*ptr2);
		*rc1 += (*ptr2) * (*(ptr2-1));
		e_0 += (*ptr1) * (*ptr1);
		e_b += (*(ptr1 - mintau)) * (*(ptr1 - mintau));
		e_f += (*(ptr1 + mintau)) * (*(ptr1 + mintau));
		r_f += (*ptr1) * (*(ptr1 + mintau));
		r_b += (*ptr1) * (*(ptr1 - mintau));
		sign = (*ptr2 + (*dither) < 0);
		if (sign != oldsgn) {
			(*zc)++;
			oldsgn = sign;
		}
		*dither = -(*dither);
		ptr1++;
		ptr2++;
	}

/* Normalized short-term autocovariance coefficient at unit sample delay */
	if (e0ap>(FLOAT)1.0)
		*rc1 /= e0ap;

/* Ratio of the energy of the first difference signal (6 dB/oct preemphasis)
to the energy of the full band signal. */
	if (ap_rms>(FLOAT)0.5)
		*qs = e_pre / ((FLOAT)2.0 * ap_rms);
	else
		*qs = e_pre;

/* ar_b is the product of the forward and reverse prediction gains,
looking backward in time (the causal case): */
	*ar_b = (r_b * r_b);
	if (e_b>(FLOAT)1.0)
		*ar_b /= e_b;
	if (e_0>(FLOAT)1.0)
		*ar_b /= e_0;

/* ar_f is the same as ar_b, but looking forward in time (non causal case).*/
	*ar_f = (r_f * r_f);
	if (e_f>(FLOAT)1.0)
		*ar_f /= e_f;
	if (e_0>(FLOAT)1.0)
		*ar_f /= e_0;

/* Normalize zc, lbe, and fbe to old fixed window length of 180 (LFRAME).
(The fraction 90/vlen has a range of .58 to 1):
*zc = round( *zc*2 * (90./vlen) );
*lbe = min( round( lp_rms*0.25 * (90./vlen) ), 32767 );
*fbe = min( round( ap_rms*0.25 * (90./vlen) ), 32767 );
And this is the actual code: */
	tmp = (FLOAT)LFRAME / (FLOAT)vlen;
	*zc = (INDEX)( (FLOAT)(*zc) * tmp + (FLOAT)0.5 );
	lp_rms *= tmp * (FLOAT)0.125;
	*lbe = (lp_rms>(FLOAT)32767.0) ? (INDEX)32767
			: (INDEX)(lp_rms + (FLOAT)0.5);
	ap_rms *= tmp * (FLOAT)0.125;
	*fbe = (ap_rms>(FLOAT)32767.0) ? (INDEX)32767
			: (INDEX)(ap_rms + (FLOAT)0.5);
}

/**********************************************************/
