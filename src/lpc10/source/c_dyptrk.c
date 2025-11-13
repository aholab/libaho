#include "c_lpc10.h"

/**********************************************************/
/* Dynamic Pitch Tracker.

Inputs:
.  amdf[LTAU] - Average Magnitude Difference Function array
.  minptr - Location of minimum AMDF value
.  voice - Voicing decision
Outputs:
.  pitch - Smoothed pitch value, 2 frames delayed
.  midx - Initial estimate of current frame pitch
Constants:
.  LTAU - Number of lags in amdf.
Globals:
.  FLOAT s[LTAU]
.  INDEX p[LTAU][2]
.  INDEX ipoint
.  FLOAT alphax */

VOID dyptrk( FLOAT amdf[LTAU], INDEX minptr, BOOL voice,
		INDEX* pitch, INDEX* midx )
{
	FLOAT sbar, minsc, maxsc, alpha;
	INDEX pbar, i, j;

/* Calculate the confidence factor {alpha}, used as a threshold slope in
SEESAW. If unvoiced, set high slope so that every point in {p} array
is marked as a potential pitch frequency. A scaled up version {alphax}
is used to maintain arithmetic precision.    */
	if (voice) {
		g_alphax = (FLOAT)0.75 * g_alphax + amdf[minptr] * (FLOAT)0.5;
		alpha = g_alphax * (FLOAT)0.06250;  /* alphax/16. */
	}
	else {
		g_alphax = (FLOAT)0.984375 * g_alphax;  /* (63./64.)*alphax */
		if (g_alphax < (FLOAT)128.0)
			alpha = (FLOAT)8.0;
		else
			alpha = g_alphax * (FLOAT)0.06250;  /* alphax/16. */
	}

/* SEESAW: Construct a pitch pointer array and intermediate winner function */

/* Left to right pass: */
	g_p[0][g_ipoint] = 0;
	pbar = 0;
	sbar = g_s[0];
	for (i = 0; i < LTAU; i++) {
		sbar += alpha;
		if (sbar < g_s[i]) {
			g_s[i] = sbar;
			g_p[i][g_ipoint] = pbar;
		}
		else {
			sbar = g_s[i];
			g_p[i][g_ipoint] = i;
			pbar = i;
		}
	}

/* Right to left pass: */
	i = pbar;
	sbar = g_s[i];
	while (i) {
		i--;
		sbar += alpha;
		if (sbar < g_s[i]) {
			g_s[i] = sbar;
			g_p[i][g_ipoint] = pbar;
		}
		else {
			i = pbar = g_p[i][g_ipoint];
			sbar = g_s[i];
		}
	}

/* Update {s} using {amdf}. Find maximum, minimum, and location of minimum */
	g_s[0] += amdf[0] * (FLOAT)0.5;
	maxsc = minsc = g_s[0];
	*midx = 0;

	for (i = 1; i < LTAU; i++) {
		g_s[i] += amdf[i] * (FLOAT)0.5;
		if (g_s[i] > maxsc)
			maxsc = g_s[i];
		if (g_s[i] < minsc) {
			minsc = g_s[i];
			*midx = i;
		}
	}

/* Subtract {minsc} from {s} to prevent overflow. */
	for (i = 0; i < LTAU; i++)
		g_s[i] -= minsc;
	maxsc -= minsc;

/* Use higher octave pitch if significant null there. */
	j = 0;
	maxsc *= (FLOAT)0.25;
	for (i = 20; i <= 40; i += 10)
		if ((*midx >= i) && (g_s[*midx - i] < maxsc))
			j = i;
	*midx -= j;

/* TRACE: look back two frames to find minimum cost pitch estimate. */
	*pitch = g_p[*midx][g_ipoint];
	g_ipoint = 1-g_ipoint;
	*pitch = g_p[*pitch][g_ipoint];

/* Original TRACE algoritm (the previous one, optimized for DEPTH=2) : */
/*
#define DEPTH 2
	j = g_ipoint;
	*pitch = *midx;
	for (i = 0; i < DEPTH; i++) {
		*pitch = g_p[*pitch][j];
		j = (j+1) % DEPTH;
	}
	g_ipoint = (g_ipoint + DEPTH - 1) % DEPTH;
*/
}

/**********************************************************/
