/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL8A.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG, FFT_FAST

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
2.0.0    01/07/97  Borja     spl8.c -> spl8?.c
1.1.2    01/08/95  Borja     quitar algunos warnings benignos.
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.1    21/05/92  Borja     Nuevo algoritmo fft() algo mas rapido.
1.0.0    16/03/92  Borja     Codificacion inicial.

======================== Contenido ========================
<DOC>
Primitivas basicas para la FFT compleja y real.

Definir NDEBUG para desconectar la validacion de parametros
con assert(). No definir este simbolo mientras se depuren
aplicaciones, ya que aunque las funciones son algo mas lentas,
el chequeo de validacion de parametros resuelve muchos problemas.

Hay codigo condicional controlado por el define FFT_FAST, para
utilizar uno de dos metodos en el calculo de la fft.
</DOC>
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "spli.h"

/**********************************************************/
/*Si FFT_FAST no esta definido, un metodo mas lento (solo un
poco) pero mucho mas compacto se utilizar para calcular la FFT */

#define FFT_FAST

/**********************************************************/
/* SQRT(1/2)....
Borland define M_SQRT_2, GNU define M_SQRT1_2.
Si no esta definida la de GNU, la definimos suponiendo que
esta definida la otra. */

#ifndef M_SQRT1_2

#define M_SQRT1_2  M_SQRT_2

#endif

/*<DOC>*/
/**********************************************************/
/* {devuelve} el numero de bits necesarios para representar
al valor {np}.
Si {np} no es una potencia de dos, se {devuelve} el menor numero
de bits necesario para representar el valor, que realmente
puede representar 2**fft_n_bits(np) > {np}.
('**' equivale a 'elevado a').
Si {np} es demasiado grande para el rango de SPL_INT, {devuelve} 0.
*/

PUBLIC SPL_INT XAPI fft_n_bits ( SPL_INT np )
/*</DOC>*/
{
	SPL_INT i, n, on;

	i = on = 0;
	n = 1;

	while (n>on) {
			if ( n >= np )
				return i;
			i++;
			on = n;
			n <<= 1;
	}

	assert(1==0);  /* trigger de error si np es demasiado grande */
	return 0;
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} TRUE si {n} es potencia de 2, o FALSE si no lo es */

PUBLIC SPL_BOOL XAPI fft_test_2pow( SPL_INT n )
/*</DOC>*/
{
	return ( ((SPL_INT)1 << fft_n_bits(n)) == n );
}

/*<DOC>*/
/**********************************************************/
/* Genera una media-tabla de senos(0 ---> -PI/2) en {vhts}, que
es un vector de {tp} puntos.

.              0       -PI/2
.        vhts: ..--------x------
.                ``--....

La tabla termina en la muestra anterior a -PI/2, y no incluye el
elemento vhts[tp]=sin(-PI/2) que queda ya fuera del vector.
{tp} debe ser >=1 */

PUBLIC SPL_VOID XAPI fft_fill_half_tsin( SPL_pFLOAT vhts, SPL_INT tp )
/*</DOC>*/
{
	SPL_INT i;
	assert(tp>0);

	vhts[0] = 0.0;
	for (i=1; i<tp; i++)
		vhts[i] = sin(((-M_PI_2)*i)/tp);
}

/*<DOC>*/
/**********************************************************/
/* Genera {vts} y {vtc}, tablas de senos y cosenos de {tp} elementos.

.              0      -PI/2     -PI        ---....      PI
.         vts: ..-------x-------..x   vtc: -------.------x
.                ``---.....---''           0       ``---..

Las tablas llegan hasta la muestra anterior a -PI (o PI).
{tp} debe ser >=1 */

PUBLIC SPL_VOID XAPI fft_fill_tsin_tcos( SPL_pFLOAT vts,
		SPL_pFLOAT vtc, SPL_INT tp )
/*</DOC>*/
{
	SPL_INT i;
	SPL_INT np;
	assert(tp>0);

	np = tp >> 1;
	vtc[np] = vts[0] = 0.0;
	vts[np] = -1.0;
	vtc[0] = 1.0;

	for (i=1; i<np; i++)
		vtc[np+i] = vts[tp-i] = vts[i] = -( vtc[np-i] =
				sin((M_PI*i)/tp) );
}

/*<DOC>*/
/**********************************************************/
/* Genera {vts} y {vtc}, tablas de senos y cosenos de {tp} elementos.

.              0      -PI/2     -PI        ---....      PI
.         vts: ..-------x-------..x   vtc: -------.------x
.                ``---.....---''           0       ``---..

Las tablas llegan hasta la muestra anterior a -PI (o PI).

Esta funcion funciona como la anterior, pero utiliza una media-tabla
de senos {vhts} en vez de calcular montones de nuevos sin(x).
{vhts} es tambien de {tp} elementos.
Utilizar cuando {vhts} tambien es necesaria (FFT de funcion real).
{tp} debe ser >= 1 */

PUBLIC SPL_VOID XAPI fft_htsin_fill_tsin_tcos( SPL_pFLOAT vts,
		SPL_pFLOAT vtc, SPL_pFLOAT vhts, SPL_INT tp )
/*</DOC>*/
{
	SPL_INT i;
	SPL_INT np;
	assert(tp>0);

	np = tp >> 1;
	vtc[np] = vts[0] = 0.0;
	vts[np] = -1.0;
	vtc[0] = 1.0;

	for (i=1; i<np; i++)
		vtc[np-i] = - ( vtc[np+i] = vts[tp-i] = vts[i] =
				vhts[i << 1] );
}

/*<DOC>*/
/**********************************************************/
/* Rellena {vti}, vector de {tp} puntos, con una tabla de
inversion de bits, de forma que el elemento i debe llevarse
a la posicion vti[i] y el elemento vti[i] llevarse a la posicion
i (es decir, intercambiar ambos). */

PUBLIC SPL_VOID XAPI fft_fill_tinv( SPL_pINT vti, SPL_INT tp )
/*</DOC>*/
{
	SPL_INT i,m,j;

	for (j=i=0; i<tp; i++) {
		vti[j] = i;
		vti[i] = j;
		j ^= (m = tp >> 1);
		while (m>j)
			j ^= (m >>= 1);
	}
}

/*<DOC>*/
/**********************************************************/
/* bit-invierte directamente los vectores {vre} y {vim} de
{np} elementos cada uno */

PUBLIC SPL_VOID XAPI fft_inverse_vecs( SPL_pFLOAT vre,
		SPL_pFLOAT vim, SPL_INT np )
/*</DOC>*/
{
	SPL_INT i,m,j;
	SPL_FLOAT temp;

	for (j=i=0; i<np; i++) {
		if (j>i) {
			__swap(vre[i],vre[j],temp);
			__swap(vim[i],vim[j],temp);
		};
		j ^= (m = np >> 1);
		while (m>j)
			j ^= (m >>= 1);
	}
}

/*<DOC>*/
/**********************************************************/
/* Scrambling del vector real {vre} de {np} puntos, convirtiendolo
en un vector complejo en {vre},{vim} de {np}/2 puntos cada uno.
Esto permite calcular una FFT real mas rapidamente. Consiste
en almacenar alternativamente cada muestra real como real
o imaginaria.
IMPORTANTE: puesto que la FFT inversa debe dividir el resultado
por el numero de puntos de la FFT y en este caso el numero real
de puntos es el doble del utilizado, sera necesario dividir entre 2
el resultado final. Para ello, basta enviar ufact*2 en {ufact}
cuando se calcula la FFT inversa. Esto NO se aplica a la FFT directa.
{np} debe ser un numero par!!! */

PUBLIC SPL_VOID XAPI fft_scramble_re_in( SPL_pFLOAT vre,
		SPL_pFLOAT vim, SPL_INT np )
/*</DOC>*/
{
	SPL_INT i,j;
	assert((np&1)==0); /* testea que np es par */

	np >>= 1;
	for (i=j=0; i<np; i++) {
		vre[i] = vre[j++];
		vim[i] = vre[j++];
	};
}

/*<DOC>*/
/**********************************************************/
/* Unscrambling del vector complejo resultado de la FFT compleja
de un vector real Scrambleado (uf!). {np} es el numero de puntos
de la FFT. {vre} y {vim} es el vector FFT ({np} puntos cada
componente) y {vhts} es una media-tabla de senos de {np}/2 puntos.
El vector unscrambleado tiene {np}+1 puntos en la parte real (la
FFT en PI tambien se devuelve), y {np}-1 puntos en la parte
imaginaria, las fases en 0 y PI son siempre 0, por lo que no
se devuelven. Debido a esto, vre[np] (valor en PI) se devuelve
en vim[0] y queda implicito que realmente, vim[0]=vim[np]=0.
De esta forma, {vre} y {vim} pueden ser vectores de solo {np}
puntos pues solo devolvemos la mitad de la fft (desde 0 hasta PI).
La parte real es simetrica, y la imaginaria antisimetrica.
{inv}==TRUE para hacer FFT inversa, o FALSE para la FFT directa.
{np} debe ser >=1 */

PUBLIC SPL_VOID XAPI fft_unscramble_cx_out( SPL_pFLOAT vre, SPL_pFLOAT vim,
		SPL_INT np, SPL_pFLOAT vhts, SPL_BOOL inv )
/*</DOC>*/
{
	SPL_INT i;
	SPL_FLOAT RSum,ISum, RDif,IDif, S;
	SPL_INT NPmi;
	SPL_INT NPd2;
	assert(np>0);

	NPmi = np;
	NPd2 = np >> 1;

	vre[0] += vim[0];
	vim[0] = vre[0] - 2.0*vim[0];

	if (inv)
		S = -1.0;
	else {
		S = 1.0;
		if (NPd2)
			vim[NPd2] = -vim[NPd2];
	};
	for (i=1; i<NPd2; i++) {
		NPmi--;
		RSum = vre[i]+vre[NPmi];
		ISum = vim[i]+vim[NPmi];
		RDif = vre[i]-vre[NPmi];
		IDif = vim[i]-vim[NPmi];
		vre[i] = 0.5*( RSum - ISum*vhts[NPd2-i] + S*RDif*vhts[i] );
		vim[i] = 0.5*( IDif + RDif*vhts[NPd2-i] + S*ISum*vhts[i] );
		vre[NPmi] = RSum-vre[i];
		vim[NPmi] = vim[i]-IDif;
	};
}

/*<DOC>*/
/**********************************************************/
/* una vez unscrambleada la salida con fft_unscramble_cx_out(), se
puede llamar a esta funcion para expandir la FFT compactada
(rango 0-PI) a una FFT completa (rango 0-2*PI). {np} debe ser
la longitud inicial del vector, por tanto, valor doble del que
se enviaba a la rutina anterior, y tambien doble que el numero
de puntos utilizado en la FFT. {vre} y {vim} deben ser de {np}
puntos cada uno.
np debe ser >=2 */

PUBLIC SPL_VOID XAPI fft_expand_cx_out( SPL_pFLOAT vre,
		SPL_pFLOAT vim, SPL_INT np )
/*</DOC>*/
{
	SPL_INT i;
	SPL_INT npd2;
	assert(np>0);
	assert((np&1)==0);

	npd2 = np >> 1;
	for (i=1; i<npd2; i++) {
		vre[np-i] = vre[i];
		vim[np-i] = -vim[i];
	};
	vre[npd2] = vim[0];
	vim[0] = vim[npd2] = 0.0;
}

/*<DOC>*/
/**********************************************************/
/* Calcula la FFT de un vector complejo {vre}+j*{vim}.
{vre} y {vim} son vectores de longitud 2**{nb} puntos.
Si {inv}==FALSE, se calcula la fft directa (DFT), mientras
que si {inv} es TRUE, se calcula la inversa (IDFT)
{vts} y {vtc} son tablas de senos y cosenos, de longitud
(2**{nb})/2 == 2**{nb-1} puntos.
La fft es escalada (multiplicada) por la constante {ufac}
en la FFT directa, y deescalada (dividida) por {ufac} en
la FFT inversa.
El factor FFT_FACTOR definido en fft.h tambien se utiliza
internamente, de la misma forma que {ufac}.

Puesto que parece que cada cual define la DFT como le da la
real gana, esta es la definicion que siguen estas funciones:

DFT directa:

.                 N-1
.                \---
.          H[k]=  >    h[n] * exp(-j*k*n*2*PI/N)
.                /---
.                 n=0


DFT inversa:

.                         N-1
.                        \---
.          h[k]= (1/N) *  >    H[n] * exp(j*k*n*2*PI/N)
.                        /---
.                         n=0
*/
/*</DOC>*/

#ifdef FFT_FAST

/* esta funcion es una traduccion y mejora de una funcion
perteneciente al paquete 'Borland Numerical Toolbox 4.0 for Turbo
Pascal' de Borland.  Borland es marca registrada de Borland
International Inc. y todas esas cosas :) */

/*<DOC>*/
PUBLIC SPL_VOID XAPI fft_fft( SPL_INT nb, SPL_BOOL inv, SPL_pFLOAT vre,
		SPL_pFLOAT vim, SPL_pFLOAT vts,
		SPL_pFLOAT vtc, SPL_FLOAT ufac )
/*</DOC>*/
{
	SPL_INT Term;
	SPL_INT CellSeparation;
	SPL_INT NumberOfCells;
	SPL_INT NumElemenvtsCell;
	SPL_INT NumElInCellLess1;
	SPL_INT NumElInCell_d2;
	SPL_INT NumElInCellDiv4;
	SPL_FLOAT RealRootOfUnity, ImagRootOfUnity;
	SPL_INT Element;
	SPL_INT CellElements;
	SPL_INT ElementInNextCell;
	SPL_INT ind;
	SPL_FLOAT RealDummy, ImagDummy;
	SPL_INT Tmp;
	SPL_INT np = (SPL_INT)1 << nb;

	if (inv)  /* Conjugar la entrada */
		for (Element=0; Element<np; Element++)
			vim[Element] = -vim[Element];

	NumberOfCells = np;
	for (CellSeparation=Term=1; Term<=nb; Term++) {
		/* NumberOfCells mitad; igual a 2**(nb - Term)	*/
		NumberOfCells >>= 1;
		/* NumElemenvtsCell doble; igual a 2**(Term-1)	*/
		NumElInCellDiv4 = ( NumElInCell_d2 = (
		NumElemenvtsCell = CellSeparation ) >> 1 ) >> 1;
		/* CellSeparation doble; igual a 2**Term	*/
		CellSeparation <<= 1;
		NumElInCellLess1 = NumElemenvtsCell-1;

		/* Caso especial: RootOfUnity == EXP(-i 0)	*/
		Element = 0;
		while (Element < np) {
			/* combinar el XAPI{Element} con el elemento en
				 posicion identica de la celda siguiente */
			ElementInNextCell = Element + NumElemenvtsCell;
			RealDummy = vre[ElementInNextCell];
			ImagDummy = vim[ElementInNextCell];
			vre[ElementInNextCell] = vre[Element] - RealDummy;
			vim[ElementInNextCell] = vim[Element] - ImagDummy;
			vre[Element] += RealDummy;
			vim[Element] += ImagDummy;
			Element += CellSeparation;
		}

		for (CellElements=1; CellElements<(NumElInCellDiv4); CellElements++) {
			ind = CellElements * NumberOfCells;
			RealRootOfUnity = vtc[ind];
			ImagRootOfUnity = vts[ind];
			Element = CellElements;

			while (Element < np) {
				/* combinar el elemento XAPI{Element} con el elemento
					 en posicion identica de la celda siguiente */
				ElementInNextCell = Element + NumElemenvtsCell;
				RealDummy = vre[ElementInNextCell] * RealRootOfUnity -
						vim[ElementInNextCell] * ImagRootOfUnity;
				ImagDummy = vre[ElementInNextCell] * ImagRootOfUnity +
						vim[ElementInNextCell] * RealRootOfUnity;
				vre[ElementInNextCell] = vre[Element] - RealDummy;
				vim[ElementInNextCell] = vim[Element] - ImagDummy;
				vre[Element] += RealDummy;
				vim[Element] += ImagDummy;
				Element += CellSeparation;
			}
		}

		/* Caso especial : RootOfUnity == EXP(-i PI/4)	*/
		if (Term>2) {
			Element = NumElInCellDiv4;
			while (Element < np) {
				/* combinar el elemento XAPI{Element} con el elemento
					 en posicion identica de la celda siguiente */
				ElementInNextCell = Element + NumElemenvtsCell;
				RealDummy = M_SQRT1_2 * (vre[ElementInNextCell] +
						vim[ElementInNextCell]);
				ImagDummy = M_SQRT1_2 * (vim[ElementInNextCell] -
						vre[ElementInNextCell]);
				vre[ElementInNextCell] = vre[Element] - RealDummy;
				vim[ElementInNextCell] = vim[Element] - ImagDummy;
				vre[Element] += RealDummy;
				vim[Element] += ImagDummy;
				Element += CellSeparation;
			}
		}

		for ( CellElements=NumElInCellDiv4+1; CellElements<(NumElInCell_d2);
					CellElements++) {
			ind = CellElements * NumberOfCells;
			RealRootOfUnity = vtc[ind];
			ImagRootOfUnity = vts[ind];
			Element = CellElements;
			while (Element < np) {
				/* combinar el elemento XAPI{Element} con el elemento
					 en posicion identica de la celda siguiente */
				ElementInNextCell = Element + NumElemenvtsCell;
				RealDummy = vre[ElementInNextCell] * RealRootOfUnity -
						vim[ElementInNextCell] * ImagRootOfUnity;
				ImagDummy = vre[ElementInNextCell] * ImagRootOfUnity +
						vim[ElementInNextCell] * RealRootOfUnity;
				vre[ElementInNextCell] = vre[Element] - RealDummy;
				vim[ElementInNextCell] = vim[Element] - ImagDummy;
				vre[Element] += RealDummy;
				vim[Element] += ImagDummy;
				Element += CellSeparation;
			}
		}

		/* Caso especial: RootOfUnity == EXP(-i PI_D2)	*/
		if (Term > 1) {
			Element = NumElInCell_d2;
			while (Element < np) {
				/* combinar el elemento XAPI{Element} con el elemento
					 en posicion identica de la celda siguiente */
				ElementInNextCell = Element + NumElemenvtsCell;
				RealDummy =  vim[ElementInNextCell];
				ImagDummy = -vre[ElementInNextCell];
				vre[ElementInNextCell] = vre[Element] - RealDummy;
				vim[ElementInNextCell] = vim[Element] - ImagDummy;
				vre[Element] += RealDummy;
				vim[Element] += ImagDummy;
				Element += CellSeparation;
			};
		};

		Tmp = NumElemenvtsCell - NumElInCellDiv4;
		for (CellElements=NumElInCell_d2+1;CellElements<Tmp;CellElements++) {
			ind = CellElements * NumberOfCells;
			RealRootOfUnity = vtc[ind];
			ImagRootOfUnity = vts[ind];
			Element = CellElements;
			while (Element < np) {
				/* combinar el elemento XAPI{Element} con el elemento
					 en posicion identica de la celda siguiente */
				ElementInNextCell = Element + NumElemenvtsCell;
				RealDummy = vre[ElementInNextCell] * RealRootOfUnity -
						vim[ElementInNextCell] * ImagRootOfUnity;
				ImagDummy = vre[ElementInNextCell] * ImagRootOfUnity +
						vim[ElementInNextCell] * RealRootOfUnity;
				vre[ElementInNextCell] = vre[Element] - RealDummy;
				vim[ElementInNextCell] = vim[Element] - ImagDummy;
				vre[Element] += RealDummy;
				vim[Element] += ImagDummy;
				Element += CellSeparation;
			};
		};

		/* Caso especial: RootOfUnity == EXP(-i 3PI/4)	*/
		if (Term > 2) {
			Element = NumElemenvtsCell - NumElInCellDiv4;
			while (Element < np) {
				/* combinar el elemento XAPI{Element} con el elemento
					 en posicion identica de la celda siguiente */
				ElementInNextCell = Element + NumElemenvtsCell;
				RealDummy = -M_SQRT1_2 * (vre[ElementInNextCell] -
						vim[ElementInNextCell]);
				ImagDummy = -M_SQRT1_2 * (vre[ElementInNextCell] +
						vim[ElementInNextCell]);
				vre[ElementInNextCell] = vre[Element] - RealDummy;
				vim[ElementInNextCell] = vim[Element] - ImagDummy;
				vre[Element] += RealDummy;
				vim[Element] += ImagDummy;
				Element += CellSeparation;
			};
		};

		for ( CellElements=NumElemenvtsCell-NumElInCellDiv4+1;
					CellElements<=NumElInCellLess1;CellElements++) {
			ind = CellElements * NumberOfCells;
			RealRootOfUnity = vtc[ind];
			ImagRootOfUnity = vts[ind];
			Element = CellElements;
			while (Element < np) {
				/* combinar el elemento XAPI{Element} con el elemento
					 en posicion identica de la celda siguiente */
				ElementInNextCell = Element + NumElemenvtsCell;
				RealDummy = vre[ElementInNextCell] * RealRootOfUnity -
						vim[ElementInNextCell] * ImagRootOfUnity;
				ImagDummy = vre[ElementInNextCell] * ImagRootOfUnity +
						vim[ElementInNextCell] * RealRootOfUnity;
				vre[ElementInNextCell] = vre[Element] - RealDummy;
				vim[ElementInNextCell] = vim[Element] - ImagDummy;
				vre[Element] += RealDummy;
				vim[Element] += ImagDummy;
				Element += CellSeparation;
			}
		}
	}

	if (inv) /* si inverso, invierte */
		ImagDummy = - (RealDummy = 1/(ufac*FFT_FACTOR*np));
	else
		ImagDummy = RealDummy = (ufac*FFT_FACTOR);

	for (Element=0; Element<np; Element++ ) {
		vre[Element] *= RealDummy;
		vim[Element] *= ImagDummy;
	};
}

#else  /* algo mas lento, pero mas compacto y ademas propio!	*/

PUBLIC SPL_VOID XAPI fft_fft( SPL_INT nb, SPL_BOOL inv, SPL_pFLOAT vre,
		SPL_pFLOAT vim, SPL_pFLOAT vts,
		SPL_pFLOAT vtc, SPL_FLOAT ufac )
{
	SPL_INT i,j,k,t;
	SPL_INT vl,vm;
	SPL_FLOAT Tempr,Tempi;
	SPL_pFLOAT re1,im1,re2,im2, ts, tc;
	SPL_INT np = (SPL_INT)1 << nb;

	if (inv)
		for (i=0; i<np; i++)
			vim[i] = -vim[i];

	vl = 1;
	for (k=1; k<=nb; k++) {
		vm = vl;
		t = np/(vl<<=1);
		re2=(re1=vre)+vm;
		im2=(im1=vim)+vm;
		for (j=0; j<t; j++) {
			ts = vts;
			tc = vtc;
			for (i=0; i<vm; i++) {
				Tempr = (*re2)*(*tc)-(*im2)*(*ts);
				Tempi = (*re2)*(*ts)+(*im2)*(*tc);
				*(re2++) = (*re1)-Tempr;
				*(im2++) = (*im1)-Tempi;
				*(re1++) += Tempr;
				*(im1++) += Tempi;
				ts += t;
				tc += t;
			};
			re1+=vm;
			re2+=vm;
			im1+=vm;
			im2+=vm;
		};
	};

	if (inv)
		Tempi = - (Tempr = 1/(ufac*FFT_FACTOR*np));
	else
		Tempi = Tempr = (ufac*FFT_FACTOR);

	for (i=0; i<np; i++) {
		vre[i] *= Tempr;
		vim[i] *= Tempi;
	};
}

#endif

/*<DOC>*/
/**********************************************************/
/* Calcula la FFT del vector complejo {vre}+j*{vim}.
Estos dos son vectores de {np} puntos cada uno.
{inv}==FALSE para calcular la FFT directa, o {TRUE} para la
inversa.
{ufac} multiplica (FFT directa) o divide (FFT inversa) al
vector resultado final.
{np} debe ser una potencia de 2.
{vtmp} es un vector auxiliar que debe crear y destruir el usuario.
Este vector debe ser de tnel_fft_cxfft(np) elementos de tipo SPL_FLOAT
(no bytes!) */

PUBLIC SPL_VOID XAPI fft_cxfft( SPL_pFLOAT vre, SPL_pFLOAT vim, SPL_INT np,
		SPL_BOOL inv, SPL_FLOAT ufac,
		SPL_pFLOAT vtmp )
/*</DOC>*/
{
	SPL_pFLOAT vts, vtc;
	SPL_INT np_d2;
	assert(fft_test_2pow(np));

	np_d2 = np >> 1;
	if (np_d2) {   /* generar tablas de senos y cosenos */
		vts = vtmp;   /* dos tablas, np/2 puntos cada una */
		vtc = vtmp+np_d2;
		fft_fill_tsin_tcos(vts,vtc,np_d2);
	}
	else 
	  vts = vtc = NULL;

	fft_inverse_vecs(vre,vim,np);  /* bit-invierte vectores de entrada */

	fft_fft(fft_n_bits(np),inv,vre,vim,vts,vtc,ufac); /* calcula la FFT */
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} el numero de elementos de tipo SPL_FLOAT (no bytes!!) que
debe enviar el usuario en el vector temporal {vtmp} a la funcion
fft_cxfft() para calcular una fft compleja de {np} puntos */

PUBLIC SPL_INT XAPI tnel_fft_cxfft( SPL_INT np )
/*</DOC>*/
{
	return np;
}

/*<DOC>*/
/**********************************************************/
/* calcula la fft real del vector {vre}+j*0.
Tanto {vre} como {vim} deben ser vectores de {np} puntos.
{vim} es ignorado a la entrada (no necesita inicializarse).
Esta funcion es mas rapida que utilizar fft_cxfft()
cuando el vector de entrada es real (vim{i}=0).
Enviar {inv}=FALSE para FFT directa, o TRUE para FFT inversa.
{ufac} multiplica (FFT directa) o divide (FFT inversa) el
vector resultado.
{np} debe ser una potencia de 2, mayor o igual que 2 (2,4,8...).
{vtmp} es un vector auxiliar que el usuario debe crear y destruir,
y que debe tener sitio para un numero tnel_fft_refft(np) de elementos
de tipo SPL_FLOAT */

PUBLIC SPL_VOID XAPI fft_refft( SPL_pFLOAT vre, SPL_pFLOAT vim, SPL_INT np,
		SPL_BOOL inv, SPL_FLOAT ufac,
		SPL_pFLOAT vtmp )
/*</DOC>*/
{
	SPL_pFLOAT vhts,vts,vtc;
	SPL_INT np_d2, npdiv4;
	assert(fft_test_2pow(np));
	assert(np>=2);

	npdiv4 = (np_d2 = np >> 1) >> 1;
	if (npdiv4) {  /* tres tablas */
		vhts = vtmp;   /* media-tabla de senos, np/4 puntos */
		vts = vhts+npdiv4;   /* tablas de senos y cosenos, np/4 puntos */
		vtc = vts+npdiv4;

		fft_fill_half_tsin(vhts,npdiv4);  /* genera media-tabla de senos */
		fft_htsin_fill_tsin_tcos(vts,vtc,vhts,npdiv4); /* tablas de senos y cosenos */
	}
	else 
	  vhts = vts = vtc = NULL;

	/* preparar para fft compleja rapida, reduce np a np/2 en la fft compleja */
	fft_scramble_re_in(vre,vim,np);

	fft_inverse_vecs(vre,vim,np_d2);  /* bit-inversion de elementos */
	if (inv) /* factor de correccion por usar la mitad de puntos (FFT inversa)*/
		ufac *= 2.0;
	fft_fft(fft_n_bits(np_d2),inv,vre,vim,vts,vtc,ufac);  /* calcula fft */

	/* obtiene la fft final, en rango 0 a PI (np/2 puntos) */
	fft_unscramble_cx_out(vre,vim,np_d2,vhts,inv);

	/* expande la fft, de (0,PI a (0,2*PI). esto produce
	los dos vectores vre y vim de np puntos cada uno */
	fft_expand_cx_out(vre,vim,np);
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} el numero de elementos de tipo SPL_FLOAT (no bytes!!) que
debe enviar el usuario en el vector temporal {vtmp} a la funcion
fft_refft() para calcular una fft real de {np} puntos */

PUBLIC SPL_INT XAPI tnel_fft_refft( SPL_INT np )
/*</DOC>*/
{
	return (np >> 2)*3;
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} el modulo al cuadrado (norma) del elemento
complejo {re}+j*{im}.
Si la raiz cuadrada de este valor (es decir, el modulo) es menor
que el umbral definido en FFT_ZERO el valor se redondea a 0.0 */

PUBLIC SPL_FLOAT XAPI fft_zcx_norm( SPL_FLOAT re, SPL_FLOAT im )
/*</DOC>*/
{
	SPL_FLOAT d;

	d = re*re+im*im;
	if (d<(FFT_ZERO*FFT_ZERO))
		return 0.0;
	else
		return d;
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} el angulo (radianes) (-PI a PI) para el valor
complejo {re}+j*{im}.
Si {re} o {im} es menor que el umbral FFT_ZERO se redondean a cero.
Con esto se evitan los saltos de fase en valores de poca amplitud,
debidos principalmente a los errores de redondeo */

PUBLIC SPL_FLOAT XAPI fft_zcx_arg( SPL_FLOAT re, SPL_FLOAT im )
/*</DOC>*/
{
	if (!((re>FFT_ZERO)||(re<-FFT_ZERO))) {
		if (im<-FFT_ZERO)
			return -M_PI_2;
		else if (im>FFT_ZERO)
			return M_PI_2;
		else
			return 0.0;
	}

	if (!((im>FFT_ZERO)||(im<-FFT_ZERO))) {
		if (re<-FFT_ZERO)
			return M_PI;
		else
			return 0.0;
	}
	else
		return atan2(im,re);
}

/*<DOC>*/
/**********************************************************/
/* {devuelve} el modulo al cuadrado (norma) del elemento
complejo {re}+j*{im}. */

PUBLIC SPL_FLOAT XAPI fft_cx_norm( SPL_FLOAT re, SPL_FLOAT im )
/*</DOC>*/
{
	return __fft_cx_norm(re,im);
}

/**********************************************************/

