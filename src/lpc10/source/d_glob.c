#include "d_lpc10.h"

/**********************************************************/

INDEX ipiti[MAXPEP];
BOOL ivuv[MAXPEP];

FLOAT rci[MAXPEP][ORDER];
FLOAT rmsi[MAXPEP];
FLOAT pc[ORDER];
FLOAT exc[MAXPIT + ORDER];
FLOAT exc2[MAXPIT + ORDER];
FLOAT noise[MAXPIT + ORDER];

/**********************************************************/
