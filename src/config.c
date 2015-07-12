/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  CONFIG.C                                     Based on CircleMUD *
*  Usage: Configuration of various aspects of The Dominion operation      *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#define __CONFIG_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"

#define TRUE	1
#define YES	1
#define FALSE	0
#define NO	0

/*
 * Below are several constants which you can change to alter certain aspects
 * of the way CircleMUD acts.  Since this is a .c file, all you have to do
 * to change one of the constants (assuming you keep your object files around)
 * is change the constant in this file and type 'make'.  Make will recompile
 * this file and relink; you don't have to wait for the whole thing to
 * recompile as you do if you change a header file.
 *
 * I realize that it would be slightly more efficient to have lots of
 * #defines strewn about, so that, for example, the autowiz code isn't
 * compiled at all if you don't want to use autowiz.  However, the actual
 * code for the various options is quite small, as is the computational time
 * in checking the option you've selected at run-time, so I've decided the
 * convenience of having all your options in this one file outweighs the
 * efficency of doing it the other way.
 *
 */

/****************************************************************************/
/****************************************************************************/


/* GAME PLAY OPTIONS */

/*
 * pk_allowed sets the tone of the entire game.  If pk_allowed is set to
 * NO, then players will not be allowed to kill, summon, charm, or sleep
 * other players, as well as a variety of other "asshole player" protections.
 * However, if you decide you want to have an all-out knock-down drag-out
 * PK Mud, just set pk_allowed to YES - and anything goes.
 */
int pk_allowed = YES;

/* is playerthieving allowed? */
int pt_allowed = YES;

/* minimum level a player must be to shout/holler/gossip/auction */
int level_can_shout = 1;

/* number of movement points it costs to holler */
int holler_move_cost = 20;

/* exp change limits */
int max_exp_gain = 50000;	/* max gainable per kill */
int max_exp_loss = 100000;	/* max losable per death */

/* number of tics (usually 75 seconds) before PC/NPC corpses decompose */
int max_npc_corpse_time = 5;
int max_pc_corpse_time = 10;

/* should items in death traps automatically be junked? */
int dts_are_dumps = NO;

/* "okay" etc. */
char *OK = "Okay.\r\n";
char *NOPERSON = "No-one by that name here.\r\n";
char *NOEFFECT = "Nothing seems to happen.\r\n";

/****************************************************************************/
/****************************************************************************/

/* Command Line Mode Password - Change this OFTEN! */
char *CLPASSWD = "+-*/&^%$#@!";

char *CL_TEXT =
     "Welcome to Command Line Mode!\r\n"
     "   D) Disconnect\r\n"
     "   N) Enter the Game\r\n"
     "   C) Custom Create a PC\r\n"
     "\r\nYour choice? ";

/* RENT/CRASHSAVE OPTIONS */

/*
 * Should the MUD allow you to 'rent' for free?  (i.e. if you just quit,
 * your objects are saved at no cost, as in Merc-type MUDs.
 */
int free_rent = NO;

/* maximum number of items players are allowed to rent */
int max_obj_save = 50;

/* receptionist's surcharge on top of item costs */
int min_rent_cost = 100;

/*
 * Should the game automatically save people?  (i.e., save player data
 * every 4 kills (on average), and Crash-save as defined below.
 */
int auto_save = YES;

/*
 * if auto_save (above) is yes, how often (in minutes) should the MUD
 * Crash-save people's objects?   Also, this number indicates how often
 * the MUD will Crash-save players' houses.
 */
int autosave_time = 5;

/* Lifetime of crashfiles and forced-rent (idlesave) files in days */
int crash_file_timeout = 15;

/* Lifetime of normal rent files in days */
int rent_file_timeout = 30;

/****************************************************************************/
/****************************************************************************/


/* ROOM NUMBERS */

/* virtual number of room that mortals should enter at */
sh_int mortal_start_room = 3039;

/* virtual number of room that immorts should enter at by default */
sh_int immort_start_room = 8;

/* virtual number of room that frozen players should enter at */
sh_int frozen_start_room = 6;

/*
 * virtual numbers of donation rooms.  note: you must change code in
 * do_drop of act.obj1.c if you change the number of non-NOWHERE
 * donation rooms.
 */
sh_int donation_room_1 = 3045;
sh_int donation_room_2 = NOWHERE;	/* unused - room for expansion */
sh_int donation_room_3 = NOWHERE;	/* unused - room for expansion */


/****************************************************************************/
/****************************************************************************/


/* GAME OPERATION OPTIONS */

/*
 * default port the game should run on if no port given on command-line
 * Note: If you're using the 'autorun' script, the port number there will
 * override this setting.  Change the PORT= line in autorun instead.
 */
int DFLT_PORT = 8400;

/* default directory to use as data directory */
char *DFLT_DIR = "lib";

/* maximum number of players allowed before game starts to turn people away */
int MAX_PLAYERS = 255;

/* maximum size of bug, typo and idea files (to prevent bombing) */
int max_filesize = 65536;

/* maximum number of password attempts before disconnection */
int max_bad_pws = 3;

/*
 * Some nameservers are very slow and cause the game to lag terribly every
 * time someone logs in.  The lag is caused by the gethostbyaddr() function
 * which is responsible for resolving numeric IP addresses to alphabetic names.
 * Sometimes, nameservers can be so slow that the incredible lag caused by
 * gethostbyaddr() isn't worth the luxury of having names instead of numbers
 * for players' sitenames.
 *
 * If your nameserver is fast, set the variable below to NO.  If your
 * nameserver is slow, of it you would simply prefer to have numbers
 * instead of names for some other reason, set the variable to YES.
 *
 * You can experiment with the setting of nameserver_is_slow on-line using
 * the SLOWNS command from within the MUD.
 */

int nameserver_is_slow = NO;


char *MENU =
"[H[J"
"\r\nWelcome to The Dominion!\r\n"
"0) Exit from The Dominion.\r\n"
"1) Enter The Dominion.\r\n"
"2) Enter your description.\r\n"
"3) Read the background story.\r\n"
"4) Change your password.\r\n"
"5) Delete your character.\r\n"
"6) See who is currently in The Dominion.\r\n"
"\r\n"
"   Make your choice: ";

char *ANSI_MENU =
"[H[J\e[?7h\e[1;24r[1m[36m"
"\r\n"
"        _________________________________\r\n"
"       ()                                )\r\n"
"        |  Welcome to The Dominion       |\r\n"
"        |                                |\r\n"
"        |  [1m[33m0[35m) Leave The Dominion.[36m        |\r\n"
"        |  [1m[33m1[35m) Enter The Dominion.[36m        |\r\n"
"        |  [1m[33m2[35m) Change your description.[36m   |\r\n"
"        |  [1m[33m3[35m) Read the background story.[36m |\r\n"
"        |  [1m[33m4[35m) Change your password.[36m      |\r\n"
"        |  [1m[33m5[35m) Delete your character.[36m     |\r\n"
"        |  [1m[33m6[35m) Those in The Dominion.[36m     |\r\n"
"        |                               /;\r\n"
"         \\  /^\\/^\\/'\\  /\\/^\\/^\\/ \\/^\\ /^\r\n"
"          ;/          \\;            ;/\r\n"
"\r\n"
"        Make your choice:[0m ";

char *SECGREETING =

"[H[J"
"\r\n"
"                          Based on CircleMUD 3.0 Beta\r\n"
"                            Created by Jeremy Elson\r\n"
"\r\n"
"                      A derivative of DikuMUD (GAMMA 0.0)\r\n"
"                                  Created by\r\n"
"                     Hans Henrik Staerfeldt, Katja Nyboe,\r\n"
"               Tom Madsen, Michael Seifert, and Sebastian Hammer\r\n"
"\r\n"
"                                                       _____------------___\r\n"
"                                                  ._--':::::'-------____\r\n"
"              .___------__       /-.._.    _---_ '|:::::::::::::::::::::---\r\n"
"         ._--'.---::::::/ `      \\ .-. '-'' *__*|/:::::::::::::::::::::::::\r\n"
"    .__-'  _-'::::::::/ ._------_| '_'  __--' _'/::::::::::::::::::::::::::\r\n"
"_--'    _-'::::::::::|.'  ._----_\\    -'  ._-':::::::::::::::::::::::::::::\r\n"
"     _-':::::::::::::\\  .'       /  .__--' -':::::::::_--_:::::::::::.-----\r\n"
" _-'::::::::::::::::::-_|       /    /   /::::::::::/      \\:::::::/\r\n"
"'::::::::::::::::::::::::----__-   .   .  |.--_:::/          \\:::/\r\n"
"----_::::::::::::::::::::/                \\  \\\\ \\/             \\/\r\n"
"    _/_/_/_/_/;;_/;;;;;;/                 \r\n"
"       _/      _/_/_/;;/   _/_/           Brought to you by,\r\n"
"      _/      _/    _/  _/_/_/_/          \r\n"
"     _/      _/    _/  _/                      Glasgian\r\n"
"    _/      _/    _/    _/_/_/               Kirahn Garou\r\n"
"                                            \r\n"
"                                            \r\n"
"      _/_/_/                              _/            _/                   \r\n"
"     _/    _/    _/_/    _/_/_/  _/_/        _/_/_/          _/_/    _/_/_/  \r\n"
"    _/    _/  _/    _/  _/    _/    _/  _/  _/    _/  _/  _/    _/  _/    _/ \r\n"
"   _/    _/  _/    _/  _/    _/    _/  _/  _/    _/  _/  _/    _/  _/    _/  \r\n"
"  _/_/_/      _/_/    _/    _/    _/  _/  _/    _/  _/    _/_/    _/    _/   \r\n"
"\r\n"
"By what name do you wish to be known? ";


char *THIRGREETING =

"[H[J"
"\r\n"
"       A derivative of DikuMUD (GAMMA 0.0) By Hans Henrik Staerfeldt, \r\n"
"       Katja Nyboe, Tom Madsen, Michael Seifert, and Sebastian Hammer\r\n"
"\r\n"
"                              _\r\n"
"                           ==(W{==========-      /===-\r\n"
"                             ||  (.--.)         /===-_---~~~~~~~~~------____\r\n"
"                             | \\_,|**|,__      |===-~___                _,-'`\r\n"
"                -==\\\\        `\\ ' `--'   ),    `//~\\\\   ~~~~`---.___.-~~\r\n"
"            ______-==|        /`\\_. .__/\\ \\    | |  \\\\           _-~`\r\n"
"      __--~~~  ,-/-==\\\\      (   | .  |~~~~|   | |   `\\        ,'\r\n"
"   _-~       /'    |  \\\\     )__/==0==-\\<>/   / /      \\      /\r\n"
" .'        /       |   \\\\      /~\\___/~~\\/  /' /        \\   /'\r\n"
"/  ____  /         |    \\`\\.__/-~~   \\  |_/'  /          \\/'\r\n"
"-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`\r\n"
"                 \\_|      /        _) | ;  ),   __--~~\r\n"
"                   '~~--_/      _-~/- |/ \\   '-~ \\    \r\n"
"  Welcome to      {\\__--_/}    / \\\\_>-|)<__\\      \\   \r\n"
"    The Dominion  /'   (_/  _-~  | |__>--<__|      |  Based on\r\n"
"                 |   _/) )-~     | |__>--<__|      |   CircleMUD 3.0\r\n"
"     Glasgian    / /~ ,_/       / /__>---<__/      |    By Jeremy Elson\r\n" "                o-o _//        /-~_>---<__-~      /   \r\n"
"      Kirahn    (^(~          /~_>---<__-      _-~    \r\n"
"       Garou   ,/|           /__>--<__/     _-~       \r\n"
"            ,//('(          |__>--<__|     /  Title Art by    .----_ \r\n"
"           ( ( '))          |__>--<__|    |    -Tua Xiong   /' _---_~\\\r\n"
"        `-)) )) (           |__>--<__|    |               /'  /     ~\\`\\\r\n"
"By what name do you wish to be known? ";


char *FORGREETING =

"[H[J"
"\r\n"
"                      A derivative of DikuMUD (GAMMA 0.0)\r\n"
"                                  Created by\r\n"
"                     Hans Henrik Staerfeldt, Katja Nyboe,\r\n"
"               Tom Madsen, Michael Seifert, and Sebastian Hammer\r\n"
"\r\n"
"                          Based on CircleMUD 3.0 Beta\r\n"
"                            Created by Jeremy Elson\r\n"
"\r\n"
"                             .:'                                 `:.\r\n"
"                            ::'          Title Screen by          `::\r\n"
"                           :: :.      Ronald Allan Stanions      .: ::\r\n"
"                            `:. `:.             .             .:'  .:'\r\n"
"                             `::. `::           !           ::' .::'\r\n"
"  _____ _                        `::.`::.    .' ! `.    .::'.::'\r\n"
" |_   _| |__   ___                 `:.  `::::'':!:``::::'   ::'\r\n"
"   | | | '_ \\ / _ \\                :'*:::.  .:' ! `:.  .:::*`:\r\n"
"   | | | | | |  __/               :: HHH::.   ` ! '   .::HHH ::\r\n"
"   |_| |_| |_|\\___|              ::: `H TH::.  `!'  .::HT H' :::\r\n"
"                                 ::..  `THHH:`:   :':HHHT'  ..::\r\n"
"       By  Glasgian, Garou,      `::      `T: `. .' :T'      ::'\r\n"
"           Kirahn, & Silvash       `:. .   :         :   . .:'\r\n"
"  ____                  _       _    `::'               `::'\r\n"
" |  _ \\  ___  _ __ ___ (_)_ __ (_) ___ :_ __`.  .  .'.  `:\r\n"
" | | | |/ _ \\| '_ ` _ \\| | '_ \\| |/ _ \\| '_  \\      .:: `:\r\n"
" | |_| | (_) | | | | | | | | | | | (_) | |`| |     :::' `:\r\n"
" |____/ \\___/|_| |_| |_|_|_| |_|_|\\___/|_| |_|     ''  .'\r\n"
"                                         :`...........':\r\n"
"                                         ` :`.     .': '\r\n"
"                                          `:  `\"\"\"'  :'\r\n"
"\r\n"
"By what name do you wish to be known? ";



char *ANSI_GREET =

"[H[J"
"\r\n"
"[2J[0m[1m                             [0;33m.:'[1;37m                                 [0;33m`:.[37m \r\n"
"[1m                             [0;33m::'[1;37m         [0m                [1m          [0;33m`::[1;37m     [0m\r\n"
"[1m                            [0;33m::[1;37m [0;33m:.[1;37m     [0m                       [1m     [0;33m.:[1;37m [0;33m::[37m \r\n"
"[1m                             [0;33m`:.[1;37m [0;33m`:.[1;37m             [33m.[37m             [0;33m.:'[1;37m  [0;33m.:'[37m \r\n"
"[1m                             [0m [33m`[1;37m [0;33m::.[1;37m [0;33m`::.[1;37m         [33m![37m          [0;33m.::'[1;37m [0;33m.::'[37m \r\n"
"[1m  [34m_____[37m [34m_[37m                         [0;33m`::.`::.[1;37m    [0;33m.'[1;37m [33m![37m [0;33m`.[1;37m    [0;33m.::'.:::'[37m \r\n"
"[1m [34m|_[37m   [34m_|[37m [34m|__[37m   [34m___[37m                  [0;33m`:.[1;37m  [0;32m`::::'':[1;33m![0;32m:''::::'[1;37m   [0;33m::'[37m \r\n"
"[1m   [34m|[37m [34m|[37m [34m|[37m [34m'_[37m [34m\\[37m [34m/[37m [34m_[37m [34m\\[37m                 [0;32m:'[1;36m*[0;32m:::.[1;37m  [0;32m.:'[1;37m [33m![37m [0;32m`:.[1;37m  [0;32m.:::[1;36m*[0;32m`:[37m \r\n"
"[1m   [34m|[37m [34m|[37m [34m|[37m [34m|[37m [34m|[37m [34m|[37m  [34m__/[37m                [0;32m::[1;37m [31mHHH[0;32m::.[1;37m   [0;32m`[1;37m [33m![37m [0;32m'[1;37m   [0;32m.::[1;31mHHH[37m [0;32m::[37m \r\n"
"[1m   [34m|_|[37m [34m|_|[37m [34m|_|\\___|[37m               [0;32m:::[1;37m [31m`H TH[0;32m::.[1;37m  [0;32m`[1;33m![0;32m'[1;37m  [0;32m.::[1;31mHT H'[37m [0;32m:::[37m \r\n"
"[1m                                  [0;32m::..[1;37m  [31m`THHH[0;32m:`:[1;37m   [0;32m:':[1;31mHHHT'[37m  [0;32m..::[37m \r\n"
"[1m [0;35mBy[1;37m  [0;35mGlasgian, Garou, Grendel[1;37m      [0;32m`::[1;37m      [31m`T[0;32m:[1;37m [0;32m`.[1;37m [0;32m.'[1;37m [0;32m:[1;31mT'[37m      [0;32m::'[37m \r\n"
"[1m     [0;35mKirahn, & Silvash   [1;37m           [0;32m`:.[1;37m  [0;32m.[1;37m   [0;32m:[1;37m         [0;32m:[1;37m   [0;32m.[1;37m [0;32m.:'[37m \r\n"
"[1m  [34m____[37m                  [33m_[37m       [33m_[37m     [0;32m`::'[1;37m               [0;32m`::'[37m \r\n"
"[1m [34m|[37m  [34m_[37m [34m\\[37m  [34m___[37m  "
"[34m_[37m [34m__[37m [34m___[37m [33m([41m [40m)[34m_"
"[37m [34m__[37m [33m([41m [40m)[37m [34m___[37m  [0;32m:"
"[1;34m_[37m [34m__[0;32m`.[1;37m  [33m.[37m  [0;32m.'.[1;37m  [0;32m`:[37m \r\n"
"[1m [34m|[37m [34m|[37m [34m|[37m [34m|/[37m [34m_[37m "
"[34m\\|[37m [34m'_[37m [34m`[37m [34m_[37m [34m\\|[37m "
"[34m|[37m [34m'_[37m [34m\\|[37m [34m|/[37m [34m_[37m "
"[34m\\|[37m [0;32m'[1;34m_[37m  [34m\\[37m       [0;32m.::"
"[1;37m [0;32m`:[37m \r\n"
"[1m [34m|[37m [34m|_|[37m [34m|[37m [34m(_)[37m [34m|"
"[37m [34m|[37m [34m|[37m [34m|[37m [34m|[37m [34m|[37m "
"[34m|[37m [34m|[37m [34m|[37m [34m|[37m [34m|[37m "
"[34m(_)[37m [34m|[37m [34m|[0;32m`[1;34m|[37m [34m|[37m"
"      [0;32m:[33m::[32m'[1;37m [0;32m`:[37m \r\n"
"[1m [34m|____/[37m [34m\\___/|_|[37m [34m|_|[37m [34m|_|_|_|[37m [34m|_|_|\\___/|_|[37m [34m|_|[37m      [0;32m''[1;37m  [0;32m.'[37m \r\n"
"[1m                                         [0;32m:`...........':[37m \r\n"
"[1m                                         [0;32m`[1;37m [0;32m:`.[1;37m     [0;32m.': '[37m \r\n"
"[1m                                          [0;32m`:[1;37m  [0;33m`[1;37m\"\"\"[0;33m'[1;37m  [0;32m:'[37m \r\n"
"\r\n[31mBy what name do you wish to be known? [0m";

char *ANSI_NAME_POLICY =
"\n\r[1m[36mNAME POLICY\n\r"
"In an effort to promote a medieval, fantasy atmosphere throughout\n\r"
"The Dominion, a name policy regarding character creation has been\n\r"
"established.  Under this new policy, the following guidelines must\r\n"
"be adhered to when choosing a new name:                       \n\r\n\r"
"  [31m-[36m Names must NOT be composed of common words, including common nouns, verbs,\n\r"
"    propositions, or any combination thereof.\n\r"
"    For example:  EvilMage, Flower, Backstab, DarkOne\n\r"
"  [31m-[36m No obscene, insulting, threatening or degrading names will be tolerated.\n\r"
"    (No examples needed)\n\r"
"  [31m-[36m No modern trademarks, cartoon characters, product names, etc. are allowed.\n\r" 
"    For example:  Beavis, Bacardi, Seinfeld\n\r"
"  [31m-[36m No incoherent collection of random letters is allowed.\n\r"
"    For example:  Wqzlks, Xeyplw\n\r"
"  [31m-[36m No names that imply rank or privilage.\n\r"
"    For example: King, Lady\n\r"
"  [31m-[36m No animal names.\n\r"
"    For example: Cougar, Puma, Snake, etc.\n\r"
"  [31m-[36m No Capital letters in the name other then the first letter\n\r" 
"    For example: STUDBOY \n\r"
"  [31m-[36m Please TRY to choose a medieval type of name.  If you have any questions\n\r"
"    regarding the acceptability of a certain name, feel free to ask a God for\n\r"
"    input and guidance.\n\r\n\r\x1B[0m";

char *NAME_POLICY =
"\n\rNAME POLICY\n\r"
"In an effort to promote a medieval, fantasy atmosphere throughout\n\r"
"The Dominion, a name policy regarding character creation has been\n\r"
"established.  Under this new policy, the following guidelines must\r\n"
"be adhered to when choosing a new name:                       \n\r\n\r"
"[1] Names must NOT be composed of common words, including common nouns,\r\n"
"    verbs, propositions, or any combination thereof.\n\r"
"    For example:  EvilMage, Flower, Backstab, DarkOne\n\r"
"[2] No obscene, insulting, threatening or degrading names will be tolerated.\n\r" "    (No examples needed)\n\r"
"[3] No modern trademarks, cartoon characters, product names, etc. are allowed.\n\r"
"    For example:  Beavis, Bacardi, Seinfeld\n\r"
"[4] No incoherent collection of random letters is allowed.\n\r"
"    For example:  Wqzlks, Xeyplw\n\r"
"[5] No names that imply rank or privilage.\n\r"
"    For example: King, Lady\n\r"
"[6] No animal names.\n\r"
"    For example: Cougar, Puma, Snake, etc.\n\r"
"[7] No Capital letters in the name other then the first letter\n\r"
"    For example: STUDBOY \n\r"
"[8] Please TRY to choose a medieval type of name.  If you have any questions\n\r"
"    regarding the acceptability of a certain name, feel free to ask a God for\n\r"
"    input and guidance.\n\r\n\r";

char *WELC_MESSG =
"\r\n"
"Welcome to The Dominion.  May your visit here be... Intriguing."
"\r\n\r\n";


char *START_MESSG =
"You open your eyes and stand, stretching your sore muscles.\r\n"
"Once the fog in your head clears you pull your jacket on and\r\n"
"sling your pack over your shoulder.  Looking around you realize\r\n"
"you are in a place far different from the one you went to sleep in.\r\n"
"It looks as if you're on your own now, you had better figure where\r\n"
"you are and what you are going to do about it.\r\n\r\n\r\n";

/****************************************************************************/
/****************************************************************************/


/* AUTOWIZ OPTIONS */

/* Should the game automatically create a new wizlist/immlist every time
   someone immorts, or is promoted to a higher (or lower) god level? */
int use_autowiz = NO;

/* If yes, what is the lowest level which should be on the wizlist?  (All
   immort levels below the level you specify will go on the immlist instead.) */
int min_wizlist_lev = LVL_GOD;
