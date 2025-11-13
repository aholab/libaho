#ifndef __INTRS_H
#define __INTRS_H

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1993 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ INTRS.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... Irqs, intrs
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.3    21/04/00  Borja     INTERRUPT para djgpp
1.0.2    18/02/96  Borja     anyadir #include "tdef.h"
1.0.1    26/07/94  Borja     Soporte C y C++ para protot. rutina servicio.
1.0.0    19/02/94  Borja     Codificacion inicial.

======================== Contenido ========================
Gestion de interrupciones y controlador de interrupciones.
....................
Interrupt controller management macros.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include <dos.h>   /* para enable(), disable()... */

#include "tdef.h"
#include "ports.h"

/**********************************************************/
/* keyword que identifica a una funcion como interrupcion */

#ifdef __OS_MSDOS32__
#define INTERRUPT
#else
#define INTERRUPT interrupt
#endif

/* tipo de las funciones de servicio de interrupcion */
#ifdef __cplusplus
typedef VOID INTERRUPT (*IntrServiceFunc) ( ... );
#else
typedef VOID INTERRUPT (*IntrServiceFunc) ( void );
#endif

/* habilitar y deshabilitar interrupciones.
....................
enable and disable interrupts */
#define INT_ENABLE() _enable()
#define INT_DISABLE() _disable()

/* {devuelve} (void interrupt far *) un puntero a la rutina de
interrupcion que sirve la interrupcion numero {intno} (UINT16)
....................
{returns}  (void interrupt far *) a pointer to the interrupt
service function for interrupt number {intno} (UINT16) */
#define INT_GET_VEC(intno)  (IntrServiceFunc)_dos_getvect(intno)

/* pone la funcion {intfunc} (void interrupt far *) como rutina de
servicio de la interrupcion numero {intno} (UINT16)
....................
install the function {intfunc} (void interrupt far *) as an
interrupt service function for interrupt number {intno} (UINT16) */
#define INT_SET_VEC(intno,intfunc)  \
		_dos_setvect(intno,(IntrServiceFunc)(intfunc))

/**********************************************************/
/* {devuelve} en numero de interrupcion para la linea IRQ {irqn}
{irqn} va de 0 a 15
....................
{returns} the interrupt number for IRQ number {irqn}.
0 <= {irqn} <=15 */
#define IRQ_INTN(irqn) ((irqn) + ((irqn < 8) ? 0x08 : 0x68))

/* registros de programacion de los 2 PICs */
#define PIC1_REG0 0x20
#define PIC1_REG1 0x21
#define PIC2_REG0 0xA0
#define PIC2_REG1 0xA1

#define PIC_EOI 0x20
#define PIC_NOP 0x0B

/* envia fin de interrupcion al PIC, para la linea  {irqn} (0 a 15)
....................
Sends an EOI (end of interrupt) command to the PIC chip for
IRQ interrupt number {irqn} (0 to 15) */
#define PIC_SEND_EOI(irqn) {  \
	if (irqn >= 8) {  \
                OUTPORT8(PIC2_REG0,PIC_EOI);  \
                OUTPORT8(PIC2_REG0,PIC_NOP);  \
                if (! INPORT8(PIC2_REG0))  \
                        OUTPORT8(PIC1_REG0,PIC_EOI);  \
	}  \
	else  \
                OUTPORT8(PIC1_REG0,PIC_EOI);  \
}

/* conecta la linea IRQ numero {irqn} del PIC
....................
enables (unmask) IRQ line {irqn}  (0-15) */
#define PIC_ENABLE(irqn) {  \
	if (irqn<8) { \
		INT_DISABLE(); \
                OUTPORT8(PIC1_REG1, INPORT8(PIC1_REG1) & (~(0x01 << (irqn))));  \
		INT_ENABLE(); \
	} \
	else { \
		INT_DISABLE(); \
                OUTPORT8(PIC2_REG1, INPORT8(PIC2_REG1) & (~(0x01 << ((irqn) - 8))));  \
		INT_ENABLE(); \
	} \
}

/* desconecta la linea IRQ numero {irqn} (0-15) del PIC
....................
disables (masks) IRQ line  {irqn} (0-15) */
#define PIC_DISABLE(irqn) {  \
	if (irqn<8) { \
		INT_DISABLE(); \
                OUTPORT8(PIC1_REG1, INPORT8(PIC1_REG1) | (0x01 << (irqn))); \
		INT_ENABLE(); \
	} \
	else { \
		INT_DISABLE(); \
                OUTPORT8(PIC2_REG1, INPORT8(PIC2_REG1) | (0x01 << ((irqn) - 8))); \
		INT_ENABLE(); \
	} \
}

/**********************************************************/

#endif
