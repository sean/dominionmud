/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SCREEN.H                                     Based on CircleMUD *
*  Usage: ANSI Color Codes & Escape Codes for cursor control.             *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#ifndef __SCREEN_H__
#define __SCREEN_H__


#define KNRM  "\x1B[0m"
#define KXXX  "\x1B[0;30m"    /* BLACK */
#define KRED  "\x1B[0;31m"
#define KBRD  "\x1B[1;31m"
#define KGRN  "\x1B[0;32m"
#define KBGR  "\x1B[1;32m"
#define KGLD  "\x1B[0;33m"
#define KYEL  "\x1B[1;33m"
#define KBLU  "\x1B[0;34m"
#define KBBL  "\x1B[1;34m"
#define KMAG  "\x1B[0;35m"
#define KBMG  "\x1B[1;35m"
#define KCYN  "\x1B[0;36m"
#define KBCN  "\x1B[1;36m"
#define KWHT  "\x1B[0;37m"
#define KGRY  "\x1B[1;30m"
#define KBLD  "\x1B[1m"
#define KFSH  "\x1B[5m"
#define KBOW  "\x1B[7m"
#define KNUL  ""

/* New screen stuff from DaleMUD - based on SillyMUD */
#define CLEAR_CURSOR_TO_END    "\033[0K" /* Clear from cursor to end of line */
#define CLEAR_BEGIN_TO_CURSOR  "\033[1K" /* Clear from begin of line to cursor */
#define CLEAR_LINE             "\033[2K" /* Clear line containing cursor */
#define CLEAR_FROM_CURSOR      "\033[0J" /* Clear screen from cursor */
#define CLEAR_TO_CURSOR        "\033[1J" /* Clear screen to cursor */
#define CLEAR_SCREEN           "\033[2J" /* Clear entire screen */
#define CLEAR_RESET            "\033[2J\033[0;0H"  /* Clear screen/Reset Cursor */
// #define CLEAR_SCREEN	       "\033[H\033[J"

#define CURSOR_POS_SAVE	       "\e7"
#define CURSOR_POS_RESTORE     "\e8" 

#define CURSOR_UP(number)            "\033[(number)A"
#define CURSOR_DOWN(number)          "\033[(number)B"
#define CURSOR_FORWARD(number)       "\033[(number)C"
#define CURSOR_BACKWARD(number)      "\033[(number)D"
#define CURSOR_POSITION(row, column) "\033[row;columnH"
#define RESET_CUSOR                  "\033[0;0H"

#define MOD_NORMAL                   "\033[0m"   /* %N */
#define MOD_BOLD                     "\033[1m"   /* %B */
#define MOD_FAINT                    "\033[2m"   /* %F */
#define MOD_UNDERLINE                "\033[4m"   /* %U */
#define MOD_BLINK                    "\033[5m"   /* %I */
#define MOD_UNKNOWN                  "\033[6m"   /* %T */
#define MOD_REVERSE                  "\033[7m"   /* %R */

#define FG_BLACK      "\033[0;30m"   /* %0 */
#define FG_RED        "\033[0;31m"   /* %1 */
#define FG_GREEN      "\033[0;32m"   /* %2 */
#define FG_BROWN      "\033[0;33m"   /* %3 */
#define FG_BLUE       "\033[0;34m"   /* %4 */
#define FG_MAGENTA    "\033[0;35m"   /* %5 */
#define FG_CYAN       "\033[0;36m"   /* %6 */
#define FG_LT_GRAY    "\033[0;37m"   /* %7 */
/* Back grounds */
#define BK_BLACK      "\033[0;40m"   /* %8 */
#define BK_RED        "\033[0;41m"   /* %9 */
#define BK_GREEN      "\033[0;42m"   /* %z */
#define BK_BROWN      "\033[0;43m"   /* %y */
#define BK_BLUE       "\033[0;44m"   /* %x */
#define BK_MAGENTA    "\033[0;45m"   /* %w */
#define BK_CYAN       "\033[0;46m"   /* %v */
#define BK_LT_GRAY    "\033[0;47m"   /* %u */

#define WINDOW(top, bottom) printf("\033[%d;%dr%s", top, bottom, CLEAR_SCREEN)

#define VT_INITSEQ    "\033[1;24r"
#define VT_CURSPOS    "\033[%d;%dH"
#define VT_CURSRIG    "\033[%dC"
#define VT_CURSLEF    "\033[%dD"
#define VT_HOMECLR    "\033[2J\033[0;0H"
#define VT_CTEOTCR    "\033[K"
#define VT_CLENSEQ    "\033[r\033[2J"
#define VT_INDUPSC    "\033M"
#define VT_INDDOSC    "\033D"
#define VT_SETSCRL    "\033[%d;24r"
#define VT_INVERTT    "\033[0;1;7m"
#define VT_BOLDTEX    "\033[0;1m"
#define VT_NORMALT    "\033[0m"
#define VT_MARGSET    "\033[%d;%dr"
#define VT_CURSAVE    "\0337"
#define VT_CURREST    "\0338"

/* conditional color.  pass it a pointer to a char_data and a color level. */
#define C_OFF	0
#define C_SPR	1
#define C_NRM	2
#define C_CMP	3
#define _clrlevel(ch) ((PRF_FLAGGED((ch), PRF_COLOR_1) ? 1 : 0) + \
		       (PRF_FLAGGED((ch), PRF_COLOR_2) ? 2 : 0))
#define clr(ch,lvl) (_clrlevel(ch) >= (lvl))
#define CCNRM(ch,lvl)  (clr((ch),(lvl))?KNRM:KNUL)
#define CCRED(ch,lvl)  (clr((ch),(lvl))?KRED:KNUL)
#define CCBRD(ch,lvl)  (clr((ch),(lvl))?KBRD:KNUL)
#define CCGRN(ch,lvl)  (clr((ch),(lvl))?KGRN:KNUL)
#define CCBGR(ch,lvl)  (clr((ch),(lvl))?KBGR:KNUL)
#define CCYEL(ch,lvl)  (clr((ch),(lvl))?KYEL:KNUL)
#define CCBLU(ch,lvl)  (clr((ch),(lvl))?KBLU:KNUL)
#define CCBBL(ch,lvl)  (clr((ch),(lvl))?KBBL:KNUL)
#define CCMAG(ch,lvl)  (clr((ch),(lvl))?KMAG:KNUL)
#define CCBMG(ch,lvl)  (clr((ch),(lvl))?KBMG:KNUL)
#define CCCYN(ch,lvl)  (clr((ch),(lvl))?KCYN:KNUL)
#define CCBCN(ch,lvl)  (clr((ch),(lvl))?KBCN:KNUL)
#define CCWHT(ch,lvl)  (clr((ch),(lvl))?KWHT:KNUL)
#define CCGLA(ch,lvl)  (clr((ch),(lvl))?KGLA:KNUL)
#define CCFSH(ch,lvl)  (clr((ch),(lvl))?KFSH:KNUL)
#define CCDBL(ch,lvl)  (clr((ch),(lvl))?KDBL:KNUL)
#define CCBLD(ch,lvl)  (clr((ch),(lvl))?KBLD:KNUL)
#define CCYOB(ch,lvl)  (clr((ch),(lvl))?KYOB:KNUL)
#define CCYOR(ch,lvl)  (clr((ch),(lvl))?KYOR:KNUL)
#define CCGRY(ch,lvl)  (clr((ch),(lvl))?KGRY:KNUL)
#define CCBRN(ch,lvl)  (clr((ch),(lvl))?KBRN:KNUL)
#define CCGLD(ch,lvl)  (clr((ch),(lvl))?KGLD:KNUL)

#define COLOR_LEV(ch) (_clrlevel(ch))

#define QNRM CCNRM(ch,C_SPR)
#define QRED CCRED(ch,C_SPR)
#define QGRN CCGRN(ch,C_SPR)
#define QYEL CCYEL(ch,C_SPR)
#define QBLU CCBLU(ch,C_SPR)
#define QMAG CCMAG(ch,C_SPR)
#define QCYN CCCYN(ch,C_SPR)
#define QWHT CCWHT(ch,C_SPR)

#endif

