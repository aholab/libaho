/**********************************************************/

#include <string.h>

#include "spl.h"
#include "uti.h"
#include "str2win.h"
#include "xfft.h"

/**********************************************************/

DOUBLE *str2win( const CHAR *strwin, DOUBLE *win, INT len )
{
	if (!strwin) win_rect(win,len);
	else if (!strcmp(strwin,"Hamm")) win_hamm(win,len);
	else if (!strcmp(strwin,"Hann")) win_hann(win,len);
	else if (!strcmp(strwin,"Bart")) win_bart(win,len);
	else if (!strcmp(strwin,"Black")) win_black(win,len);
	else if (!strcmp(strwin,"Rect")) win_rect(win,len);
	else if (!strcmp(strwin,"None")) win_rect(win,len);
	else die_beep("Invalid window (%s)",strwin);

	return win;
}

/**********************************************************/

INT str2xfftwin( const CHAR *strwin )
{
	if (!strwin) return XFFT_WIN_RECT;
	else if (!strcmp(strwin,"Hamm")) return XFFT_WIN_HAMM;
	else if (!strcmp(strwin,"Hann")) return XFFT_WIN_HANN;
	else if (!strcmp(strwin,"Bart")) return XFFT_WIN_BART;
	else if (!strcmp(strwin,"Black")) return XFFT_WIN_BLACK;
	else if (!strcmp(strwin,"Rect")) return XFFT_WIN_RECT;
	else if (!strcmp(strwin,"None")) return XFFT_WIN_RECT;
	else die_beep("Invalid window (%s)",strwin);
  
	return XFFT_WIN_RECT;
}

/**********************************************************/

DOUBLE *xfftwin2win( INT xfftwin, DOUBLE *win, INT len )
{
	switch (xfftwin) {
	case XFFT_WIN_HAMM:win_hamm(win,len);
	case XFFT_WIN_HANN:win_hann(win,len);
	case XFFT_WIN_BART:win_bart(win,len);
	case XFFT_WIN_BLACK:win_black(win,len);
	default: win_rect(win,len);break;
	}

	return win;
}

/**********************************************************/
