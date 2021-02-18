// Omega is free software, distributed under the ISC license

#pragma once
#include "config.h"
#include <curses.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

enum { CASHVALUE = -2, ABORT = -1, };
enum {
    MAXWIDTH = 64,		// moderately arbitrary but probably cannot be easily changed
    MAXLENGTH = 64,
    SMALLSCREENLENGTH = 16,
    MAXITEMS = 16,		// number of slots in inventory. Cannot be changed without work
    MAXPACK = 26,		// number of slots in pack. Should be <= 26.
    PAWNITEMS = 20,		// number of items in pawn shop. Should be <= 26
    REROLLS = 31,		// number of rerolls allowed +1
    MAXROOMS = 48,
    MAXCONNECTIONS = 4,
    STRING_LEN = 100,
    MAXLEVELS = 21,		// Arbitrary. Max of the levels in the dungeons
    ASTRALLEVELS = 5,		// levels in each dungeon
    SEWERLEVELS = 18,
    CASTLELEVELS = 16,
    CAVELEVELS = 10,
    VOLCANOLEVELS = 20,
    NOCITYMOVE = 250		// cannot use M command on site with this aux value
};

// Overall Game Progress Vector Bits
enum EGameStatus {
    SKIP_MONSTERS	= (1<<0),
    SKIP_PLAYER		= (1<<1),
    FAST_MOVE		= (1<<2),
    MOUNTED		= (1<<3),
    LOST		= (1<<4),
    CHEATED		= (1<<5),
    SUPPRESS_PRINTING	= (1<<6),
    SPOKE_TO_DRUID	= (1<<7),
    SPOKE_TO_ORACLE	= (1<<8),
    COMPLETED_CAVES	= (1<<9),
    COMPLETED_SEWERS	= (1<<10),
    COMPLETED_CASTLE	= (1<<11),
    COMPLETED_VOLCANO	= (1<<12),
    COMPLETED_ASTRAL	= (1<<13),
    COMPLETED_CHALLENGE	= (1<<14),
    KILLED_DRAGONLORD	= (1<<15),
    KILLED_EATER	= (1<<16),
    KILLED_LAWBRINGER	= (1<<17),
    GAVE_STARGEM	= (1<<18),
    DESTROYED_ORDER	= (1<<19),
    UNDEAD_GUARDS	= (1<<20),
    PREPARED_VOID	= (1<<21),
    SOLD_CONDO		= (1<<22),
    BANK_BROKEN		= (1<<23),
    CLUB_MEMBER		= (1<<24)
};

enum EEnvironment : int8_t {
    // non-existant environments for the random number seeding routine
    E_RESTORE = -2,
    E_RANDOM,
    // general environment types
    E_NEVER_NEVER_LAND,
    E_COUNTRYSIDE,
    E_CITY,
    E_VILLAGE,
    E_TACTICAL_MAP,
    E_ARENA,
    E_HOVEL,
    E_HOUSE,
    E_MANSION,
    E_CAVES,
    E_SEWERS,
    E_CASTLE,
    E_VOLCANO,
    E_ASTRAL,
    E_DLAIR,
    E_ABYSS,
    E_STARPEAK,
    E_MAGIC_ISLE,
    E_TEMPLE,
    E_CIRCLE,
    E_COURT,
    E_MAX = E_COURT,
    E_FIRST_DUNGEON = E_CAVES,
    E_LAST_DUNGEON = E_ASTRAL,
    E_NUMDUNGEONS = E_LAST_DUNGEON-E_FIRST_DUNGEON+1
};

// player game status
enum { DEAD = 1, QUIT, WIN, BIGWIN };

// as in attack low, block high, etc.
// These values may be added to the ones above to get things like
// block high, cut low, etc. CLEVER is only used by some monsters
// to cheat with....
enum { LOW = 1, CENTER, HIGH, CLEVER };
// weapon types
enum { CUTTING = 1, THRUSTING, STRIKING, MISSILE };

// random aux constants
// aux field for private residences in city
enum { UNLOCKED, BURGLED = 2, LOCKED };
// bow and crossbow object aux fields
enum { UNLOADED, LOADED };
// alignment used randomly throughout
enum { LAWFUL = 1, CHAOTIC, NEUTRAL };

// spells
enum ESpell {
    S_MON_DET, S_OBJ_DET, S_MISSILE, S_FIREBOLT, S_TELEPORT,
    S_LBALL, S_SLEEP, S_DISRUPT, S_DISINTEGRATE, S_POLYMORPH,
    S_HEAL, S_DISPEL, S_IDENTIFY, S_BREATHE, S_INVISIBLE,
    S_WARP, S_ENCHANT, S_BLESS, S_RESTORE, S_CURE,
    S_TRUESIGHT, S_HELLFIRE, S_KNOWLEDGE, S_HERO, S_RETURN,
    S_DESECRATE, S_HASTE, S_SUMMON, S_SANCTUARY, S_ACCURACY,
    S_RITUAL, S_FEAR, S_APPORT, S_SHADOWFORM, S_ALERT,
    S_REGENERATE, S_SANCTIFY, S_CLAIRVOYANCE, S_DRAIN, S_LEVITATE,
    S_WISH, NUMSPELLS
};

// ranks in guilds, etc
enum { LEGION, ARENA, COLLEGE, THIEVES, ORDER, CIRCLE, NOBILITY, PRIESTHOOD, ADEPT, NUMGUILDS };
enum { NOT_IN_LEGION,	LEGIONAIRE, CENTURION, FORCE_LEADER, COLONEL, COMMANDANT, FORMER_LEGIONAIRE = -1 };
enum { NOT_IN_ARENA,	TRAINEE, BESTIARIUS, RETIARIUS, GLADIATOR, CHAMPION, FORMER_GLADIATOR = -1 };
enum { NOT_IN_COLLEGE,	NOVICE, STUDENT, PRECEPTOR, MAGE, ARCHMAGE, FORMER_MAGE = -1 };
enum { NOT_A_THIEF,	TMEMBER, ATHIEF, THIEF, TMASTER, SHADOWLORD, FORMER_THIEF = -1 };
enum { NOT_IN_ORDER,	GALLANT, GUARDIAN, CHEVALIER, PALADIN, JUSTICIAR, FORMER_PALADIN = -1 };
enum { NOT_IN_CIRCLE,	INITIATE, ENCHANTER, SORCEROR, HIGHSORCEROR, PRIME, FORMER_SOURCEROR = -1 };
enum { NOT_NOBILITY,	COMMONER, ESQUIRE, KNIGHT, LORD, DUKE, FORMER_LORD = -1 };
enum { NOT_A_BELIEVER,	LAY, ACOLYTE, PRIEST, SPRIEST, HIGHPRIEST, FORMER_PRIEST = -1 };
enum { NOT_A_RELIGION,	ODIN, SET, ATHENA, HECATE, DRUID, DESTINY, NUMRELIGIONS, ATHEISM = -1 };

// Top-level NPCs. Value of monster::aux2
enum {
    NPC_HIGHSCORE,
    NPC_HIGHPRIEST_ODIN,
    NPC_HIGHPRIEST_SET,
    NPC_HIGHPRIEST_ATHENA,
    NPC_HIGHPRIEST_HECATE,
    NPC_HIGHPRIEST_DRUID,
    NPC_HIGHPRIEST_DESTINY,
    NPC_SHADOWLORD,
    NPC_COMMANDANT,
    NPC_ARCHMAGE,
    NPC_PRIME,
    NPC_CHAMPION,
    NPC_DUKE,
    NPC_LORD_OF_CHAOS,
    NPC_LORD_OF_LAW,
    NPC_JUSTICIAR,
    NPC_MAX
};

// MONSTER STATUS/ABILITY BITS
enum {
    HOSTILE	= (1<<0),
    MOBILE 	= (1<<1),
    ASLEEP	= (1<<2),
    WANDERING	= (1<<3),
    HUNGRY	= (1<<4),
    NEEDY	= (1<<5),
    GREEDY	= (1<<6),
    EDIBLE	= (1<<7),
    FLYING	= (1<<8),
    SWIMMING	= (1<<9),
    ONLYSWIM	= (1<<10),
    INTANGIBLE	= (1<<11),
    M_INVISIBLE	= (1<<12),
    POISONOUS	= (1<<13)
};

// PLAYER STATUS INDICES
enum {
    ACCURACY, BLINDED, SLOWED, DISPLACED, SLEPT,
    DISEASED, POISONED, HASTED, BREATHING, INVISIBLE,
    REGENERATING, VULNERABLE, BERSERK, IMMOBILE, ALERT,
    AFRAID, HERO, LEVITATING, ACCURATE, TRUESIGHT,
    SHADOWFORM, ILLUMINATION, DEFLECTION, PROTECTION, RETURNING,
    // PROTECTION is deviant -- indicates protective value, not duration
    // RETURNING is somewhat deviant--how many turns 'til RETURN spell goes off
    NUMSTATI
};

// player immunity indices
// also monster immunity bits (2^n)
// also damage types
enum EDamageType {
    EVERYTHING=-1,
    UNSTOPPABLE, NORMAL_DAMAGE, FLAME, COLD, ELECTRICITY,
    POISON, ACID, FEAR, SLEEP, NEGENERGY,
    OTHER_MAGIC, THEFT, GAZE, INFECTION, NUMIMMUNITIES
};

// location lstatus bits
enum {
    SEEN	= (1<<0),
    LIT		= (1<<1),
    SECRET	= (1<<2),
    STOPS	= (1<<3),
    CHANGED	= (1<<4)
};

// room string id for use in roomname()
enum ERoomName {
    RS_EMPTY, RS_WALLSPACE, RS_CORRIDOR, RS_CAVERN, RS_GOBLINKING,
    RS_DRAGONLORD, RS_PONDS, RS_OCEAN, RS_WYRM, RS_ADEPT,
    RS_ODIN, RS_SET, RS_ATHENA, RS_HECATE, RS_DRUID,
    RS_DESTINY, RS_COUNTRYSIDE, RS_ARENA, RS_SEWER_DUCT, RS_DRAINED_SEWER,
    RS_DROWNED_SEWER, RS_KITCHEN, RS_BEDROOM, RS_BATHROOM, RS_DININGROOM,
    RS_SECRETPASSAGE, RS_CLOSET, RS_LOWERASTRAL, RS_EARTHPLANE, RS_WATERPLANE,
    RS_AIRPLANE, RS_FIREPLANE, RS_HIGHASTRAL, RS_VOLCANO, RS_STARPEAK,
    RS_MAGIC_ISLE, RS_CIRCLE, RS_ZORCH, RS_COURT, RS_ROOMBASE,
    RS_GARDEROBE = RS_ROOMBASE, RS_DUNGEON_CELL, RS_TILED_CHAMBER, RS_CRYSTAL_CAVERN, RS_MASTER_BEDROOM,
    RS_STOREROOM, RS_CHARRED_ROOM, RS_MARBLE_HALL, RS_EERIE_CAVE, RS_TREASURE_CHAMBER,
    RS_SMOKED_ROOM, RS_APARTMENT, RS_ANTECHAMBER, RS_HAREM, RS_MULTIPURPOSE,
    RS_STALACTITES, RS_GREENHOUSE, RS_WATERCLOSET, RS_STUDY, RS_LIVING_ROOM,
    RS_DEN, RS_ABATOIR, RS_BOUDOIR, RS_STAR_CHAMBER, RS_MANMADE_CAVERN,
    RS_SEWER_CONTROL_ROOM, RS_HIGH_MAGIC_SHRINE, RS_MAGIC_LABORATORY, RS_PENTAGRAM_ROOM, RS_OMEGA_ROOM,
    RS_ROOMLAST, NUMROOMNAMES = RS_ROOMBASE-RS_ROOMLAST
};

enum EObjColor : attr_t {
    CLR_BLACK_BROWN		= COLOR_PAIR(5),
    CLR_BLACK_RED		= COLOR_PAIR(3),
    CLR_BLACK_WHITE		= COLOR_PAIR(4),
    CLR_BLUE_BLACK		= COLOR_PAIR(6),
    CLR_BLUE_WHITE		= COLOR_PAIR(7),
    CLR_BRIGHT_WHITE_BLACK	= COLOR_PAIR(17)|A_BOLD,
    CLR_BRIGHT_WHITE_BLUE	= COLOR_PAIR(18)|A_BOLD,
    CLR_BRIGHT_WHITE_RED	= COLOR_PAIR(19)|A_BOLD,
    CLR_BROWN_BLACK		= COLOR_PAIR(21),
    CLR_BROWN_RED		= COLOR_PAIR(23),
    CLR_BROWN_WHITE		= COLOR_PAIR(24),
    CLR_CYAN_BLACK		= COLOR_PAIR(8),
    CLR_GREEN_BLACK		= COLOR_PAIR(9),
    CLR_GREEN_BROWN		= COLOR_PAIR(12),
    CLR_GREEN_RED		= COLOR_PAIR(11),
    CLR_GREY_BLACK		= COLOR_PAIR(1)|A_BOLD,
    CLR_GREY_BROWN		= COLOR_PAIR(5)|A_BOLD,
    CLR_GREY_GREEN		= COLOR_PAIR(2)|A_BOLD,
    CLR_GREY_RED		= COLOR_PAIR(3)|A_BOLD,
    CLR_GREY_WHITE		= COLOR_PAIR(4)|A_BOLD,
    CLR_LIGHT_BLUE_BLACK	= COLOR_PAIR(6)|A_BOLD,
    CLR_LIGHT_BLUE_WHITE	= COLOR_PAIR(7)|A_BOLD,
    CLR_LIGHT_GREEN_BLACK	= COLOR_PAIR(9)|A_BOLD,
    CLR_LIGHT_GREEN_BLUE	= COLOR_PAIR(10)|A_BOLD,
    CLR_LIGHT_PURPLE_BLACK	= COLOR_PAIR(13)|A_BOLD,
    CLR_LIGHT_RED_BLACK		= COLOR_PAIR(15)|A_BOLD,
    CLR_LIGHT_RED_WHITE		= COLOR_PAIR(16)|A_BOLD,
    CLR_PURPLE_BLACK		= COLOR_PAIR(13),
    CLR_PURPLE_WHITE		= COLOR_PAIR(14),
    CLR_RED_BLACK		= COLOR_PAIR(15),
    CLR_RED_WHITE		= COLOR_PAIR(16),
    CLR_WHITE_BLACK		= COLOR_PAIR(17),
    CLR_WHITE_BLUE		= COLOR_PAIR(18),
    CLR_WHITE_BROWN		= COLOR_PAIR(20),
    CLR_WHITE_RED		= COLOR_PAIR(19),
    CLR_YELLOW_BLACK		= COLOR_PAIR(21)|A_BOLD,
    CLR_YELLOW_BLUE		= COLOR_PAIR(22)|A_BOLD,
    CLR_YELLOW_BROWN		= COLOR_PAIR(25)|A_BOLD,
    CLR_YELLOW_WHITE		= COLOR_PAIR(24)|A_BOLD
};

// objects, locations, and terrain; characters to draw
enum EObjchar : chtype {
    NULL_ITEM,
    SPACE		= (' ' | CLR_WHITE_BLACK),
    WALL		= ('#' | CLR_GREY_BLACK),
    PORTCULLIS		= ('7' | CLR_WHITE_BLACK),
    OPEN_DOOR		= ('|' | CLR_BROWN_BLACK),
    CLOSED_DOOR		= ('-' | CLR_BROWN_BLACK),
    WHIRLWIND		= ('6' | CLR_LIGHT_BLUE_BLACK),
    ABYSS		= ('0' | CLR_BLACK_BROWN),
    VOID_CHAR		= (' ' | CLR_WHITE_BLACK),
    LAVA		= ('`' | CLR_RED_BLACK),
    HEDGE		= ('\"'| CLR_GREEN_BLACK),
    WATER		= ('~' | CLR_BLUE_BLACK),
    FIRE		= (';' | CLR_LIGHT_RED_BLACK),
    TRAP		= ('^' | CLR_RED_BLACK),
    LIFT		= ('_' | CLR_BRIGHT_WHITE_BLACK),
    STAIRS_UP		= ('<' | CLR_WHITE_BLACK),
    STAIRS_DOWN		= ('>' | CLR_WHITE_BLACK),
    FLOOR		= ('.' | CLR_BROWN_BLACK),
    PLAYER		= ('@' | CLR_WHITE_BLACK),
    CORPSE		= ('+' | CLR_RED_BLACK),
    STATUE		= ('1' | CLR_GREY_BLACK),
    RUBBLE		= ('4' | CLR_GREY_BLACK),
    ALTAR		= ('8' | CLR_LIGHT_BLUE_BLACK),
    CASH		= ('$' | CLR_YELLOW_BLACK),
    PILE		= ('*' | CLR_BRIGHT_WHITE_BLACK),
    FOOD		= ('%' | CLR_BROWN_BLACK),
    WEAPON		= (')' | CLR_GREY_BLACK),
    MISSILEWEAPON	= ('(' | CLR_BROWN_BLACK),
    SCROLL		= ('?' | CLR_YELLOW_BLACK),
    POTION		= ('!' | CLR_LIGHT_GREEN_BLACK),
    ARMOR		= (']' | CLR_GREY_BLACK),
    SHIELD		= ('[' | CLR_BROWN_BLACK),
    CLOAK		= ('}' | CLR_CYAN_BLACK),
    BOOTS		= ('{' | CLR_BROWN_BLACK),
    STICK		= ('/' | CLR_BROWN_BLACK),
    RING		= ('=' | CLR_YELLOW_BLACK),
    THING		= ('\\'| CLR_WHITE_BLACK),
    ARTIFACT		= ('&' | CLR_YELLOW_BLACK),
    PLAINS		= ('-' | CLR_LIGHT_GREEN_BLACK),
    TUNDRA		= ('_' | CLR_GREY_BLACK),
    ROAD		= ('.' | CLR_BROWN_BLACK),
    MOUNTAINS		= ('^' | CLR_GREY_BLACK),
    PASS		= ('v' | CLR_BROWN_BLACK),
    RIVER		= ('~' | CLR_BLUE_BLACK),
    CITY		= ('O' | CLR_WHITE_BLACK),
    VILLAGE		= ('o' | CLR_WHITE_BLACK),
    FOREST		= ('(' | CLR_LIGHT_GREEN_BLACK),
    JUNGLE		= (')' | CLR_GREEN_BLACK),
    SWAMP		= ('=' | CLR_GREEN_BLACK),
    VOLCANO		= ('!' | CLR_RED_BLACK),
    CASTLE		= ('%' | CLR_GREY_BLACK),
    TEMPLE		= ('X' | CLR_BROWN_BLACK),
    CAVES		= ('*' | CLR_BLACK_BROWN),
    DESERT		= ('\"'| CLR_YELLOW_BLACK),
    CHAOS_SEA		= ('+' | CLR_LIGHT_PURPLE_BLACK),
    STARPEAK		= ('|' | CLR_LIGHT_BLUE_BLACK),
    DRAGONLAIR		= ('$' | CLR_BROWN_BLACK),
    MAGIC_ISLE		= ('&' | CLR_PURPLE_BLACK),
    CHAIR		= ('5' | CLR_BROWN_BLACK),
    SAFE		= ('3' | CLR_GREY_BLACK),
    FURNITURE		= ('2' | CLR_BROWN_BLACK),
    BED			= ('9' | CLR_CYAN_BLACK)
};

// Verbosity levels
enum EVerbosity : uint8_t { TERSE, MEDIUM, VERBOSE };

// Player.options bit indexes
enum EOption {
    OPT_BELLICOSE,
    OPT_JUMPMOVE,
    OPT_RUNSTOP,
    OPT_PICKUP,
    OPT_CONFIRM,
    OPT_PACKADD,
    OPT_COMPRESS,
    NUMTFOPTIONS,			// number of options with true/false values
    VERBOSITY_LEVEL = NUMTFOPTIONS,	// The slot number of the two options not in Player.options
    SEARCH_DURATION,
    NUMOPTIONS				// total number of player options
};

// Player.options bits
enum {
    BELLICOSE	= (1<<OPT_BELLICOSE),
    JUMPMOVE	= (1<<OPT_JUMPMOVE),
    RUNSTOP	= (1<<OPT_RUNSTOP),
    PICKUP	= (1<<OPT_PICKUP),
    CONFIRM 	= (1<<OPT_CONFIRM),
    PACKADD 	= (1<<OPT_PACKADD),
    COMPRESS	= (1<<OPT_COMPRESS)
};

// running sum of itemtypes, for indexing into Objects array
// This has to be changed whenever an item is added
enum {
    THINGID, THING_DOOR_OPENER = THINGID, THING_UNCUT_DIAMONDS, THING_LOCKPICK, THING_HUMONGOUS_EMERALD, THING_GOLD_INGOT,
    THING_BEARER_BONDS, THING_SALT_WATER, THING_KEY, THING_TORCH, THING_GROT1,
    THING_GROT2, THING_GROT3, THING_GROT4, THING_GROT5, THING_GROT6,
    THING_GROT7, THING_JUSTICIAR_BADGE, THING_DART_TRAP_COMPONENT, THING_ACID_TRAP_COMPONENT, THING_SNARE_TRAP_COMPONENT,
    THING_FIRE_TRAP_COMPONENT, THING_TELEPORT_TRAP_COMPONENT, THING_SLEEP_TRAP_COMPONENT, THING_DISINTEGRATE_TRAP_COMPONENT, THING_ABYSS_TRAP_COMPONENT,
    THING_MANADRAIN_TRAP_COMPONENT,

    FOODID, FOOD_RATION = FOODID, FOOD_LEMBAS, FOOD_STIMTAB, FOOD_POWTAB, FOOD_NUTRITAB,
    FOOD_IMMUNOTAB, FOOD_JELLY_BABY, FOOD_POISONED_RATION, FOOD_SZECHUAN_PEPPER, FOOD_PIZZA,
    FOOD_HUNAN_TAKEOUT, FOOD_TWINKIES, FOOD_CHEESE_DOODLES, FOOD_PECAN_TWIRLS, FOOD_VEAL_PICCATTA,
    FOOD_GRAIN,

    SCROLLID, SCROLL_BLANK = SCROLLID, SCROLL_SPELLS, SCROLL_BLESSING, SCROLL_ACQUIREMENT, SCROLL_ENCHANTMENT,
    SCROLL_TELEPORT, SCROLL_WISHING, SCROLL_CLARIVOYANCE, SCROLL_DISPLACEMENT, SCROLL_IDENTIFICATION,
    SCROLL_HINT_SHEET, SCROLL_JANES_GUIDE, SCROLL_ALERTNESS, SCROLL_FLUX, SCROLL_CHARGING,
    SCROLL_WARP, SCROLL_SELF_KNOWLEDGE, SCROLL_LAW, SCROLL_HEROISM, SCROLL_TRUESIGHT,
    SCROLL_DISPELLING, SCROLL_SUMMON, SCROLL_LIGHT, SCROLL_DEFLECTION,

    POTIONID, POTION_WATER = POTIONID, POTION_HEALING, POTION_OBJECT_DETECTION, POTION_MONSTER_DETECTION, POTION_NETURALIZE_POISON,
    POTION_OF_ALERTNESS, POTION_OF_SPEED, POTION_RESTORATION, POTION_AZOTH, POTION_REGENERATION,
    POTION_OF_INVISIBILITY, POTION_BREATHING, POTION_AUGMENTATION, POTION_BANISHING_FEAR, POTION_CHAOS,
    POTION_ACCURACY, POTION_OF_LEVITATION, POTION_OF_CURING,

    WEAPONID, WEAPON_DAGGER = WEAPONID, WEAPON_SHORT_SWORD, WEAPON_BROAD_SWORD, WEAPON_BASTARD_SWORD, WEAPON_KATANA,
    WEAPON_GREAT_SWORD, WEAPON_EPEE, WEAPON_RAPIER, WEAPON_ESTOC, WEAPON_CUTLASS,
    WEAPON_HAND_AXE, WEAPON_BATTLE_AXE, WEAPON_GREAT_AXE, WEAPON_MACE, WEAPON_WAR_HAMMER,
    WEAPON_MORNINGSTAR, WEAPON_FLAIL, WEAPON_CLUB, WEAPON_QUARTERSTAFF, WEAPON_SPEAR,
    WEAPON_HALBERD, WEAPON_TRIDENT, WEAPON_WHIP, WEAPON_LIGHTSABRE, WEAPON_DEMONBLADE,
    WEAPON_MACE_OF_DISRUPTION, WEAPON_LONGBOW, WEAPON_CROSSBOW, WEAPON_ARROW, WEAPON_BOLT,
    WEAPON_BOLA, WEAPON_VORPAL_SWORD, WEAPON_DESECRATOR, WEAPON_FIRESTAR, WEAPON_DEFENDER,
    WEAPON_VICTRIX, WEAPON_GOBLIN_HEWER, WEAPON_SCYTHE, WEAPON_GIANT_CLUB, WEAPON_SCYTHE_OF_DEATH,
    WEAPON_ACID_WHIP,

    ARMORID, ARMOR_FLAK_JACKET = ARMORID, ARMOR_SOFT_LEATHER, ARMOR_CUIRBOUILLI, ARMOR_RING_MAIL, ARMOR_CHAIN_MAIL,
    ARMOR_SCALE_MAIL, ARMOR_PARTIAL_PLATE, ARMOR_FULL_PLATE, ARMOR_PLATE, ARMOR_LAMELLAR_PLATE,
    ARMOR_MITHRIL_CHAIN, ARMOR_MITHRIL_PLATE, ARMOR_DRAGONSCALE, ARMOR_PRISMATRIX, ARMOR_POWERED_COMBAT,
    ARMOR_POWERED_SCOUT, ARMOR_DEMONHIDE,

    SHIELDID, SHIELD_BUCKLER = SHIELDID, SHIELD_SMALL_ROUND, SHIELD_LARGE_ROUND, SHIELD_HEATER, SHIELD_HOPLON,
    SHIELD_TOWER, SHIELD_PLASTEEL, SHIELD_OF_DEFLECTION,

    CLOAKID, CLOAK_OF_WOOL = CLOAKID, CLOAK_OF_NEGIMMUNITY, CLOAK_OF_INVISIBILITY, CLOAK_OF_SKILL, CLOAK_OF_PROTECTION,
    CLOAK_OF_DISPLACEMENT, CLOAK_OF_TRUESIGHT,

    BOOTID, BOOTS_OF_SPEED = BOOTID, BOOTS_OF_HEROISM, BOOTS_OF_LEVITATION, BOOTS_OF_AGILITY, BOOTS_OF_JUMPING,
    BOOTS_OF_LEATHER, BOOTS_SEVEN_LEAGUE,

    RINGID, RING_OF_TRUESIGHT = RINGID, RING_BURDEN, RING_OF_STRENGTH, RING_OF_GAZE_IMMUNITY, RING_OF_FIRE_RESISTANCE,
    RING_OF_BRASS_AND_GLASS, RING_OF_POISON_RESISTANCE, RING_OF_REGENERATION, RING_OF_SELF_KNOWLEDGE, RING_OF_PROTECTION,

    STICKID, STAFF_OF_FIREBOLTS = STICKID, WALKING_STICK, STAFF_OF_SLEEP, WAND_OF_BALL_LIGHTNING, ROD_OF_SUMMONING,
    ROD_OF_HIDING, STAFF_OF_LIGHTNING_BOLTS, WAND_OF_FIREBALLS, ROD_OF_DISINTEGRATION, STAFF_OF_DISRUPTION,
    SNOWBALL_STICK, STAFF_OF_MISSILES, ROD_OF_APPORTATION, STAFF_OF_DISPELLING, STAFF_OF_HEALING,
    WAND_OF_POLYMORPH, WAND_OF_FEAR,

    ARTIFACTID, ORB_OF_MASTERY = ARTIFACTID, ORB_OF_FIRE, ORB_OF_WATER, ORB_OF_EARTH, ORB_OF_AIR,
    ORB_BURNT_OUT, CRYSTAL_BALL, HOLY_HAND_GRENADE, AMULET_OF_YENDOR, KEY_OF_KOLWYNIA,
    POTION_OF_DEATH, STAFF_OF_ENCHANTMENT, TELEPORTATION_HELM, POTION_OF_LIFE, HOLY_SYMBOL_OF_ODIN,
    HOLY_SYMBOL_OF_SET, HOLY_SYMBOL_OF_ATHENA, HOLY_SYMBOL_OF_HECATE, HOLY_SYMBOL_OF_DRUIDISM, HOLY_SYMBOL_OF_DESTINY,
    JUGGERNAUT_OF_KARNAK, STAR_GEM, SCEPTRE_OF_HIGH_MAGIC, AMULET_OF_THE_PLAINS,

    CASHID,
    CORPSEID,	// Corpse's aux field is monster id
    TOTALITEMS,

    NUMTHINGS	= FOODID - THINGID,
    NUMFOODS	= SCROLLID - FOODID,
    NUMSCROLLS	= POTIONID - SCROLLID,
    NUMPOTIONS	= WEAPONID - POTIONID,
    NUMWEAPONS	= ARMORID - WEAPONID,
    NUMARMOR	= SHIELDID - ARMORID,
    NUMSHIELDS	= CLOAKID - SHIELDID,
    NUMCLOAKS	= BOOTID - CLOAKID,
    NUMBOOTS	= RINGID - BOOTID,
    NUMRINGS	= STICKID - RINGID,
    NUMSTICKS	= ARTIFACTID - STICKID,
    NUMARTIFACTS= CASHID - ARTIFACTID,

    NO_THING = uint8_t(-1)
};

// describing unique items and monsters
enum EUniqueness { COMMON, UNIQUE_UNMADE, UNIQUE_MADE, UNIQUE_TAKEN };
enum EObjectAttr { OBJECT_KNOWN = (1<<2), OBJECT_UNIQUENESS = OBJECT_KNOWN-1 };

// general item function id's
enum {
    // note some of these functions are for other types of items too
    I_NO_OP, I_NOTHING,
    // scroll functions
    I_BLESS = 101, I_ACQUIRE, I_ENCHANT, I_TELEPORT, I_WISH,
    I_CLAIRVOYANCE, I_DISPLACE, I_ID, I_FIREFLASH, I_SPELLS,
    I_JANE_T, I_ALERT, I_FLUX, I_CHARGE, I_WARP, I_KNOWLEDGE,
    I_LAW, I_HINT, I_HERO, I_TRUESIGHT, I_ILLUMINATE,
    I_DEFLECT,
    // potion functions
    I_HEAL = 201, I_OBJDET, I_MONDET, I_SLEEP_SELF, I_RESTORE,
    I_NEUTRALIZE_POISON, I_SPEED, I_AZOTH, I_REGENERATE, I_INVISIBLE,
    I_BREATHING, I_FEAR_RESIST, I_AUGMENT, I_CHAOS, I_ACCURACY,
    I_LEVITATION, I_CURE,
    // stick functions
    I_FIREBOLT = 301, I_LBOLT, I_MISSILE, I_SLEEP_OTHER, I_FIREBALL,
    I_LBALL, I_SUMMON, I_HIDE, I_DISRUPT, I_DISINTEGRATE,
    I_SNOWBALL, I_APPORT, I_DISPEL, I_POLYMORPH, I_FEAR,
    // food functions
    I_FOOD = 401, I_LEMBAS, I_STIM, I_POW, I_IMMUNE,
    I_POISON_FOOD, I_CORPSE, I_PEPPER_FOOD, I_CANNIBAL, I_INEDIBLE,
    // boots functions
    I_PERM_SPEED = 501, I_PERM_HERO, I_PERM_LEVITATE, I_PERM_AGILITY,
    I_BOOTS_JUMPING, I_BOOTS_7LEAGUE,
    // cloak functions
    I_PERM_DISPLACE = 601, I_PERM_NEGIMMUNE, I_PERM_INVISIBLE, I_PERM_ACCURACY,
    I_PERM_PROTECTION, I_PERM_TRUESIGHT,
    // ring functions
    I_PERM_VISION = 701, I_PERM_BURDEN, I_PERM_STRENGTH, I_PERM_GAZE_IMMUNE,
    I_PERM_FIRE_RESIST, I_PERM_POISON_RESIST, I_PERM_REGENERATE, I_PERM_KNOWLEDGE,
    // armor functions
    I_PERM_ENERGY_RESIST = 801, I_PERM_BREATHING, I_PERM_FEAR_RESIST, I_NORMAL_ARMOR,
    // artifact functions
    I_ORBFIRE = 901, I_ORBWATER, I_ORBEARTH, I_ORBAIR, I_ORBMASTERY,
    I_ORBDEAD, I_CRYSTAL, I_ANTIOCH, I_KOLWYNIA, I_DEATH,
    I_ENCHANTMENT, I_HELM, I_LIFE, I_JUGGERNAUT, I_SYMBOL,
    I_STARGEM, I_SCEPTRE, I_PLANES,
    // weapons functions
    I_NORMAL_WEAPON = 1001, I_LIGHTSABRE, I_DEMONBLADE, I_MACE_DISRUPT, I_TANGLE,
    I_ARROW, I_BOLT, I_VORPAL, I_DESECRATE, I_FIRESTAR,
    I_DEFEND, I_VICTRIX, I_EMPIRE, I_SCYTHE, I_ACIDWHIP,
    // thing functions
    I_PICK = 1101, I_KEY, I_SHOVEL, I_EXCAVATOR, I_PERM_ILLUMINATE,
    I_TRAP, I_RAISE_PORTCULLIS,
    // shield functions
    I_PERM_DEFLECT = 1201,
    I_NORMAL_SHIELD
};

// monster function ids
// Its conceivable for a function of one type to be called when another
// would usually occur. A monster's special function may be an extra move,
// for example.
enum {
    M_NO_OP = -1,
    // talk functions
    M_TALK_STUPID = 101,
    M_TALK_SILENT,
    M_TALK_HUNGRY,
    M_TALK_GREEDY,
    M_TALK_TITTER,
    M_TALK_MAN,
    M_TALK_ROBOT,
    M_TALK_EVIL,
    M_TALK_BURBLE,
    M_TALK_SLITHY,
    M_TALK_MIMSY,
    M_TALK_SEDUCTOR,
    M_TALK_MP,
    M_TALK_IM,
    M_TALK_GUARD,
    M_TALK_GHOST,
    M_TALK_HINT,
    M_TALK_BEG,
    M_TALK_EF,
    M_TALK_GF,
    M_TALK_MORGON,
    M_TALK_LB,
    M_TALK_DEMONLOVER,
    M_TALK_ASSASSIN,
    M_TALK_NINJA,
    M_TALK_THIEF,
    M_TALK_MERCHANT,
    M_TALK_HORSE,
    M_TALK_PARROT,
    M_TALK_ANIMAL,
    M_TALK_HYENA,
    M_TALK_SERVANT,
    M_TALK_SCREAM,
    M_TALK_DRUID,
    M_TALK_ARCHMAGE,
    M_TALK_PRIME,
    // ability functions
    M_SP_SURPRISE = 201,
    M_SP_MP,
    M_SP_THIEF,
    M_SP_AGGRAVATE,
    M_SP_POISON_CLOUD,
    M_SP_HUGE,
    M_SP_SUMMON,
    M_SP_ILLUSION,
    M_SP_FLUTTER,
    M_SP_ESCAPE,
    M_SP_SPELL,
    M_SP_EXPLODE,
    M_SP_DEMON,
    M_SP_ACID_CLOUD,
    M_SP_WHIRL,
    M_SP_GHOST,
    M_SP_WHISTLEBLOWER,
    M_SP_EATER,
    M_SP_LAWBRINGER,
    M_SP_DRAGONLORD,
    M_SP_DE,
    M_SP_DEMONLOVER,
    M_SP_SEDUCTOR,
    M_SP_MASTER,
    M_SP_WYRM,
    M_SP_BLACKOUT,
    M_SP_BOG,
    M_SP_MERCHANT,
    M_SP_WERE,
    M_SP_SERVANT,
    M_SP_AV,
    M_SP_LW,
    M_SP_SWARM,
    M_SP_ANGEL,
    M_SP_MB,
    M_SP_MIRROR,
    M_SP_RAISE,
    M_SP_DEATH,
    M_SP_COURT,
    M_SP_LAIR,
    M_SP_PRIME,
    // rangestrike functions
    M_STRIKE_MISSILE = 301,
    M_STRIKE_FBOLT,
    M_STRIKE_LBALL,
    M_STRIKE_FBALL,
    M_STRIKE_BLIND,
    M_STRIKE_SNOWBALL,
    M_STRIKE_MASTER,
    M_STRIKE_SONIC,
    // combat functions
    M_MELEE_NORMAL = 401,
    M_MELEE_FIRE,
    M_MELEE_DRAGON,
    M_MELEE_MP,
    M_MELEE_ELEC,
    M_MELEE_POISON,
    M_MELEE_NG,
    M_MELEE_SUCCUBUS,
    M_MELEE_SPIRIT,
    M_MELEE_DISEASE,
    M_MELEE_SLEEP,
    M_MELEE_COLD,
    M_MELEE_MASTER,
    M_MELEE_GRAPPLE,
    M_MELEE_DEATH,
    // movement functions
    M_MOVE_NORMAL = 501,
    M_MOVE_FLUTTER,
    M_MOVE_TELEPORT,
    M_MOVE_FOLLOW,
    M_MOVE_RANDOM,
    M_MOVE_SMART,
    M_MOVE_SPIRIT,
    M_MOVE_SCAREDY,
    M_MOVE_CONFUSED,
    M_MOVE_ANIMAL,
    M_MOVE_LEASH
};

// Some monster ID's : (Those that are explicitly named in code)
// Actually, there are still many magic constants floating around.
// Eventually I'll get around to making each monster's id a constant....
// done, thanks to David Given.
//
// MLx -> index to Monsters starting for level x
// MLx -> number of monsters of level x or less
// NML_x -> number of monsters of level x
// NML-X must be changed whenever a monster is added
// This whole thing MUST be repaired.  Monster levels must
// be represented elsewhere.
//
enum {
    RANDOM = -1,

    ML0, HORNET = ML0, MEND_PRIEST, ITIN_MERCH, GUARD, NPC,
    SHEEP, MERCHANT, ZERO_NPC, HISCORE_NPC,

    ML1, GRUNT = ML1, TSETSE, FNORD, SEWER_RAT, AGGRAVATOR,
    BLIPPER, GOBLIN, PHANTASTICON, ROBOT, GEEK,
    BOROGROVE, QUAIL, BADGER, HAWK, DEER,
    CAMEL, ANTEATER, BUNNY, TROUT, BASS,
    PARROT, HYENA,

    ML2, APPR_NINJA = ML2, NIGHT_GAUNT, SNEAK_THIEF, EYE, TOVE,
    NASTY, GHOST, ENCHANTOR, MURK, GOBLIN_CHIEF,
    WOLF, ANT, ELEPHANT, HORSE,

    ML3, SALAMANDER = ML3, CATOBLEPAS, L_FDEMON, ACID_CLOUD, PHANTOM,
    GOBLIN_KING, PTERODACTYL, GOBLIN_SHAMAN, LION, BRIGAND,
    BEAR, MAMBA, MANOWAR, WEREHUMAN, THOUGHTFORM,

    ML4, MANTICORE = ML4, TASMANIAN, AUTO_MINOR, DENEBIAN, JUBJUB,
    HAUNT, INCUBUS, SATYR, CROC, TORPOR,
    DOBERMAN, FUZZY, SERV_LAW, SERV_CHAOS, SWARM,
    BAN_SIDHE, GRUE, GENIN,

    ML5, DRAGONETTE = ML5, TESLA, WYVERN, CATEAGLE, FROST_DEMON,
    SPECTRE, NECROMANCER, SHADOW, BOGTHING, ASTRAL_VAMP,
    LAVA_WORM, MANABURST, OUTER_DEMON, MIRRORSHADE,

    ML6, FIRE_ELEM = ML6, AIR_ELEM, WATER_ELEM, EARTH_ELEM, BANDERSNATCH,
    LICHE, TRITON, MAST_THIEF, TRICER, RAKSHASA,
    DEMON_SERP, ANGEL, CHUNIN,

    ML7, BEHEMOTH = ML7, NAZGUL, UNICORN, ROUS, ILL_FIEND,
    GREAT_WYRM, FLAME_DEV, LURKER, SANDMAN, MIRRORMAST,
    ELDER_GRUE, LOATHLY, ZOMBIE, RICOCHET, INNER_DEMON,

    ML8, GOOD_FAIRY = ML8, BAD_FAIRY, AUTO_MAJOR, DRAGON, JABBERWOCK,
    FDEMON_L, TIGERSHARK, JONIN, SHADOW_SLAY, MIL_PRIEST,
    COMA, HIGH_ANGEL,

    ML9, JOTUN = ML9, INVIS_SLAY, KING_WYV, DEATHSTAR, THAUMATURGIST,
    VAMP_LORD, ARCHANGEL, DEMON_PRINCE,

    ML10, DEATH = ML10, EATER, LAWBRINGER, DRAGON_LORD, DEMON_EMP,
    LORD_EARTH, LORD_AIR, LORD_WATER, LORD_FIRE, ELEM_MASTER,

    NUMMONSTERS,

    NML_0 = ML1-ML0, NML_1 = ML2-ML1, NML_2 = ML3-ML2,
    NML_3 = ML4-ML3, NML_4 = ML5-ML4, NML_5 = ML6-ML5,
    NML_6 = ML7-ML6, NML_7 = ML8-ML9, NML_8 = ML9-ML8,
    NML_9 = ML10-ML9, NML_10 = NUMMONSTERS-ML10
};

// location functions
enum {
    L_NO_OP, L_LIFT, L_BALANCESTONE, L_FIRE, L_WHIRLWIND,
    L_VOIDSTONE, L_WARNING, L_ARENA_EXIT, L_HOUSE_EXIT, L_SAFE
};

// city level shop and guild functions
// following are those in CitySiteList
enum ELocation {
    CITYSITEBASE = 10,
    L_CHARITY, L_ARMORER, L_CLUB, L_GYM, L_THIEVES_GUILD,
    L_COLLEGE, L_HEALER, L_CASINO, L_TAVERN, L_MERC_GUILD,
    L_ALCHEMIST, L_SORCERORS, L_CASTLE, L_ARENA, L_DPW,
    L_LIBRARY, L_PAWN_SHOP, L_BANK, L_CONDO, L_ORACLE,
    L_ORDER, L_DINER, L_COMMANDANT, L_CRAP, L_TEMPLE,
    L_COUNTRYSIDE, L_BROTHEL, L_JAIL, L_TEMPLE_WARNING, L_LAWSTONE,
    L_CHAOSTONE,
    // final abyss sites ignore levitation
    L_EARTH_STATION, L_FIRE_STATION, L_WATER_STATION, L_AIR_STATION, L_VOID_STATION,
    L_VOID, L_VOICE1, L_VOICE2, L_VOICE3, L_SACRIFICESTONE,
    // circle hq sites
    L_TOME1, L_TOME2, L_ENTER_CIRCLE, L_CIRCLE_LIBRARY,
    // other site functions
    L_DRUID, L_ALTAR, L_GARDEN, L_ADEPT, L_SEWER,
    L_OMEGA, L_CARTOGRAPHER, L_STABLES, L_COMMONS, L_GRANARY,
    L_MAZE, L_HOVEL, L_HOUSE, L_MANSION, L_OCCUPIED_HOUSE,
    L_TACTICAL_EXIT, L_VAULT, L_CEMETARY, L_THRONE, L_ESCALATOR,
    L_ENTER_COURT, L_TRIFID, L_FINAL_ABYSS, L_RAISE_PORTCULLIS, L_MINDSTONE,
    // random sites
    L_CHAOS, L_WATER, L_LAVA, L_MAGIC_POOL, L_PORTCULLIS_TRAP,
    L_DROP_EVERY_PORTCULLIS = L_PORTCULLIS_TRAP+2, L_PORTCULLIS,
    // traps
    L_TRAP_DART, L_TRAP_PIT, L_TRAP_DOOR, L_TRAP_SNARE, L_TRAP_BLADE,
    L_TRAP_FIRE, L_TRAP_TELEPORT, L_TRAP_DISINTEGRATE, L_TRAP_SLEEP_GAS, L_TRAP_ACID,
    L_TRAP_MANADRAIN, L_TRAP_ABYSS, L_TRAP_SIREN,
    // more random sites
    L_STATUE_WAKE, L_STATUE_RANDOM, L_HEDGE, L_RUBBLE, L_ABYSS,
    // Computed limits
    NUMCITYSITES = L_JAIL-CITYSITEBASE,
    LEVITATION_AVOIDANCE = L_CHAOS-1,	// above LEVITATION_AVOIDANCE, no effect if player is levitating
    TRAP_BASE = L_TRAP_DART,
    NUMTRAPS = L_TRAP_SIREN-TRAP_BASE+1
};

// player possession slots
// slot 0 should not be filled when out of inventory_control()
enum {
    O_UP_IN_AIR, O_READY_HAND, O_WEAPON_HAND,
    O_LEFT_SHOULDER, O_RIGHT_SHOULDER,
    O_BELT1, O_BELT2, O_BELT3, O_SHIELD, O_ARMOR,
    O_BOOTS, O_CLOAK, O_RING1, O_RING2, O_RING3, O_RING4
};

// Curses defs
enum { COLOR_DEFAULT = -1 };
enum { KEY_ESCAPE = 0x1b };
struct KEY_CTRL_MOD {	inline constexpr int operator| (char c) { return c-'a'+1; } };
#define KEY_CTRL	KEY_CTRL_MOD()

// structure definitions

struct citysite {
    bool	known;
    uint8_t	_spacer;
    uint8_t	x;
    uint8_t	y;
};

struct spell { uint8_t powerdrain; };

struct object_data {
    uint8_t id;
    uint8_t level;
    uint16_t weight;
    int8_t plus;
    uint8_t fragility;
    int16_t charge;
    uint8_t dmg;
    uint8_t hit;
    int16_t aux;
    uint16_t basevalue;
    uint8_t used;
    int8_t blessing;
    uint8_t type;
    uint8_t uniqueness;
    int16_t usef;
    chtype objchar;
    const char* objstr;
    const char* truename;
    const char* cursestr;
    static const constexpr streamsize stream_alignment = 2;
};

struct object : public object_data {
public:
    uint16_t number;
    int8_t x;
    int8_t y;
public:
    constexpr	object (void) : object_data(),number(1),x(0),y(0) {}
		object (int8_t nx, int8_t ny, unsigned tid, uint16_t n = 1);
		object (const object_data& o)	: object_data(o), number(1), x(0), y(0) {}
    object&	operator= (const object_data& o){ *static_cast<object_data*>(this) = o; return *this; }
    bool	operator== (const object& v) const;
    void	read (istream& is);
    template <typename Stm>
    void	write (Stm& os) const;
    static const constexpr streamsize stream_alignment = 2;
};
using pob = object*;

struct monster_data {
    uint8_t id;
    uint8_t level;
    int16_t hp;
    uint8_t hit;
    uint8_t ac;
    uint8_t dmg;
    uint8_t sense;
    uint8_t wakeup;
    uint8_t speed;
    uint8_t sleep;
    uint8_t treasure;
    uint16_t xpv;
    uint16_t corpseweight;
    uint16_t corpsevalue;
    uint8_t transformid;
    uint8_t startthing;
    uint8_t uniqueness;
    int16_t talkf;
    int16_t movef;
    int16_t meleef;
    int16_t strikef;
    int16_t specialf;
    uint16_t status;
    uint16_t immunity;
    chtype monchar;
    const char* monstring;
    const char* corpsestr;
    const char* meleestr;
    static const constexpr streamsize stream_alignment = 4;
};

struct monster : public monster_data {
    vector<object>	possessions;
    uint16_t		aux1;
    uint16_t		aux2;
    bool		attacked;
    uint8_t		click;
    uint8_t		x;
    uint8_t		y;
public:
    inline monster&	operator= (const monster_data& v)	{ *static_cast<monster_data*>(this) = v; possessions.clear(); return *this; }
    void		read (istream& is);
    template <typename Stm>
    void		write (Stm& os) const;
    static const constexpr streamsize stream_alignment = 4;
    const char*		name (void) const PURE;
    const char*		by_name (void) const PURE;
    inline const char*	definite_article (void) const PURE	{ return uniqueness == COMMON ? "the " : ""; }
    inline void		pickup (const object& o)		{ possessions.push_back(o); }
};

struct player_pod {
    uint32_t	x,y;
    uint32_t	xp;
    uint32_t	options;
    uint32_t	cash;
    int16_t	absorption;
    int16_t	alignment;
    int16_t	defense;
    int16_t	dmg;
    int16_t	food;
    int16_t	hit;
    int16_t	hp;
    int16_t	level;
    uint16_t	itemweight;
    uint16_t	mana;
    uint16_t	maxhp;
    uint16_t	maxmana;
    uint16_t	maxweight;
    uint8_t	agi;
    uint8_t	con;
    uint8_t	dex;
    uint8_t	iq;
    uint8_t	pow;
    uint8_t	str;
    uint8_t	maxagi;
    uint8_t	maxcon;
    uint8_t	maxdex;
    uint8_t	maxiq;
    uint8_t	maxpow;
    uint8_t	maxstr;
    uint8_t	patron;
    uint8_t	speed;
    uint8_t	click;
    char	preference;
public:
    inline	player_pod (void)
		    :x(0),y(0),xp(0),options(0),cash(0),absorption(0),alignment(0)
		    ,defense(0),dmg(0),food(0),hit(0),hp(0),level(0),itemweight(0)
		    ,mana(0),maxhp(0),maxmana(0),maxweight(0),agi(0),con(0),dex(0)
		    ,iq(0),pow(0),str(0),maxagi(0),maxcon(0),maxdex(0),maxiq(0)
		    ,maxpow(0),maxstr(0),patron(0),speed(0),click(0),preference(0) {}
    inline void	read (istream& is)	{ is.read (this, sizeof(*this)); }
    template <typename Stm>
    inline void	write (Stm& os) const	{ os.write (this, sizeof(*this)); }
};

struct player : public player_pod {
    int8_t		rank [NUMGUILDS];
    uint16_t		immunity [NUMIMMUNITIES];
    uint16_t		status [NUMSTATI];
    uint16_t		guildxp [NUMGUILDS];
    string		name;
    string		meleestr;
    object		possessions [MAXITEMS];
    vector<object>	pack;
public:
			player (void);
    void		add_possession (unsigned slot, const object& o);
    void		swap_possessions (unsigned slot);
    void		remove_possession (unsigned slot, unsigned number = -1);
    void		remove_possession (object* o, unsigned number = -1);
    void		remove_all_possessions (void);
    inline bool		has_possession (unsigned slot) const	{ return possessions[slot].id != NO_THING; }
    inline bool		immune_to (EDamageType dtype) const	{ return immunity[dtype] > 0; }
    inline uint16_t	calcmana (void) const			{ return pow*(level+1); }
    bool		on_sanctuary (void) const;
    void		calc_melee (void);
    void		read (istream& is);
    template <typename Stm>
    void		write (Stm& os) const;
    static const constexpr streamsize stream_alignment = 4;
};

// dungeon locations
struct location {
    chtype	locchar;	// terrain type
    uint8_t	aux;		// signifies various things
    uint8_t	p_locf;		// function executed when moved on
    uint8_t	lstatus;	// seen,stopsrun,lit,secret,
    uint8_t	roomnumber;	// so room can be named
public:
    inline bool	operator== (const location& l) const	{ return locchar == l.locchar; }
    chtype	showchar (void) const noexcept PURE;
};

class level {
    vector<location>	_site;	// dungeon data
public:
    vector<monster>	mlist;		// List of monsters on level
    vector<object>	things;		// List of objects on level
    EEnvironment	environment;	// where kind of level is this?
    uint8_t		width;
    uint8_t		height;
    uint8_t		lastx;		// Last player position
    uint8_t		lasty;
    uint8_t		depth;		// which level is this
    uint8_t		generated;	// has the level been made (visited) yet?
    uint8_t		tunnelled;	// amount of tunnelling done on this level
public:
			level (void);
    void		clear (void);
    void		resize (uint8_t x, uint8_t y)	{ width = x; height = y; }
    bool		ok_to_free (void) const;
    bool		IsTransient (void) const;
    uint8_t		MaxDepth (void) const;
    void		read (istream& is);
    template <typename Stm>
    void		write (Stm& os) const;
    static const constexpr streamsize stream_alignment = 4;
    template <typename SiteFunc>
    static inline void	load_map (EEnvironment e, const char* edata, SiteFunc sf);
    void		Generate (EEnvironment e, uint8_t subeid);
    inline void		Generate (void)	{ Generate (environment, IsDungeon() ? depth : generated); }
    monster*		creature (int x, int y);
    object*		thing (int x, int y);
    void		make_thing (int x, int y, unsigned tid, unsigned n = RANDOM);
    void		remove_things (int x, int y);
    void		add_thing (int x, int y, const object& o, unsigned n = RANDOM);
    void		tunnelcheck (void);
    inline location&	site (unsigned x, int y)	{ return _site[y*MAXWIDTH+x]; }
    const location&	site (unsigned x, int y) const	{ return const_cast<level*>(this)->site(x,y); }
    inline bool		IsDungeon (void) const		{ return unsigned(environment-E_FIRST_DUNGEON) < E_NUMDUNGEONS; }
    inline uint8_t	VillageId (void) const		{ return generated; }
    inline void		SetVillageId (uint8_t id)	{ generated = id; }
    inline uint8_t	TempleDeity (void) const	{ return generated; }
    inline void		SetTempleDeity (uint8_t id)	{ generated = id; }
};
using plv = level*;

class CWorld {
    enum { c_MaxLevels = 32 };
    using lvec_t = vector<level>;
public:
    inline		CWorld (void)			: _levels() {}
    void		LoadEnvironment (EEnvironment e, uint8_t subeid = 0);
    lvec_t::iterator	FindEnvironment (EEnvironment e, uint8_t subeid = 0);
    EEnvironment	LastEnvironment (void) const;
    void		DeleteLevel (EEnvironment e, uint8_t subeid);
    void		MoveInCountry (uint8_t x, uint8_t y);
    void		read (istream& is)		{ is >> _levels; }
    template <typename Stm>
    void		write (Stm& os) const		{ os << _levels; }
    static const constexpr streamsize stream_alignment = 4;
private:
    lvec_t		_levels;
};
