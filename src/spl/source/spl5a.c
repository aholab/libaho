/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL5a.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG, LP_NEGSUM

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.1    07/12/93  Borja     soporte LP_NEGSUM_NEG() y LP_POSSUM_NEG()
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Algoritmos de prediccion lineal (LP).

El funcionamiento de todos los algoritmos LP esta determinado
por el #define LP_NEGSUM en SPL.H :

+ Si LP_NEGSUM esta definido:
.        H(z) = G / ( 1 - sum{ak * z**-k} )
.        Estructuras lattice propagan error con -Ki
.        s'(n) = sum{ak*s(n-k)}

+ Si por el contrario LP_NEGSUM no esta definido:
.        H(z) = G / ( 1 + sum{ak * z**-k} )
.        Estructuras lattice propagan error con +Ki
.        s'(n) = - sum{ak*s(n-k)}

(x**y indica x elevado a y)
en cualquiera de los dos casos, en un predictor de orden p se
cumple ap=Kp, siendo ap el ultimo coeficiente del predictor
lineal, y Kp el correspondiente coeficiente de reflexion.
Realmente los coeficientes Ki *son* los coeficientes de reflexion
del modelo del tubo sin perdidas solo si LP_NEGSUN *no* esta
definido. Si LP_NEGSUM *si* esta definido, entonces los Ki son
realmente los coeficientes de reflexion autenticos cambiados de signo.

Se han definido dos macros para gestionar automaticamente los cambios
de signo en funcion de LP_NEGSUM:
-  LP_NEGSUM_NEG(n)  convierte {n} en -{n} solo si LP_NEGSUM esta definido
-  LP_POSSUM_NEG(n)  convierte {n} en -{n} solo si LP_NEGSUM NO esta definido

Para un analisis LP de orden {p} (funciones lpa_???), definimos:

- {vac} es el vector de las {p}+1 primeras autocorrelaciones, desde
R(0) hasta R(p) ==> {p}+1 elementos, vac[0]=R(0).

- {mcv} es la 'pseudo matriz' de covarianzas de orden {p}, obtenida
con las rutinas xcovm_???(). Este vector tiene tnel_xcovm(p)
elementos. Tambien valen las rutinas xwcovm_??? (Metodo mejorado
segun Atal), que utiliza covarianzas modificadas aunque son mucho
mas lentas.

- {vki} es un vector de resultado donde se devuelven los coeficientes
de refrexion k(1) hasta k(p) ==> {p} elementos, vki[0]=k(1).

- {vai} es un vector de resultado donde se devuelven los coeficientes
LPC desde a(1) hasta a(p) ==> {p} elementos, vai[0]=a(1).

- {vti} es un vector de resultado donde se devuelven las areas
del tubo sin perdidas A(1) hasta A(p) ==> {p} elementos, vti[0]=A(1).

- {p} es el orden del analisis. Aunque algunas funciones soportan {p}==0,
la mayor parte no funciona correctamente, por tanto {p}>0.

- {vtmp} es un vector temporal que debe pasarse a la rutina para
realizar operaciones temporales, y que debe tener, como minimo,
espacio para el numero de ELEMENTOS indicado en cada caso.
Se indican elementos, no bytes! Cada rutina de analisis tiene
asociada otra del mismo nombre y prefijo 'tnel_' que devuelve
el numero de elementos SPL_FLOAT que debe tener este vector para un
orden {p} concreto.

Las funciones lpa_??? {devuelven} la potencia del error de prediccion
E=G**2 (si H(z)=G/A(z) es el filtro reconstructor).

Hay varias funciones para cada metodo de calculo: ???_ka, ???_k,
???_a, ???_e, ???_a_ne... que pueden devolver los coeficientes de
reflexion(k) los coeficientes lpc(a), solo la potencia del error (e),
sin la potencia del error (ne), o los lpc y los coef. reflexion a
la vez (ka), etc. Las combinaciones en algunos casos no implementadas
se pueden obtener con llamadas a otras funciones sin perdida de
eficiencia, y por eso no se han implementado.

Ref prediccion lineal, LPC, LSF/LSP:
.     Digital Processing of Speech Signals
.     L.R. Rabiner / R.W Schafer
.     Prentice Hall

.     J. Makhoul
.     Linear Prediction: A Tutorial Review
.     Proc. IEEE vol 63, pp. 561-580, Apr. 1075

.     PUBLIC. Delsarte, Y. V. Genin
.     The Split Levinson Algorithm
.     IEEE trans. ASSP, vol 34, n. 3, Jun. 1986

.     Furui
.     ?????? desaparecido en combate

.     K.K Paliwal, B.S. Atal
.     Efficient Vector Quantization of LPC Parameters at 24 Bits/Frame
.     IEEE. Trans. Speech, and Audio Processing, Vol 1, N.1 Jan 93

.     F.K. Soong B.H. Juang
.     Optimal Quantization of LSP Parameters
.     IEEE. Trans. Speech, and Audio Processing, Vol 1, N.1 Jan 93

Ref. covarianzas modificadas, metodo Cholesky estabilizado:
.     S. Singhal and B. S. Atal
.     Improving performance of multi-pulse LPC coders at low bit rates.
.     Proc. Int. Conf. ASSP vol 1, Paper 1.3

Ref. metodo Cholesky estabilizado, noise shapping:
.     B. S. Atal, M. R. Schroeder
.     Predictive Coding of Speech Signals and Subjective error criteria
.     IEEE Trans. ASSP, vol 27, N.3, Jun. 1979

.     B. S. Atal
.     Predictive Coding of Speech at Low bit Rates
.     IEEE Trans. Comm. Vol COM-30, N.4, April 1982


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
/* Metodo de Durbin para las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes de reflexion {vki} y los coeficientes
lpc {vai} para analisis de orden {p}.
La funcion {devuelve} la potencia del error de prediccion E=G**2 */

PUBLIC SPL_FLOAT XAPI lpa_cor_dur_ka( SPL_pFLOAT vac, SPL_pFLOAT vki,
		SPL_pFLOAT vai, SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,j;
	SPL_FLOAT E;  /*error prediccion E(i)*/
	SPL_FLOAT sum, ki;
	SPL_pFLOAT vo, vn, vx; /* 2 vectores: old, new , y uno auxiliar */
	assert(p>0);

	/* inicializa vectores old(vo) y new(vn) en funcion del order p */
	if (p&1) {  /* si el orden es impar */
		vn = vai;
		vo = vtmp;
	}              /* al final, vn encaja con vai */
	else {
		vo = vai;
		vn = vtmp;
	}

	E = (vac[0]+vac[1])*
			(1.0+LP_NEGSUM_NEG(vn[0]=vki[0]=LP_POSSUM_NEG(vac[1]/vac[0])));

	for ( i=1; i<p; i++ ) {
		__swap(vn,vo,vx);  /* a(i) ---> a(i-1) */

		/* Sumatorio[j=1,j=i-1]( R[i-j] * a(i-1)[j] ) */
		__xsum(j,0,i,vac[i-j]*vo[j],sum,0.0);

		/* a(i)[i] = k[i] = -(R[i]+Sum[...])/E(i-1) */
		vn[i] = vki[i] = ki = LP_POSSUM_NEG(vac[i+1]+LP_NEGSUM_NEG(sum))/E;

		/* a(i)[j]=a(i-1)[j]+k[i]*a(i-1)[i-j] */
		for ( j=0; j<i; j++ )
			vn[j] = vo[j] + LP_NEGSUM_NEG(ki*vo[i-j-1]);

		/* E(i) = (1-k[i]^2)*E(i-1) */
		E *= (1.0-ki*ki);
	}

	return E; /* devuelve G*G */
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cor_dur_ka( SPL_INT p )
{
	assert(p>0);

	return (p-1);
}

/**********************************************************/
/* Metodo de Durbin para las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes de reflexion {vki} para analisis
de orden {p}.
La {funcion} devuelve la potencia del error de prediccion */

PUBLIC SPL_FLOAT XAPI lpa_cor_dur_k( SPL_pFLOAT vac, SPL_pFLOAT vki,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,j;
	SPL_FLOAT E;  /*error prediccion E(i)*/
	SPL_FLOAT sum, ki;
	SPL_pFLOAT vo, vn, vx; /* 2 vectores: old, new , y uno auxiliar */
	assert(p>0);

	vo = vtmp;
	vn = vtmp+(--p);

	E = (vac[0]+vac[1])*
			(1.0+LP_NEGSUM_NEG(vn[0]=vki[0]=LP_POSSUM_NEG(vac[1]/vac[0])));

	for ( i=1; i<p; i++ ) {
		__swap(vn,vo,vx);  /* a(i) ---> a(i-1) */

		/* Sumatorio[j=1,j=i-1]( R[i-j] * a(i-1)[j] ) */
		__xsum(j,0,i,vac[i-j]*vo[j],sum,0.0);

		/* a(i)[i] = k[i] = -(R[i]+Sum[...])/E(i-1) */
		vn[i] = vki[i] = ki = LP_POSSUM_NEG(vac[i+1]+LP_NEGSUM_NEG(sum))/E;

		/* a(i)[j]=a(i-1)[j]+k[i]*a(i-1)[i-j] */
		for ( j=0; j<i; j++ )
			vn[j] = vo[j] + LP_NEGSUM_NEG(ki*vo[i-j-1]);

		/* E(i) = (1-k[i]^2)*E(i-1) */
		E *= (1.0-ki*ki);
	}

	__xsum(j,0,p,vac[p-j]*vn[j],sum,0.0);
	vki[p] = LP_POSSUM_NEG(vac[p+1]+LP_NEGSUM_NEG(sum))/E;

	return E*(1.0-vki[p]*vki[p]); /* devuelve G*G */
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cor_dur_k( SPL_INT p )
{
	assert(p>0);

	return (2*(p-1));
}

/**********************************************************/
/* Metodo de Durbin para las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes lpc {vai} para analisis de orden {p}.
La funcion {devuelve} la potencia del error de prediccion */

PUBLIC SPL_FLOAT XAPI lpa_cor_dur_a( SPL_pFLOAT vac, SPL_pFLOAT vai,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,j;
	SPL_FLOAT E;  /*error prediccion E(i)*/
	SPL_FLOAT sum, ki;
	SPL_pFLOAT vo, vn, vx; /* 2 vectores: old, new , y uno auxiliar */
	assert(p>0);

	/* inicializa vectores old(vo) y new(vn) en funcion del order p */
	if (p&1) { /* si el orden es impar */
		vn = vai;
		vo = vtmp;
	}        /* al final, vn encaja con vai */
	else {
		vo = vai;
		vn = vtmp;
	}

	E = (vac[0]+vac[1])*(1.0+LP_NEGSUM_NEG(vn[0]=LP_POSSUM_NEG(vac[1]/vac[0])));

	for ( i=1; i<p; i++ ) {
		__swap(vn,vo,vx);  /* a(i) ---> a(i-1) */

		/* Sumatorio[j=1,j=i-1]( R[i-j] * a(i-1)[j] ) */
		__xsum(j,0,i,vac[i-j]*vo[j],sum,0.0);

		/* a(i)[i] = k[i] = -(R[i]+Sum[...])/E(i-1) */
		vn[i] = ki = LP_POSSUM_NEG(vac[i+1]+LP_NEGSUM_NEG(sum))/E;

		/* a(i)[j]=a(i-1)[j]+k[i]*a(i-1)[i-j] */
		for ( j=0; j<i; j++ )
			vn[j] = vo[j] + LP_NEGSUM_NEG(ki*vo[i-j-1]);

		/* E(i) = (1-k[i]^2)*E(i-1) */
		E *= (1.0-ki*ki);
	}

	return E; /* devuelve G*G */
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cor_dur_a( SPL_INT p )
{
	assert(p>0);

	return (p-1);
}

/**********************************************************/
/* Metodo de Durbin para las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac}
la funcion devuelve la potencia del error de prediccion para
analisis de orden {p}. */

PUBLIC SPL_FLOAT XAPI lpa_cor_dur_e( SPL_pFLOAT vac, SPL_INT p,
		SPL_pFLOAT vtmp )
{
	SPL_INT i,j;
	SPL_FLOAT E;  /*error prediccion E(i)*/
	SPL_FLOAT sum, ki;
	SPL_pFLOAT vo, vn, vx; /* 2 vectores: old, new , y uno auxiliar */
	assert(p>0);

	vo = vtmp;
	vn = vtmp+(--p);

	E = (vac[0]+vac[1])*(1.0 +
			LP_NEGSUM_NEG(vn[0]=LP_POSSUM_NEG(vac[1]/vac[0])));

	for ( i=1; i<p; i++ ) {
		__swap(vn,vo,vx);  /* a(i) ---> a(i-1) */

		/* Sumatorio[j=1,j=i-1]( R[i-j] * a(i-1)[j] ) */
		__xsum(j,0,i,vac[i-j]*vo[j],sum,0.0);

		/* a(i)[i] = k[i] = -(R[i]+Sum[...])/E(i-1) */
		vn[i] = ki = LP_POSSUM_NEG(vac[i+1]+LP_NEGSUM_NEG(sum))/E;

		/* a(i)[j]=a(i-1)[j]+k[i]*a(i-1)[i-j] */
		for ( j=0; j<i; j++ )
			vn[j] = vo[j] + LP_NEGSUM_NEG(ki*vo[i-j-1]);

		/* E(i) = (1-k[i]^2)*E(i-1) */
		E *= (1.0-ki*ki);
	}

	__xsum(j,0,p,vac[p-j]*vn[j],sum,0.0);
	ki = LP_POSSUM_NEG(vac[p+1]+LP_NEGSUM_NEG(sum))/E;

	return E*(1.0-ki*ki); /* devuelve G*G */
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cor_dur_e( SPL_INT p )
{
	assert(p>0);

	return (2*(p-1));
}

/**********************************************************/
/* Metodo de Atal para las correlaciones. Mas lento que Durbin.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes de reflexion {vki} para analisis
de orden {p}.
La funcion {devuelve} la potencia del error de prediccion */

PUBLIC SPL_FLOAT XAPI lpa_cor_atl_a( SPL_pFLOAT vac, SPL_pFLOAT vai,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,j;
	SPL_FLOAT nsum, dsum;
	SPL_pFLOAT vo, vn, vx; /* 2 vectores: old, new , y uno auxiliar */
	assert(p>0);

	/* inicializa vectores old(vo) y new(vn) en funcion del order p */
	if (p&1) {  /* si el orden es impar */
		vn = vai;
		vo = vtmp;
	}          /* al final, vn encaja con vai */
	else {
		vo = vai;
		vn = vtmp;
	}

	nsum = dsum = vac[1]*(vn[0] = LP_POSSUM_NEG(vac[1]/vac[0]));

	for ( i=1; i<p; i++ ) {
		__swap(vn,vo,vx);  /* a(i) ---> a(i-1) */

		vn[i]= LP_POSSUM_NEG(vac[i+1]+LP_NEGSUM_NEG(nsum))/
					 (vac[0]+LP_NEGSUM_NEG(dsum));
		nsum = vac[1]*vn[i];
		dsum = vac[i+1]*vn[i];

		for ( j=0; j<i; j++ ) {
			vn[j] = vo[j]+LP_NEGSUM_NEG(vn[i]*vo[i-j-1]);
			nsum+=vac[i-j+1]*vn[j];
			dsum+=vac[j+1]*vn[j];
		}
	}

	return vac[0]+LP_NEGSUM_NEG(dsum);
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cor_atl_a( SPL_INT p )
{
	assert(p>0);

	return (p-1);
}

/**********************************************************/

