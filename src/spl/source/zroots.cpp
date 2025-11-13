/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1994 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ ZROOTS.CPP
Nombre paquete............... SPL
Lenguaje fuente.............. C++ (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.1.2    30/19/97  Borja     cambios en documentacion
1.1.1    30/07/95  Borja     scope funciones explicito
1.1.0    08/12/94  Borja     revision general (tipos,idx,nel,tnel...)
1.0.0    06/07/93  Borja     Codificacion inicial.

======================== Contenido ========================
Calculo de raices complejas de polinomios complejos (metodo de
Laguerre). Ver descripcion en la funcion laguerre().
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#ifndef __cplusplus
#error Must use C++ compiler
#endif

/**********************************************************/

#include <math.h>
#include "zroots.hpp"

#ifdef __CC_MSVC__
_STD_BEGIN
#endif

/**********************************************************/
/* Uso interno por Laguerre */

PRIVATE SPL_VOID Lag_InitAndTest( SPL_INT & Degree,
		SPL_pCOMPLEX Poly,
		SPL_INT & NumRoots,
		SPL_pCOMPLEX Roots,
		SPL_INT & InitDegree,
		SPL_pCOMPLEX InitPoly )
{
	NumRoots = 0;
	InitDegree = Degree;

	//memcpy(InitPoly,Poly,(Degree+1)*sizeof(SPL_COMPLEX));
	for (SPL_INT i=0; i<=Degree; i++)
		InitPoly[i] = Poly[i];

	// reduce el grado hasta que el primer coeficiente sea  cero
	while ((Degree>0)&&(abs(Poly[Degree])<ZROOTS_NEARLYZERO))
		Degree--;

	// Aplana el polinomio hasta que el termino independiente  cero
	//while ((abs(Poly[0])==0)&&(Degree>0)) {
	while ((abs(Poly[0])<ZROOTS_NEARLYZERO)&&(Degree>0)) {
		Roots[NumRoots++]=0;  //Zero is a root
		Degree--;
		for (SPL_INT Term=0; Term<=Degree; Term++)
			Poly[Term]=Poly[Term+1];
	}
}

/**********************************************************/
/* Uso interno por Laguerre */

PRIVATE SPL_VOID Lag_EvaluatePoly( SPL_INT Degree,
		SPL_pCOMPLEX Poly,
		SPL_COMPLEX X,
		SPL_COMPLEX & yValue,
		SPL_COMPLEX & yPrime,
		SPL_COMPLEX & yFLOATPrime )
{
	SPL_INT Loop;
	SPL_COMPLEX yDPdummy;
	SPL_COMPLEX Deriv;
	SPL_COMPLEX Deriv2;

	yDPdummy = Deriv = Deriv2 = Poly[Degree];
	for (Loop=Degree-1; Loop>=2; Loop--) {
		Deriv = Poly[Loop]+(Deriv*X);
		Deriv2 = Deriv+(Deriv2*X);
		yDPdummy = Deriv2+(yDPdummy*X);
	}
	yFLOATPrime = 2.*yDPdummy; // segunda derivada del polinomio en X
	if (Degree>1) {
		Deriv = Poly[1]+(Deriv*X);
		Deriv2 = Deriv+(Deriv2*X);
	}
	yPrime = Deriv2;  // primera derivada del polinomio en X
	if (Degree>0)
		Deriv = Poly[0]+(Deriv*X);
	yValue = Deriv;  // valor del polinomio en X
}


/* la siguiente funcion desconectada es como la anterior, pero basada en
la version original. Es mas lenta, y encima utiliza memoria dinamica */

/*
PRIVATE SPL_INT Lag_EvaluatePoly( SPL_INT Degree,
		SPL_pCOMPLEX Poly,
		SPL_COMPLEX X,
		SPL_COMPLEX & yValue,
		SPL_COMPLEX & yPrime,
		SPL_COMPLEX & yFLOATPrime )
{
	SPL_INT Loop;
	SPL_COMPLEX yDPdummy;
	SPL_pCOMPLEX Deriv;
	SPL_pCOMPLEX Deriv2;

	Deriv = new SPL_COMPLEX {Degree+1};
	Deriv2 = new SPL_COMPLEX {Degree+1};
	if ((!Deriv)||(!Deriv2))
		return ZROOTS_ERR_NOMEM;

	Deriv{Degree} = Poly{Degree};
	for (Loop=Degree-1; Loop>=0; Loop--)
		Deriv{Loop} = Poly{Loop}+(Deriv{Loop+1}*X);
	yValue = Deriv{0}; // valor en X

	Deriv2{Degree} = Deriv{Degree};
	for (Loop=Degree-1; Loop>=1; Loop--)
		Deriv2{Loop} = Deriv{Loop}+(X*Deriv2{Loop+1});
	yPrime = Deriv2{1};  // primera derivada en X

	yDPdummy = Deriv2{Degree};
	for (Loop=Degree-1; Loop>=2; Loop--)
		yDPdummy = Deriv2{Loop}+(yDPdummy*X);
	yFLOATPrime = 2*yDPdummy; // segunda derivada en X

	delete(Deriv2);
	delete(Deriv);
	return ZROOTS_ERR_NOERR;
}
*/

/**********************************************************/
/* Uso interno por Laguerre */

PRIVATE SPL_VOID Lag_ConstructDifference( SPL_INT Degree,
		SPL_COMPLEX yValue,
		SPL_COMPLEX yPrime,
		SPL_COMPLEX yFLOATPrime,
		SPL_COMPLEX & Dif )
{
	SPL_COMPLEX SRoot;
	SPL_COMPLEX Numer1, Numer;

	SRoot = sqrt( (SPL_FLOAT)(Degree-1)*( ((SPL_FLOAT)(Degree-1)*yPrime*yPrime) -
		((SPL_FLOAT)Degree*yValue*yFLOATPrime)) );
	Numer1 = yPrime+SRoot;
	Numer = yPrime-SRoot;
	if (abs(Numer1)>abs(Numer))
		Numer = Numer1;
	if (abs(Numer)<ZROOTS_NEARLYZERO)
		Dif = 0;
	else
		// la diferencia es el inverso de la fraccion
		Dif = (yValue*(SPL_FLOAT)Degree)/Numer;
}

/**********************************************************/
/* Uso interno por laguerre. Esta es la funcion que toma la decision
de convergencia */

PRIVATE SPL_INT Lag_TestForRoot( SPL_FLOAT X,
		SPL_FLOAT Dif,
		SPL_FLOAT Y,
		SPL_FLOAT Tol )
{
	return (fabs(Y)<=ZROOTS_NEARLYZERO)  // Y=0
		||(fabs(Dif)<fabs(X*Tol))  // cambio relativo en X
//		||(fabs(Dif)<Tol)  // Cambio absoluto en X. No usado
//		||(fabs(Y)<=Tol)  // Cambio absoluto en Y. No usado
	;
}

/**********************************************************/
/* Uso interno por Laguerre */

PRIVATE SPL_INT Lag_FindOneRoot( SPL_INT Degree,
		SPL_pCOMPLEX Poly,
		SPL_FLOAT Tol,
		SPL_INT MaxIter,
		SPL_COMPLEX & Root )  // valor inicial
{
	SPL_COMPLEX yValue, yPrime, yFLOATPrime, Dif;
	SPL_INT Iter=0;
	SPL_INT Found=0;

	Lag_EvaluatePoly(Degree,Poly,Root,yValue,yPrime,yFLOATPrime);
	while ((Iter<MaxIter)&&(!Found)) {
		Iter++;
		Lag_ConstructDifference(Degree,yValue,yPrime,
				yFLOATPrime,Dif);
		Root -= Dif;
		Lag_EvaluatePoly(Degree,Poly,Root,yValue,
				yPrime,yFLOATPrime);
		Found = Lag_TestForRoot(abs(Root),abs(Dif),
				abs(yValue),Tol);
	}
	//error si Iter>MaxIter
	return ((Found)?ZROOTS_ERR_NOERR:ZROOTS_ERR_ITEROUT);
}

/**********************************************************/
/* uso interno por Laguerre */

PRIVATE SPL_VOID Lag_ReducePoly( SPL_INT & Degree,
		SPL_pCOMPLEX Poly,
		SPL_COMPLEX Root )
{
	SPL_INT Term;
	SPL_COMPLEX Dummy1, Dummy2;

	Dummy1 = Poly[Degree-1];
	Poly[Degree-1] = Poly[Degree];
	for (Term=Degree-1; Term>=1; Term--) {
		Dummy2 = Poly[Term-1];
		Poly[Term-1] = Poly[Term]*Root+Dummy1;
		Dummy1 = Dummy2;
	}
	Degree--;
}

/* Esta es otra version, algo mas rapida (muy muy poco), pero
necesita memoria dinamica: */

/*
PRIVATE SPL_VOID Lag_ReducePoly( SPL_INT & Degree,
		SPL_pCOMPLEX Poly,
		SPL_COMPLEX Root )
{
	SPL_INT Term;
	SPL_pCOMPLEX NewPoly;

	NewPoly = new SPL_COMPLEX {Degree+1};

	NewPoly{Degree-1} = Poly{Degree};
	for (Term=Degree-1; Term>=1; Term--)
		NewPoly{Term-1} = Poly{Term}+NewPoly{Term}*Root;
	memcpy(Poly,NewPoly,sizeof(SPL_COMPLEX)*Degree);

	Degree--;
	delete NewPoly;
}
*/

/* <DOC> */
/**********************************************************/
/* Calculo de raices complejas por el metodo de Laguerre.
Dado un polinomio en z de orden n :

.            P(z) = z0 + z1*z^1 + z2*z^2 + ... + zn*z^n

calcula las raices (P(z)=0) complejas.

La funcion laguerre() recibe los siguientes argumentos:

- SPL_pCOMPLEX {vPoly} : Coeficientes z0 a zn del polinomio de grado n.
.                        vPoly[0]=z0, vPoly[1]=z1 ... vPoly[Degree]=zn,
.                        Son por tanto {Degree}+1 (=n+1) elementos de
.                        tipo complex.
.                        Aqui se devuelve el 'polinomio resto' (ver abajo).

- SPL_INT {Degree} : Grado n del polinomio.
.                    Aqui se devuelve el grado del 'polinomio resto'
.                    (ver abajo)

- SPL_pCOMPLEX {vRoots} : En este array la funcion mete las raices
.                         encontradas. Para un polinomio de grado n,
.                         como maximo hay n raices. Por tanto este array
.                         debe tener {Degree} elementos de tipo complex.
.                         IMPORTANTE: El usuario debe enviar una
.                         estimacion inicial de las raices, que se
.                         utilizaran como punto de partida en la
.                         busqueda (aunque sean valores 0). Pero
.												  SIEMPRE RECORDAR QUE SE DEBE INICIALIZAR
.												  CON ALGUN VALOR.

- SPL_INT {NumRoots} : La funcion devuelve el numero de raices encontradas
.                      en este entero.

- SPL_pCOMPLEX {Tmp} : Array temporal de elementos de tipo SPL_COMPLEX.
.                      Este array debe tener tnel_laguerre(Degree)
.                      elementos de tipo complex.

- SPL_INT {MaxIter} : Maximo numero de iteraciones que se le permite al
.                     algoritmo para encontrar una raiz. Si se supera este
.                     limite, el algoritmo se interrumpe y la funcion
.                     devuelve un codigo de error.
.                     Tipicamente este valor es ZROOTS_MAXITER (=300)

- SPL_FLOAT {Tol} : Tolerancia con la que se quieren obtener las raices.
.                    Tipicamente este valor es ZROOTS_TOL (=1e-6)

La funcion {devuelve} un codigo de error, que es cero si todo ha ido
bien. En caso de error, la funcion {devuelve} un numero negativo,
que puede ser:

- ZROOTS_ERR_ITEROUT (menor que cero) :
.     Si se ha superado el maximo numero de iteraciones permitidas.
- ZROOTS_ERR_NOMEM (menor que cero) :
.     Si no se ha podido reservar memoria dinamica suficiente. Por
.     ahora, este codigo no sucede nunca, pues no se reserva memoria
.     en ningun momento.

La funcion, que recibe en {Poly} los coeficientes del polinomio original,
devuelve en este mismo array el 'polinomio resto', resultante de
eliminar del polinomio todas las raices encontradas. Logicamente,
si se encuentran todas las raices, este polinomio sera de grado cero.
En {Degree} la funcion devuelve el grado final de este polinomio resto.

Generalmente la funcion encuentra todas las raices, excepto para
ordenes muy elevados, por ejemplo P(z)= z^800 - 1 no se le da
precisamente bien...

Ref:
.    La funcion laguerre() y sus funciones asociadas son traducciones
.    al C, con adaptacion y mejoras de rutinas pertenecientes al
.    paquete 'Borland Numerical Toolbox 4.0 for Turbo Pascal' de Borland.
.    Borland es marca registrada de Borland International Inc.
*/

PUBLIC SPL_INT XAPI laguerre( SPL_pCOMPLEX vPoly, SPL_INT & Degree,
		SPL_pCOMPLEX vRoots, SPL_INT & NumRoots,
		SPL_pCOMPLEX Tmp,
		SPL_INT MaxIter, SPL_FLOAT Tol )
/* </DOC> */
{
	SPL_INT InitDeg;
	SPL_INT Error = ZROOTS_ERR_NOERR;

	Lag_InitAndTest(Degree,vPoly,NumRoots,vRoots,InitDeg,Tmp);
	while ((Degree>0)&&(Error==ZROOTS_ERR_NOERR)) {
		if (!(Error=Lag_FindOneRoot(Degree,vPoly,Tol,
				MaxIter,vRoots[NumRoots]))) {
			if (!(Error=Lag_FindOneRoot(InitDeg,Tmp,
					Tol,MaxIter,vRoots[NumRoots]))) {
				// Reduce el polinomio
				Lag_ReducePoly(Degree,vPoly,
						vRoots[NumRoots]);
				NumRoots++;
			}
		}
	}
	return Error;
}

/* <DOC> */
/**********************************************************/
/* {devuelve} el numero de elementos de tipo SPL_COMPLEX que debe
tener el array temporal {Tmp} que recibe la funcion laguerre,
si el polinomio a analizar es de orden {Degree} */

PUBLIC SPL_INT XAPI tnel_laguerre( SPL_INT Degree )
/* </DOC> */
{
	return Degree+1;
}

/**********************************************************/

#ifdef __CC_MSVC__
_STD_END
#endif
