#include "glob.h"
#include <unistd.h>
#include <stdlib.h>

//----------------------------------------------------------------------

static void restore_level(istream& is);
static void save_level(ostream& os, plv level);
static memblock compress (const cmemlink& buf);
static memblock decompress (const cmemlink& buf);

//----------------------------------------------------------------------

struct SGHeader {
    char	o;
    char	m;
    char	e;
    char	g;
    char	a;
    uint8_t	format;
    uint8_t	gamever;
    bool	compressed;
};

//----------------------------------------------------------------------

// Various functions for doing game saves and restores
// The game remembers various player information, the city level,
// the country level, and the last or current dungeon level

//*************** SAVE FUNCTIONS

// Checks to see if save file already exists.
// Checks to see if save file can be opened for write.
// The player, the city level, and the current dungeon level are saved.

bool save_game (void)
{
    char savestr [128];
    snprintf (ArrayBlock(savestr), OMEGA_SAVED_GAME, getenv("HOME"));
    mkpath (savestr);

    plv current, levelToSave;
    print1 ("Saving Game....");
    bool writeok = false;
    try {
	memblock buf (UINT16_MAX);
	ostream os (buf);

	const SGHeader h = {'o','m','e','g','a',OMEGA_SAVE_FORMAT,OMEGA_VERSION,optionp(COMPRESS)};
	os << h << Player;

	save_level (os, Country);
	save_level (os, City);

	if (Current_Environment == E_CITY || Current_Environment == E_COUNTRYSIDE)
	    levelToSave = Dungeon;
	else if (Current_Environment == Current_Dungeon)
	    levelToSave = Dungeon;
	else
	    levelToSave = Level;
	int i;
	for (i = 0, current = levelToSave; current; current = current->next, i++);
	os << i;
	for (current = levelToSave; current; current = current->next)
	    if (current != Level)
		save_level (os, current);
	if (levelToSave)
	    save_level (os, Level);	// put current level last

	buf.resize (os.pos());

	if (h.compressed)
	    buf.swap (compress (buf));
	buf.write_file (savestr);
	print1 ("Game Saved.");
	writeok = true;
    } catch (...) {
	print1 ("Something didn't work... save aborted.");
	morewait();
	clearmsg();
    }
    return (writeok);
}

// read player data, city level, dungeon level,
// check on validity of save file, etc.
// return true if game restored, false otherwise
bool restore_game (void)
{
    char savestr [128];
    snprintf (ArrayBlock(savestr), OMEGA_SAVED_GAME, getenv("HOME"));
    if (0 != access (savestr, R_OK))
	return (false);

    memblock buf;
    try {
	buf.read_file (savestr);
	istream is (buf);
	print1 ("Restoring...");

	SGHeader header;
	is >> header;
	if (header.format != OMEGA_SAVE_FORMAT)
	    runtime_error::emit ("incorrect saved game file format");
	if (header.compressed) {
	    buf.swap (decompress (buf));
	    is.relink (buf);
	}
	is >> Player;

	restore_level (is);	// the countryside
	restore_level (is);	// the city level
	int i;
	is >> i;
	for (; i > 0; i--) {
	    restore_level (is);
	    if (Level->environment == Current_Dungeon) {
		Level->next = Dungeon;
		Dungeon = Level;
	    }
	    if (Current_Environment == E_CITY)
		Level = City;
	}
	print3 ("Restoration complete.");
	ScreenOffset = -100;	// to force a redraw
	setgamestatus (SKIP_MONSTERS);
    } catch (const exception& e) {
	char errbuf[80];
	snprintf (ArrayBlock(errbuf), "Error restoring %s: %s", savestr, e.what().c_str());
	print1 (errbuf);
	morewait();
	return (false);
    }
    return (true);
}

template <typename Stm>
static inline void globals_serialize (Stm& stm)
{
    stm & ios::align(8)
	& GameStatus & Time & Gymcredit & Balance & SpellKnown
	& FixedPoints & Command_Duration & Date & HiMagicUse & LastDay
	& Pawndate & StarGemUse & winnings & Arena_Opponent & Blessing
	& Chaostone & Constriction & Current_Dungeon & Current_Environment & HelmHour
	& HiMagic & Imprisonment & LastCountryLocX & LastCountryLocY & LastTownLocX
	& LastTownLocY & Last_Environment & Lawstone & Lunarity & Mindstone
	& Phase & Precipitation & RitualHour & RitualRoom & Searchnum
	& Spellsleft & SymbolUseHour & Tick & Verbosity & ViewHour
	& Villagenum & ZapHour & club_hinthour & onewithchaos & saved
	& tavern_hinthour & twiddle;	// End is currently 4-grain
}

streamsize player::stream_size (void) const
{
    sizestream ss (player_pod::stream_size());
    ss << rank;
    ss.skipalign (stream_align(immunity));
    ss << immunity << status << guildxp;
    ss.skipalign (stream_align(name));
    ss << name << meleestr;

    globals_serialize (ss);
    ss.write (Password, sizeof(Password));
    ss.write (CitySiteList, sizeof (CitySiteList));
    ss.write (deepest, sizeof(deepest));
    ss.write (level_seed, sizeof(level_seed));
    ss.write (Spells, sizeof (Spells));
    ss.write (ObjectAttrs, sizeof(ObjectAttrs));

    // Save player possessions
    ss.skipalign (stream_align (possessions));
    ss << possessions << pack;
    ss << Pawnitems << Condoitems;
    return (ss.pos());
}

// also saves some globals like Level->depth...
void player::write (ostream& os) const
{
    player_pod::write (os);
    os << rank;
    os.skipalign (stream_align(immunity));
    os << immunity << status << guildxp;
    os.skipalign (stream_align(name));
    os << name << meleestr;

    // Save globals
    globals_serialize (os);
    os.write (Password, sizeof(Password));
    os.write (CitySiteList, sizeof (CitySiteList));
    os.write (deepest, sizeof(deepest));
    os.write (level_seed, sizeof(level_seed));
    os.write (Spells, sizeof (Spells));
    os.write (ObjectAttrs, sizeof(ObjectAttrs));

    // Save player possessions
    os.skipalign (stream_align (possessions));
    os << possessions << pack;
    os << Pawnitems << Condoitems;
}

void player::read (istream& is)
{
    player_pod::read (is);
    is >> rank;
    is.align (stream_align(immunity));
    is >> immunity >> status >> guildxp;
    is.align (stream_align(name));
    is >> name >> meleestr;

    // Restore globals
    globals_serialize (is);
    is.read (Password, sizeof(Password));
    is.read (CitySiteList, sizeof(CitySiteList));
    is.read (deepest, sizeof(deepest));
    is.read (level_seed, sizeof(level_seed));
    is.read (Spells, sizeof(Spells));
    is.read (ObjectAttrs, sizeof(ObjectAttrs));
    switch (Current_Dungeon) {
	case E_ASTRAL:	MaxDungeonLevels = ASTRALLEVELS; break;
	case E_SEWERS:	MaxDungeonLevels = SEWERLEVELS; break;
	case E_CASTLE:	MaxDungeonLevels = CASTLELEVELS; break;
	case E_CAVES:	MaxDungeonLevels = CAVELEVELS; break;
	case E_VOLCANO:	MaxDungeonLevels = VOLCANOLEVELS; break;
    }

    is.align (stream_align (possessions));
    is >> possessions >> pack;
    is >> Pawnitems >> Condoitems;
}

// Save whatever is pointed to by level
static void save_level (ostream& os, plv level)
{
    unsigned run;
    os << level->environment
	<< level->width << level->height << level->lastx << level->lasty
	<< level->depth << level->generated << level->numrooms << level->tunnelled;
    os.skipalign(4);
    uint16_t i, j;
    for (j = 0; j < MAXLENGTH; j++) {
	for (i = 0; i < MAXWIDTH; i++) {
	    if (level->site(i,j).lstatus & CHANGED) {	// this loc has been changed
		for (run = i + 1; run < MAXWIDTH &&	// find how many in a row
		     level->site(run,j).lstatus & CHANGED; run++);
		os << i << j << run;
		for (; i < run; i++)
		    os << level->site(i,j);
	    }
	}
    }
    os << i << j;	// signify end
    // since we don't mark the 'seen' bits as CHANGED, need to save a bitmask
    uint32_t mask = 0;
    run = 8*sizeof (mask);
    for (j = 0; j < MAXLENGTH; j++) {
	for (i = 0; i < MAXWIDTH; i++) {
	    if (run == 0) {
		run = 8 * sizeof(mask);
		os << mask;
		mask = 0;
	    }
	    mask >>= 1;
	    if (level->site(i,j).lstatus & SEEN)
		mask |= (1UL << (sizeof(mask)*8 - 1));
	    run--;
	}
    }
    if (run < 8*sizeof (mask))
	os << mask;
    os << level->mlist << level->things;
}

static void restore_level (istream& is)
{
    unsigned run;
    uint16_t i, j;
    uint32_t mask = 0;

    Level = new level;
    clear_level (Level);
    is >> Level->environment
	>> Level->width >> Level->height >> Level->lastx >> Level->lasty
	>> Level->depth >> Level->generated >> Level->numrooms >> Level->tunnelled;
    is.skipalign (4);
    Level->generated = true;
    EEnvironment temp_env = Current_Environment;
    Current_Environment = Level->environment;
    switch (Level->environment) {
	case E_COUNTRYSIDE:
	    Country = Level;
	    load_country();
	    break;
	case E_CITY:
	    City = Level;
	    load_city();
	    break;
	case E_VILLAGE:
	    load_village (Country->site(LastCountryLocX,LastCountryLocY).aux);
	    break;
	case E_CAVES:
	case E_SEWERS:
	case E_CASTLE:
	case E_ASTRAL:
	case E_VOLCANO:
	    generate_level (-1, Level->depth);
	    break;
	case E_HOVEL:
	case E_MANSION:
	case E_HOUSE:
	    load_house (Level->environment);
	    break;
	case E_DLAIR:
	    load_dlair (gamestatusp (KILLED_DRAGONLORD));
	    break;
	case E_STARPEAK:
	    load_speak (gamestatusp (KILLED_LAWBRINGER));
	    break;
	case E_MAGIC_ISLE:
	    load_misle (gamestatusp (KILLED_EATER));
	    break;
	case E_TEMPLE:
	    load_temple (Country->site(LastCountryLocX,LastCountryLocY).aux);
	    break;
	case E_CIRCLE:
	    load_circle();
	    break;
	case E_COURT:
	    load_court();
	    break;
	default:
	    print3 ("This dungeon not implemented!");
	    break;
    }
    Current_Environment = temp_env;
    is >> i >> j;
    while (j < MAXLENGTH && i < MAXWIDTH) {
	is >> run;
	for (; i < run; i++)
	    is >> Level->site(i,j);
	is >> i >> j;
    }
    run = 0;
    for (j = 0; j < MAXLENGTH; j++) {
	for (i = 0; i < MAXWIDTH; i++) {
	    if (run == 0) {
		run = 8 * sizeof(mask);
		is >> mask;
	    }
	    if (mask & 1)
		lset (i, j, SEEN);
	    mask >>= 1;
	    run--;
	}
    }
    is >> Level->mlist >> Level->things;
}

//----------------------------------------------------------------------

enum { MONSTERCHANGEABLESIZE = offsetof(monster_data,monstring) };

void monster_data::read (istream& is) noexcept
{
    uint8_t components;
    is >> id >> components;
    is.skip (2);
    if (id >= ArraySize(Monsters))
	id = DEATH;	// To make the error obvious
    *this = Monsters[id];
    if (components & 1)
	is.read (this, MONSTERCHANGEABLESIZE);
    if (components & 2) {
	monstring = strdup (is.read_strz());
	corpsestr = strdup (is.read_strz());
	is.align(stream_align(*this));
    }
}

void monster_data::write (ostream& os) const noexcept
{
    uint8_t components = 0;
    if (id < ArraySize(Monsters)) {
	if (0 != memcmp(this, &Monsters[id], MONSTERCHANGEABLESIZE))
	    components |= 1;
	if ((monstring && monstring != Monsters[id].monstring) || (corpsestr && corpsestr != Monsters[id].corpsestr))
	    components |= 2;
    }
    os << id << components;
    os.skip (2);
    if (components & 1)
	os.write (this, MONSTERCHANGEABLESIZE);
    if (components & 2) {
	os.write_strz (monstring);
	os.write_strz (corpsestr);
	os.skipalign(stream_align(*this));
    }
}

streamsize monster_data::stream_size (void) const noexcept
{
    streamsize sz = 4;
    if (id < ArraySize(Monsters)) {
	if (0 != memcmp(this, &Monsters[id], MONSTERCHANGEABLESIZE))
	    sz += MONSTERCHANGEABLESIZE;
	if ((monstring && monstring != Monsters[id].monstring) || (corpsestr && corpsestr != Monsters[id].corpsestr))
	    sz += Align(strlen(monstring)+1+strlen(corpsestr)+1,stream_align(*this));
    }
    return (sz);
}

//----------------------------------------------------------------------

void monster::write (ostream& os) const
{
    monster_data::write (os);
    os << aux1 << aux2 << attacked << click << x << y;
    os << possessions;
    os.skipalign (stream_align(*this));
}

void monster::read (istream& is)
{
    monster_data::read (is);
    is >> aux1 >> aux2 >> attacked >> click >> x >> y;
    is >> possessions;
    is.align (stream_align(*this));
}

streamsize monster::stream_size (void) const noexcept
{
    streamsize r = monster_data::stream_size();
    r += stream_size_of(aux1)+stream_size_of(aux2)+stream_size_of(attacked)+stream_size_of(click)+stream_size_of(x)+stream_size_of(y);
    r += stream_size_of(possessions);
    return (Align(r,stream_align(*this)));
}

//----------------------------------------------------------------------

enum { OBJECTCHANGEABLESIZE = offsetof(object_data,objchar)-offsetof(object_data,weight) };

void object_data::read (istream& is) noexcept
{
    uint8_t components;
    is >> id >> components;
    if (id >= ArraySize(Objects))
	return;
    *this = Objects[id];
    if (components & 1)
	is.read (&weight, OBJECTCHANGEABLESIZE);
    if (components & 2) {
	objstr = strdup (is.read_strz());
	truename = strdup (is.read_strz());
	cursestr = strdup (is.read_strz());
	is.align(stream_align(*this));
    }
}

void object_data::write (ostream& os) const noexcept
{
    uint8_t components = 0;
    if (id < ArraySize(Objects)) {
	if (0 != memcmp(&weight, &Objects[id].weight, OBJECTCHANGEABLESIZE))
	    components |= 1;
	if ((objstr && objstr != Objects[id].objstr) || (truename && truename != Objects[id].truename) || (cursestr && cursestr != Objects[id].cursestr))
	    components |= 2;
    }
    os << id << components;
    if (components & 1)
	os.write (&weight, OBJECTCHANGEABLESIZE);
    if (components & 2) {
	os.write_strz (objstr);
	os.write_strz (truename);
	os.write_strz (cursestr);
	os.skipalign(stream_align(*this));
    }
}

streamsize object_data::stream_size (void) const noexcept
{
    streamsize sz = stream_size_of(id) + stream_size_of(level);
    if (id < ArraySize(Objects)) {
	if (0 != memcmp(&weight, &Objects[id].weight, OBJECTCHANGEABLESIZE))
	    sz += OBJECTCHANGEABLESIZE;
	if ((objstr && objstr != Objects[id].objstr) || (truename && truename != Objects[id].truename) || (cursestr && cursestr != Objects[id].cursestr))
	    sz += Align(strlen(objstr)+1+strlen(truename)+1+strlen(cursestr)+1,stream_align(*this));
    }
    return (sz);
}

//----------------------------------------------------------------------

enum : uint8_t { RUN_CODE = 0xCD };

static memblock compress (const cmemlink& buf)
{
    memblock obuf (buf.size());
    ostream os (obuf);
    const uint8_t *i = (const uint8_t*) buf.begin();
    os << *(const SGHeader*)i << buf.size();
    for (streampos io = sizeof(SGHeader), ileft; (ileft = buf.size()-io); ++io) {
	unsigned mm = 0, mdm = 0, dml = min(255u,min(io,ileft));
	for (unsigned m = 0, dm = 1; dm <= dml; ++dm) {
	    for (m = 0; m < dml && i[io+m-dm] == i[io+m]; ++m) {}
	    if (m > mm) {
		mm = m;
		mdm = dm;
	    }
	}
	if (mm > 3) {
	    os << RUN_CODE << uint8_t(mdm) << uint8_t(mm);
	    io += mm-1;
	} else if (i[io] == RUN_CODE) {
	    os.uiwrite (vpack(RUN_CODE,uint8_t(0),uint8_t(1),uint8_t(0)));
	    os.skip (-1);
	} else
	    os << i[io];
    }
    obuf.memlink::resize (os.pos());
    return (obuf);
}

static memblock decompress (const cmemlink& buf)
{
    istream is (buf.begin(), buf.size());
    sized_type<sizeof(SGHeader)>::type h;
    memblock::size_type ucsz;
    is >> h >> ucsz;
    memblock obuf (ucsz);
    ostream os (obuf);
    os << h;
    for (uint8_t b,o,l; is.remaining();) {
	is >> b; os << b;
	if (b == RUN_CODE) {
	    is >> o >> l;
	    os.skip (-1);
	    copy_n (os.ipos()-o, l, os.ipos());
	    os.skip (l);
	}
    }
    obuf.memlink::resize (os.pos());
    return (obuf);
}
