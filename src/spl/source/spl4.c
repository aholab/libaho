/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL4.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.4    16/05/96  Borja     bugs en algunos valores de retorno
1.1.3    15/05/96  Borja     bugs en documentacion
1.1.2    30/07/95  Borja     scope funciones explicito
1.1.1    14/04/95  Borja     correccion nombres funciones nel_?() e idx_?()
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Autocorrelacion, covarianza, y covarianzas modificadas.
Delante de cada grupo de funciones de comenta detalladamente
el contenido.

Definir NDEBUG para desconectar la validacion de parametros
con assert(). No definir este simbolo mientras se depuren
aplicaciones, ya que aunque las funciones son algo mas lentas,
el chequeo de validacion de parametros resuelve muchos problemas.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "spli.h"

/**********************************************************/
/* Funciones de autocorrelacion (acorr_???) y autocorrelacion
normalizada a la del origen (acorr_???_n).

Reciben como parametros basicos:

- SPL_pFLOAT {v} : vector de longitud {N} del que se quieren calcular
.                  autocorrelaciones.
- SPL_INT {N} : Longitud del vector {v}.
.               {N}>=0

Ademas reciben otros parametros:

* Las funciones acorr_i_? reciben:
.    - SPL_INT {i} : Autocorrelacion R(i) a calcular.
.                    {i}>=0
.    La funcion {devuelve} R(i).

* Las funciones acorr_ab_? reciben:
.	 - SPL_pFLOAT {vac}: Vector de autocorrelaciones a rellenar.
.                         Tiene ({b}-{a}+1) elementos.
.    - SPL_INT {a} : Autocorrelacion inicial R(a) a calcular.
.                    {a}>=0
.    - SPL_INT {b} : Autocorrelacion final R(b) a calcular.
.                    {b}>=0; {b}>=a
.    La funcion calcula desde R(a) hasta R(b) y mete los resultados
.    en {vac}.
.	 Por tanto este vector debe tener ({b}-{a}+1) elementos SPL_FLOAT
.    (este valor lo {devuelve} la funcione nel_acorr_ab(a,b) ).
.    La funcion {devuelve} el puntero {vac} en la version que no normaliza
.    las autocorrelaciones, y {devuelve} la autocorrelacion en el origen
.    en la version que si normaliza las autocorrelaciones.

Las funciones acorr_???_n normalizan el valor de las autocorrelaciones
calculadas al valor de la autocorrelacion en el origen R(0).
Si R(0) es nula, la normalizacion no se efectua (equivale a normalizar
a R(0)=1) y la funcion {devuelve} 0.0

Ref:
.    Digital Processing of Speech Signals
.    L.R. Rabiner / R.W Schafer
.    Prentice Hall
*/
/**********************************************************/
/* {devuelve} la autocorrelacion {i} del vector {v} de {N} elementos.
{i} debe ser positivo >= 0. {N}>=0 */

PUBLIC SPL_FLOAT XAPI acorr_i( SPL_pFLOAT v, SPL_INT N, SPL_INT i )
{
	SPL_INT k;
	SPL_FLOAT sum;
	assert(N>=0);
	assert(i>=0);

	__xsum(k,i,N,v[k]*v[k-i],sum,0.0);

	return sum;
}

/**********************************************************/
/* {devuelve} la autocorrelacion {i} normalizada a la autocorrelacion
en el origen, del vector {v} de {N} elementos.
{i} debe ser positivo >= 0. {N}>=0.
Si R(0) es nula, la normalizacion no se efectua. */

PUBLIC SPL_FLOAT XAPI acorr_i_n( SPL_pFLOAT v, SPL_INT N, SPL_INT i )
{
	SPL_INT k;
	SPL_FLOAT sum, r0;
	assert(N>=0);
	assert(i>=0);

	__xsum(k,0,N,v[k]*v[k],r0,0.0);
	__xsum(k,i,N,v[k]*v[k-i],sum,0.0);

	if (r0)
		return sum/r0;
	else
		return sum;
}

/**********************************************************/
/* mete en {vac} las autocorrelaciones desde {a} hasta {b},
del vector {v} de {N} elementos.
Por tanto {vac} debe tener sitio para ({b}-{a}+1) elementos (funcion
nel_acorr_ab() ) de tipo SPL_FLOAT.
{devuelve} el vector de autocorrelaciones {vac}.
{a} y {b} deben ser positivos >= 0, con {a}<={b}. {N}>=0 */

PUBLIC SPL_pFLOAT XAPI acorr_ab( SPL_pFLOAT v, SPL_INT N, SPL_pFLOAT vac,
		SPL_INT a, SPL_INT b )
{
	SPL_INT i,k;
	SPL_FLOAT sum;
	assert(N>=0);
	assert(a>=0);
	assert(b>=0);
	assert(b>=a);

	for (i=a; i<=b; i++) {
		__xsum(k,i,N,v[k]*v[k-i],sum,0.0);
		vac[i-a] = sum;
	}
	return vac;
}

/**********************************************************/
/* mete en {vac} las autocorrelaciones desde {a} hasta {b}
del vector {v} de {N} elementos normalizadas a la autocorrelacion
en el origen.
Por tanto {vac} debe tener sitio para ({b}-{a}+1) elementos (funcion
nel_acorr_ab() ) de tipo SPL_FLOAT.
La funcion {devuelve} el valor de la autocorrelacion en el origen.
{a} y {b} deben ser positivos >= 0, con {a}<={b}. {N}>=0.
Si R(0) es nula, la normalizacion no se efectua y la funcion
{devuelve} 0.0 */

PUBLIC SPL_FLOAT XAPI acorr_ab_n( SPL_pFLOAT v, SPL_INT N, SPL_pFLOAT vac,
		SPL_INT a, SPL_INT b )
{
	SPL_INT i,k;
	SPL_FLOAT sum, r0;
	assert(N>=0);
	assert(a>=0);
	assert(b>=0);
	assert(b>=a);

	__xsum(k,0,N,v[k]*v[k],r0,0.0);

	if (!r0) {
		for (i=a; i<=b; i++) {
			__xsum(k,i,N,v[k]*v[k-i],sum,0.0);
			vac[i-a] = sum;
		}
	}
	else {
		for (i=a; i<=b; i++) {
			__xsum(k,i,N,v[k]*v[k-i],sum,0.0);
			vac[i-a] = sum/r0;
		}
	}

	return r0;
}

/**********************************************************/
/* {devuelve} el numero de elementos SPL_FLOAT que debe tener un
vector de autocorrelaciones para poder almacenar las
autocorrelaciones entre R(a) y R(b), ambas incluidas.
{a} y {b} deben ser positivos >= 0, con {a}<={b} */

PUBLIC SPL_INT XAPI nel_acorr_ab( SPL_INT a, SPL_INT b )
{
	assert(a>=0);
	assert(b>=0);
	assert(b>=a);

	return b-a+1;
}

/**********************************************************/
/* Funciones de covarianza (covar_???) y covarianza
normalizada a la del origen (covar_???_n).

Reciben como parametros basicos:

- SPL_pFLOAT {v} : vector de longitud {N} del que se quieren calcular
.                   covarianzas.
- SPL_INT {N} : Longitud del vector {v}.  {N}>=0
- SPL_INT {p} : Longitud de la 'cola' dentro de las {N} muestras pasadas.
.               La ventana de analisis esta realmente formada por las
.               ultimas {N}-{p} muestras del vector.
.               {p}>=0

Ademas reciben otros parametros:

* Las funciones covar_ij_? reciben:
.    - SPL_INT {i}, SPL_INT {j} : para calcular la covarianza en (i,j)
.                                 {i}>=0, {j}>=0
.      La funcion {devuelve} covarianza(i,j)

* Las funciones xcovm_??_? crean 'pseudo matrices' de covarianzas,
.      que representan el sistema de ecuaciones completo que hay
.      que resolver (por cholesky) para obtener los LPC.

Hay un juego de funciones similar a las covar y xcovm pero denominadas
covar_w y xcovm_w. Calculan covarianzas modificadas por un enventanado,
tipicamente hamming, con objeto de obtener analisis predictivos
mejores (Atal : los coeficientes no bailan tanto ... ), pero produce
anchos de banda algo mas estrechos. Estas funciones reciben la ventana
en el vector {vw}, de {N}-{p} puntos.

Ref:
.    Digital Processing of Speech Signals
.    L.R. Rabiner / R.W Schafer
.    Prentice Hall

Ref. covarianzas modificadas:
.    S. Singhal and B. S. Atal
.    Improving performance of multi-pulse LPC coders at low bit rates.
.    Proc. Int. Conf. ASSP vol 1, Paper 1.3
*/
/**********************************************************/
/* {devuelve} la covarianza ({i},{j}) del vector {v} de {N} elementos.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos ({N}-{p}) elementos.
Los {p} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} puedan llegar a valer {p}, por
ejemplo en una matriz de covarianzas de orden {p}.
{i},{j} deben ser positivos <= {p}. {N}>=0. {p}<{N} */

PUBLIC SPL_FLOAT XAPI covar_ij( SPL_pFLOAT v, SPL_INT N, SPL_INT p,
		SPL_INT i, SPL_INT j )
{
	SPL_INT k;
	SPL_FLOAT sum;
	assert(N>=0);
	assert(i>=0);
	assert(j>=0);
	assert(i<=p);
	assert(j<=p);
	assert(p<N);

	__xsum(k,p,N,v[k-i]*v[k-j],sum,0.0);

	return sum;
}

/**********************************************************/
/* {devuelve} la covarianza ({i},{j}) del vector {v} de {N} elementos,
normalizada a la covarianza en 0,0.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos ({N}-{p}) elementos.
Los {p} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} puedan llegar a valer {p}, por
ejemplo en una matriz de covarianzas de orden {p}.
{i},{j} deben ser positivos <= {p}. {N}>=0. {p}<{N}
Si la covarianza en (0,0) es nula, la normalizacion no se efectua */

PUBLIC SPL_FLOAT XAPI covar_ij_n( SPL_pFLOAT v, SPL_INT N, SPL_INT p,
		SPL_INT i, SPL_INT j )
{
	SPL_INT k;
	SPL_FLOAT sum, c00;
	assert(N>=0);
	assert(i>=0);
	assert(j>=0);
	assert(i<=p);
	assert(j<=p);
	assert(p<N);

	__xsum(k,p,N,v[k]*v[k],c00,0.0);
	__xsum(k,p,N,v[k-i]*v[k-j],sum,0.0);

	if (c00)
		return sum/c00;
	else
		return sum;
}

/**********************************************************/
/* {devuelve} el indice dentro de un array para el elemento {i},{j} de
una matriz de covarianzas de orden {p}.
{i} son filas, {j} columnas.
Aunque el elemento ({i},{j}) es igual que el ({j},{i}),
enviar SIEMPRE {j}<={i}.
{i}>=1, {j}>=1.
El funcionamiento es erroneo si se envia {i}>{p} o {j}>{p}. Esta situacion
no se detecta nunca (assert) por esta funcion, ya que no necesita
recibir el valor de {p}.

La estructura que se almacena en esta matriz es la siguiente:

.        C(1,1)
.        C(2,1)  C(2,2)
.        ....
.        C(p,1) ............C(p,p)

La matriz de covarianzas por ser simetrica se almacena como matriz
triangular (la primera fila tiene un elemento, la segunda 2...
hasta p lineas).

Se almacena todo por lineas de izq. a derecha y de arriba a abajo:
.           C(1,1) C(2,1) C(2,2) C(3,1) ... C(p,p)
p es el orden de la matriz de covarianzas.

El elemento ({i},{j}) representa a la covarianza C({i},{j}) en prediccion
de corto termino, en la que el indice encaja con el retardo aplicado
a la trama de analisis. En prediccion de largo termino o de algun otro
tipo general, ({i},{j}) representara la covarianza C(M(i),M(j)) siendo
M(i) y M(j) dos retardos cualesquiera. En cualquier caso, para {i}=0 o
{j}=0, M(0) es obligatoriamente un retardo nulo: M(0)=0.  */

/* Esta macro esta en spl.h
#define idx_covm(i,j) idx_matrix_sim(i,j)
*/

/**********************************************************/
/* {devuelve} el numero de elementos (SPL_FLOAT) que se deben
almacenar en una matriz de covarianzas de orden {p}. {p}>=0 */

/* esta macro esta en spl.h
#define nel_covm(p)  nel_matrix_sim(p)
*/

/**********************************************************/
/* {devuelve} el indice dentro de un array para el elemento {i},{j} de
una "pseudo-matriz" de covarianzas de orden {p}, que es un sistema
de ecuaciones de covarianzas (prediccion lineal de orden {p}).
{i} son filas, {j} columnas.
Aunque el elemento ({i},{j}) es igual que el ({j},{i}),
enviar SIEMPRE {j}<={i}.
{i}>=0, {j}>=0. {i}<={p}, {j}<={p}.
La estructura que se almacena en esta pseudo-matriz es la siguiente:

.        C(0,0)                   <------- (factor de potencia)
.        C(1,0)  C(2,0) .... C(p,0) <----- (termino independiente)
.        C(1,1)                    <------- (matriz de...
.        C(2,1)  C(2,2)            <-----|
.        ....                      <-----|
.        C(p,1) ............C(p,p) <------- ...covarianzas)

El factor de potencia es un elemento. El termino independiente es un
vector de {p} elementos. La matriz de covarianzas por ser simetrica
se almacena como matriz triangular (la primera fila tiene un elemento,
la segunda 2... hasta {p} lineas).

Se almacena todo por lineas de izq. a derecha y de arriba a abajo:
.  C(0,0)  C(1,0) C(2,0) ... C(p,0)   C(1,1) C(2,1) C(2,2) C(3,1) ... C(p,p)
{p} es el orden de la matriz de covarianzas

El elemento ({i},{j}) representa a la covarianza C(i,j) en prediccion
de corto termino, en la que el indice encaja con el retardo aplicado
a la trama de analisis. En prediccion de largo termino o de algun otro
tipo general, ({i},{j}) representara la covarianza C(M(i),M(j)) siendo
M(i) y M(j) dos retardos cualesquiera. En cualquier caso, para {i}=0 o
{j}=0, M(0) es obligatoriamente un retardo nulo: M(0)=0.  */

PUBLIC SPL_INT XAPI idx_xcovm( SPL_INT i, SPL_INT j, SPL_INT p )
{
	assert(i>=0);
	assert(j>=0);
	assert(j<=i);
	assert(i<=p);

	return __idx_xcovm(i,j,p);
}

/**********************************************************/
/* {devuelve} el numero de elementos que se deben almacenar en una
pseudo-matriz de covarianzas de orden {p} ({p}>=0) */

PUBLIC SPL_INT XAPI nel_xcovm( SPL_INT p )
{
	assert(p>=0);

	return __idx_xcovm(p+1,1,p);
}

/**********************************************************/
/* mete en {mcv} la "pseudo-matriz" de covarianzas de orden {p} para
analisis predictivo de corto termino del vector {v}.
Esta matriz debe tener sitio para nel_xcovm(p) elementos SPL_FLOAT.
El vector {v} es de {N} elementos.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos ({N}-{p}) elementos.
Los {p} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} llegan a valer {p} en la matriz
de covarianzas de orden {p}.
{N}>=0. {p}>=0. {p}<{N}
{devuelve} un puntero a {mcv} */

PUBLIC SPL_pFLOAT XAPI xcovm_st( SPL_pFLOAT v, SPL_INT N,
		SPL_INT p, SPL_pFLOAT mcv )
{
	SPL_INT i,j,k,l;
	SPL_FLOAT sum;
	assert(N>=0);
	assert(p>=0);
	assert(p<N);

	for (i=0; i<=p; i++) {  /* calcula C(i,0), i=0...p */
		__xsum(k,p,N,v[k-i]*v[k],sum,0.0);
		mcv[i]=sum;
	}

	j=p+1;  /* calcula el resto a partir de los anteriores */
	for (i=0; i<p; i++) {  /* p filas */
		j+=i;       /* en el elemento j comienza la fila */
		l=0;
		sum=mcv[i];   /* covarianza de partida */
		/* calcula cov. de la diagonal de la fila i+1 */
		for (k=1; k<=p-i; ) {
			mcv[j+l]=(sum+=v[p-k]*v[p-k-i]-v[N-k]*v[N-k-i]);
			k++;
			l+=(k+i);
		}
	}

	return mcv;
}

/**********************************************************/
/* mete en {mcv} la "pseudo-matriz" de covarianzas de orden {p}
normalizadas a la covarianza en (0,0), para analisis predictivo
de corto termino del vector {v}.
Esta matriz debe tener sitio para nel_xcovm(p) elementos SPL_FLOAT.
El vector {v} es de {N} elementos.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos ({N}-{p}) elementos.
Los {p} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} llegan a valer {p} en la matriz
de covarianzas de orden {p}.
{N}>=0. {p}>=0. {p}<{N}
La funcion {devuelve} el valor de la covarianza en (0,0).
Si la covarianza en (0,0) es nula, la normalizacion no se efectua y
la funcion {devuelve} 0.0 */

PUBLIC SPL_FLOAT XAPI xcovm_st_n( SPL_pFLOAT v, SPL_INT N,
		SPL_INT p, SPL_pFLOAT mcv )
{
	SPL_INT i;
	SPL_FLOAT c00;

	xcovm_st(v,N,p,mcv);
	if (mcv[0]) {
		c00=mcv[0];
		for (i=nel_xcovm(p); i>0;)
			mcv[--i] /= c00;
		return c00;
	}

	return 0.0;
}

/**********************************************************/
/* mete en {mcv} la "pseudo-matriz" de covarianzas de orden {p} para
analisis predictivo de largo termino del vector {v}. El retardo
del predictor es {M}, y se utilizan las covarianzas con indices
en {M}, {M}+1 ... {M}+{p}-1.
Esta matriz debe tener sitio para nel_xcovm(p) elementos SPL_FLOAT.
El vector {v} es de {N} elementos.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos {fN} elementos.
Los {N}-{fN} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} llegan a valer {M}+{p}-1 en la
matriz de covarianzas de orden {p}. Por tanto, la cola {N}-{fN} debera
tener por lo menos {M}+{p}-1 elementos.
{N}>=0. {p}>=0. {M}>=0
{devuelve} un puntero a {mcv} */

PUBLIC SPL_pFLOAT XAPI xcovm_lt( SPL_pFLOAT v, SPL_INT N, SPL_INT fN,
		SPL_INT p, SPL_INT M, SPL_pFLOAT mcv )
{
	SPL_INT i,j,k,l;
	SPL_FLOAT sum;
	SPL_INT xp=N-fN;
	assert(fN>=0);
	assert(fN<N);
	assert(p>=0);
	assert(xp>=M+p-1);
	assert(M>=0);

	if (p) {
		__xsum(k,xp,N,v[k]*v[k],sum,0.0); /* calcula C(0,0)*/
		mcv[0]=sum;
	}

	for (i=0; i<p; ) {  /* calcula C(M+i-1,0), i=1...p */
		__xsum(k,xp,N,v[k-i-M]*v[k],sum,0.0);
		i++;
		mcv[i]=sum;
	}

	j=p+1;  /* calcula el resto */
	for (i=0; i<p; i++) {  /* p filas */
		j+=i;    /* en el elemento j comienza la fila */
		__xsum(k,xp,N,v[k-i-M]*v[k-M],sum,0.0); /* calcula el primero */
		mcv[j]=sum;
		l=(i+2);
		/* calcula los de la diagonal a partir del primero */
		for (k=1; k<p-i; ) {
			mcv[j+l]=(sum+=v[xp-M-k]*v[xp-M-k-i]-v[N-M-k]*v[N-M-k-i]);
			k++;
			l+=(k+1+i);
		}
	}

	return mcv;
}

/**********************************************************/
/* mete en {mcv} la "pseudo-matriz" de covarianzas normalizadas
a la del origen de orden {p} para analisis predictivo de largo
termino del vector {v}. El retardo del predictor es {M}, y se
utilizan las covarianzas con indices en {M}, {M}+1 ... {M}+{p}-1.
Esta matriz debe tener sitio para nel_xcovm(p) elementos SPL_FLOAT.
El vector {v} es de {N} elementos.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos {fN} elementos.
Los {N}-{fN} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} llegan a valer {M}+{p}-1 en la
matriz de covarianzas de orden {p}. Por tanto, la cola {N}-{fN} debera
tener por lo menos {M}+{p}-1 elementos.
{N}>=0. {p}>=0. {M}>=0
La funcion {devuelve} el valor de la covarianza en (0,0).
Si la covarianza en (0,0) es nula, la normalizacion no se efectua y
la funcion {devuelve} 0.0 */

PUBLIC SPL_FLOAT XAPI xcovm_lt_n( SPL_pFLOAT v, SPL_INT N, SPL_INT fN,
		SPL_INT p, SPL_INT M, SPL_pFLOAT mcv )
{
	SPL_INT i;
	SPL_FLOAT c00;

	xcovm_lt(v,N,fN,p,M,mcv);
	if (mcv[0]) {
		c00=mcv[0];
		for (i=nel_xcovm(p); i>0;)
			mcv[--i] /= c00;
		return c00;
	}

	return 0.0;
}

/**********************************************************/
/* {devuelve} la covarianza ponderada ({i},{j}) del vector {v} de
{N} elementos.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos ({N}-{p}) elementos.
Los {p} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} puedan llegar a valer {p}, por
ejemplo en una matriz de covarianzas de orden {p}.
La ventana de ponderacion se pasa en el vector {vw}, de {N}-{p} puntos.
{i},{j} deben ser positivos <= {p}. {N}>=0. {p}<{N} */

PUBLIC SPL_FLOAT XAPI covar_w_ij( SPL_pFLOAT v, SPL_INT N, SPL_INT p,
		SPL_INT i, SPL_INT j, SPL_pFLOAT vw )
{
	SPL_INT k;
	SPL_FLOAT sum;
	assert(N>=0);
	assert(i>=0);
	assert(j>=0);
	assert(i<=p);
	assert(j<=p);
	assert(p<N);

	__xsum(k,p,N,v[k-i]*v[k-j]*vw[k-p],sum,0.0);

	return sum;
}

/**********************************************************/
/* {devuelve} la covarianza ponderada({i},{j}) del vector {v} de {N}
elementos, normalizada a la covarianza (ponderada) en 0,0.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos ({N}-{p}) elementos.
Los {p} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} puedan llegar a valer {p}, por
ejemplo en una matriz de covarianzas de orden {p}.
La ventana de ponderacion se pasa en el vector {vw}, de {N}-{p} puntos.
{i},{j} deben ser positivos <= {p}. {N}>=0. {p}<{N}
Si la covarianza en (0,0) es nula, la normalizacion no se efectua */

PUBLIC SPL_FLOAT XAPI covar_w_ij_n( SPL_pFLOAT v, SPL_INT N, SPL_INT p,
		SPL_INT i, SPL_INT j, SPL_pFLOAT vw )
{
	SPL_INT k;
	SPL_FLOAT sum, c00;
	assert(N>=0);
	assert(i>=0);
	assert(j>=0);
	assert(i<=p);
	assert(j<=p);
	assert(p<N);

	__xsum(k,p,N,v[k]*v[k]*vw[k-p],c00,0.0);
	__xsum(k,p,N,v[k-i]*v[k-j],sum,0.0);

	if (c00)
		return sum/c00;
	else
		return sum;
}

/**********************************************************/
/* mete en {mcv} la "pseudo-matriz" de covarianzas ponderadas
de orden {p} para analisis predictivo de corto termino del vector {v}.
Esta matriz debe tener sitio para nel_xcovm(p) elementos SPL_FLOAT.
El vector {v} es de {N} elementos.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos ({N}-{p}) elementos.
Los {p} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} llegan a valer {p} en la matriz
de covarianzas de orden {p}.
La ventana de ponderacion se pasa en el vector {vw}, de {N}-{p} puntos.
{N}>=0. {p}>=0. {p}<{N}
{devuelve} un puntero a {mcv} */

PUBLIC SPL_pFLOAT XAPI xcovm_w_st( SPL_pFLOAT v, SPL_INT N, SPL_INT p,
		SPL_pFLOAT mcv, SPL_pFLOAT vw )
{
	SPL_INT i,j,k,l;
	SPL_FLOAT sum;
	assert(N>=0);
	assert(p>=0);
	assert(p<N);

	for (i=0; i<=p; i++) { /* calcula C(i,0), i=0...p */
		__xsum(k,p,N,v[k-i]*v[k]*vw[k-p],sum,0.0);
		mcv[i]=sum;
		}

	l=p+1;
	for (i=1; i<=p; i++) {
		for (j=1; j<=i; j++) {
			__xsum(k,p,N,v[k-i]*v[k-j]*vw[k-p],sum,0.0);
			mcv[l++]=sum;
			}
		}

	return mcv;
}

/**********************************************************/
/* mete en {mcv} la "pseudo-matriz" de covarianzas ponderadas
de orden {p} normalizadas a la covarianza (ponderada) en (0,0),
para analisis predictivo de corto termino del vector {v}.
Esta matriz debe tener sitio para nel_xcovm(p) elementos SPL_FLOAT.
El vector {v} es de {N} elementos.
De este vector, la 'trama de analisis' (frame) esta formada
solo por los ultimos ({N}-{p}) elementos.
Los {p} primeros elementos son la 'cola' necesaria para calcular la
covarianza {i},{j} en la que {i} o/y {j} llegan a valer {p} en la matriz
de covarianzas de orden {p}.
La ventana de ponderacion se pasa en el vector {vw}, de {N}-{p} puntos.
{N}>=0. {p}>=0. {p}<{N}
La funcion {devuelve} el valor de la covarianza modificada en (0,0).
Si la covarianza en (0,0) es nula, la normalizacion no se efectua y
la funcion {devuelve} 0.0 */

PUBLIC SPL_FLOAT XAPI xcovm_w_st_n( SPL_pFLOAT v, SPL_INT N, SPL_INT p,
		SPL_pFLOAT mcv, SPL_pFLOAT vw )
{
	SPL_INT i;
	SPL_FLOAT c00;

	xcovm_w_st(v,N,p,mcv,vw);
	if (mcv[0]) {
		c00=mcv[0];
		for (i=nel_xcovm(p); i>0;)
			mcv[--i] /= c00;
	      
		return c00;
	      }

	return 0.0;
}

/**********************************************************/

