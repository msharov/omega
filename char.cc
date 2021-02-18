// Omega is free software, distributed under the ISC license

#include "glob.h"
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

//----------------------------------------------------------------------

static void initstats(void);
static void save_omegarc(void);
static bool load_omegarc (const char* filename);
static void user_character_stats(void);
static void omegan_character_stats(void);
static bool personal_question_yn (const char* q);

//----------------------------------------------------------------------

// set player to begin with
player::player (void)
: player_pod()
, rank{}
, immunity{}
, status{}
, guildxp{}
, name("Player")
, meleestr()
, possessions{}
, pack()
{
    meleestr.append ('\0', 64);
}

void initplayer (void)
{
    Player.itemweight = 0;
    Player.food = 36;
    Player.options = RUNSTOP| CONFIRM| COMPRESS;
    fill (Player.immunity, 0);
    fill (Player.status, 0);
    fill (Player.rank, 0);
    fill (Player.guildxp, 0);
    for (auto& p : Player.possessions)
	p.id = NO_THING;
    Player.pack.clear();
    Player.patron = 0;
    Player.alignment = 0;
    Player.cash = 250;

    bool oldchar = false;
    snprintf (ARRAY_BLOCK(Str1), OMEGA_PLAYER_FILE, getenv("HOME"));
    if (0 == access (Str1, R_OK) && personal_question_yn ("Use saved character? [yn] ")) {
	clearmsg();
	if (load_omegarc (Str1))
	    oldchar = true;
    }
    if (!oldchar)
	initstats();
    Searchnum = max<uint8_t> (1, min<uint8_t> (9, Searchnum));
    Player.hp = Player.maxhp = Player.maxcon;
    Player.mana = Player.maxmana = Player.calcmana();
    Player.click = 1;
    Player.meleestr = "CCBC";
    calc_melee();
    ScreenOffset = -100;	// to force a redraw
}

static void initstats (void)
{
    do {
	mprint ("Do you want to run a character [c] or play yourself [p]?");
	char response = (char) mcigetc();
	if (response == 'c')
	    omegan_character_stats();
	else if (response == 'p') {
	    user_character_stats();
	    mprint ("Do you want to save this set-up to .omegarc in your home directory? [yn] ");
	    if (ynq() == 'y') {
		mprint ("First, set options.");
		setoptions();
		save_omegarc();
	    }
	    displayfile (Data_Intro);
	} else
	    continue;
    } while (false);
    xredraw();
}

static inline void omegarc_read_player_static (istream& stm)
{
    stm >> Player.iq >> Player.maxiq
	>> Player.pow >> Player.maxpow
	>> Player.dex >> Player.maxdex
	>> Player.agi >> Player.maxagi
	>> Player.str >> Player.maxstr
	>> Player.con >> Player.maxcon
	>> Player.cash >> Player.options
	>> Player.preference;
}

template <typename Stm>
static inline void omegarc_write_player_static (Stm& stm)
{
    stm << Player.iq << Player.maxiq
	<< Player.pow << Player.maxpow
	<< Player.dex << Player.maxdex
	<< Player.agi << Player.maxagi
	<< Player.str << Player.maxstr
	<< Player.con << Player.maxcon
	<< Player.cash << Player.options
	<< Player.preference;
}

template <typename Stm>
static inline void omegarc_write (Stm& stm)
{
    const uint8_t fmt = OMEGA_PLAYER_FORMAT;
    const uint8_t savefmt = OMEGA_SAVE_FORMAT;
    stm << fmt << savefmt << Searchnum << Verbosity << Player.name;
    omegarc_write_player_static (stm);
}

static void save_omegarc (void)
{
    snprintf (ARRAY_BLOCK(Str1), OMEGA_PLAYER_FILE, getenv("HOME"));
    mkpath (Str1);
    sstream ss;
    omegarc_write (ss);
    memblock buf (ss.size());
    ostream os (buf);
    omegarc_write (os);
    buf.write_file (Str1);
}

static bool load_omegarc (const char* filename)
{
    memblock buf;
    buf.read_file (filename);
    istream is (buf);

    uint8_t fmt, savefmt;
    if (is.remaining() < stream_sizeof(fmt)+stream_sizeof(savefmt)+stream_sizeof(Searchnum)+stream_sizeof(Verbosity))
	return false;
    is >> fmt >> savefmt >> Searchnum >> Verbosity;
    if (fmt != OMEGA_PLAYER_FORMAT)
	return false;
    is >> Player.name;
    sstream ss;
    omegarc_write_player_static (ss);
    if (is.remaining() < ss.size())
	return false;
    omegarc_read_player_static (is);
    return true;
}

static bool personal_question_yn (const char* q)
    { mprint (q); return ynq() == 'y'; }
static unsigned personal_question_num (const char* q)
    { mprint (q); return parsenum(); }

static void user_character_stats (void)
{
    int num, iqpts = 0, numints = 0, ok, agipts = 0, dexpts = 0, powpts = 0, conpts = 0;
    mprint ("OK, now try to answer the following questions honestly:");
    morewait();
    num = personal_question_num ("How many pounds can you bench press? ");
    if (num < 30)
	Player.str = Player.maxstr = 3;
    else if (num < 90)
	Player.str = Player.maxstr = num / 10;
    else
	Player.str = Player.maxstr = 9 + ((num - 120) / 30);
    if (Player.str > 18) {
	mprint ("Even if it's true, I don't believe it.");
	morewait();
	clearmsg();
	Player.str = Player.maxstr = 18;
    }

    if (personal_question_yn ("Took an official IQ test? [yn] ")) {
	num = personal_question_num ("So, whadja get? ") / 10;
	if (num > 18) {
	    mprint ("Even if it's true, I don't believe it.");
	    morewait();
	    clearmsg();
	    num = 18;
	}
	iqpts += num;
	numints++;
    }

    if (personal_question_yn ("Took Undergraduate entrance exams? [yn] ")) {
	do {
	    num = personal_question_num ("So, what percentile? ");
	    ok = (num < 100);
	    if (!ok) {
		mprint ("That's impossible!");
		morewait();
		clearmsg();
	    }
	} while (!ok);
	iqpts += (num - 49) * 9 / 50 + 9;
	++numints;
    }
    if (personal_question_yn ("Took Graduate entrance exams? [yn] ")) {
	do {
	    num = personal_question_num ("So, what percentile? ");
	    ok = (num < 100);
	    if (!ok) {
		mprint ("That's impossible!");
		morewait();
		clearmsg();
	    }
	} while (!ok);
	iqpts += (num - 49) * 9 / 50 + 9;
	++numints;
    }

    if (!numints) {
	if (personal_question_yn ("Pretty dumb, aren't you? [yn] ")) {
	    Player.iq = random_range (3) + 3;
	    mprint ("I thought so....");
	} else {
	    Player.iq = random_range (6) + 8;
	    mprint ("Well, not *that* dumb.");
	}
	morewait();
	clearmsg();
    } else
	Player.iq = iqpts / numints;
    Player.maxiq = Player.iq;
    agipts = 0;
    if (personal_question_yn ("Can you dance? [yn] ")) {
	++agipts;
	if (personal_question_yn (" Well? [yn] "))
	    agipts += 2;
    }
    if (personal_question_yn ("Do you have training in a martial art or gymnastics? [yn] ")) {
	agipts += 2;
	if (personal_question_yn ("Do you have dan rank or equivalent? [yn] "))
	    agipts += 4;
    }
    clearmsg();
    if (personal_question_yn ("Do you play some field sport? [yn] ")) {
	++agipts;
	if (personal_question_yn (" Are you good? [yn] "))
	    ++agipts;
    }
    if (personal_question_yn ("Do you cave, mountaineer, etc.? [yn] "))
	agipts += 3;
    if (personal_question_yn ("Do you skate or ski? [yn] ")) {
	agipts += 2;
	if (personal_question_yn (" Well? [yn] "))
	    agipts += 2;
    }
    if (personal_question_yn ("Are you physically handicapped? [yn] "))
	agipts -= 4;
    if (personal_question_yn ("Are you accident prone? [yn] "))
	agipts -= 4;
    if (!personal_question_yn ("Can you use a bicycle? [yn] "))
	agipts -= 4;
    Player.agi = Player.maxagi = 9 + agipts / 2;
    if (personal_question_yn ("Do you play video games? [yn] ")) {
	dexpts += 2;
	if (personal_question_yn ("Do you get high scores? [yn] "))
	    dexpts += 4;
    }
    clearmsg();
    if (personal_question_yn ("Are you an archer, fencer, or marksman? [yn] ")) {
	dexpts += 2;
	if (personal_question_yn ("A good one? [yn] "))
	    dexpts += 4;
    }
    clearmsg();
    if (personal_question_yn ("Have you ever picked a lock? [yn] ")) {
	dexpts += 2;
	mprint ("Really. Well, the police are being notified.");
    }
    morewait();
    clearmsg();
    num = personal_question_num ("What's your typing speed (words per minute) ");
    if (num > 125) {
	mprint ("Tell me another one....");
	morewait();
	clearmsg();
	num = 125;
    }
    dexpts += num / 25;
    if (personal_question_yn ("Hold your arm out. Tense your fist. Hand shaking? [yn] "))
	dexpts -= 3;
    if (personal_question_yn ("Ambidextrous, are you? [yn] "))
	dexpts += 4;
    if (personal_question_yn ("Can you cut a deck of cards with one hand? [yn] "))
	dexpts += 2;
    if (personal_question_yn ("Can you tie your shoes blindfolded? [yn] "))
	dexpts -= 3;
    Player.dex = Player.maxdex = 6 + dexpts / 2;
    if (!personal_question_yn ("Do you ever get colds? [yn] "))
	conpts += 4;
    else if (personal_question_yn (" Frequently? [yn] "))
	conpts -= 4;
    if (personal_question_yn ("Had any serious accident or illness this year? [yn] "))
	conpts -= 4;
    else
	conpts += 4;
    if (personal_question_yn ("Have a chronic disease? [yn] "))
	conpts -= 4;
    if (personal_question_yn ("Overweight or underweight by more than 20 percent? [yn] "))
	conpts -= 2;
    if (personal_question_yn ("High Blood Pressure? [yn] "))
	conpts -= 2;
    if (personal_question_yn ("Smoke? [yn] "))
	conpts -= 3;
    if (personal_question_yn ("Take aerobics classes? [yn] "))
	conpts += 2;
    num = personal_question_num ("How many miles can you run? ");
    if (num > 25) {
	mprint ("Right. Sure. Give me a break.");
	morewait();
	clearmsg();
	conpts += 8;
    } else if (num < 1)
	conpts -= 3;
    else if (num < 5)
	conpts += 2;
    else if (num < 10)
	conpts += 4;
    else
	conpts += 8;
    Player.con = Player.maxcon = 12 + conpts / 3;
    if (personal_question_yn ("Do animals react oddly to your presence? [yn] ")) {
	mprint ("How curious that must be.");
	morewait();
	clearmsg();
	powpts += 2;
    }
    if (personal_question_yn ("Can you see auras? [yn] ")) {
	mprint ("How strange.");
	morewait();
	powpts += 3;
    }
    if (personal_question_yn ("Ever have an out-of-body experience? [yn] ")) {
	mprint ("Wow, man. Fly the friendly skies....");
	morewait();
	clearmsg();
	powpts += 3;
    }
    if (personal_question_yn ("Did you ever cast a spell? [yn] ")) {
	powpts += 3;
	if (personal_question_yn (" Did it work? [yn] ")) {
	    powpts += 7;
	    mprint ("Sure it did.");
	    morewait();
	    clearmsg();
	}
    }
    if (personal_question_yn ("Do you have ESP? [yn] ")) {
	powpts += 3;
	mprint ("Somehow, I knew you were going to say that.");
	morewait();
	clearmsg();
    }
    if (personal_question_yn ("Do you have PK? [yn] ")) {
	powpts += 6;
	mprint ("I can't tell you how much that moves me.");
	morewait();
	clearmsg();
    }
    if (personal_question_yn ("Do you believe in ghosts? [yn] ")) {
	powpts += 2;
	mprint ("I do! I do! I do believe in ghosts!");
	morewait();
	clearmsg();
    }
    if (personal_question_yn ("Are you Irish? [yn] ")) {
	powpts += 2;
	mprint ("Is that blarney or what?");
	morewait();
    }
    Player.pow = Player.maxpow = 3 + powpts / 2;
    clearmsg();
    mprint ("Please enter your character's name: ");
    Player.name = msgscanstring();
    Player.name[0] = toupper (Player.name[0]);
    mprint ("Are you sexually interested in males or females? [mf] ");
    do
	Player.preference = (char) mcigetc();
    while (Player.preference != 'm' && Player.preference != 'f' && Player.preference != 'y' && Player.preference != 'n');	// :-)
}

static void omegan_character_stats (void)
{
    mprint ("To reroll hit ESCAPE; hit any other key to accept these stats.");
    unsigned i = 0;
    do {
	++i;
	sprintf (Str1, "You have only %d chance%s to reroll... ", REROLLS - i, (i == (REROLLS - 1)) ? "" : "s");
	mprint (Str1);
	unsigned share;	// Baseline for related stats
	Player.iq  = Player.maxiq  = 4 + random_range(5) + (share = random_range(12));
	Player.pow = Player.maxpow = 4 + random_range(5) + share;
	Player.dex = Player.maxdex = 4 + random_range(5) + (share = random_range(12));
	Player.agi = Player.maxagi = 4 + random_range(5) + share;
	Player.str = Player.maxstr = 4 + random_range(5) + (share = random_range(12));
	Player.con = Player.maxcon = 4 + random_range(5) + share;
	Player.cash = random_range (500);
	calc_melee();
	dataprint();
    } while (i < REROLLS && mgetc() == KEY_ESCAPE);
    clearmsg();
    mprint ("Please enter your character's name: ");
    Player.name = msgscanstring();
    Player.name[0] = toupper (Player.name[0]);
    mprint ("Is your character sexually interested in males or females? [mf] ");
    do
	Player.preference = (char) mcigetc();
    while (Player.preference != 'm' && Player.preference != 'f' && Player.preference != 'y' && Player.preference != 'n');	// :-)
}
