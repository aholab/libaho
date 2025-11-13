#include <math.h>
#include "c_lpc10.h"

/**********************************************************/
/* Invert a covariance matrix using Choleski decomposition method
Inputs:
.  psi[ORDER] - Column vector to be predicted
In/Outputs:
.  phi[ORDER,ORDER] - Covariance matrix
Outputs:
.  rc[ORDER] - Pseudo reflection coefficients
Constants:
.  ORDER - Analysis order */


VOID invert( FLOAT phi[ORDER][ORDER], FLOAT psi[ORDER], FLOAT rc[ORDER] )
{
#define EPS ((FLOAT)1.0e-10)
	INDEX i, j, k;
	FLOAT save;

/* Decompose PHI into V * D * V' where V is a triangular matrix whose
main diagonal elements are all 1, V' is the transpose of V, and
D is a vector.  Here D(n) is stored in location V(n,n). */
	for (j = 0; j < ORDER; j++) {
		for (k = 0; k < j; k++) {
			save = phi[j][k] * phi[k][k];
			for (i = j; i < ORDER; i++)
				phi[i][j] -= phi[i][k] * save;
		}

/* Compute intermediate results, which are similar to RC's */
		if ((FLOAT)fabs(phi[j][j]) < EPS) { /* if algorithm terminate early...*/
			for (i = j; i < ORDER; i++)
				rc[i] = (FLOAT)0.0; /* ...zero out higher order RC's */
			return;
		}

		rc[j] = psi[j];
		for (k = 0; k < j; k++)
			rc[j] -= rc[k] * phi[j][k];

		phi[j][j] = (FLOAT)1. / phi[j][j];
		rc[j] *= phi[j][j];

/* Clipping is not necessary. Later we check for stability. */
/*
		if (rc[j] > (FLOAT)0.999)
			rc[j] = (FLOAT)0.999;
		else if (rc[j] < (FLOAT)-0.999)
			rc[j] = (FLOAT)-0.999;
*/
	}
}

/**********************************************************/
