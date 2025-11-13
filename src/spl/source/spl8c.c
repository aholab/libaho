/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL8C.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... experimental
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
2.0.0    01/07/97  Borja     spl8.c -> spl8?.c

======================== Contenido ========================
Pruebas con la DST y DCT
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "spli.h"

/**********************************************************/

/**********************************************************/
/* PROVISIONAL PROVISIONAL PROVISIONAL PROVISIONAL PROVISION
AL PROVISIONAL PROVISIONAL PROVISIONAL PROVISIONAL PROVIS */
/**********************************************************/

/**********************************************************/
/*pruebas transformada seno/coseno segun Numerical Recipes*/
/**********************************************************/
/* Genera una media-tabla de senos(0 ---> -PI/2) en {vhts}, que
es un vector de {tp} puntos.

.              0       -PI/2
.        vhts: ..--------x------
.                ``--....

La tabla termina en la muestra anterior a -PI/2, y no incluye el
elemento vhts{tp}=sin(-PI/2) que queda ya fuera del vector.

Esta funcion es similar a fft_fill_half_tsin(), pero utiliza otra
media-tabla de senos {vhts2} en vez de calcular montones de nuevos sin(x).
{vhts2} es de {tp}*2 elementos (el doble que {vths}).
Utilizar cuando {vhts2} tambien sea necesaria (transformadas seno y
coseno).
{tp} debe ser >= 0 */

PUBLIC SPL_VOID XAPI htsin2_fill_htsin( SPL_pFLOAT vhts, SPL_pFLOAT vhts2, SPL_INT tp )
{
	SPL_INT i;
	assert(tp>=0);

	for (i=0; i<tp; i++)
	vhts[i]=vhts2[(SPL_INT)i << 1];
}

/**********************************************************/
/* Prepara el vector real {vre} de {np} puntos para calcular la
transformada seno rapida (fst).
La funcion necesita el vector {vhst2}, media-tabla de senos
de {np}/2 puntos.
{np} debe ser un numero par >0 !!! */

PUBLIC SPL_VOID XAPI fst_re_in( SPL_pFLOAT vre, SPL_pFLOAT vhst2, SPL_INT np )
{
	SPL_INT i,np2;
	SPL_FLOAT n1,n2;
	assert((np&1)==0); /* testea que np es par */
	assert(np>0);

	np2 = np >> 1;
	vre[0]=0.0;
	for (i=1; i<np2; i++) {
		n1 = -vhst2[i]*(vre[i]+vre[np-i]);
		n2 = 0.5*(vre[i]-vre[np-i]);
		vre[i] = n1+n2;
		vre[np-i] = n1-n2;
	}
	vre[np2] *= 2.0;
}

/**********************************************************/
/* Prepara el vector real {vre} de {np} puntos para calcular la
transformada coseno rapida (fst).
La funcion necesita el vector {vhst2}, media-tabla de senos
de {np}/2 puntos.
Ademas la funcion {devuelve} un valor necesario para el calculo
de la transformada coseno.
{np} debe ser un numero par >0 !!! */

PUBLIC SPL_FLOAT XAPI fct_re_in( SPL_pFLOAT vre, SPL_pFLOAT vhst2, SPL_INT np )
{
	SPL_INT i,np2;
	SPL_FLOAT n1,n2,n3,sum;
	assert((np&1)==0); /* testea que np es par */
	assert(np>0);

	np2 = np >> 1;
	sum=vre[0];

	for (i=1; i<np2; i++) {
		n1 = 0.5*(vre[i]+vre[np-i]);
		n2 = vhst2[i] * (n3 = vre[np-i]-vre[i]);
		vre[i] = n1-n2;
		vre[np-i] = n1+n2;
		sum+= (n3*vhst2[np2-i]);
	}
	return sum;
}

/**********************************************************/
/* esta funcion es similar a fft_scramble_re_in() pero intercambiando
las partes real e imaginaria (se recibe el vector real de {np}
puntos en {vre_i}, y se crea el vector complejo {vim_r}+j*{vre_i}
de {np}/2 puntos. Hecho asi por eficiencia para la transformada seno.
A la hora de hacer la fft, tener en cuenta que la parte real va en
{vim_r} y la imaginaria en {vre_i}.
{np} debe ser un numero par!!! */

PUBLIC SPL_VOID XAPI fst_scramble_re_in( SPL_pFLOAT vre_i, SPL_pFLOAT vim_r, SPL_INT np )
{
	SPL_INT i,j;
	assert((np&1)==0); /* testea que np es par */

	np >>= 1;
	for (i=j=0; i<np; i++) {
		vim_r[i] = vre_i[j++];
		vre_i[i] = vre_i[j++];
	};
}

/**********************************************************/
/* recupera la transformada seno rapida a partir de la fft efectuada
sobre el vector modificado por frt_re_in(). La fft se ha tenido que
realizar utilizando fst_scramble_re_in() como funcion de scrambling,
por lo que {vre_i} y {vim_r} son la parte imaginaria y real
respectivamente de la fft, una vez efectuado el fft_unscramble_cx_out(),
y son de {np}/2 puntos cada uno (mitad de puntos de la fft).
{np} es la longitud del vector original, y por tanto la del vector
de la transformada seno.
La transformada seno se devuelve en {vre_i} y sera de {np} puntos. Por
tanto, aunque en {vre_i} solo se envian {np}/2 puntos, se devuelven {np},
y {vim_r} queda indefinido.
{np} debe ser una potencia de 2 y >0.*/

PUBLIC SPL_VOID XAPI fst_re_out( SPL_pFLOAT vre_i, SPL_pFLOAT vim_r, SPL_INT np )
{
	SPL_INT i, np2;
	SPL_FLOAT sum;
	assert((np&1)==0); /* testea que np es par */
	assert(np>0);

	np2 = np >> 1;

	for (i=np2-1; i>0; i--)
		vre_i[(SPL_INT)i << 1] = vre_i[i];
	vre_i[0] = 0.0;
	sum = (vre_i[1] = vim_r[0]*0.5);
	for (i=1; i<np2; i++) {
		sum += vim_r[i];
		vre_i[((SPL_INT)i << 1)+1] = sum;
	}
}

/**********************************************************/
/* calcula la transformada seno (fst) vector real {vre} de {np} puntos.
La fst es simetrica (directa e inversa son iguales).
{ufac} multiplica el vector resultado.
{np} debe ser una potencia de 2, mayor o igual que 2 (2,4,8...).
{vtmp} es un vector auxiliar que el usuario debe crear y destruir,
y que debe tener sitio para un numero tnel_fst(np) de elementos
de tipo SPL_FLOAT */

PUBLIC SPL_VOID XAPI fst( SPL_pFLOAT vre, SPL_INT np, SPL_BOOL inv, SPL_FLOAT ufac, SPL_pFLOAT vtmp )
{
	SPL_pFLOAT vhts2,vhts,vts,vtc,vim;
	SPL_INT np_d2, npdiv4;
	assert(fft_test_2pow(np));
	assert(np>=2);

	npdiv4 = (np_d2 = np >> 1) >> 1;
	vim=vtmp;
	vhts2=vim+np_d2;
	fft_fill_half_tsin(vhts2,np_d2);  /* genera media-tabla de senos dobles */
	if (npdiv4) {  /* tres tablas */
		vhts = vhts2+np_d2;   /* media-tabla de senos, np/4 puntos */
		vts = vhts+npdiv4;   /* tablas de senos y cosenos, np/4 puntos */
		vtc = vts+npdiv4;

		htsin2_fill_htsin(vhts,vhts2,npdiv4); /* genera media-tabla de senos */
		fft_htsin_fill_tsin_tcos(vts,vtc,vhts,npdiv4); /* tablas de senos y cosenos */
	}
	else 
	  vhts = vts = vtc = NULL;

	/* preparar para fft compleja rapida, reduce np a np/2 en la fft compleja,
	y tener en cuenta que el vector imaginario esta realmente en vre. */
	fst_scramble_re_in(vre,vim,np);

	fft_inverse_vecs(vre,vim,np_d2);  /* bit-inversion de elementos */
	if (inv) /* factor de correccion por usar la mitad de puntos (FFT inversa)*/
		ufac *= 2.0;
	/* calcula fft, con vre y vim intercambiados */
	fft_fft(fft_n_bits(np_d2),inv,vim,vre,vts,vtc,ufac);

	/* obtiene la fft final, en rango 0 a PI (np/2 puntos) */
	fft_unscramble_cx_out(vre,vim,np_d2,vhts,inv);

	/* extrae la fst a partir de la fft */
	fst_re_out(vre,vim,np);
}

/**********************************************************/
/* {devuelve} el numero de elementos de tipo SPL_FLOAT (no bytes!!) que
debe enviar el usuario en el vector temporal {vtmp} a la funcion
fst() para calcular una fst de {np} puntos */

PUBLIC SPL_INT XAPI tnel_fst( SPL_INT np )
{
	return np+(np >> 2)*3;
}

/**********************************************************/
