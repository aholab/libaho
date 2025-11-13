#include "c_lpc10.h"

/**********************************************************/
/* Voicing window placement.

Arguments
.  osbuf - Buffer which holds sorted indexes of onsets
.  osptr - Free pointer into {osbuf}
.  vwin - Buffer of Voicing Window Positions (Modified)
.  obound - This variable is set by this procedure and used
.           in placing analysis windows (placea).  Bit 1
.           indicates whether an onset bounds the left side
.           of the voicing window, and bit 2 indicates whether
.           an onset bounds the right side of the voicing window.
Variables
.  lrange, hrange - Range in which window is placed
.  osptr1 - {osptr} excluding samaples in 3F

Voicing Window Placement
.
.         __________________ __________________ ______________
.        |                  |                  |
.        |        1F        |        2F        |        3F ...
.        |__________________|__________________|______________
.
.    Previous |
.      Window |
.  ...________|
.
.             |                                |
.      ------>| This window's placement range  |<------
.             |                                |
.
.   There are three cases.  Note that these are different from those
.   given in the LPC-10e phase 1 report.
.
.   1.  If there are no onsets in this range, then the voicing window
.   is centered in the pitch window.  If such a placement is not within
.   the window's placement range, then the window is placed in the left-
.   most portion of the placement range.  Its length is always MAXWIN.
.
.   2.  If the first onset is in 2F and there is sufficient room to place
.   the window immediately before this onset, then the window is placed
.   there, and its length is set to the maximum possible under these
.   constraints.
.
.    "Critical Region Exception":  If there is another onset in 2F
.    such that a window can be placed between the two onsets, the
.    window is placed there (ie, as in case 3).
.
.   3.  Otherwise, the window is placed immediately AFter the onset.  The
.   window's length
.   is the longest length that can fit in the range under these constraints,
.   except that the window may be shortened even further to avoid overlapping
.   other onsets in the placement range.  In any case, the window's length
.   is at least MINWIN.
.
.   Note that the values of MINWIN and LFRAME must be chosen such
.   that case 2 = false implies case 3 = true.   This means that
.   MINWIN <= LFRAME/2.  If this were not the case, then a fourth case
.   would have to be added for when the window cannot fit either before
.   or AFter the onset.
.
.   Note also that onsets which weren't in 2F last time may be in 1F this
.   time, due to the filter delays in computing onsets.  The result is that
.   occasionally a voicing window will overlap that onset.  The only way
.   to circumvent this problem is to add more delay in processing input
.   speech.  In the trade-off between delay and window-placement, window
.   placement lost. */

VOID placev( INDEX osbuf[OSLEN], INDEX osptr,
		INDEX* obound, INDEX vwin[AF][2] )
{
	INDEX lrange, hrange, i, q, osptr1, tmp;
	BOOL crit;

/* Compute the placement range  */
	lrange = ((vwin[AF-2][1] > (AF-2)*LFRAME)
			? vwin[AF-2][1]+1 : (AF-2)*LFRAME+1);
	hrange = AF * LFRAME;

/* Compute OSPTR1, so the following code only looks at relevant onsets. */
	for (osptr1 = osptr; osptr1 > 0; osptr1--) {
		if (osbuf[osptr1 - 1] <= hrange)
			break;
	}

/* Check for case 1 first (fast case): */
	if ((!osptr1) || (osbuf[osptr1 - 1] < lrange)) {
		vwin[AF-1][0] = (vwin[AF-2][1]>=DVWINL)?vwin[AF-2][1]+1:DVWINL;
		vwin[AF-1][1] = vwin[AF-1][0] + MAXWIN - 1;
		*obound = 0;
	}
	else {
/* Search backward in OSBUF for first onset in range.
This code relies on the above check being performed first. */
		for (q = osptr1-1; q > 0; q--) {
			if (osbuf[q-1] < lrange)
				break;
		}

/* Check for case 2 (placement before onset):
Check for critical region exception: */
		crit = FALSE;
		for (i = q + 1; i < osptr1; i++) {
			if (osbuf[i] - osbuf[q] >= MINWIN) {
				crit = TRUE;
				break;
			}
		}

		tmp = lrange+(MINWIN-1);
		if ((crit==FALSE)&&(osbuf[q]> MMAX((AF-1)*LFRAME, tmp))) {
				vwin[AF-1][1] = osbuf[q] - 1;
				tmp = vwin[AF-1][1] - (MAXWIN-1);
				vwin[AF-1][0] = MMAX(lrange, tmp);
				*obound = 2;
		}

/* Case 3 (placement after onset)   */
		else {
			vwin[AF-1][0] = osbuf[q];

			while (1) {
				q++;
				if (q < osptr1) {
					if (osbuf[q] <= vwin[AF-1][0] + MAXWIN) {
						if (osbuf[q] < vwin[AF-1][0] + MINWIN)
							continue;
						vwin[AF-1][1] = osbuf[q] - 1;
						*obound = 3;
					}
					else {
						tmp = vwin[AF-1][0] + MAXWIN - 1;
						vwin[AF-1][1] = MMIN(tmp, hrange);
						*obound = 1;
					}
				}
				else {
					tmp = vwin[AF-1][0] + MAXWIN - 1;
					vwin[AF-1][1] = MMIN(tmp, hrange);
					*obound = 1;
				}
				break;
			}	/* while */
		}
	}
}

/**********************************************************/
