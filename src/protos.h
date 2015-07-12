/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  PROTOS.H                                     Based on CircleMUD *
*  Usage: Header Files for The Dominion                                   *
*  Programmer(s): by Sean Mountcastle (Glasgian)                          *
\*********************************************************************** */

#ifndef __PROTOS_H__
#define __PROTOS_H__

#include "structs.h"
#include "utils.h"
#include "interpre.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "spells.h"
#include "house.h"
#include "creation.h"
#include "screen.h"
#include "boards.h"
#include "save.h"

#ifdef __MAIL_C__
#include "mail.h"
#endif

#if defined(__SHOP_C__) || defined (__CREATION_C__)
#include "shop.h"
#endif

#endif /* __PROTOS_H__ */
