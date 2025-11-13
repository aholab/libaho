#include "c_lpc10.h"

/**********************************************************/
/* Load a covariance matrix
Inputs:
.  awinf - Analysis window finish
.  speech[] - Speech buffer ({awinf} elements)
Outputs:
.  phi[ORDER,ORDER] - Covariance matrix
.  psi[ORDER] - Prediction vector
Constants:
.  ORDER - Analysis order */

/* definiendo MLOAD2, variante para el calculo de la matriz
de covarianza: desplaza PHI una muestra hacia adelante, con lo que la
primera columna es PSI (menos cuentas, pero algo distinto...) */
#ifndef ORIG
#define MLOAD2
#endif


VOID mload( INDEX awinf, FLOAT speech[],
		FLOAT phi[ORDER][ORDER], FLOAT psi[ORDER] )
{
	INDEX r, c, i;

	awinf--;  /* decrement awinf, as we always use awinf-1 */


#ifndef MLOAD2
/* Load first column of triangular covariance matrix PHI */
	for (r = 0; r < ORDER; r++) {
		phi[r][0] = (FLOAT)0.0;
		for (i = ORDER - 1; i < awinf; i++)
			phi[r][0] += speech[i] * speech[i - r];
	}

/* Load last element of vector PSI */
	psi[ORDER - 1] = (FLOAT)0.0;
	for (i = ORDER; i <= awinf; i++)
		psi[ORDER - 1] += speech[i] * speech[i - ORDER];

/* End correct to get additional columns of PHI */
	for (r = 1; r < ORDER; r++)
		for (c = 1; c <= r; c++)
			phi[r][c] = phi[r - 1][c - 1]
					- speech[awinf - r] * speech[awinf - c]
					+ speech[(ORDER - 1) - r] * speech[(ORDER - 1) - c];

/* End correct to get additional elements of PSI */
	for (c = 1; c < ORDER; c++)
		psi[c - 1] = phi[c][0]
				- speech[ORDER - 1] * speech[(ORDER - 1) - c]
				+ speech[awinf] * speech[awinf - c];
#else
/* Load first column of triangular covariance matrix PHI */
	for (r = 0; r < ORDER; r++) {
		phi[r][0] = (FLOAT)0.0;
		for (i = ORDER; i <= awinf; i++)
			phi[r][0] += speech[i] * speech[i - r];
	}

/* Load last element of vector PSI */
	psi[ORDER - 1] = (FLOAT)0.0;
	for (i = ORDER; i <= awinf; i++)
		psi[ORDER - 1] += speech[i] * speech[i - ORDER];

/* End correct to get additional columns of PHI */
	for (r = 0; r < ORDER-1; r++)
		for (c = 0; c <= r; c++)
			phi[r+1][c+1] = phi[r][c]
					- speech[awinf - r] * speech[awinf - c]
					+ speech[(ORDER - 1) - r] * speech[(ORDER - 1) - c];

/* End correct to get additional elements of PSI */
	for (c = 1; c < ORDER; c++)
		psi[c-1] = phi[c][0];
#endif
}

/**********************************************************/
