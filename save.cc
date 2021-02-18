// Omega is free software, distributed under the ISC license

#include "glob.h"
#include <unistd.h>
#include <stdlib.h>
#include <zlib.h>

//----------------------------------------------------------------------

static memblock compress (const cmemlink& buf);
static memblock decompress (const cmemlink& buf, uint32_t size);

//----------------------------------------------------------------------

struct SGHeader {
    char	o,m,e,g;
    uint8_t	format;
    bool	compressed;
    uint16_t	gamever;
    uint32_t	size;
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
    snprintf (ARRAY_BLOCK(savestr), OMEGA_SAVED_GAME, getenv("HOME"));
    mkpath (savestr);

    mprint ("Saving Game....");
    bool writeok = false;
    SGHeader h = {'o','m','e','g',OMEGA_SAVE_FORMAT,optionp(COMPRESS),OMEGA_VERSION,0};
    sstream ss;
    ss << h << Player << World;
    h.size = ss.size();
    memblock buf (ss.size());
    ostream os (buf);
    os << h << Player << World;
    if (h.compressed)
	buf.replace (buf.iat (stream_sizeof(h)), buf.size()-stream_sizeof(h), compress(buf));
    buf.write_file (savestr);
    mprint ("Game Saved.");
    writeok = true;
    return writeok;
}

// read player data, city level, dungeon level,
// check on validity of save file, etc.
// return true if game restored, false otherwise
bool restore_game (void)
{
    char savestr [PATH_MAX];
    snprintf (ARRAY_BLOCK(savestr), OMEGA_SAVED_GAME, getenv("HOME"));
    if (0 != access (savestr, R_OK))
	return false;

    memblock buf;
    buf.read_file (savestr);
    SGHeader header;
    if (buf.size() < stream_sizeof(header))
	return false;

    mprint ("Restoring...");
    istream is (buf);
    is >> header;
    if (header.compressed)
	buf = decompress (buf, header.size);

    istream gdis (buf);
    gdis >> header;
    if (header.format != OMEGA_SAVE_FORMAT)
	return false;
    gdis >> Player >> World;

    mprint ("Restoration complete.");
    ScreenOffset = -100;	// to force a redraw
    setgamestatus (SKIP_MONSTERS);
    return true;
}

//----------------------------------------------------------------------
//{{{ long4 - serialize a uint64_t as two uint32_ts.
struct long4 {
    uint64_t& _v;
    inline long4 (uint64_t& v):_v(v){}
    inline void read (istream& is) { uint32_t v1,v2; is >> v1 >> v2; _v = (uint64_t(v2)<<32)|v1; }
    inline void write (ostream& os) const { os << uint32_t(_v) << uint32_t(_v>>32); }
    inline void write (sstream& ss) const { ss.write (&_v, sizeof(_v)); }
};
//}}}

template <typename T, unsigned N>
static inline void read_array (istream& is, T (&a)[N])
{
    is.align (stream_alignof(a[0]));
    for (auto i : a)
	is >> i;
}

template <typename Stm, typename T, unsigned N>
static inline void write_array (Stm& os, const T (&a)[N])
{
    os.align (stream_alignof(a[0]));
    for (auto i : a)
	os << i;
}

static inline void globals_read (istream& stm)
{
    long4 lSpellKnown (SpellKnown);
    stm >> GameStatus >> Time >> Balance >> lSpellKnown >> FixedPoints >> Gymcredit 	// 4
	>> Date >> Command_Duration >> HiMagicUse >> LastDay				// 2
	>> Pawndate >> StarGemUse >> winnings
	>> Searchnum >> Verbosity							// 1
	>> Tick >> twiddle >> Lunarity >> Phase
	>> Precipitation >> Imprisonment >> Arena_Opponent >> Spellsleft
	>> Constriction >> onewithchaos >> saved >> RitualRoom
	>> Lawstone >> Mindstone >> Chaostone >> Blessing
	>> HiMagic >> tavern_hinthour >> club_hinthour >> HelmHour
	>> RitualHour >> SymbolUseHour >> ViewHour >> ZapHour;
}

template <typename Stm>
static inline void globals_write (Stm& stm)
{
    long4 lSpellKnown (SpellKnown);
    stm << GameStatus << Time << Balance << lSpellKnown << FixedPoints << Gymcredit 	// 4
	<< Date << Command_Duration << HiMagicUse << LastDay				// 2
	<< Pawndate << StarGemUse << winnings
	<< Searchnum << Verbosity							// 1
	<< Tick << twiddle << Lunarity << Phase
	<< Precipitation << Imprisonment << Arena_Opponent << Spellsleft
	<< Constriction << onewithchaos << saved << RitualRoom
	<< Lawstone << Mindstone << Chaostone << Blessing
	<< HiMagic << tavern_hinthour << club_hinthour << HelmHour
	<< RitualHour << SymbolUseHour << ViewHour << ZapHour;
}

//----------------------------------------------------------------------

void player::read (istream& is)
{
    player_pod::read (is);
    read_array (is, rank);
    read_array (is, immunity);
    read_array (is, status);
    read_array (is, guildxp);
    is.align (stream_alignof (name));
    is >> name >> meleestr;

    // Restore globals
    globals_read (is);
    is.read (Password, sizeof(Password));
    is.read (CitySiteList, sizeof(CitySiteList));
    is.read (deepest, sizeof(deepest));
    is.read (level_seed, sizeof(level_seed));
    is.read (Spells, sizeof(Spells));
    is.read (ObjectAttrs, sizeof(ObjectAttrs));
    read_array (is, possessions);
    is >> pack;
    is >> Pawnitems >> Condoitems;
}

// also saves some globals like Level->depth...
template <typename Stm>
void player::write (Stm& os) const
{
    player_pod::write (os);
    write_array (os, rank);
    write_array (os, immunity);
    write_array (os, status);
    write_array (os, guildxp);
    os.align (stream_alignof (name));
    os << name << meleestr;

    // Save globals
    globals_write (os);
    os.write (Password, sizeof(Password));
    os.write (CitySiteList, sizeof (CitySiteList));
    os.write (deepest, sizeof(deepest));
    os.write (level_seed, sizeof(level_seed));
    os.write (Spells, sizeof (Spells));
    os.write (ObjectAttrs, sizeof(ObjectAttrs));

    // Save player possessions
    write_array (os, possessions);
    os << pack;
    os << Pawnitems << Condoitems;
}

//----------------------------------------------------------------------

// Save whatever is pointed to by level
template <typename Stm>
void level::write (Stm& os) const
{
    os << environment << width << height << lastx
	<< lasty << depth << generated << tunnelled;

    // Save changed locations
    for (uint8_t j = 0; j < height; ++j) {
	for (uint8_t i = 0; i < width; ++i) {
	    if (site(i,j).lstatus & CHANGED) {	// this loc has been changed
		uint16_t iend = i;			// find where the changes end
		while (++iend < width && (site(iend,j).lstatus & CHANGED)) {}
		os << i << j << iend;
		for (; i < iend; ++i)
		    os << site(i,j);
	    }
	}
    }
    os << uint32_t(0);	// signify end

    // Save SEEN bits (which do not mark site CHANGED)
    bool bSeen = false;
    uint8_t run = 0;
    for (uint8_t j = 0; j < height; ++j) {
	for (uint8_t i = 0; i < width; ++i, ++run) {
	    while (bSeen != bool(site(i,j).lstatus & SEEN) || run == UINT8_MAX) {
		bSeen = !bSeen;
		os << run;
		run = 0;
	    }
	}
    }
    os << run;
    os.align(4);

    os << things << mlist;
}

void level::read (istream& is)
{
    is >> environment >> width >> height >> lastx
	>> lasty >> depth >> generated >> tunnelled;

    Generate();	// Load the default environment contents

    // Load changed locations
    uint16_t iend;
    do {
	uint8_t i, j;
	is >> i >> j >> iend;
	if (iend >= width || j >= height)
	    return;
	for (; i < iend; ++i)
	    is >> site(i,j);
    } while (iend);

    // Load SEEN bits
    bool bSeen = true;	// inverted when reading first run
    uint8_t run = 0;
    for (uint8_t j = 0; j < height; ++j) {
	for (uint8_t i = 0; i < width; ++i, --run) {
	    while (!run) {
		is >> run;
		bSeen = !bSeen;
	    }
	    if (bSeen)
		site(i,j).lstatus |= SEEN;
	}
    }
    is.align(4);

    is >> things >> mlist;
}

//----------------------------------------------------------------------

enum { MONSTERCHANGEABLESIZE = offsetof(monster_data,monstring) };

//----------------------------------------------------------------------

void monster::read (istream& is)
{
    uint8_t components;
    is >> id >> components >> x >> y;
    if (id >= size(Monsters))
	id = DEATH;	// To make the error obvious
    *this = Monsters[id];
    if (components & 1)
	is.read (this, MONSTERCHANGEABLESIZE);
    if (components & 2)
	is >> aux1 >> aux2;
    if (components & 4)
	is >> attacked >> click;
    if (components & 8) {
	monstring = strdup (is.read_strz());
	corpsestr = strdup (is.read_strz());
    }
    is.align(4);
    if (components & 16)
	is >> possessions;
}

template <typename Stm>
void monster::write (Stm& os) const
{
    uint8_t components = 0;
    if (id < size(Monsters)) {
	if (0 != memcmp(this, &Monsters[id], MONSTERCHANGEABLESIZE))
	    components |= 1;
	if (aux1 || aux2)
	    components |= 2;
	if (attacked || click)
	    components |= 4;
	if ((monstring && monstring != Monsters[id].monstring) || (corpsestr && corpsestr != Monsters[id].corpsestr))
	    components |= 8;
	if (!possessions.empty())
	    components |= 16;
    }
    os << id << components << x << y;
    if (components & 1)
	os.write (this, MONSTERCHANGEABLESIZE);
    if (components & 2)
	os << aux1 << aux2;
    if (components & 4)
	os << attacked << click;
    if (components & 8) {
	os.write_strz (monstring);
	os.write_strz (corpsestr);
    }
    os.align(4);
    if (components & 16)
	os << possessions;
}

//----------------------------------------------------------------------

enum { OBJECTCHANGEABLESIZE = offsetof(object_data,objchar)-offsetof(object_data,weight) };

void object::read (istream& is)
{
    uint8_t components;
    is >> id >> components >> number >> x >> y;
    if (id < size(Objects))
	*this = Objects[id];
    if (components & 1)
	is.read (&weight, OBJECTCHANGEABLESIZE);
    if (components & 2) {
	objstr = strdup (is.read_strz());
	truename = strdup (is.read_strz());
	cursestr = strdup (is.read_strz());
	is.align(2);
    }
}

template <typename Stm>
void object::write (Stm& os) const
{
    uint8_t components = 0;
    if (id < size(Objects)) {
	if (0 != memcmp(&weight, &Objects[id].weight, OBJECTCHANGEABLESIZE))
	    components |= 1;
	if ((objstr && objstr != Objects[id].objstr) || (truename && truename != Objects[id].truename) || (cursestr && cursestr != Objects[id].cursestr))
	    components |= 2;
    }
    os << id << components << number << x << y;
    if (components & 1)
	os.write (&weight, OBJECTCHANGEABLESIZE);
    if (components & 2) {
	os.write_strz (objstr);
	os.write_strz (truename);
	os.write_strz (cursestr);
	os.align(2);
    }
}

//----------------------------------------------------------------------

static memblock compress (const cmemlink& buf)
{
    memblock obuf (buf.size());
    z_stream s;
    memset (&s, 0, sizeof(s));
    deflateInit (&s, Z_DEFAULT_COMPRESSION);
    s.next_in = const_cast<uint8_t*>((const uint8_t*)buf.begin()) + sizeof(SGHeader);
    s.avail_in = buf.size() - sizeof(SGHeader);
    memcpy (obuf.begin(), buf.begin(), sizeof(SGHeader));
    s.next_out = (uint8_t*) obuf.begin() + sizeof(SGHeader);
    s.avail_out = obuf.size() - sizeof(SGHeader);
    int r;
    while (Z_OK == (r = deflate (&s, Z_FINISH))) {}
    if (r != Z_STREAM_END)	// Fallback to uncompressed
	obuf.link (const_cast<uint8_t*>((const uint8_t*)buf.begin()), buf.size());
    else
	obuf.resize (obuf.size()-s.avail_out);
    deflateEnd (&s);
    return obuf;
}

static memblock decompress (const cmemlink& buf, uint32_t size)
{
    z_stream s;
    memset (&s, 0, sizeof(s));
    inflateInit (&s);
    s.next_in = const_cast<uint8_t*>((const uint8_t*)buf.begin()) + sizeof(SGHeader);
    s.avail_in = buf.size() - sizeof(SGHeader);
    memblock obuf (size);
    memcpy (obuf.begin(), buf.begin(), sizeof(SGHeader));
    s.next_out = (uint8_t*) obuf.begin() + sizeof(SGHeader);
    s.avail_out = obuf.size() - sizeof(SGHeader);
    int r;
    while (Z_OK == (r = inflate (&s, Z_FINISH))) {}
    if (r != Z_STREAM_END)	// Fallback to uncompressed
	obuf.link (const_cast<uint8_t*>((const uint8_t*)buf.begin()), buf.size());
    else
	obuf.resize (obuf.size()-s.avail_out);
    inflateEnd (&s);
    return obuf;
}
