/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ SPL5b.C
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
/* Metodo Split-Levinson simetrico para las correlaciones. Fast!
El mas rapido para el metodo de las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes de reflexion {vki} y los coeficientes
lpc {vai} para analisis de orden {p}.
La funcion {devuelve} la potencia del error de prediccion E=G**2 */

PUBLIC SPL_FLOAT XAPI lpa_cor_sls_ka( SPL_pFLOAT vac, SPL_pFLOAT vki,
		SPL_pFLOAT vai, SPL_INT p, SPL_pFLOAT vtmp )
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

	vki[0]=LP_NEGSUM_NEG((lk=2.0-(ak=(tk=vac[0]+vac[1])/vac[0]))-1.0);
	pa[0]=lk-ak;
	t=0;

	for (k=1; k<p; k++) {
		__xsum(i,0,t,(vac[i+1]+vac[k-i])*pa[i],sum,vac[0]+vac[k+1]);
		if (k&1) {
			sum+=vac[t+1]*pa[t];
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}
		else {
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			pn[t]=pa[t-1]+pa[t-1]-ak*po[t-1];
		}
		vki[k]=LP_NEGSUM_NEG((lk=2.0-ak/lk)-1.0);
		px=po;
		po=pa;
		pa=pn;
		pn=px;
	}

	vai[0]=LP_NEGSUM_NEG(1.0+pa[0]-lk);
	if (p&1) /* si el orden es impar */
		t++;
	else
		po[t-1]*=lk;
	for (i=1; i<t; i++)
		vai[i]=vai[i-1]+LP_NEGSUM_NEG(pa[i])+LP_POSSUM_NEG(po[i-1]*=lk);
	p--;
	for (i=t; i<p; i++)
		vai[i]=vai[i-1]+LP_NEGSUM_NEG(pa[p-i])+LP_POSSUM_NEG(po[p-i]);
	vai[p]=vki[p];

	return lk*tk;
#undef po
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cor_sls( SPL_INT p )
{
	return 3*((p+1)/2);
}

/**********************************************************/
/* Metodo Split-Levinson simetrico para las correlaciones. Fast!
El mas rapido para el metodo de las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes de reflexion {vki} para analisis
de orden {p}.
La funcion {devuelve} la potencia del error de prediccion */

PUBLIC SPL_FLOAT XAPI lpa_cor_sls_k( SPL_pFLOAT vac, SPL_pFLOAT vki,
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

	vki[0]=LP_NEGSUM_NEG((lk=2.0-(ak=(tk=vac[0]+vac[1])/vac[0]))-1.0);
	pa[0]=lk-ak;
	t=0;

	for (k=1; k<p; k++) {
		__xsum(i,0,t,(vac[i+1]+vac[k-i])*pa[i],sum,vac[0]+vac[k+1]);
		if (k&1) {
			sum+=vac[t+1]*pa[t];
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}
		else {
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			pn[t]=pa[t-1]+pa[t-1]-ak*po[t-1];
		}
		vki[k]=LP_NEGSUM_NEG((lk=2.0-ak/lk)-1.0);
		px=po;
		po=pa;
		pa=pn;
		pn=px;
	}

	return lk*tk;
#undef po
}

/**********************************************************/
/* Metodo Split-Levinson simetrico para las correlaciones. Fast!
El mas rapido para el metodo de las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes lpc {vai} para analisis de orden {p}.
La funcion {devuelve} la potencia del error de prediccion */

PUBLIC SPL_FLOAT XAPI lpa_cor_sls_a( SPL_pFLOAT vac, SPL_pFLOAT vai,
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

	pa[0]=2.0-2.0*((tk=vac[0]+vac[1])/vac[0]);
	t=0;

	for (k=1; k<p; k++) {
		__xsum(i,0,t,(vac[i+1]+vac[k-i])*pa[i],sum,vac[0]+vac[k+1]);
		if (k&1) {
			sum+=vac[t+1]*pa[t];
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}
		else {
			ak=sum/tk;
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
		t++;
	}
	else {
		for (i=0; i<t-1; i++) {
			sum+=po[i];
			lk+=pa[i];
		}
		lk=(2.0*(lk+pa[i]))/(sum+sum+po[i]);
		po[t-1]*=lk;
	}
	vai[0]=LP_NEGSUM_NEG(1.0+pa[0]-lk);
	for (i=1; i<t; i++)
		vai[i]=vai[i-1]+LP_NEGSUM_NEG(pa[i])+LP_POSSUM_NEG((po[i-1]*=lk));
	p--;
	for (i=t; i<p; i++)
		vai[i]=vai[i-1]+LP_NEGSUM_NEG(pa[p-i])+LP_POSSUM_NEG(po[p-i]);
	vai[p]=LP_NEGSUM_NEG(lk-1.0);

	return lk*tk;
}

/**********************************************************/
/* Metodo Split-Levinson simetrico para las correlaciones. Fast!
El mas rapido para el metodo de las correlaciones.
A partir de las {p}+1 primeras autocorrelaciones {vac} la
funcion {devuelve} la potencia del error de prediccion para
analisis de orden {p} */

PUBLIC SPL_FLOAT XAPI lpa_cor_sls_e( SPL_pFLOAT vac, SPL_INT p,
		SPL_pFLOAT vtmp )
{
	SPL_INT i,k,t;
#define po vtmp
	SPL_pFLOAT pa, pn, px; /* tres vectores, old,actual,new, y uno auxiliar */
	SPL_FLOAT sum,tk,ak,lk;
	assert(p>0);

	t = (p+1)/2;
	/*po=vtmp en el define */
	pa=po+t;
	pn=pa+t;

	pa[0]=2.0-2.0*((tk=vac[0]+vac[1])/vac[0]);
	t=0;

	for (k=1; k<p; k++) {
		__xsum(i,0,t,(vac[i+1]+vac[k-i])*pa[i],sum,vac[0]+vac[k+1]);
		if (k&1) {
			sum+=vac[t+1]*pa[t];
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}
		else {
			ak=sum/tk;
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
/* Metodo Split-Levinson antisimetrico para las correlaciones. Fast!
El tan rapido como el simetrico.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes de reflexion {vki} y los coeficientes
lpc {vai} para analisis de orden {p}.
La funcion {devuelve} la potencia del error de prediccion */

PUBLIC SPL_FLOAT XAPI lpa_cor_sla_ka( SPL_pFLOAT vac, SPL_pFLOAT vki,
		SPL_pFLOAT vai, SPL_INT p, SPL_pFLOAT vtmp )
{
	SPL_INT i,k,t;
#define po vtmp
	SPL_pFLOAT pa, pn, px; /* tres vectores, old,actual,new, y uno auxiliar */
	SPL_FLOAT sum,tk,ak,lk;
	assert(p>0);

	t = p/2;
	/*po=vtmp en el define */
	pa=po+t;
	pn=pa+t;

	vki[0]=LP_NEGSUM_NEG(1.0-(lk=2.0-(ak=(tk=vac[0]-vac[1])/vac[0])));
	pa[0]=0.0;
	t=0;

	for (k=1; k<p; k++) {
		if (k&1) {
			__xsum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			if (t)
				pn[t]=pa[t-1]-ak*po[t-1];
		}
		else {
			__sum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}

		vki[k]=LP_NEGSUM_NEG(1.0-(lk=2.0-ak/lk));
		px=po;
		po=pa;
		pa=pn;
		pn=px;
	}

	vai[0]=LP_NEGSUM_NEG(1.0+pa[0]-lk);
	p--;
	if (p&1) {
		for (i=1; i<=t; i++)
			vai[i]=vai[i-1]+LP_NEGSUM_NEG(pa[i])+LP_POSSUM_NEG(po[i-1]*=lk);
		vai[t+1]=vai[t]+LP_POSSUM_NEG(pa[t]);
		for (i=t+2; i<p; i++)
			vai[i]=vai[i-1]+LP_POSSUM_NEG(pa[p-i])+LP_NEGSUM_NEG(po[p-i]);
	}
	else if (t) {
		for (i=1; i<t; i++)
			vai[i]=vai[i-1]+LP_NEGSUM_NEG(pa[i])+LP_POSSUM_NEG(po[i-1]*=lk);
		vai[t]=vai[t-1]+LP_POSSUM_NEG(po[t-1]*=lk);
		for (i=t+1; i<p; i++)
			vai[i]=vai[i-1]+LP_POSSUM_NEG(pa[p-i])+LP_NEGSUM_NEG(po[p-i]);
	}
	vai[p]=vki[p];

	return lk*tk;
#undef po
}

/**********************************************************/

PUBLIC SPL_INT XAPI tnel_lpa_cor_sla( SPL_INT p )
{
	return 3*(p/2);
}

/**********************************************************/
/* Metodo Split-Levinson antisimetrico para las correlaciones. Fast!
Es tan rapido como el simetrico.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes de reflexion {vki} para analisis
de orden {p}.
La funcion {devuelve} la potencia del error de prediccion */

PUBLIC SPL_FLOAT XAPI lpa_cor_sla_k( SPL_pFLOAT vac, SPL_pFLOAT vki,
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

	vki[0]=LP_NEGSUM_NEG(1.0-(lk=2.0-(ak=(tk=vac[0]-vac[1])/vac[0])));
	pa[0]=0.0;
	t=0;

	for (k=1; k<p; k++) {
		if (k&1) {
			__xsum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			if (t)
				pn[t]=pa[t-1]-ak*po[t-1];
		}
		else {
			__sum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<=t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			t++;
		}

		vki[k]=LP_NEGSUM_NEG(1.0-(lk=2.0-ak/lk));
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
Es tan rapido como el simetrico.
A partir de las {p}+1 primeras autocorrelaciones {vac}
calcula los coeficientes lpc {vai} para analisis de orden {p}.
La funcion {devuelve} la potencia del error de prediccion */

PUBLIC SPL_FLOAT XAPI lpa_cor_sla_a( SPL_pFLOAT vac, SPL_pFLOAT vai,
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

	lk=2.0-(ak=(tk=vac[0]-vac[1])/vac[0]);
	pa[0]=0.0;
	t=0;

	for (k=1; k<p; k++) {
		if (k&1) {
			__xsum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			if (t)
				pn[t]=pa[t-1]-ak*po[t-1];
		}
		else {
			__sum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			ak=sum/tk;
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

	vai[0]=LP_NEGSUM_NEG(1.0+pa[0]-lk);
	p--;
	if (p&1) {
		for (i=1; i<=t; i++)
			vai[i]=vai[i-1]+LP_NEGSUM_NEG(pa[i])+LP_POSSUM_NEG(po[i-1]*=lk);
		vai[t+1]=vai[t]+LP_POSSUM_NEG(pa[t]);
		for (i=t+2; i<=p; i++)
			vai[i]=vai[i-1]+LP_POSSUM_NEG(pa[p-i])+LP_NEGSUM_NEG(po[p-i]);
	}
	else if (t) {
		for (i=1; i<t; i++)
			vai[i]=vai[i-1]+LP_NEGSUM_NEG(pa[i])+LP_POSSUM_NEG(po[i-1]*=lk);
		vai[t]=vai[t-1]+LP_POSSUM_NEG(po[t-1]*=lk);
		for (i=t+1; i<=p; i++)
			vai[i]=vai[i-1]+LP_POSSUM_NEG(pa[p-i])+LP_NEGSUM_NEG(po[p-i]);
	}

	return lk*tk;
#undef po
}

/**********************************************************/
/* Metodo Split-Levinson antisimetrico para las correlaciones. Fast!
El tan rapido como el simetrico.
A partir de las {p}+1 primeras autocorrelaciones {vac} la
funcion {devuelve} la potencia del error de prediccion para
analisis de orden {p} */

PUBLIC SPL_FLOAT XAPI lpa_cor_sla_e( SPL_pFLOAT vac, SPL_INT p,
		SPL_pFLOAT vtmp )
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

	lk=2.0-(ak=(tk=vac[0]-vac[1])/vac[0]);
	pa[0]=0.0;
	t=0;

	for (k=1; k<p; k++) {
		if (k&1) {
			__xsum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			ak=sum/tk;
			tk=sum;
			pn[0]=pa[0]+1.0-ak;
			for (i=1; i<t; i++)
				pn[i]=pa[i]+pa[i-1]-ak*po[i-1];
			if (t)
				pn[t]=pa[t-1]-ak*po[t-1];
		}
		else {
			__sum(i,0,t,(vac[i+1]-vac[k-i])*pa[i],sum,vac[0]-vac[k+1]);
			ak=sum/tk;
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

