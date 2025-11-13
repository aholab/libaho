/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL1.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.2    15/05/96  Borja     bugs en documentacion
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Manejo de matrices especiales, resolucion de sistemas
de ecuaciones, calculo de autovalores....

Definir NDEBUG para desconectar la validacion de parametros
con assert(). No definir este simbolo mientras se depuren
aplicaciones, ya que aunque las funciones son algo mas lentas,
el chequeo de validacion de parametros resuelve muchos problemas.


Matrices y sistemas de ecuaciones basados en matrices.
Los elementos de las matrices se referencian por (i,j) siendo
i la fila y j la columna. i=1...n, j=1...n.

Matrices simetricas: matrices matrix_sim
y matrices triangulares matrix_tri
----------------------------------------

La estructura que se almacena en estas matrices es la siguiente:

.        a(1,1)
.        a(2,1)  a(2,2)
.        ....
.        a(n,1) ............a(n,n)

la primera fila tiene un elemento, la segunda 2... hasta n lineas para
una matriz cuadrada de orden n. La mitad superior de la matriz es la
simetrica de la inferior en las matrices simetricas, o nula en las
matrices triangulares.

Se almacena todo por lineas de izq. a derecha y de arriba a abajo:
.       a(1,1) a(2,1) a(2,2) a(3,1) ... a(n,n)

La funcion idx_matrix_sim(i,j) {devuelve} el indice dentro del array
lineal y empezando en cero, para el elemento (i,j) de la matriz.
Por ejemplo, idx_matrix_sim(3,1) {devuelve} 3 (cuarto elemento del array).
Similar para las triangulares con idx_matrix_tri(i,j).
En las funciones nel_matrix_tri(n) y nel_matrix_sim(n) se {devuelve} el
numero de elementos que se deben almacenarse para representar una
matriz triangular o simetrica.


Matrices diagonales: matrices matrix_dgn
----------------------------------------

La estructura que se almacena en estas matrices es la siguiente:

.         a(1,1)
.                 a(2,2)
.                          ...
.                                 a(n,n)

Cada fila tiene solo un elemento, es resto de elementos hasta formar
la matriz cuadrada de orden n son nulos.

Se almacenan solo los elementos no nulos, en un vector lineal:
.       a(1,1) a(2,2) a(3,3) ... a(n,n)

La funcion idx_matrix_dgn(i) {devuelve} el indice dentro del array lineal
y empezando en cero, para el elemento (i,i) de la matriz. Por ejemplo,
idx_matrix_dgn(3,3) {devuelve} 2 (tercer elemento del array).
La funcion nel_matrix_dgn(n) {devuelve} el numero de elementos
que deben almacenarse para representar una matriz diagonal.

Ref:
.    Digital Processing of Speech Signals
.    L.R. Rabiner / R.W Schafer
.    Prentice Hall

Ref Autovalores y autovectores:
.    Numerica Recipes
.    W.H. Press, B.PUBLIC.Flannery,...
.    Cambridge University Press
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "spli.h"

/*<DOC>*/
/**********************************************************/
/* {devuelve} el indice dentro de un array para el elemento
({i},{j}) de una matriz simetrica de cualquier orden.
{i} son filas, {j} columnas.
Aunque el elemento ({i},{j}) es igual que el ({j},{i}),
enviar _siempre_ {j}<={i}.
{i}>=1, {j}>=1.

La estructura que se almacena en esta matriz es la siguiente:

.        a(1,1)
.        a(2,1)  a(2,2)
.        ....
.        a(n,1) ............a(n,n)

la primera fila tiene un elemento, la segunda 2... hasta n lineas para
una matriz cuadrada de orden n. La mitad superior de la matriz es la
simetrica de la inferior.

Se almacena todo por lineas de izq. a derecha y de arriba a abajo:
.     a(1,1) a(2,1) a(2,2) a(3,1) ... a(n,n) */

PUBLIC SPL_INT XAPI idx_matrix_sim( SPL_INT i, SPL_INT j )
/*</DOC>*/
{
	assert(i>0);
	assert(j>0);
	assert(j<=i);

	return __idx_matrix_sim(i,j);
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} el numero de elementos que se deben almacenar en una
matriz simetrica de orden {n}. {n}>=0 */

PUBLIC SPL_INT XAPI nel_matrix_sim( SPL_INT n )
/*</DOC>*/
{
	assert(n>=0);

	return __idx_matrix_sim(n+1,1);
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} el indice dentro de un array para el elemento ({i},{j})
de una matriz triangular de cualquier orden.
{i} son filas, {j} columnas.
Aunque el elemento ({i},{j}) es igual que el ({j},{i}),
enviar _siempre_ {j}<={i}.
{i}>=1, {j}>=1.

La estructura que se almacena en esta matriz es la siguiente:

.       a(1,1)
.       a(2,1)  a(2,2)
.       ....
.       a(n,1) ............ a(n,n)

la primera fila tiene un elemento, la segunda 2... hasta n lineas para
una matriz cuadrada de orden n. La mitad superior de la matriz es nula.

Se almacena todo por lineas de izq. a derecha y de arriba a abajo:
.     a(1,1) a(2,1) a(2,2) a(3,1) ... a(n,n) */

/* Estas macros se encuentran en SPL.H
#define __idx_matrix_tri(i,j)  __idx_matrix_sim(i,j)
#define idx_matrix_tri(i,j)  idx_matrix_sim(i,j)
*/
/*</DOC>*/

/*<DOC>*/
/**********************************************************/
/* {devuelve} el numero de elementos que se deben almacenar en una
matriz triangular de orden {n}. {n}>=0 */

/* Esta macro esta en SPL.H
#define nel_matrix_tri(n) nel_matrix_sim(n)
*/
/*</DOC>*/

/*<DOC>*/
/**********************************************************/
/* {devuelve} el indice dentro de un array para el elemento
({i},{j}) de una matriz diagonal de cualquier orden.
{i} son filas, {j} columnas.
{i}>=1, {j}>=1.

La estructura que se almacena en esta matriz es la siguiente:

.        a(1,1)
.                a(2,2)
.                       ...
.                           a(n,n)

la primera fila tiene un elemento, la segunda otro... hasta n lineas para
una matriz cuadrada de orden n. La parte inferior y la superior a la
diagonal es nula.

Se almacena todo por lineas de izq. a derecha y de arriba a abajo:
.          a(1,1) a(2,2) a(3,3) ... a(n,n)  */

PUBLIC SPL_INT XAPI idx_matrix_dgn( SPL_INT i )
/*</DOC>*/
{
	assert(i>0);

	return __idx_matrix_dgn(i);
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} el numero de elementos que se deben almacenar en una
matriz diagonal de orden {n}. {n}>=0 */

PUBLIC SPL_INT XAPI nel_matrix_dgn( SPL_INT n )
/*</DOC>*/
{
	assert(n>=0);

	return n;
}

/*<DOC>*/
/**********************************************************/
/* Triangulacion de matriz simetrica definida positiva.
Dada una matriz simetrica {msi} de orden {n}>=0, calcula una
matriz triangular {mtr} del mismo orden, tal que {mtr} multiplicada
por {mtr} transpuesta es igual a {msi}.
Solo para matrices simetricas definidas positivas.
{msi} y {mtr} _no_ pueden ser punteros a la misma matriz en memoria.
El formato de {msi} y {mtr} es el explicado para matrices simetricas.
Ambas matrices deben ser de nel_matrix_sim(n) elementos.
{n}>=0, {msi}!={mtr} */

PUBLIC SPL_VOID XAPI matrix_sim2tri( SPL_pFLOAT msi, SPL_pFLOAT mtr,
		SPL_INT n )
/*</DOC>*/
{
	SPL_INT i,j,k, i1, i1o, i2;
	SPL_FLOAT sum;
	assert(n>=0);
	assert(msi!=mtr);

	i1=0;
	for (i=0; i<n; i++) {
		i1o=i1;
		i2=0;
		for (j=0; j<i; ) {
			__xsub(k,0,j,mtr[i1o+k]*mtr[i2+k],sum,msi[i1]);
			mtr[i1++]=sum/mtr[i2+j];
			j++;
			i2+=j;
		}
		__xsub(k,0,i,mtr[i1o+k]*mtr[i1o+k],sum,msi[i1]);
		mtr[i1++]= sqrt(sum);
	}
}

/*<DOC>*/
/**********************************************************/
/* Triangulacion de matriz simetrica definida positiva por Cholesky.
Dada una matriz simetrica {msi} de orden {n}>=0, calcula una
matriz triangular {mtr} del mismo orden, y una matriz diagonal {mdi}
tambien del mismo orden, tal que {mtr} multiplicada por {mdi} y por
{mtr} transpuesta es igual a {msi}.
La matriz triangular tiene todos los elementos de la diagonal principal
iguales a 1. Por tanto, esta matriz se puede almacenar como una matriz
triangular de orden {n}-1.
Solo para matrices simetricas definidas positivas.
{msi}, {mdi} y {mtr} _no_ pueden ser la misma matriz en memoria.
El formato de {msi} y {mtr} es el explicado para matrices simetricas.
La matriz {msi} es de nel_matrix_sim(n) elementos, mientras que la matriz
triangular {mtr} sera de nel_matrix_tri(n-1) elementos.
El formato de {mdi} es el explicado para matrices diagonales, y
debe tener nel_matrix_dgn(n) elementos */

PUBLIC SPL_VOID XAPI matrix_sim2tri_dgn( SPL_pFLOAT msi, SPL_pFLOAT mtr,
		SPL_pFLOAT mdi, SPL_INT n )
/*</DOC>*/
{
	SPL_INT i,j,k;
	SPL_INT bi,bj,ai;
	SPL_FLOAT sum;
	assert(n>=0);
	assert(msi!=mtr);
	assert(msi!=mdi);

	mdi[0] = msi[0];
	bj=0;
	for (j=0; j<n-1; ) {
		ai=bj+j+1;
		bi=bj;
		for (i=j+2; i<=n; i++) {
			__xsub(k,0,j,mtr[bi+k]*mtr[bj+k-j]*mdi[k],sum,msi[ai+j])
			mtr[bi+j] = sum/mdi[j];
			bi+=(i-1);
			ai+=i;
		}
		j++;
		__xsub(k,0,j,mtr[bj+k]*mtr[bj+k]*mdi[k],sum,msi[bj+j+j]);
		mdi[j] = sum;
		bj+=j;
	}
}

/*<DOC>*/
/**********************************************************/
/* resolucion de ecuaciones con matriz triangular.
Dado un sistema de ecuaciones lineales, matricialmente:

.                   A*x=c

donde
.      A=matriz cuadrada triangular de dimension n
.      x=vector incognita de dimension n
.      c=vector de terminos independientes de dimension n

Conocidos A y c, esta funcion resuelve el sistema (calcula el
vector x).
Parametros:
- SPL_pFLOAT {mA} : Matriz triangular A de dimension {n}.
.                    de nel_matrix_tri(n) elementos.
- SPL_pFLOAT {vc} : Vector de terminos independientes c.
.                    de {n} elementos.
- SPL_INT {n} : Orden o dimension del sistema.
- SPL_pFLOAT {vx} : Vector de resultado, de {n} elementos. */

PUBLIC SPL_VOID XAPI ecsys_tri( SPL_pFLOAT mA, SPL_pFLOAT vc, SPL_INT n,
		SPL_pFLOAT vx )
/*</DOC>*/
{
	SPL_INT i,k, bi;
	SPL_FLOAT sum;
	assert(n>=0);

	bi=0;
	for (i=0; i<n; i++) {
		bi+=i;
		__xsub(k,0,i,mA[bi+k]*vx[k],sum,vc[i]);
		vx[i]=sum/mA[bi+i];
	}
}

/*<DOC>*/
/**********************************************************/
/* resolucion de ecuaciones con matriz simetrica triangulada.
Dado un sistema de ecuaciones lineales, matricialmente:

.                   A*x=c

donde
.      A=matriz cuadrada de dimension n
.      x=vector incognita de dimension n
.      c=vector de terminos independientes de dimension n

Si A es simetrica definida positiva, se puede expresar como

.                  A=B*B'

Donde B es una matriz triangular, y B' es su transpuesta.
Conocidos B y c, esta funcion resuelve el sistema (calcula el
vector x). B se puede obtener a partir de A mediante la funcion
matrix_sim2tri().
Parametros:
- SPL_pFLOAT {mB} : Matriz triangular B de dimension {n}.
.                    de nel_matrix_tri(n) elementos.
- SPL_pFLOAT {vc} : Vector de terminos independientes c, de {n} elementos.
- SPL_INT {n} : Orden o dimension del sistema
- SPL_pFLOAT {vx} : Vector de resultado, de {n} elementos. */

PUBLIC SPL_VOID XAPI ecsys_simtri( SPL_pFLOAT mB, SPL_pFLOAT vc, SPL_INT n,
		SPL_pFLOAT vx )
/*</DOC>*/
{
	SPL_INT i,k, bi, bk;
	SPL_FLOAT sum;
	assert(n>=0);

	bi=0;
	for (i=0; i<n; i++) {
		bi+=i;
		__xsub(k,0,i,mB[bi+k]*vx[k],sum,vc[i]);
		vx[i]=sum/mB[bi+i];
	}

	for (i=n-1; i>=0; i--) {
		bk=bi;
		__xsub(k,i+1,n,mB[(bk+=k)+i]*vx[k],sum,vx[i]);
		vx[i]=sum/mB[bi+i];
		bi-=i;
	}
}

/*<DOC>*/
/**********************************************************/
/* resolucion de ecuaciones por Cholesky. Es mucho mas seguro
que el metodo de la matriz triangulada.
Dado un sistema de ecuaciones lineales, matricialmente:

.                   A*x=c

donde
.      A=matriz cuadrada de dimension n
.      x=vector incognita de dimension n
.      c=vector de terminos independientes de dimension n

Si A es simetrica definida positiva, se puede expresar como

.                   A=B*d*B'

Donde B es una matriz triangular inferior (diagonal principal=1),
B' es su transpuesta, y d es una matriz diagonal.
Al ser B triangular inferior, se puede guardar como matriz triangular
de dimension n-1.
Conocidos B, d y c, esta funcion resuelve el sistema (calcula el
vector x). B y d se pueden obtener a partir de A mediante la funcion
matrix_sim2tri_dgn().
Parametros:
- SPL_pFLOAT {mB} : Matriz triangular inferior B de dimension {n}-1.
.                    de nel_matrix_tri(n-1) elementos.
- SPL_pFLOAT {md} : Matriz diagonal d de dimension {n}.
.                    de nel_matriz_dgn(n) elementos.
- SPL_pFLOAT {vc} : Vector de terminos independientes c.
.                    de {n} elementos.
- SPL_INT {n} : Orden o dimension del sistema
- SPL_pFLOAT {vx} : Vector de resultado.
.                    de {n} elementos.
*/

PUBLIC SPL_VOID XAPI ecsys_tri_dgn( SPL_pFLOAT mB, SPL_pFLOAT md,
		SPL_pFLOAT vc, SPL_INT n, SPL_pFLOAT vx )
/*</DOC>*/
{
	SPL_INT i,k, bi, bk;
	SPL_FLOAT sum;
	assert(n>=0);

	bi=0;
	for (i=0; i<n; i++) {
		__xsub(k,0,i,mB[bi+k]*vx[k],sum,vc[i]);
		vx[i]=sum;
		bi+=i;
	}

	bi-=i-1;
	for (i=n-1; i>=0; ) {
		bk=bi;
		__xsub(k,i+1,n,mB[(bk+=k-1)+i]*vx[k],sum,vx[i]/md[i]);
		vx[i]=sum;
		i--;
		bi-=i;
	}
}

/*<DOC>*/
/**********************************************************/
/* Calculo de autovalores y autovectores normalizados de una
matriz tridiagonal simetrica, por metodo QL con desplazamientos
implicitos (Ref: Numerical Recipes, rutina tqli() ).
El orden de la matriz se envia en {n}, y solo es necesario enviar
la diagonal principal ({n} elementos) en {vd}, y la subdiagonal
inferior (o la superior, pues es la misma) en {ve}. La subdiagonal
solo tiene {n}-1 elementos, pero el vector {ve} que se envie, debe
tener espacio para un elemento mas al final, por lo que en total
tambien sera de {n} elementos. El valor de este ultimo elemento
dentro del vector {ve} es irrelevante.
La funcion calcula los autovalores y los devuelve en el propio
vector {vd}. Ademas {ve} queda con valores indefinidos.
El calculo de los autovectores es opcional, pues requiere muchas
operaciones. Si se envia NULL en {mz}, los autovectores no se calculan.
Para calcular los autovectores, enviar en {mz} un puntero a un array de
{n}*{n} elementos (NOTA IMPORTANTE ABAJO), en el que se almacenara en
cada fila el autovector correspondiente a cada autovalor almacenado
en {d}. El autovector correspondiente al autovalor d[0] se almacena
en z[0]-->z[n-1] (es un vector de {n} componentes). Despues, el
correspondiente a d[1], se almacena en z[n]-->z[2*n-1]; asi hasta
los {n} autovectores. En general, el autovector de d[x] esta
en z[n*x]-->z[n*x+n-1].
NOTA IMPORTANTE: Si se va a calcular los autovectores, la matriz de
autovectores {mz} debe enviarse inicializada como una matriz unidad!!!
es decir, todo a ceros (z[i*n+j]=0 para i,j=0,1...n-1, i!=j) y la
diagonal principal a unos (z[n*i+i]=1 para i=0,1...n-1). Esto es asi
porque esta funcion tiene otros usos (Numerical Recipes)....

La funcion {devuelve} un valor booleano, que sera FALSE (0) en caso de
que todo funcione correctamente. Pero si en algun momento se supera
el numero maximo de iteraciones permitidas (EIGEN_MAXITER, definido
en SPL.H) entonces {devuelve} TRUE, y los resultados quedan indefinidos */

PUBLIC SPL_BOOL XAPI eigen_tdg( SPL_pFLOAT vd, SPL_pFLOAT ve, SPL_INT n,
		SPL_pFLOAT mz )
/*</DOC>*/
{
#define __EIGEN_SIGN(a,b) ((b)<0 ? -fabs(a) : fabs(a))
	SPL_INT m,l,iter,i,j,k;
	SPL_FLOAT s,r,p,g,f,dd,c,b;

	ve[n-1]=0.0;
	for (l=0;l<n;l++) {
		iter=0;
		do {
			for (m=l;m<n-1;m++) {
				dd=fabs(vd[m])+fabs(vd[m+1]);
				if ((fabs(ve[m])+dd) == dd)
				break;
			}
			if (m != l) {
				if ((iter++)==EIGEN_MAXITER)  /* demasiadas iteraciones */
					return SPL_TRUE;
				g=(vd[l+1]-vd[l])/(2.0*ve[l]);
				r=sqrt((g*g)+1.0);
				g=vd[m]-vd[l]+ve[l]/(g+__EIGEN_SIGN(r,g));
				s=c=1.0;
				p=0.0;
				for (i=m-1;i>=l;i--) {
					f=s*ve[i];
					b=c*ve[i];
					if (fabs(f) >= fabs(g)) {
						c=g/f;
						r=sqrt((c*c)+1.0);
						ve[i+1]=f*r;
						c *= (s=1.0/r);
					}
					else {
						s=f/g;
						r=sqrt((s*s)+1.0);
						ve[i+1]=g*r;
						s *= (c=1.0/r);
					}
					g=vd[i+1]-p;
					r=(vd[i]-g)*s+2.0*c*b;
					p=s*r;
					vd[i+1]=g+p;
					g=c*r-b;
					if (mz!=NULL) {  /* autovectores, si mz no es nul */
						j=i*n;
						for (k=0;k<n;k++) {
							f=mz[j+n+k];
							mz[j+n+k]=s*mz[j+k]+c*f;
							mz[j+k]=c*mz[j+k]-s*f;
						}
					}
				}
				vd[l]=vd[l]-p;
				ve[l]=g;
				ve[m]=0.0;
			}
		} while (m != l);
	}
	return SPL_FALSE;
#undef __EIGEN_SIGN
}

/**********************************************************/

