/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  SPELLS.H                                     Based on CircleMUD *
*  Usage: Header: Constants and prototypes for spell system               *
*  Programmer(s): Original code by Jeremy Elson (Ras)                     *
*                 All Modifications by Sean Mountcastle (Glasgian)        *
\*********************************************************************** */

#ifndef _SPELLS_H
#define _SPELLS_H

#define DEFAULT_STAFF_LVL       12
#define DEFAULT_WAND_LVL        12

#define CAST_UNDEFINED  -1
#define CAST_SPELL      0
#define CAST_POTION     1
#define CAST_WAND       2
#define CAST_STAFF      3
#define CAST_SCROLL     4
#define CAST_BREATH     5

#define MAG_DAMAGE      (1 << 0)
#define MAG_AFFECTS     (1 << 1)
#define MAG_UNAFFECTS   (1 << 2)
#define MAG_POINTS      (1 << 3)
#define MAG_ALTER_OBJS  (1 << 4)
#define MAG_GROUPS      (1 << 5)
#define MAG_MASSES      (1 << 6)
#define MAG_AREAS       (1 << 7)
#define MAG_SUMMONS     (1 << 8)
#define MAG_CREATIONS   (1 << 9)
#define MAG_MANUAL      (1 << 10)

#define TYPE_UNDEFINED               -1
#define SPHERE_NONE                  -1

#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */


/* PLAYER SKILL/SPELL SPHERES - Numbered from 1 to MAX_SPHERES */
/* Do not change these numbers they are saved in the pfile!!!! */
#define SPHERE_GENERAL                1 /* General Knowledge Sphere       */
#define SPHERE_PRAYER                 2 /* Prayer Sphere                  */
#define SPHERE_ARCANE                 3 /* Arcane Sphere                  */
#define SPHERE_GTHIEVERY              4 /* General Thievery Sphere        */
#define SPHERE_COMBAT                 5 /* Combat Proficiency Sphere      */
#define SPHERE_UNUSED01               6 /* Room for improvement           */

#define MAIN_SPHERES                  6
/* Sub-Spheres begin here with 7 (8 per Sphere)                           */
/* Prayer */
#define START_SUBSPHERES              7
#define SPHERE_ACTS_OF_DEITIES        7 /* Acts of Deities                */
#define SPHERE_AFFLICTIONS            8 /* Afflictions                    */
#define SPHERE_CURES                  9 /* Cures                          */
#define SPHERE_HAND_OF_DEITIES       10 /* Hand of Deities                */
#define SPHERE_FAITH                 11 /* Faith                          */
#define SPHERE_PRAY_UNUSED01         12 /* Room for improvement           */
#define SPHERE_PRAY_UNUSED02         13 /* Room for improvement           */
#define SPHERE_PRAY_UNUSED03         14 /* Room for improvement           */
/* Arcane */
#define SPHERE_CONJURATION           15 /* Conjuration Summoning          */
#define SPHERE_DIVINATION            16 /* Divination                     */
#define SPHERE_ENCHANTMENT           17 /* Enchantment Alteration         */
#define SPHERE_ENTROPIC              18 /* Entropic Body                  */
#define SPHERE_INVOCATION            19 /* Invocation Evocation           */
#define SPHERE_PHANTASMIC            20 /* Phantasmic Illusion            */
#define SPHERE_ARCA_UNUSED01         21 /* Room for improvement           */
#define SPHERE_ARCA_UNUSED02         22 /* Room for improvement           */
/* General Thievery */
#define SPHERE_LOOTING               23 /* Looting                        */
#define SPHERE_MURDER                24 /* Murder                         */
#define SPHERE_THIEVING              25 /* Thieving                       */
#define SPHERE_GTHI_UNUSED01         26 /* Room for improvement           */
#define SPHERE_GTHI_UNUSED02         27 /* Room for improvement           */
#define SPHERE_GTHI_UNUSED03         28 /* Room for improvement           */
#define SPHERE_GTHI_UNUSED04         29 /* Room for improvement           */
#define SPHERE_GTHI_UNUSED05         30 /* Room for improvement           */
/* Combat Proficiency */
#define SPHERE_HAND_TO_HAND          31 /* Hand to Hand Combat            */
#define SPHERE_MARTIAL_ARTS          32 /* Martial Arts                   */
#define SPHERE_WEAPONS               33 /* Weapons/Arms Proficiency       */
#define SPHERE_COMB_UNUSED01         34 /* Room for improvement           */
#define SPHERE_COMB_UNUSED02         35 /* Room for improvement           */
#define SPHERE_COMB_UNUSED03         36 /* Room for improvement           */
#define SPHERE_COMB_UNUSED04         37 /* Room for improvement           */
#define SPHERE_COMB_UNUSED05         38 /* Room for improvement           */
/* General Knowledge */
#define SPHERE_GENE_UNUSED01         39 /* Room for improvement           */
#define SPHERE_GENE_UNUSED02         40 /* Room for improvement           */
#define SPHERE_GENE_UNUSED03         41 /* Room for improvement           */
#define SPHERE_GENE_UNUSED04         42 /* Room for improvement           */
#define SPHERE_GENE_UNUSED05         43 /* Room for improvement           */
#define SPHERE_GENE_UNUSED06         44 /* Room for improvement           */
#define SPHERE_GENE_UNUSED07         45 /* Room for improvement           */
#define SPHERE_GENE_UNUSED08         46 /* Room for improvement           */
/* Unused sphere */
#define SPHERE_UNUS_UNUSED05         47 /* Room for improvement           */
#define SPHERE_UNUS_UNUSED06         48 /* Room for improvement           */
#define SPHERE_UNUS_UNUSED07         49 /* Room for improvement           */
#define SPHERE_UNUS_UNUSED08         50 /* Room for improvement           */

#define MAX_SPHERES                  50 /* No more spheres after this     */

/* PLAYER SPELLS -- Numbered from 51 to MAX_SPELLS */

#define SPELL_ARMOR                  51 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT               52 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                  53 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLIND                  54 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS          55 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING         56 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM                  57 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH            58 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                  59 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOR_SPRAY            60 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        61 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            62 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           63 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             64 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL_CRITIC            65 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL_LIGHT             66 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  67 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SOUL_SEARCH            68 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVIS           69 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           70 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 71 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            72 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             73 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         74 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           75 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               76 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   77 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   78 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              79 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         80 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          81 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          82 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          83 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROT_FROM_EVIL         84 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           85 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              86 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         87 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  88 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               89 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 90 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          91 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         92 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          93 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             94 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ANIMATE_DEAD           95 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_GOOD            96 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_ARMOR            97 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_HEAL             98 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_RECALL           99 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INFRAVISION           100 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WATERWALK             101 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ICE_SHOWER            102 /* TD 3/17/95 */
#define SPELL_FIRE_STORM            103 /* TD 3/17/95 */
#define SPELL_METEOR_SWARM          104 /* TD 3/17/95 */
#define SPELL_POWER_KILL            105 /* TD 3/17/95 */
#define SPELL_WATERBREATH           106 /* TD 3/17/95 */
#define SPELL_REFRESH               107 /* TD 3/17/95 */
#define SPELL_ASTRAL_WALK           108 /* TD 3/17/95 */
#define SPELL_HASTE                 109 /* TD 3/17/95 */
#define SPELL_MIN_GLOBE             110 /* TD 3/17/95 */
#define SPELL_MAJ_GLOBE             111 /* TD 3/17/95 */
#define SPELL_FLY                   112 /* TD 4/6/95  */
#define SPELL_DISPEL_MAGIC          113 /* TD 4/6/95  */
#define SPELL_PORTAL                114 /* TD 5/10/95 */
#define SPELL_ENCHANT_ARMOR         115 /* TD 5/25/95 */
#define SPELL_HEAL_SERIOUS          116 /* TD 9/02/95 */
#define SPELL_DISEASE               117 /* TD 9/04/95 */
#define SPELL_MONSUM_I              118 /* TD 9/09/95 */
#define SPELL_MONSUM_II             119 /* TD 9/09/95 */
#define SPELL_MONSUM_III            120 /* TD 9/09/95 */
#define SPELL_MONSUM_IV             121 /* TD 9/09/95 */
#define SPELL_GATE_I                122 /* TD 9/09/95 */
#define SPELL_GATE_II               123 /* TD 9/09/95 */
#define SPELL_GATE_III              124 /* TD 9/09/95 */
#define SPELL_GATE_IV               125 /* TD 9/09/95 */
#define SPELL_SUM_FIRE_ELE          126 /* TD 9/09/95 */
#define SPELL_SUM_WATER_ELE         127 /* TD 9/09/95 */
#define SPELL_SUM_EARTH_ELE         128 /* TD 9/09/95 */
#define SPELL_SUM_AIR_ELE           129 /* TD 9/09/95 */
#define SPELL_AERIAL_SERVANT        130 /* TD 9/09/95 */
#define SPELL_DISPEL_ILLUSION       131 /* TD 9/09/95 */
#define SPELL_ILLUSIONARY_PRES_I    132 /* TD 9/09/95 */
#define SPELL_ILLUSIONARY_PRES_II   133 /* TD 9/09/95 */
#define SPELL_ILLUSIONARY_PRES_III  134 /* TD 9/09/95 */
#define SPELL_ILLUSIONARY_PRES_IV   135 /* TD 9/09/95 */
#define SPELL_GROUP_PORTAL          136 /* TD 10/02/95 */
#define SPELL_PASS_DOOR             137 /* TD 12/21/95 */
#define SPELL_KNOCK                 138 /* TD 12/21/95 */  /* unlock door */
#define SPELL_CURE_PARALYSIS        139 /* TD 12/21/95 */  /* rem para    */
#define SPELL_SILENCE               140 /* TD 12/21/95 */
#define SPELL_TROLLS_BLOOD          141 /* TD 12/21/95 */ /* regen       */
#define SPELL_LIFESTEAL             142 /* TD 12/21/95 */ /* take/gain li*/
#define SPELL_MEDUSA_GAZE           143 /* TD 12/21/95 */ /* petrify     */
#define SPELL_FORCE_BOLT            144 /* TD 12/21/95 */
#define SPELL_PURIFY                145 /* TD 12/21/95 */
#define SPELL_GROUP_PASS_DOOR       146 /* TD 12/21/95 */
#define SPELL_CURE_DISEASE          147 /* TD 01/07/96 */
#define SPELL_WIZARD_EYE            148 /* TD 02/16/96 */
#define SPELL_SOUL_SHOCK            149 /* TD 3/14/96 */
#define SPELL_CONTAMINATE           150 /* TD 3/14/96 */
#define SPELL_UNDEAD_EYES           151 /* TD 3/14/96 */
#define SPELL_GHOSTLY_SHIELD        152 /* TD 3/14/96 */
#define SPELL_SOUL_SUM_I            153 /* TD 3/14/96 */
#define SPELL_SOUL_SUM_II           154 /* TD 3/14/96 */
#define SPELL_SOUL_SUM_III          155 /* TD 3/14/96 */
#define SPELL_SOUL_SUM_IV           156 /* TD 3/15/96 */
#define SPELL_DECAY                 157 /* TD 3/14/96 */
#define SPELL_PROTECTION            158 /* TD 3/14/96 */
#define SPELL_CRIPPLE               159 /* TD 3/14/96 */
#define SPELL_CURSE_WEAPON          160 /* TD 3/14/96 */
#define SPELL_CURSE_ARMOR           161 /* TD 3/14/96 */
#define SPELL_DEATHKNELL            162 /* TD 3/14/96 */
#define SPELL_BLACKMANTLE           163 /* TD 3/14/96 */
#define SPELL_PLAGUE                164 /* TD 3/14/96 */
#define SPELL_MASK_PRESENCE         165 /* TD 3/14/96 */
#define SPELL_BLINDING_FLASH        166 /* TD 3/14/96 */
#define SPELL_FALSE_GLOW            167 /* TD 3/14/96 */
#define SPELL_LIGHTSTRIKE           168 /* TD 3/14/96 */
#define SPELL_SMOKE_SHIELD          169 /* TD 3/14/96 */
#define SPELL_CALM                  170 /* TD 3/14/96 */
#define SPELL_FEAR                  171 /* TD 3/14/96 */
#define SPELL_VERTIGO               172 /* TD 3/14/96 */
#define SPELL_DOUBLE_VISION         173 /* TD 3/14/96 */
#define SPELL_VANISH                174 /* TD 3/14/96 */
#define SPELL_FLAME_BOLT            175 /* TD 3/14/96 */
#define SPELL_HYPNOSIS              176 /* TD 3/14/96 */
#define SPELL_PENATRATING_GAZE      177 /* TD 3/14/96 */
#define SPELL_DETECT_ILLUSION       178 /* TD 3/18/96 */
#define SPELL_RESURRECTION          179 /* TD 3/24/96 */
#define SPELL_HARM_LIGHT            180 /* TD 3/24/96 */
#define SPELL_HARM_SERIOUS          181 /* TD 3/24/96 */
#define SPELL_HARM_CRITICAL         182 /* TD 3/24/96 */
#define SPELL_PARALYZE              183 /* TD 3/24/96 */
#define SPELL_CURE_POISON           184 /* TD 3/24/96 */
#define SPELL_KNIT_BONE             185 /* TD 3/24/96 */
#define SPELL_RESTORE_LIMB          186 /* TD 3/24/96 */
#define SPELL_FLAMESTRIKE           187 /* TD 3/24/96 */
#define SPELL_PROT_FROM_GOOD        188 /* TD 6/19/96 */
#define SPELL_FIREWALL              189 /* TD 7/31/96 */
#define SPELL_BLADEBARRIER          190 /* TD 7/31/96 */
#define SPELL_CORRUPT               191
#define SPELL_BALANCE               192

#define NUM_SPELLS                  192
/* Insert new spells here, up to MAX_SPELLS */
#define MAX_SPELLS                 1200

/* PLAYER SKILLS - Numbered from MAX_SPELLS+1 to MAX_SKILLS */
#define SKILL_BACKSTAB             1201 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                 1202 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                 1203 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                 1204 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK            1205 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PUNCH                1206 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE               1207 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                1208 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                1209 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRACK                1210 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_GARROTTE             1211 /* TD 3/17/95 */
#define SKILL_QUIVERING_PALM       1212 /* TD 3/17/95 */
#define SKILL_CIRCLE               1213 /* TD 3/17/95 */
#define SKILL_DBL_ATTACK           1214 /* TD 3/17/95 */
#define SKILL_TRI_ATTACK           1215 /* TD 3/17/95 */
#define SKILL_TREADLIGHT           1216 /* TD 3/28/95 */
#define SKILL_RIDE                 1217 /* TD 3/28/95 */
#define SKILL_SHADOW               1218 /* TD 4/18/95 */
#define SKILL_BERSERK              1219 /* TD 4/24/95 */
#define SKILL_DOORBASH             1220 /* TD 4/24/95 */
#define SKILL_DEATHSTROKE          1221 /* TD 4/28/95 */
#define SKILL_PUMMEL               1222 /* TD 4/28/95 */
#define SKILL_PEEK                 1223 /* TD 11/12/95 */
#define SKILL_HUNT                 1224 /* TD 01/22/96 */
#define SKILL_WRESTLING            1225 /* TD 01/27/96 */
#define SKILL_PUGILISM             1226 /* TD 01/27/96 */
#define SKILL_SWIM                 1227 /* TD 01/27/96 */
#define SKILL_CLIMB                1228 /* TD 01/27/96 */
#define SKILL_DETECT_TRAP          1229 /* TD 01/27/96 */
#define SKILL_DRAGON_RIDE          1230 /* TD 01/27/96 */
#define SKILL_READ_MAGIC           1231 /* TD 01/27/96 */
#define SKILL_BRIBERY              1232 /* TD 01/27/96 */
#define SKILL_DISARM               1233 /* TD 01/27/96 */
#define SKILL_EVALUATE             1234 /* TD 01/27/96 */
#define SKILL_HAGGLE               1235 /* TD 01/27/96 */
#define SKILL_MANUFACTURE_POISONS  1236 /* TD 01/27/96 */
#define SKILL_MANUFACTURE_POTIONS  1237 /* TD 01/27/96 */
#define SKILL_MANUFACTURE_SCROLLS  1238 /* TD 01/27/96 */
#define SKILL_MARKSMANSHIP         1239 /* TD 01/27/96 */
#define SKILL_SIGN                 1240 /* TD 01/27/96 */
#define SKILL_SECRET_LANGUAGE      1241 /* TD 01/27/96 */
#define SKILL_POLEARMS             1242 /* TD 01/27/96 */
#define SKILL_SWORDS               1243 /* TD 01/27/96 */
#define SKILL_BLUNT                1244 /* TD 01/27/96 */
#define SKILL_THROWING             1245 /* TD 01/27/96 */
#define SKILL_BANDAGE              1246 /* TD 03/23/96 */
#define SKILL_KNOW_UNDEAD          1247 /* TD 03/23/96 */
#define SKILL_KNOW_VEGGIE          1248 /* TD 03/23/96 */
#define SKILL_KNOW_DEMON           1249 /* TD 03/23/96 */
#define SKILL_KNOW_ANIMAL          1250 /* TD 03/23/96 */
#define SKILL_KNOW_REPTILE         1251 /* TD 03/23/96 */
#define SKILL_KNOW_HUMANOID        1252 /* TD 03/23/96 */
#define SKILL_KNOW_OTHER           1253 /* TD 03/23/96 */
#define SKILL_REMOVE_TRAP          1254 /* TD 03/23/96 */
#define SKILL_SEARCH               1255 /* TD 03/23/96 */
#define SKILL_SET_TRAP             1256 /* TD 03/23/96 */
#define SKILL_POISON_WEAPON        1257 /* TD 03/23/96 */
#define SKILL_STAB                 1258 /* TD 03/23/96 */
#define SKILL_CUDGEL               1259 /* TD 03/23/96 */
#define SKILL_DISGUISE             1260 /* TD 03/23/96 */
#define SKILL_SPY                  1261 /* TD 03/23/96 */
#define SKILL_BODYSLAM             1262 /* TD 03/23/96 */
#define SKILL_GRAPPLE              1263 /* TD 03/23/96 */
#define SKILL_HEADBUTT             1264 /* TD 03/23/96 */
#define SKILL_SWITCH_OPPONENTS     1265 /* TD 03/23/96 */
#define SKILL_DODGE                1266 /* TD 03/23/96 */
#define SKILL_FEIGN_DEATH          1267 /* TD 03/23/96 */
#define SKILL_FIRST_AID            1268 /* TD 03/23/96 */
#define SKILL_KARATE               1269 /* TD 03/23/96 */
#define SKILL_PIERCE               1270 /* TD 03/23/96 */
#define SKILL_RANGE                1271 /* TD 03/23/96 */
#define SKILL_SHARPEN              1272 /* TD 03/23/96 */
#define SKILL_SMYTHE               1273 /* TD 03/23/96 */
#define SKILL_TACTICS              1274 /* TD 03/23/96 */
#define SKILL_TURN_UNDEAD          1275 /* TD 08/18/96 */
#define SKILL_ZEN_THINKING         1276

/* New skills may be added here up to MAX_SKILLS (1700)*/
#define MAX_SKILLS                 1700 /* TD 02/28/96 */

/*
 *  NON-PLAYER AND OBJECT SPELLS AND SKILLS
 *  The practice levels for the spells and skills below are _not_ recorded
 *  in the playerfile; therefore, the intended use is for spells and skills
 *  associated with objects (such as SPELL_IDENTIFY used with scrolls of
 *  identify) or non-players (such as NPC-only spells).
 */

#define SPELL_IDENTIFY               1701
#define SPELL_FIRE_BREATH            1702
#define SPELL_GAS_BREATH             1703
#define SPELL_FROST_BREATH           1704
#define SPELL_ACID_BREATH            1705
#define SPELL_LIGHTNING_BREATH       1706

#define TOP_SPELL_DEFINE             1999
/* NEW NPC/OBJECT SPELLS can be inserted here up to 1999 */

/* WEAPON ATTACK TYPES */
#define TYPE_HIT                     2000
#define TYPE_STING                   2001
#define TYPE_WHIP                    2002
#define TYPE_SLASH                   2003
#define TYPE_BITE                    2004
#define TYPE_BLUDGEON                2005
#define TYPE_CRUSH                   2006
#define TYPE_POUND                   2007
#define TYPE_CLAW                    2008
#define TYPE_MAUL                    2009
#define TYPE_THRASH                  2010
#define TYPE_PIERCE                  2011
#define TYPE_BLAST                   2012
#define TYPE_PUNCH                   2013
#define TYPE_STAB                    2014
#define TYPE_SMASH                   2015
#define TYPE_SMITE                   2016
#define TYPE_DROWNING                2017
#define TYPE_GARROTTE		     2018
/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_WOUNDS                  2049
#define TYPE_SUFFERING               2050

#define IS_WEAPON(type)              ((type) >= TYPE_HIT && (type) < TYPE_SUFFERING)



#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4
#define SAVING_FIRE   5
#define SAVING_COLD   6
#define SAVING_ACID   7
#define SAVING_ELEC   8


#define TAR_IGNORE        	   1
#define TAR_CHAR_ROOM     	   2
#define TAR_CHAR_WORLD    	   4
#define TAR_FIGHT_SELF    	   8
#define TAR_FIGHT_VICT   	  16
#define TAR_SELF_ONLY    	  32 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF     	  64 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     	 128
#define TAR_OBJ_ROOM    	 256
#define TAR_OBJ_WORLD   	 512
#define TAR_OBJ_EQUIP  		1024
#define TAR_DOOR       		2048
#define TAR_MUST_WIELD 		4096 /* Only a check, use with i.e. TAR_CHAR_ROOM */

struct spell_info_type {
   byte min_position;   /* Position for caster   */
   byte mana_min;       /* Min amount of mana used by a spell (highest lev) */
   byte mana_max;       /* Max amount of mana used by a spell (lowest lev) */
   byte mana_change;    /* Change in mana used by spell from lev to lev */

   byte sphere;         /* The sphere that contains this spell/sphere */
   int  percent;        /* sortof like the level needed to use it     */
   int  pretime;        /* The time that it takes for the casting to complete */
   int  routines;
   byte violent;
   int  targets;        /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
   char *singular;
   char *plural;
};


#define ASPELL(spellname) \
        void    spellname(byte level, struct char_data *ch, \
		  struct char_data *victim, struct obj_data *obj)

#define MANUAL_SPELL(spellname) spellname(level, caster, cvict, ovict);

ASPELL(spell_create_water);
ASPELL(spell_recall);
ASPELL(spell_teleport);
ASPELL(spell_summon);
ASPELL(spell_locate_object);
ASPELL(spell_charm);
ASPELL(spell_information);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_detect_poison);
ASPELL(spell_enchant_armor);
ASPELL(spell_astral_walk);
ASPELL(spell_dispel_magic);
ASPELL(spell_portal);
ASPELL(spell_dispel_illusion);
ASPELL(spell_pass_door);
ASPELL(spell_knock);
ASPELL(spell_purify);
ASPELL(spell_corrupt);
ASPELL(spell_balance);
ASPELL(spell_wizard_eye);
ASPELL(spell_curse_armor);
ASPELL(spell_curse_weapon);
ASPELL(spell_control_weather);
ASPELL(spell_blade_barrier);
ASPELL(spell_calm);

/* basic magic calling functions */

int find_skill_num(char *name);

void mag_damage(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int savetype);

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int savetype);

void mag_group_switch(int level, struct char_data *ch, struct char_data *tch, 
  int spellnum, int savetype);

void mag_groups(int level, struct char_data *ch, int spellnum, int savetype);

void mag_masses(int level, struct char_data *ch, int spellnum, int savetype);

void mag_areas(byte level, struct char_data *ch, int spellnum, int savetype);

void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
 struct char_data *tch, int spellnum, int savetype);

void mag_points(int level, struct char_data *ch, struct char_data *victim,
 int spellnum, int savetype);

void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int type);

void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
  int spellnum, int type);

void mag_creations(int level, struct char_data *ch, int spellnum);

int     call_magic(struct char_data *caster, struct char_data *cvict,
  struct obj_data *ovict, int spellnum, int level, int casttype);

void    mag_objectmagic(struct char_data *ch, struct obj_data *obj,
			char *argument);

int     cast_spell(struct char_data *ch, struct char_data *tch,
  struct obj_data *tobj, int spellnum);


/* other prototypes */
void spell_sphere(int spell, int sphere, int level, int timer);
void skill_sphere(int spell, int sphere, int level);
void init_spell_levels(void);
char *skill_name(int num);

#endif
