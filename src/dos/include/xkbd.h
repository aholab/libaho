#ifndef __XKBD_H__
#define __XKBD_H__

/**********************************************************/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
Copyright: 1995 - Grupo de Voz (DAET) ETSII/IT-Bilbao

Nombre fuente................ XKBD.H
Nombre paquete............... -
Lenguaje fuente.............. C (Borland C/C++ 3.1)
Estado....................... Completado
Dependencia Hard/OS.......... -
Codigo condicional........... -

Codificacion................. Borja Etxebarria

Version  dd/mm/aa  Autor     Proposito de la edicion
-------  --------  --------  -----------------------
1.0.0    28/06/98  Borja     repasillo gordo
0.0.2    14/06/98  Borja     repasillo general
0.0.1    22/05/96  Borja     include xkeydef.h
0.0.0    23/11/95  Borja     Codificacion inicial.

======================== Contenido ========================
Gestion de teclado.
===========================================================
*/
/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**********************************************************/

#include "tdef.h"

/**********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/

BOOL xkbd_kbhit( VOID );
UINT16 xkbd_getch( VOID );
UINT16 xkbd_getstat( VOID );
UINT16 xkbd_getchstat( VOID );

UINT16 xkbd_toupper( UINT16 code );
UINT16 xkbd_tolower( UINT16 code );

/**********************************************************/

#ifdef __cplusplus
}  /* extern "C" */
#endif

/**********************************************************/

/*<DOC>*/
/* Bits para comprobar estado de valores leidos con xkbd_getch(),
xkbd_getstat(), y xkbd_getchstat() */
#define XKBD_FUNC ((UINT16)0x8000)
#define xkbd_isfunc(n)  ((n)&XKBD_FUNC)
#define xkbd_isch(n)  (!xkbd_isfunc(n))

#define XKBD_SHIFT ((UINT16)0x1000)
#define XKBD_CTRL ((UINT16)0x2000)
#define XKBD_ALT ((UINT16)0x4000)
#define XKBD_STATBITS (XKBD_SHIFT|XKBD_CTRL|XKBD_ALT)

#define xkbd_testshift(n)  ((n) & XKBD_SHIFT)
#define xkbd_testctrl(n)   ((n) & XKBD_CTRL)
#define xkbd_testalt(n)    ((n) & XKBD_ALT)
#define xkbd_setshift(n)   ((n) | XKBD_SHIFT)
#define xkbd_setctrl(n)    ((n) | XKBD_CTRL)
#define xkbd_setalt(n)     ((n) | XKBD_ALT)
#define xkbd_resetshift(n) ((n) & ~XKBD_SHIFT)
#define xkbd_resetctrl(n)  ((n) & ~XKBD_CTRL)
#define xkbd_resetalt(n)   ((n) & ~XKBD_ALT)

#define xkbd_resetstat(k)  ((k) & ~XKBD_STATBITS)
#define xkbd_peekstat(k)   ((k) & XKBD_STATBITS)

/**********************************************************/

#define __FK XKBD_FUNC

/* Valores que {devuelve} xkbd_getch() */
enum KeyType {
	K_None = 0x00,
	K_Esc = 0x1B,
	K_BackSpace = 0x08,
	K_Tab = 0x09,
	K_Enter = 0x0D,
	K_Space = 0x20,
	K_F1 = __FK | 0x3B,
	K_F2 = __FK | 0x3C,
	K_F3 = __FK | 0x3D,
	K_F4 = __FK | 0x3E,
	K_F5 = __FK | 0x3F,
	K_F6 = __FK | 0x40,
	K_F7 = __FK | 0x41,
	K_F8 = __FK | 0x42,
	K_F9 = __FK | 0x43,
	K_F10 = __FK | 0x44,
	K_F11 = __FK | 0x85,
	K_F12 = __FK | 0x86,
	K_Home = __FK | 0x47,
	K_UpArrow = __FK | 0x48,
	K_PgUp = __FK | 0x49,
	K_LeftArrow = __FK | 0x4B,
	K_KPCenter = __FK | 0x4C,
	K_RightArrow = __FK | 0x4D,
	K_End = __FK | 0x4F,
	K_DownArrow = __FK | 0x50,
	K_PgDn = __FK | 0x51,
	K_Ins = __FK | 0x52,
	K_Del = __FK | 0x53,
	K_ShiftTab = __FK | 0x0F,

	K_ShiftF1 = __FK | 0x54,
	K_ShiftF2 = __FK | 0x55,
	K_ShiftF3 = __FK | 0x56,
	K_ShiftF4 = __FK | 0x57,
	K_ShiftF5 = __FK | 0x58,
	K_ShiftF6 = __FK | 0x59,
	K_ShiftF7 = __FK | 0x5A,
	K_ShiftF8 = __FK | 0x5B,
	K_ShiftF9 = __FK | 0x5C,
	K_ShiftF10 = __FK | 0x5D,
	K_ShiftF11 = __FK | 0x87,
	K_ShiftF12 = __FK | 0x88,

	K_Ctrl2 = __FK | 0x03,
	K_Ctrl6 = 0x1E,
	K_CtrlHypen = 0x1F,
	K_CtrlBackSpace = 0x7F,
	K_CtrlTab = __FK | 0x94,
	K_CtrlQ = 0x11,
	K_CtrlW = 0x17,
	K_CtrlE = 0x05,
	K_CtrlR = 0x12,
	K_CtrlT = 0x14,
	K_CtrlY = 0x19,
	K_CtrlU = 0x15,
	K_CtrlI = K_Tab,
	K_CtrlO = 0x0F,
	K_CtrlP = 0x10,
	K_CtrlOpenSqrBracket = K_Esc,
	K_CtrlCloseSqrBracket = 0x1D,
	K_CtrlEnter = 0x0A,
	K_CtrlA = 0x01,
	K_CtrlS = 0x13,
	K_CtrlD = 0x04,
	K_CtrlF = 0x06,
	K_CtrlG = 0x07,
	K_CtrlH = K_BackSpace,
	K_CtrlJ = K_CtrlEnter,
	K_CtrlK = 0x0B,
	K_CtrlL = 0x0C,
	K_CtrlBackSlash = 0x1C,
	K_CtrlZ = 0x1A,
	K_CtrlX = 0x18,
	K_CtrlC = 0x03,
	K_CtrlV = 0x16,
	K_CtrlB = 0x02,
	K_CtrlN = 0x0E,
	K_CtrlM = K_Enter,
	K_CtrlGrayMul = __FK | 0x96,
	K_CtrlF1 = __FK | 0x5E,
	K_CtrlF2 = __FK | 0x5F,
	K_CtrlF3 = __FK | 0x60,
	K_CtrlF4 = __FK | 0x61,
	K_CtrlF5 = __FK | 0x62,
	K_CtrlF6 = __FK | 0x63,
	K_CtrlF7 = __FK | 0x64,
	K_CtrlF8 = __FK | 0x65,
	K_CtrlF9 = __FK | 0x66,
	K_CtrlF10 = __FK | 0x67,
	K_CtrlF11 = __FK | 0x89,
	K_CtrlF12 = __FK | 0x8A,
	K_CtrlHome = __FK | 0x77,
	K_CtrlUpArrow = __FK | 0x8D,
	K_CtrlPgUp = __FK | 0x84,
	K_CtrlGrayMinus = __FK | 0x8E,
	K_CtrlLeftArrow = __FK | 0x73,
	K_CtrlKPCenter = __FK | 0x8F,
	K_CtrlRightArrow = __FK | 0x74,
	K_CtrlGrayPlus = __FK | 0x90,
	K_CtrlEnd = __FK | 0x75,
	K_CtrlDownArrow = __FK | 0x91,
	K_CtrlPgDn = __FK | 0x76,
	K_CtrlIns = __FK | 0x92,
	K_CtrlDel = __FK | 0x93,
	K_CtrlGrayDiv = __FK | 0x95,
	K_CtrlPrtScr = K_CtrlP,
	K_CtrlBreak = K_CtrlC,

	K_AltEsc = __FK | 0x01,
	K_Alt1 = __FK | 0x78,
	K_Alt2 = __FK | 0x79,
	K_Alt3 = __FK | 0x7A,
	K_Alt4 = __FK | 0x7B,
	K_Alt5 = __FK | 0x7C,
	K_Alt6 = __FK | 0x7D,
	K_Alt7 = __FK | 0x7E,
	K_Alt8 = __FK | 0x7F,
	K_Alt9 = __FK | 0x80,
	K_Alt0 = __FK | 0x81,
	K_AltHypen = __FK | 0x82,
	K_AltEqual = __FK | 0x83,
	K_AltBackSpace = __FK | 0x0E,
	K_AltTab = __FK | 0xA5,
	K_AltQ = __FK | 0x10,
	K_AltW = __FK | 0x11,
	K_AltE = __FK | 0x12,
	K_AltR = __FK | 0x13,
	K_AltT = __FK | 0x14,
	K_AltY = __FK | 0x15,
	K_AltU = __FK | 0x16,
	K_AltI = __FK | 0x17,
	K_AltO = __FK | 0x18,
	K_AltP = __FK | 0x19,
	K_AltOpenSqrBracket = __FK | 0x1A,
	K_AltCloseSqrBracket = __FK | 0x1B,
	K_AltEnter = __FK | 0x1C,
	K_AltA = __FK | 0x1E,
	K_AltS = __FK | 0x1F,
	K_AltD = __FK | 0x20,
	K_AltF = __FK | 0x21,
	K_AltG = __FK | 0x22,
	K_AltH = __FK | 0x23,
	K_AltJ = __FK | 0x24,
	K_AltK = __FK | 0x25,
	K_AltL = __FK | 0x26,
	K_AltSemiColon = __FK | 0x27,
	K_AltCloseCuotMark = __FK | 0x28,
	K_AltOpenCuotMark = __FK | 0x29,
	K_AltBackSlash = __FK | 0x2B,
	K_AltZ = __FK | 0x2C,
	K_AltX = __FK | 0x2D,
	K_AltC = __FK | 0x2E,
	K_AltV = __FK | 0x2F,
	K_AltB = __FK | 0x30,
	K_AltN = __FK | 0x31,
	K_AltM = __FK | 0x32,
	K_AltComma = __FK | 0x33,
	K_AltPoint = __FK | 0x34,
	K_AltDiv = __FK | 0x35,
	K_AltGrayMul = __FK | 0x37,
	K_AltF1 = __FK | 0x68,
	K_AltF2 = __FK | 0x69,
	K_AltF3 = __FK | 0x6A,
	K_AltF4 = __FK | 0x6B,
	K_AltF5 = __FK | 0x6C,
	K_AltF6 = __FK | 0x6D,
	K_AltF7 = __FK | 0x6E,
	K_AltF8 = __FK | 0x6F,
	K_AltF9 = __FK | 0x70,
	K_AltF10 = __FK | 0x71,
	K_AltF11 = __FK | 0x8B,
	K_AltF12 = __FK | 0x8C,
	K_AltGrayMinus = __FK | 0x4A,
	K_AltGrayPlus = __FK | 0x4E,
	K_AltGrayDiv = __FK | 0xA4,
	K_AltHome = __FK | 0x97,
	K_AltUpArrow = __FK | 0x98,
	K_AltPgUp = __FK | 0x99,
	K_AltLeftArrow = __FK | 0x9B,
	K_AltRightArrow = __FK | 0x9D,
	K_AltEnd = __FK | 0x9F,
	K_AltDownArrow = __FK | 0xA0,
	K_AltPgDn = __FK | 0xA1,
	K_AltIns = __FK | 0xA2,
	K_AltDel = __FK | 0xA3,
	K_AltGrayEnter = __FK | 0xA6,
};
/*</DOC>*/

#undef __FK

/**********************************************************/

#endif
