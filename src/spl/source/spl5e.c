/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL5e.C
Nombre paquete............... SPL
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... NDEBUG, LP_NEGSUM

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.2.0    24/06/97  Borja     incorporar lp_k2ta()
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.1    07/12/93  Borja     soporte LP_NEGSUM_NEG() y LP_POSSUM_NEG()
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Algoritmos de prediccion lineal (LP). Ver SPL5A.C para mas detalles.

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
/* convierte los coeficientes de reflexion {vki} en coeficientes
LPC {vai}. El orden del analisis es {p}. Los vectores {vki} y {vai} tienen
{p} elementos cada uno. El usuario debe crear, pasar y luego destruir
un vector temporal de tnel_lp_k2a(p) elementos en {vtmp}.
La funcion devuelve el propio vector {vai}.
{p}>0 */

PUBLIC SPL_pFLOAT XAPI lp_k2a( SPL_pFLOAT vki, SPL_pFLOAT vai,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,j;
	SPL_FLOAT ki;
	SPL_pFLOAT vo, vn, vx; /* 2 vectores: old, new , y uno auxiliar */
	assert(p>0);

	/* inicializa vectores old(vo) y new(vn) en funcion del order p */
	if (p&1) {  /* si el orden es impar */
		vn = vai;
		vo = vtmp;
	}         /* al final, vn encaja con vai */
	else {
		vo = vai;
		vn = vtmp;
	}

	vn[0] = vki[0];
	for ( i=1; i<p; i++ ) {
		__swap(vn,vo,vx);  /* a(i) ---> a(i-1) */

		vn[i] = ki = vki[i];
		/* a(i)[j]=a(i-1)[j]+k[i]*a(i-1)[i-j] */
		for ( j=0; j<i; j++ )
			vn[j] = vo[j] + LP_NEGSUM_NEG(ki*vo[i-j-1]);
	}
	return vai;
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lp_k2a( SPL_INT p )
{
	assert(p>0);

	return (p-1);
}

/**********************************************************/
/* convierte los coeficientes LPC {vai} en coeficientes de
reflexion {vki}. El orden del analisis es {p}. Los vectores {vki} y {vai}
tienen {p} elementos cada uno. El usuario debe crear, pasar y luego
destruir un vector temporal de tnel_lp_a2k(p) elementos en {vtmp}.
La funcion devuelve el propio vector {vki}.
{p}>0 */

PUBLIC SPL_pFLOAT XAPI lp_a2k( SPL_pFLOAT vai, SPL_pFLOAT vki,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,j;
	SPL_FLOAT f, ai;
#define vn vtmp
	SPL_pFLOAT vo, vx; /* 2 vectores: old, new , y uno auxiliar */
	assert(p>0);

	i=p-1;
	vki[i] = ai = vai[i];
	f = 1-ai*ai;
	i--;
	for (j=0; j<=i; j++)
		vki[j] = (vai[j]+LP_POSSUM_NEG(ai*vai[i-j]))/f;

	/* vn=vtmp en el define */
	vo=vki;

	for ( ;i>0; ) {
		ai=vo[i];
		f = 1-ai*ai;
		i--;
		for (j=0; j<=i; j++)
			vn[j] = (vo[j]+LP_POSSUM_NEG(ai*vo[i-j]))/f;

		vki[i]=vn[i];
		__swap(vn,vo,vx);  /* a(i) ---> a(i-1) */
	}
	return vki;
#undef vn
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lp_a2k( SPL_INT p )
{
	assert(p>0);

	return (p-1);
}

/**********************************************************/
/* convierte los coeficientes de reflexion {vki} en areas del
tubo sin perdidas (vti). El orden del analisis es {p}. Los
vectores {vki} y {vti} tienen {p} elementos cada uno.
La funcion devuelve el propio vector {vti}.
{p}>0. {vki}!={vti}.
Notese que son areas, no area-ratios, y se han normalizado
para area=1 (area del primer "tubito")!!
No se puede decir que las areas obtenidas encajen con las 
areas reales del tracto vocal, sin embargo, aplicando
preenfasis para eliminar el efecto del pulso glotal y 
la radiacion, mas o menos encajara.
Se puede pasar del area al radio de un tubo circular haciendo
radio=sqrt(area/PI). */

PUBLIC SPL_pFLOAT XAPI lp_k2ta( SPL_pFLOAT vki, SPL_pFLOAT vti, SPL_INT p )
{
	SPL_INT i;
	SPL_FLOAT k;

	assert(p>0);
	p--;
	vti[0]=1;
	for (i=0; i<p; i++) {
		k = LP_POSSUM_NEG(vki[i]);
		if (k==-1)
			vti[i+1]=vti[i];
		else
			vti[i+1] = vti[i]*(1-k)/(1+k);
	}
  return vti;
}

/**********************************************************/
/* a partir de los coeficientes LPC de orden {p} en {vai} ({p} elementos)
calcula las {p} primeras autocorrelaciones R(1) a R(p) de la respuesta
impulsional del filtro reconstructor H(z)=1/(1+-sum(ak*z^-k)),
normalizadas a la autocorrelacion en el origen en {vrh} ({p} elementos).
La funcion devuelve el valor de la autocorrelacion en el origen
(valor de normalizacion).
Las autocorrelaciones de la respuesta impulsional coinciden con
las del segmento de analisis utilizadas para el calculo de los
LPC por cualquiera de los metodos de autocorrelacion (Durbin...)
normalizadas a la del origen. Por tanto, a partir de estas
autocorrelaciones (con un 1 por delante) se pueden recuperar los
coeficientes LPC ( es decir, la rutina lp_rh2a() realmente es
cualqiera de los metodos para la autocorrelacion: lpa_cor_dur_a()...)
El vector temporal {vtmp} tiene tnel_lp_a2rh(p) elementos.
{p}>0 */

PUBLIC SPL_FLOAT XAPI lp_a2rh( SPL_pFLOAT vai, SPL_pFLOAT vrh,
		SPL_INT p, SPL_pFLOAT vtmp )
{
#define vn vtmp
	SPL_INT i,j;
	SPL_FLOAT ai,ai12,sum;
	SPL_pFLOAT vo;
	assert(p>0);

	vo=vai;
	/*vn=vtmp en el define */
	for (i=(--p); i>=1; ) {
		ai=vo[i--];
		ai12=1-ai*ai;
		for (j=0; j<=i; j++)
			vn[j] = (vo[j]+LP_POSSUM_NEG(ai*vo[i-j]))/ai12;
		vo=vn;
		vn += (i+1);
	}

	vn--;
	for (i=0; i<p; ) {
#ifdef LP_NEGSUM
		__xsum(j,0,i,vn[j]*vrh[i-j-1],sum,vn[i]);
#else
		__xsub(j,0,i,vn[j]*vrh[i-j-1],sum,-vn[i]);
#endif
		vrh[i]=sum;
		i++;
		vn -= (i+1);
	}
#ifdef LP_NEGSUM
	__xsum(j,0,p,vai[j]*vrh[p-j-1],sum,vai[p]);
#else
	__xsub(j,0,p,vai[j]*vrh[p-j-1],sum,-vai[p]);
#endif
	vrh[p]=sum;

#ifdef LP_NEGSUM
	__sub(j,0,p,vai[j]*vrh[j],sum,1.0);
#else
	__sum(j,0,p,vai[j]*vrh[j],sum,1.0);
#endif
	return 1.0/sum;
#undef vn
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lp_a2rh( SPL_INT p )
{
	assert(p>0);

	return ((p-1)*p)/2;
}

/**********************************************************/
/* a partir de los coeficientes lpc {vai} de orden {p} calcula las
correlaciones de la respuesta impulsional del filtro inverso A(z).
A(z) es un filtro FIR de {p}+1 elementos (la respuesta impulsional
consiste en un 1 seguido de los coeficientes lpc {vai}) por lo que
solo hay {p}+1 autocorrelaciones distintas de 0.
El vector {vai} tiene {p} elementos (coeficientes LPC de a(1) a a(p).
El vector {vra} que se rellena, tiene {p}+1 elementos (correlaciones
desde 0 hasta {p})
La funcion {devuelve} el vector {vra}.
*/

PUBLIC SPL_pFLOAT XAPI lp_a2ra( SPL_pFLOAT vai,
		SPL_pFLOAT vra, SPL_INT p )
{
	SPL_INT i,k;
	SPL_FLOAT sum;
	assert(p>=0);

	__xsum(k,0,p,vai[k]*vai[k],sum,1.0);
	vra[0]=sum;

	for (i=1; i<=p; i++) {
#ifdef __LP_NEGSUM
		__xsum(k,i,p,vai[k]*vai[k-i],sum,-vai[i-1]);
#else
		__xsum(k,i,p,vai[k]*vai[k-i],sum,vai[i-1]);
#endif
		vra[i]=sum;
	}

	return vra;
}

/**********************************************************/
/* calcula los cepstrum LPC de H(z). {vai} el el vector de
coeficientes lpc de orden {p} ({p} elementos) y en {vci} se devuelven
los cepstrum c(1) a c(p)  ({p} elementos). Para obtener c(0), llamar
a la funcion lp_c0(e) siendo {e} la potencia del error de prediccion.
La funcion {devuelve} el vector {vci} */

PUBLIC SPL_pFLOAT XAPI lp_a2c( SPL_pFLOAT vai, SPL_pFLOAT vci, SPL_INT p )
{
	SPL_INT i,k;
	SPL_FLOAT sum;
	assert(p>=0);

	for (i=0; i<p; i++) {
#ifdef LP_NEGSUM
		__sum(k,1,i,k/(i+1)*vci[k-1]*vai[i-k],sum,vai[i]);
#else
		__sub(k,1,i,k/(i+1)*vci[k-1]*vai[i-k],sum,-vai[i]);
#endif
		vci[i]=sum;
	}

	return vci;
}

/**********************************************************/
/* {devuelve} el cepstrum c(0). {e} es la potencia del error de
prediccion.
{e}>0 */

PUBLIC SPL_FLOAT XAPI lp_c0( SPL_FLOAT e )
{
	assert(e>0.0);

	return 0.5*log(e);
}

/**********************************************************/
/* genera el filtro de anchos de banda expandidos {vbwea} a partir
del filtro lpc {vai}, de orden {p}.

.              P(z) ---------> P(z/gamma)
.              a[k] ---------> a[k]*gamma^k

{vai} y {vbwea} tienen {p} elementos.
{gamma} es el factor de expandido. 1 no hace nada, 0 tiene efecto
maximo.
Los vectores {vai} y {vbwea} SI pueden ser el mismo.
0<={gamma}<=1.
La funcion {devuelve} el vector {vbwea} */

PUBLIC SPL_pFLOAT XAPI lp_a2bwea( SPL_pFLOAT vai, SPL_pFLOAT vbwea,
		SPL_INT p, SPL_FLOAT gamma )
{
	SPL_INT i;
	SPL_FLOAT agm;
	assert(gamma>=0.0);
	assert(gamma<=1.0);
	assert(p>=0);

	agm=1.0;
	for (i=0; i<p; i++) {
		agm*=gamma;
		vbwea[i]=vai[i]*agm;
	}
	return vbwea;
}

/**********************************************************/
/* Predictor P(z).
efectua la prediccion de orden {p} para la muestra actual en
funcion de {p} muestras anteriores(contiguas o no). En {vs} se
envian las {p} muestras utilizadas (p elementos):
.        vs[0]=s[n-p] hasta vs[p-1]=s[n-1]  (corto termino)
.        vs[0]=s[n-p-M+1] hasta vs[p-1]=s[n-M]  (largo termino: retardo M)
y predice s[n] (valor que {devuelve} la funcion) utilizando los
coeficientes lpc {vai} ({p} elementos). */

PUBLIC SPL_FLOAT XAPI lps_P_a( SPL_pFLOAT vs, SPL_pFLOAT vai, SPL_INT p )
{
	SPL_INT i;
	SPL_FLOAT sum;
	assert(p>=0);

	__xsum(i,0,p,vai[i]*vs[p-i-1],sum,0.0);

	return LP_POSSUM_NEG(sum);
}

/**********************************************************/
/* Filtro inverso A(z).
efectua la prediccion de orden {p} para la muestra actual en
funcion de {p} muestras anteriores(contiguas o no). En {vs} se envian las
{p} muestras utilizadas({p} elementos):
.           vs[0]=s[n-p] hasta vs[p-1]=s[n-1]  (corto termino)
.           vs[0]=s[n-p-M+1] hasta vs[p-1]=s[n-M]  (largo termino: retardo M)
En {sn} se envia la muestra en el instante actual s[n].
Se predice la muestra actual s[n] utilizando los coeficientes lpc {vai}
({p} elementos).
La funcion {devuelve} el error de prediccion e[n] del filtro, restando
al valor de la muestra actual el valor de prediccion. */

PUBLIC SPL_FLOAT XAPI lps_A_a( SPL_pFLOAT vs, SPL_pFLOAT vai,
		SPL_INT p, SPL_FLOAT sn )
{
	SPL_INT i;
	SPL_FLOAT sum;
	assert(p>=0);

	__xsum(i,0,p,vai[i]*vs[p-i-1],sum,0.0);

	return sn+LP_NEGSUM_NEG(sum);
}

/**********************************************************/
/* Filtro reconstructor H(z).
efectua la prediccion de orden {p} para la muestra actual en
funcion de {p} muestras anteriores(contiguas o no). En {vs} se envian las
{p} muestras utilizadas({p} elementos):
.         vs[0]=s[n-p] hasta vs[p-1]=s[n-1]  (corto termino)
.         vs[0]=s[n-p-M+1] hasta vs[p-1]=s[n-M]  (largo termino: retardo M)
En {en} se envia el error en el instante actual e[n].
Se predice la muestra actual s[n] utilizando los coeficientes lpc {vai}
({p} elementos).
La funcion {devuelve} el valor de prediccion corregido (sumado) con
el error e[n]. */

PUBLIC SPL_FLOAT XAPI lps_H_a( SPL_pFLOAT vs, SPL_pFLOAT vai,
		SPL_INT p, SPL_FLOAT en )
{
	SPL_INT i;
	SPL_FLOAT sum;
	assert(p>=0);

	__xsum(i,0,p,vai[i]*vs[p-i-1],sum,0.0);

	return en+LP_POSSUM_NEG(sum);
}

/**********************************************************/
/* a partir de los coeficientes LPC {vai} de orden {p}, devuelve
en {van} las N primeras muestras de la respuesta impulsional a(n)
del filtro A(z), desde a(0) hasta a(N-1).
La respuesta impulsional es finita, con a(0)=1, a(1)= +-a1
hasta a(p)= +-ap.
+- depende de si LP_NEGSUM esta o no definido. Si esta
definido, es -, si no esta definido, es +.
A partir de a(p+1), la respuesta impulsional es nula.

La funcion {devuelve} {van}. */

PUBLIC SPL_pFLOAT XAPI lps_A_an_a( SPL_pFLOAT vai, SPL_INT p,
		SPL_pFLOAT van, SPL_INT N )
{
	SPL_INT i;
	assert(p>=0);
	assert(N>=0);

	if (N>0)
		van[0]=1.0;

	if (p>=N)
		p=N-1;

	for (i=1; i<=p; i++)
		van[i]=LP_NEGSUM_NEG(vai[i-1]);

	for (i=p+1; i<N; i++)
		van[i]=0.0;

	return van;
}

/**********************************************************/
/* a partir de los coeficientes LPC {vai} de orden {p}, devuelve
en {vhn} las N primeras muestras de la respuesta impulsional h(n)
del filtro H(z), desde h(0) hasta h(N-1).
La respuesta impulsional es infinita, con h(0)=1.

La funcion {devuelve} {vhn}. */

PUBLIC SPL_pFLOAT XAPI lps_H_hn_a( SPL_pFLOAT vai, SPL_INT p,
		SPL_pFLOAT vhn, SPL_INT N )
{
	SPL_INT i;
	assert(p>=0);
	assert(N>=0);

	if (N>0)
		vhn[0]=1.0;

	if (N>p) {
		for (i=1; i<=p; i++)
			vhn[i]=lps_H_a(vhn+(i-1),vai,i,0.0);
	}

	for (i=p+1; i<N; i++)
		vhn[i]=lps_H_a(vhn+(i-1),vai,p,0.0);

	return vhn;
}

/**********************************************************/
/* a partir de los coeficientes LPC {vai} de orden {p}, devuelve
en {vpn} las {N} primeras muestras de la respuesta impulsional p(n)
del filtro P(z), desde p(0) hasta p(N-1).
La respuesta impulsional es finita, con p(0)=0, p(1)= -+a1
hasta p(p)= -+ap.
-+ depende de si LP_NEGSUM esta o no definido. Si esta
definido, es +, si no esta definido, es -.
A partir de p(p+1), la respuesta impulsional es nula.

La funcion {devuelve} {vpn}. */

PUBLIC SPL_pFLOAT XAPI lps_P_pn_a( SPL_pFLOAT vai, SPL_INT p,
		SPL_pFLOAT vpn, SPL_INT N )
{
	SPL_INT i;
	assert(p>=0);
	assert(N>=0);

	if (N>0)
		vpn[0]=0.0;

	if (p>=N)
		p=N-1;

	for (i=1; i<=p; i++)
		vpn[i] = LP_POSSUM_NEG(vai[i-1]);

	for (i=p+1; i<N; i++)
		vpn[i]=0.0;

	return vpn;
}

/**********************************************************/

