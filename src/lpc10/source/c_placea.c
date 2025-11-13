#include "c_lpc10.h"

/**********************************************************/
/* Place the Analysis window based on the voicing window placement,
onsets, tentative voicing decision, and pitch. Place also energy window.

Case 1:  Sustained Voiced Speech
If the five most recent voicing decisions are
voiced, then the window is placed phase-synchronously with the
previous window, as close to the present voicing window if possible.
If onsets bound the voicing window, then preference is given to
a phase-synchronous placement which does not overlap these onsets.

Case 2:  Voiced Transition
If at least one voicing decision in AF is voiced, and there are no
onsets, then the window is placed as in case 1.

Case 3:  Unvoiced Speech or Onsets
If both voicing decisions in AF are unvoiced, or there are onsets,
then the window is placed coincident with the voicing window.

Note:  During phase-synchronous placement of windows, the length
is not altered from MAXWIN, since this would defeat the purpose
of phase-synchronous placement.

Inputs:
.  ipitch - pitch in current frame
.  voibuf[AF+1][2] - voicing decision for half-frames.
.  obound - number of onsets in current frame
.  vwin[AF][2] - voicing window position
Outputs:
.  awin[AF][2] - analysis window position
.  ewin[2] - energy window position for current frame (ewin[AF-1])
Constants:
.  LFRAME - analysis frame length (frame rate)
.  MAXWIN - max. analysis window length
.  LRANGE - low limit for window placement
.  HRANGE - high limit for window placement */

VOID placea( INDEX ipitch, BOOL voibuf[AF+1][2], INDEX obound,
		INDEX vwin[AF][2], INDEX awin[AF][2], INDEX ewin[2] )
{
#define LRANGE ((AF-2)*LFRAME + 1)
#define HRANGE (AF*LFRAME)
	INDEX i, j, k;
	BOOL ephase, winv, allv;

/* Check for case 1 and case 2 */
	allv = voibuf[AF-2][1]
			&& voibuf[AF-1][0] && voibuf[AF-1][1]
			&& voibuf[AF][0]   && voibuf[AF][1];

	winv = (voibuf[AF][0] || voibuf[AF][1]);

	if (allv || (winv && (obound == 0))) {
/* APHASE:  Phase synchronous window placement.
Get minimum lower index of the window. */
		i = ((LRANGE-1) - awin[AF-2][0] + ipitch ) / ipitch;
		i = i * ipitch + awin[AF-2][0];
/* MAXWIN = the actual length of this frame's analysis window. */
/* Calculate the location where a perfectly centered window would start. */
		k = (vwin[AF-1][0] + vwin[AF-1][1] - (MAXWIN-1)) >> 1;  /* IDIV 2 */
/* Choose the actual location to be the pitch multiple
closest to this: round((k-i)/ipitch) */
		awin[AF-1][0] = (k>i) ?
				i + (INDEX)((2* (k - i) + ipitch) / (2 * ipitch)) * ipitch
				: i;
		awin[AF-1][1] = awin[AF-1][0] + (MAXWIN-1);
/* If there is an onset bounding the right of the voicing window and the
analysis window overlaps that, then move the analysis window backward
to avoid this onset. */
		if ((obound >= 2) && (awin[AF-1][1] > vwin[AF-1][1])) {
			awin[AF-1][0] -= ipitch;
			awin[AF-1][1] -= ipitch;
		}
/* Similarly for the left of the voicing window. */
		if (((obound == 1) || (obound == 3))
				&& (awin[AF-1][0] < vwin[AF-1][0])) {
			awin[AF-1][0] += ipitch;
			awin[AF-1][1] += ipitch;
		}
/* If this placement puts the analysis window above HRANGE, then
move it backward an integer number of pitch periods. */
		while (awin[AF-1][1] > HRANGE) {
			awin[AF-1][0] -= ipitch;
			awin[AF-1][1] -= ipitch;
		}
/* Similarly if the placement puts the analysis window below LRANGE. */
		while (awin[AF-1][0] < LRANGE) {
			awin[AF-1][0] += ipitch;
			awin[AF-1][1] += ipitch;
		}
/* Make Energy window be phase-synchronous. */
		ephase = TRUE;
	}
	else {  /* Case 3 */
		awin[AF-1][0] = vwin[AF-1][0];
		awin[AF-1][1] = vwin[AF-1][1];
		ephase = FALSE;
	}

/* RMS is computed over an integer number of pitch periods in the analysis
window.  When it is not placed phase-synchronously, it is placed as close
as possible to onsets. */
	if (winv==0) {
		ewin[0] = vwin[AF-1][0];
		ewin[1] = vwin[AF-1][1];
	}
	else {
		j = (INDEX)((awin[AF-1][1] - awin[AF-1][0] + 1) / ipitch) * ipitch;
		if (j==0) {
			ewin[0] = vwin[AF-1][0];
			ewin[1] = vwin[AF-1][1];
		}
		else if ((ephase==0) && (obound == 2)) {
			ewin[0] = awin[AF-1][1] - j + 1;
			ewin[1] = awin[AF-1][1];
		}
		else {
			ewin[0] = awin[AF-1][0];
			ewin[1] = awin[AF-1][0] + j - 1;
		}
	}
}

/**********************************************************/
