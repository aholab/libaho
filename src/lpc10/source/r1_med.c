/**********************************************************************
*
*	MEDIAN Version 45G
*
**********************************************************************
*
*  Find median of three values
*
* Input:
*  D1,D2,D3 - Three input values
* Output:
*  MEDIAN - Median value
*/

#include "sr_lpc10.h"

INT median( INT d1, INT d2, INT d3 )
{
	if ((d2>d1) && (d2>d3)) 
		return ((d3>d1)?d3:d1);
	else if ((d2<d1) && (d2<d3)) 
		return ((d3<d1)?d3:d1);

	return d2;
}
