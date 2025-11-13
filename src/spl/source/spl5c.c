/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL5c.C
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
1.0.2    07/12/93  Borja     soporte LP_NEGSUM_NEG() y LP_POSSUM_NEG()
1.0.1    05/12/93  Borja     funciones para LSFs.
1.0.0    16/09/93  Borja     Codificacion inicial.

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
/* a partir de los coeficientes lpc de orden {p} enviados en el vector
{vai} ({p} coeficientes, desde a(1) hasta a(p)) calcula los
polinomios singulares simetrico y antisimetrico (p(x) y q(x))
en los que se ha hecho un cambio de variable z por x (Furui).
Los vectores resultado se devuelven en {vpi} y {vqi} respectivamente.
El vector {vtmp} es un vector temporal de tnel_lp_a2pq(p) elementos.
Solo se ha implementado el proceso para orden {p} par (si alguien
tiene ganas...). Para orden {p} impar simplemente se calculan
los polinomios para el siguiente orden (par) {p}+1, asumiendo
que el coeficiente a(p+1)=0.
Los polinomios p(x) y q(x) son de orden {p}/2, y por tanto cada uno
de los vectores {vpi} y {vqi} debe tene (({p}/2)+1) elementos,
pero si el orden {p} es impar, como se calculan los polinomios del
siguiente orden, estos vectores seran de ({p}/2+2) elementos.
En cualquier caso, tanto para {p} par como impar, se cumple que
estos vectores son de ({p}+3)/2 elementos (operacion con enteros).

Los ceros xi de estos polomios p(xi)=0 y q(xi)=0 dan directamente
las frecuencias espectrales de linea wi mediante el calculo

.       xi=cos(wi)  -----> wi=(+-)acos(xi)  (radianes)

Los polinomios se devuelve empezando por el termino independiente:

.       p(x) = vpi[0] + vpi[1]*x + vpi[2]*(x**2) +...+ vpi[p/2]*(x**(p/2))
.       q(x) = vqi[0] + vqi[1]*x + vqi[2]*(x**2) +...+ vqi[p/2]*(x**(p/2))

siempre con {p} par (si es impar, se usa {p}={p}+1).
Los ceros xi de p(x) y q(x) cumplen -1<xi<1 y ademas los ceros
de p(x) estan entrelazados con los de q(x)  (Furui).

Enviar siempre p>=1. */

PUBLIC SPL_VOID XAPI lp_a2pq( SPL_pFLOAT vai, SPL_INT p, SPL_pFLOAT vpi,
		SPL_pFLOAT vqi, SPL_pFLOAT vtmp )
{
	SPL_INT i,j,k, p2;
#define pp vtmp
	SPL_pFLOAT qq,nc,oc,t;
	assert(p>0);

	p2=(p+1)/2;    /* +1 para tener en cuenta si p es impar */
	/*pp=vtmp, en el define*/
	qq=pp+p2;
	nc=qq+p2;
	oc=nc+(p2/2)+1;

	if (p&1) {      /* si p es impar */
		p++;
		pp[0]=LP_NEGSUM_NEG(vai[0])-1.0;
		qq[0]=LP_NEGSUM_NEG(vai[0])+1.0;
	}
	else {
		pp[0]=LP_NEGSUM_NEG(vai[0]+vai[p-1])-1.0;
		qq[0]=LP_NEGSUM_NEG(vai[0]-vai[p-1])+1.0;
	}
	for (i=1; i<p2; i++) {
		pp[i]= (-pp[i-1])+LP_NEGSUM_NEG(vai[i]+vai[p-1-i]);
		qq[i]= (qq[i-1])+LP_NEGSUM_NEG(vai[i]-vai[p-1-i]);
	}

	for (i=1; i<=p2; i++)
		vpi[i]=vqi[i]=0.0;
	vpi[0]=pp[p2-1]/2.0;
	vqi[0]=qq[p2-1]/2.0;

	/*ciclo de cosenos, para desarrollar todos los cos(x**n) en cos(x) */
	oc[0]=nc[0]=1.0;
	for (i=1; i<p2; ) {
		k=0;
		for (j=i; j>=0; j-=2) {
			vpi[j]+=nc[k]*pp[p2-1-i];
			vqi[j]+=nc[k]*qq[p2-1-i];
			k++;
		}

		__swap(nc,oc,t);
		j=(++i)/2;
		if (!(i&1)) {
			nc[j]=-nc[j-1];
			j--;
		}
		for ( ; j>=1; j--)
			nc[j]=oc[j]*2-nc[j-1];
		nc[0]=oc[0]*2.0;
	}
	k=0;
	for (j=p2; j>=0; j-=2) {
		vpi[j]+=nc[k];
		vqi[j]+=nc[k];
		k++;
	}
#undef pp
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lp_a2pq( SPL_INT p )
{
	assert(p>0);

	if (p&1)	/* si el orden es impar */
		p++;
	return p+p/2+2;
}

/**********************************************************/
/* Metodo Split-Levinson simetrico para las correlaciones. Fast!
El mas rapido para el metodo de las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac} la
funcion devuelve unos coeficientes (alfa-sub-i) {vxs} ({p} coeficientes)
que definen el polinomio predictor singular. La funcion, {devuelve}
la potencia del error de prediccion para analisis de orden {p}. La
funcion lpa_cor_sls_b_ne() hace lo mismo que esta pero no devuelve
el error.

En la referencia The Split Levinson Algorithm (PUBLIC. Delsarte, Y. V. Genin)
se definen estos coeficientes, y como derivar otros coeficientes beta
a partir de ellos, con los cuales se forma un determinante (Jacobiano)
del que se extraen los LSFs (autovalores...) */

PUBLIC SPL_FLOAT XAPI lpa_cor_sls_xa( SPL_pFLOAT vac, SPL_pFLOAT vxs,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,k,t;
#define po vtmp
	SPL_pFLOAT pa, pn, px; /* tres vectores, old,actual,new, y uno auxiliar */
	SPL_FLOAT sum,tk,ak,lk;
	assert(p>0);

	t = (p+1)/2;
	/* po=vtmp en el define */
	pa=po+t;
	pn=pa+t;

	pa[0]=2.0-2.0*(vxs[0]=(tk=vac[0]+vac[1])/vac[0]);
	t=0;

	for (k=1; k<p; k++) {
		__xsum(i,0,t,(vac[i+1]+vac[k-i])*pa[i],sum,vac[0]+vac[k+1]);
		if (k&1) {
			sum+=vac[t+1]*pa[t];
			vxs[k]=ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}
		else {
			vxs[k]=ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			pn[t]=pa[t-1]+pa[t-1]-ak*po[t-1];
		}
		px=po;
		po=pa;
		pa=pn;
		pn=px;
	}

	lk=sum=1.0;
	if (p&1) { /* impar */
		for (i=0; i<t; i++) {
			sum+=po[i];
			lk+=pa[i];
		}
		lk=(lk+lk+pa[i])/(sum+sum);
	}
	else {
		for (i=0; i<t-1; i++) {
			sum+=po[i];
			lk+=pa[i];
		}
		lk=(2.0*(lk+pa[i]))/(sum+sum+po[i]);
	}
	return lk*tk;
#undef po
}

/**********************************************************/
/* Metodo Split-Levinson simetrico para las correlaciones. Fast!
El mas rapido para el metodo de las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac} la
funcion devuelve unos coeficientes (alfa-sub-i) {vxs} ({p} coeficientes)
que definen el polinomio predictor singular. La funcion NO {devuelve}
la potencia del error de prediccion, para ahorrar operaciones. La funcion
lpa_cor_sls_xa() hace lo mismo que esta pero devuelve el error.

En la referencia The Split Levinson Algorithm (PUBLIC. Delsarte, Y. V. Genin)
se definen estos coeficientes, y como derivar otros coeficientes beta
a partir de ellos, con los cuales se forma un determinante (Jacobiano)
del que se extraen los LSFs (autovalores...) */

PUBLIC SPL_VOID XAPI lpa_cor_sls_xa_ne( SPL_pFLOAT vac, SPL_pFLOAT vxs,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,k,t;
#define po vtmp
	SPL_pFLOAT pa, pn, px; /* tres vectores, old,actual,new, y uno auxiliar */
	SPL_FLOAT sum,tk,ak;
	assert(p>0);

	t = (p+1)/2;
	/* po=vtmp en el define */
	pa=po+t;
	pn=pa+t;

	pa[0]=2.0-2.0*(vxs[0]=(tk=vac[0]+vac[1])/vac[0]);
	t=0;

	for (k=1; k<p; k++) {
		__xsum(i,0,t,(vac[i+1]+vac[k-i])*pa[i],sum,vac[0]+vac[k+1]);
		if (k&1) {
			sum+=vac[t+1]*pa[t];
			vxs[k]=ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}
		else {
			vxs[k]=ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			pn[t]=pa[t-1]+pa[t-1]-ak*po[t-1];
		}
		px=po;
		po=pa;
		pa=pn;
		pn=px;
	}

#undef po
}

/**********************************************************/
/* Metodo Split-Levinson antisimetrico para las correlaciones. Fast!
El tan rapido como el simetrico.
A partir de las {p}+1 primeras autocorrelaciones {vac} la
funcion devuelve unos coeficientes (alfa-sub-i) {vxa} ({p} coeficientes)
que definen el polinomio predictor singular antisimetrico. La funcion,
{devuelve} la potencia del error de prediccion para analisis de orden {p}
(coincide con el valor que devuelve lpa_cor_sls_xa(). La funcion
lpa_cor_sla_xa_ne() hace lo mismo que esta pero no devuelve el error.

En la referencia The Split Levinson Algorithm (PUBLIC. Delsarte, Y. V. Genin)
se definen estos coeficientes, y como derivar otros coeficientes beta
a partir de ellos, con los cuales se forma un determinante (Jacobiano)
del que se extraen los LSFs (autovalores...) */

PUBLIC SPL_FLOAT XAPI lpa_cor_sla_xa( SPL_pFLOAT vac, SPL_pFLOAT vxa,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,k,t;
#define po vtmp
	SPL_pFLOAT pa, pn, px; /* tres vectores, old,actual,new, y uno auxiliar */
	SPL_FLOAT sum,tk,ak,lk;
	assert(p>0);

	t = p/2;
	/* po=vtmp en el define */
	pa=po+t;
	pn=pa+t;

	lk=2.0-(vxa[0]=ak=(tk=vac[0]-vac[1])/vac[0]);
	pa[0]=0.0;
	t=0;

	for (k=1; k<p; k++) {
		if (k&1) {
			__xsum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			vxa[k]=ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			if (t)
				pn[t]=pa[t-1]-ak*po[t-1];
		}
		else {
			__sum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			vxa[k]=ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}
		lk=2.0-ak/lk;

		px=po;
		po=pa;
		pa=pn;
		pn=px;
	}

	return lk*tk;
#undef po
}

/**********************************************************/
/* Metodo Split-Levinson antisimetrico para las correlaciones. Fast!
El tan rapido como el simetrico.
A partir de las {p}+1 primeras autocorrelaciones {vac} la
funcion devuelve unos coeficientes (alfa-sub-i) {vxa} ({p} coeficientes)
que definen el polinomio predictor singular antisimetrico. La
funcion NO {devuelve} la potencia del error de prediccion, para
ahorrar operaciones. La funcion lpa_cor_sla_xa() hace lo mismo que
esta pero devuelve el error.

En la referencia The Split Levinson Algorithm (PUBLIC. Delsarte, Y. V. Genin)
se definen estos coeficientes, y como derivar otros coeficientes beta
a partir de ellos, con los cuales se forma un determinante (Jacobiano)
del que se extraen los LSFs (autovalores...) */

PUBLIC SPL_VOID XAPI lpa_cor_sla_xa_ne( SPL_pFLOAT vac, SPL_pFLOAT vxa,
		SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,k,t;
#define po vtmp
	SPL_pFLOAT pa, pn, px; /* tres vectores, old,actual,new, y uno auxiliar */
	SPL_FLOAT sum,tk,ak;
	assert(p>0);

	t = p/2;
	/* po=vtmp en el define */
	pa=po+t;
	pn=pa+t;

	vxa[0]=ak=(tk=vac[0]-vac[1])/vac[0];
	pa[0]=0.0;
	t=0;

	for (k=1; k<p; k++) {
		if (k&1) {
			__xsum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			vxa[k]=ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			if (t)
				pn[t]=pa[t-1]-ak*po[t-1];
		}
		else {
			__sum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			vxa[k]=ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}
		px=po;
		po=pa;
		pa=pn;
		pn=px;
	}

#undef po
}

/**********************************************************/
/* Calculo de los coeficientes espectrales de linea LSF o LSP
(line spectrum pairs).
Utiliza el metodo de las correlaciones (Split Levinson).
A partir de las {p}+1 primeras autocorrelaciones {vac} la
funcion devuelve los coeficientes LSFs en {vlx} ({p} coeficientes)
que definen el polinomio predictor optimo de orden {p}.
Si en {E} se envia un puntero a un SPL_FLOAT, se almacena en el el
valor de la potencia del error de prediccion, pero si en {E} se
envia NULL, la funcion no calcula este valor, reduciendo el
numero de operaciones. {vtmp} es un vector temporal de trabajo que
debe enviar el usuario, y debe tener sitio para tnel_lpa_cor_slv_lx(p)
valores SPL_FLOAT.
Puesto que la funcion debe calcular autovalores de un Jacobiano,
puede suceder que se sobrepase el numero maximo de iteraciones
permitidas (ver eigen_tdg()), en cuyo caso el resultado queda
indefinido, y la funcion {devuelve} TRUE. En caso de que todo
se desarrolle correctamente, la funcion {devuelve} FALSE.

Los valores de los LSF devueltos estan en el rango {1,-1}, y corresponden
realmente con el valor del coseno de la frecuencia LSF correspondiente.
Ver lp_lx2lw() y lp_lw2lx() para conversion entre frecuencias y cosenos.

Los LSF son los ceros de los polinomios singulares sim‚trico P(z)
y antisimetrico Q(z). Para un analisis de orden {p}, P(z) tiene
{p}+1 raices y Q(z) otras {p}+1, siempre por pares simetricos conjugados.
En caso de que {p} sea par, la raiz que sobra es un valor trivial, siendo
un cero en z=-1 para P(z) y uno en z=1 para Q(z). Si {p} es impar, entonces
P(z) tiene ({p}+1)/2 pares conjugados (un total de {p}+1 raices, ninguna
trivial) mientras que Q(z) tiene solo ({p}-1)/2 pares conjugados y dos
raices triviales, una en z=-1 y otra en z=1.
Ademas, las raices de P(z) y Q(z) van entrelazadas entre si, (y siempre
estan en el circulo de radio unidad). Puesto que z=1 es siempre raiz
singular de Q(z), siempre sucede que la primera raiz no singular
pertenece a P(z), la siguiente a Q(z), etc.

Esta funcion, en lugar de devolver las raices como frecuencias w(i),
devuelve las raices como x(i), con el cambio de variable x=cos(w).
Ver lp_lx2lw() y lp_lw2lx() para conversion entre frecuencias y cosenos.
La funcion no devuelve las raices singulares, ni las que estan en
el semiplano inferior (conjugadas). Tan solo se devuelven las raices
entre w=0 y w=Pi, y por eso los valores devueltos (x=cos(x)) varian entre
1 para w=0 y -1 para w=Pi. Ademas las raices se devuelven ordenadas, y
entrelazadas, es decir vlx[0] es la primera raiz (no singular) de P(x),
vlx[1] la primera de Q(x), vlx[2] la segunda de P(x) y asi sucesivamente
hasta {p} raices.
Ademas, mientras se conserve este orden de entrelazado, y se cumpla
w(i-1)<w(i)<w(i+1) se asegura la estabilidad del predictor.
p>0.

Ref:
.     PUBLIC. Delsarte, Y. V. Genin
.     The Split Levinson Algorithm
.     IEEE trans. ASSP, vol 34, n. 3, Jun. 1986

.     Furui
.     ????

.     K.K Paliwal, B.S. Atal
.     Efficient Vector Quantization of LPC Parameters at 24 Bits/Frame
.     IEEE. Trans. Speech, and Audio Processing, Vol 1, N.1 Jan 93

.     F.K. Soong B.H. Juang
.     Optimal Quantization of LSP Parameters
.     IEEE. Trans. Speech, and Audio Processing, Vol 1, N.1 Jan 93  */

PUBLIC SPL_BOOL XAPI lpa_cor_slv_lx( SPL_pFLOAT vac, SPL_pFLOAT vlx,
		SPL_INT p, SPL_FLOAT * E, SPL_pFLOAT vtmp )
{
#define SORT_LSFS  /* si se quita este define, las raices no se ordenan.
		  Asi que no quitarlo!!. Tan solo esta por si algun
		  dia hace falta una funcion mas rapida que no ordene*/

	SPL_INT i, j, n;
#ifdef SORT_LSFS
	SPL_INT m;
#endif

#define vx vtmp
	SPL_pFLOAT vd, ve;
	assert(p>0);

	n = (p+1)/2;
	/*vx=vtmp en el define */
	vd = vx+p;
	ve = vd+n;

	if (E==NULL)	/* no devolver ni calcular E */
		lpa_cor_sls_xa_ne(vac,vx,p,vd);
	else
		(*E)=lpa_cor_sls_xa(vac,vx,p,vd);

	vx[0]*=0.5;
	for (i=1; i<p; i++)
		vx[i] *= 0.25;
	if (p&1) { /* p+1 par */
		vd[0]=vx[0];
		j=0;
	}
	else {
		vd[0]=vx[0]+vx[1];
		j=1;
	}
	for (i=1; i<n; i++) {
		ve[i-1]=sqrt(vx[j]*vx[j+1]);
		vd[i]=vx[j+1]+vx[j+2];
		j+=2;
	}
	if (eigen_tdg(vd,ve,n,NULL)!=SPL_FALSE)  /* calcula autovalores...*/
		return SPL_TRUE;	/* y fin en caso de error */
#ifdef SORT_LSFS
	for (i=0; i<n; i++) {
		for (j=m=0; j<n; j++)  /* busca el mayor */
			if (vd[j]>vd[m])
				m=j;
		vlx[i*2]= 2.0*vd[m]-1.0;
		vd[m]=0.0;
	}
#else
	for (i=0; i<n; i++) /* guardar sin ordenar */
		vlx[i] = 2.0*vd[i]-1.0;
#endif

	lpa_cor_sla_xa_ne(vac,vx,p,vd);
	for (i=1; i<p; i++)
		vx[i] *= 0.25;
	if (p&1) {	/* p impar */
		vd[0]=vx[1]+vx[2];
		j=2;
	}
	else {
		vd[0]=vx[1];
		j=1;
	}
	p-=n;
	for (i=1; i<p; i++) {
		ve[i-1]=sqrt(vx[j]*vx[j+1]);
		vd[i]=vx[j+1]+vx[j+2];
		j+=2;
	}
	if (eigen_tdg(vd,ve,p,NULL)!=SPL_FALSE)  /* calcula autovalores...*/
		return SPL_TRUE;	/* y fin en caso de error */

#ifdef SORT_LSFS
	for (i=0; i<p; i++) {
		for (j=m=0; j<p; j++)  /* busca el mayor */
			if (vd[j]>vd[m])
				m=j;
		vlx[i*2+1]= 2.0*vd[m]-1.0;
		vd[m]=0.0;
	}
#else
	for (i=0; i<p; i++)  /* guardar sin orden */
		vlx[n+i] = 2.0*vd[i]-1.0;
#endif

	return SPL_FALSE;

#undef vx
#undef SORT_LSFS
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cor_slv_lx( SPL_INT p )
{
	SPL_INT i1, i2;
	assert(p>0);

	i1 = tnel_lpa_cor_sls_xa(p);

	i2 = tnel_lpa_cor_sls_xa_ne(p);
	if (i1<i2)
		i1=i2;

	i2 = tnel_lpa_cor_sla_xa_ne(p);
	if (i1<i2)
		i1=i2;

	if (i1<=p)
		i1=p+1;

	return p+2+i1;
}

/**********************************************************/
/* A partir de los {p} coeficientes enviados en el vector {vlx}
y que representan los cosenos de las LSFs (rango 1, -1), devuelve en
el vector {vlw} (tambien de {p} elementos) las LSFs correspondientes
(rango 0, Pi).
Es decir, calcula vlw[i]=arccos(vlx[i]).
La funcion {devuelve} el vector {vlw}. Ademas {vlw} puede coincidir
con {vlx}.
p>=0. */

PUBLIC SPL_pFLOAT XAPI lp_lx2lw( SPL_pFLOAT vlx, SPL_pFLOAT vlw, SPL_INT p )
{
	SPL_INT i;
	assert(p>=0);

	for (i=0; i<p; i++)
		vlw[i]=acos(vlx[i]);

	return vlw;
}

/**********************************************************/
/* A partir de los {p} coeficientes enviados en el vector {vlw}
y que representan las LSFs (rango 0, Pi), devuelve en el
vector {vlx} (tambien de {p} elementos) los cosenos de las LSFs
correspondientes (rango 1, -1).
Es decir, calcula vlx[i]=cos(vlw[i]).
La funcion {devuelve} el vector {vlx}. Ademas {vlx} puede coincidir
con {vlw}.
p>=0. */

PUBLIC SPL_pFLOAT XAPI lp_lw2lx( SPL_pFLOAT vlw, SPL_pFLOAT vlx, SPL_INT p )
{
	SPL_INT i;
	assert(p>=0);

	for (i=0; i<p; i++)
		vlx[i]=cos(vlw[i]);

	return vlx;
}

/**********************************************************/
/* a partir del vector de coeficientes LPC {vai} de {p}
elementos (analisis predictivo de orden {p}, calcula los
cosenos de las LSFs, y los devuelve en el vector {vlx} de {p}
elementos. Ver lpa_cor_slv_lx() para mas informacion sobre {vlx}.
Si se obtiene el resultado, la funcion {devuelve} FALSE, pero si
se supera el numero maximo de iteraciones al obtener los autovalores,
la funcion {devuelve} TRUE y el resultado es indefinido.
{vtmp} es un vector temporal de tnel_lp_a2lx(p) elementos.
p>0. */

PUBLIC SPL_BOOL XAPI lp_a2lx( SPL_pFLOAT vai, SPL_pFLOAT vlx, SPL_INT p,
		SPL_pFLOAT vtmp )
{
	assert(p>0);

	lp_a2rh(vai,vtmp+1,p,vtmp+p+1);   /* calcula autocorrelaciones */
	vtmp[0]=1.0;    /* estan normalizadas a la del origen */

	return lpa_cor_slv_lx(vtmp,vlx,p,NULL,vtmp+p+1);
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lp_a2lx( SPL_INT p )
{
	SPL_INT i1, i2;
	assert(p>0);

	i1 = tnel_lp_a2rh(p);

	i2=tnel_lpa_cor_slv_lx(p);
	if (i1<i2)
		i1=i2;

	return i1+p+1;
}

/**********************************************************/
/* a partir de los {p} cosenos de las LSFs enviados en {vlx} para un
analisis predictivo de orden {p}, calcula los coeficientes LPC
correspondientes en {vai} (tambien de {p} elementos.
{vtmp} es un vector temporal de tnel_lp_lx2a(p) elementos.
La funcion {devuelve} el puntero {vai}.
El vector {vlx} debe cumplir la condicion de que todas las LSFs esten
ordenadas de forma ascendente, (y raices de P(z) y Q(z) entrelazadas,
comenzando por la menor raiz no trivial de P(z). Este es precisamente
el formato que utiliza lpa_cor_slv_lx() al calcular {vlx}. Ver esta
funcion para mas informacion.
p>0.  */

PUBLIC SPL_pFLOAT XAPI lp_lx2a( SPL_pFLOAT vlx, SPL_pFLOAT vai,
		SPL_INT p, SPL_pFLOAT vtmp )
{
#define pp vtmp
	SPL_pFLOAT qq, ac;
	SPL_FLOAT lx2;
	SPL_INT i, j, np, nq;
	assert(p>0);

	np=(p+1)/2;
	nq=p/2;
	/* pp=vtmp en el define */
	qq = pp+np;
	ac = qq+nq;

	for (i=0; i<np; i++)
		pp[i]=0.0;
	for (i=0; i<nq; i++)
		qq[i]=0.0;
	/* inicializar con ceros triviales */
	if (p&1)  /* p impar, PUBLIC sin ceros, Q con ceros en z=1 y z=-1 */
		qq[1]=-1.0;
	else {  /* p par, PUBLIC con cero en z=-1, Q con cero en z=1 */
		pp[0]=1.0;
		qq[0]=-1.0;
	}

	for (i=0; i<np; i++) {   /* ceros de PUBLIC */
		lx2 = 2.0*vlx[i*2];
		ac[0]=-lx2;
		ac[1]=1.0;
		for (j=2; j<np; j++)
			ac[j] = pp[j-2];   /* z**2 */
		for (j=1; j<np; j++)
			ac[j] -= lx2*pp[j-1];   /* 2*lx*z */
		for (j=0; j<np; j++)
			pp[j]+=ac[j];
	}

	for (i=0; i<nq; i++) {   /* ceros de Q */
		lx2 = 2.0*vlx[i*2+1];
		ac[0]=-lx2;
		ac[1]=1.0;
		for (j=2; j<nq; j++)
			ac[j] = qq[j-2];   /* z**2 */
		for (j=1; j<nq; j++)
			ac[j] -= lx2*qq[j-1];   /* 2*lx*z */
		for (j=0; j<nq; j++)
			qq[j]+=ac[j];
	}

	for (i=0; i<nq; i++) {   /* reconstruye coeficientes LPC */
		vai[i] = LP_NEGSUM_NEG(0.5)*(pp[i]+qq[i]);
		vai[p-i-1] = LP_NEGSUM_NEG(0.5)*(pp[i]-qq[i]);
	}
	if (p&1)       /* si p impar */
		vai[nq] = LP_NEGSUM_NEG(0.5)*pp[nq];

	return vai;
#undef pp
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lp_lx2a( SPL_INT p )
{
	return p+(p+5)/2;
}

/**********************************************************/

