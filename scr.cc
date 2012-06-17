// omega (c) 1987,1988,1989 by Laurence Raphael Brothers
// scr.c
// functions that use curses routines directly
// plus a few file i/o stuff
// also some in file.c

#include "glob.h"
#include <unistd.h>

//----------------------------------------------------------------------

static int bufferappend(const char* s);
static void blankoutspot(int i, int j);
static void blotspot(int i, int j);
static void buffercycle(const char* s);
static void dobackspace(void);
static void drawplayer(void);
static void lightspot(int x, int y);

//----------------------------------------------------------------------

static constexpr inline attr_t CHARATTR (chtype c) { return (c & ~A_CHARTEXT); }

static WINDOW *Levelw, *Dataw, *Flagw, *Timew, *Menuw, *Locw, *Morew, *Phasew;
static WINDOW *Comwin, *Msg1w, *Msg2w, *Msg3w, *Msgw;

void phaseprint (void)
{
    werase (Phasew);
    wprintw (Phasew, "Moon's Phase:\n");
    switch (Phase / 2) {
	case 0:		wprintw (Phasew, "NEW"); break;
	case 1:
	case 11:	wprintw (Phasew, "CRESCENT"); break;
	case 2:
	case 10:	wprintw (Phasew, "1/4"); break;
	case 3:
	case 9:		wprintw (Phasew, "HALF"); break;
	case 4:
	case 8:		wprintw (Phasew, "3/4"); break;
	case 5:
	case 7:		wprintw (Phasew, "GIBBOUS"); break;
	case 6:		wprintw (Phasew, "FULL"); break;
    }
    wrefresh (Phasew);
}

void show_screen (void)
{
    werase (Levelw);
    int top = max (0, ScreenOffset);
    int bottom = min ((int)Level->height, ScreenOffset + ScreenLength);
    for (int j = top; j < bottom; j++) {
	wmove (Levelw, screenmod (j), 0);
	for (unsigned i = 0; i < Level->width; i++) {
	    chtype c = SPACE;
	    if (Current_Environment == E_COUNTRYSIDE) {
		if (c_statusp (i, j, SEEN))
		    c = Country->site(i,j).showchar;
	    } else {
		if (loc_statusp (i, j, SEEN))
		    c = getspot (i, j, false);
	    }
	    wattrset (Levelw, CHARATTR(c));
	    waddch (Levelw, c & 0xff);
	}
    }
    wrefresh (Levelw);
}

char mgetc (void)
{
    return (wgetch (Msgw));
}

// case insensitive mgetc -- sends uppercase to lowercase
int mcigetc (void)
{
    int c = wgetch (Msgw);
    if ((c >= (int) 'A') && (c <= (int) 'Z'))
	return (c + (int) ('a' - 'A'));
    else
	return (c);
}

char menugetc (void)
{
    return (wgetch (Menuw));
}

char lgetc (void)
{
    return (wgetch (Levelw));
}

int ynq (void)
{
    char p = '*';		// the user's choice; start with something impossible
				// to prevent a loop.
    while ((p != 'n') && (p != 'y') && (p != 'q') && (p != KEY_ESCAPE) && (p != EOF) && (p != ' '))
	p = wgetch (Msgw);
    switch (p) {
	case 'y':
	    wprintw (Msgw, "yes. ");
	    break;
	case 'n':
	    wprintw (Msgw, "no. ");
	    break;

	case KEY_ESCAPE:
	    p = 'q';		// fall through to 'q'
	case ' ':
	    p = 'q';		// fall through to 'q'
	case 'q':
	    wprintw (Msgw, "quit. ");
	    break;
	default:
	    assert (p == EOF);
    }
    wrefresh (Msgw);
    return (p);
}

int ynq1 (void)
{
    char p = '*';		// the user's choice; start with something impossible
				// to prevent a loop.
    while ((p != 'n') && (p != 'y') && (p != 'q') && (p != KEY_ESCAPE) && (p != ' ') && (p != EOF))
	p = wgetch (Msg1w);
    switch (p) {
	case 'y':
	    wprintw (Msg1w, "yes. ");
	    break;
	case 'n':
	    wprintw (Msg1w, "no. ");
	    break;

	case KEY_ESCAPE:
	    p = 'q';		// fall through to 'q'
	case ' ':
	    p = 'q';		// fall through to 'q'
	case 'q':
	    wprintw (Msg1w, "quit. ");
	    break;
	default:
	    assert (p == EOF);
    }
    wrefresh (Msg1w);
    return (p);
}

int ynq2 (void)
{
    char p = '*';		// the user's choice; start with something impossible
				// to prevent a loop.
    while ((p != 'n') && (p != 'y') && (p != 'q') && (p != KEY_ESCAPE) && (p != ' ') && (p != EOF))
	p = wgetch (Msg2w);
    switch (p) {
	case 'y':
	    wprintw (Msg2w, "yes. ");
	    break;
	case 'n':
	    wprintw (Msg2w, "no. ");
	    break;

	case KEY_ESCAPE:
	    p = 'q';		// fall through to 'q'
	case ' ':
	    p = 'q';		// fall through to 'q'
	case 'q':
	    wprintw (Msg2w, "quit. ");
	    break;
	default:
	    assert (p == EOF);
    }
    wrefresh (Msg2w);
    return (p);
}

// puts up a morewait to allow reading if anything in top two lines
void checkclear (void)
{
    int y1 = getcury (Msg1w), y2 = getcury (Msg2w);
    if ((y1 != 0) || (y2 != 0)) {
	morewait();
	werase (Msg1w);
	werase (Msg2w);
	wnoutrefresh (Msg1w);
	wrefresh (Msg2w);
    }
}

// for external call
void clearmsg (void)
{
    werase (Msg1w);
    werase (Msg2w);
    werase (Msg3w);
    Msgw = Msg1w;
    wnoutrefresh (Msg1w);
    wnoutrefresh (Msg2w);
    wrefresh (Msg3w);
}

void clearmsg3 (void)
{
    werase (Msg3w);
    wrefresh (Msg3w);
}

void clearmsg1 (void)
{
    werase (Msg1w);
    werase (Msg2w);
    Msgw = Msg1w;
    wnoutrefresh (Msg1w);
    wrefresh (Msg2w);
}

void erase_level (void)
{
    werase (Levelw);
    wrefresh (Levelw);
}

// direct print to first msg line
void print1 (const char* s)
{
    if (!gamestatusp (SUPPRESS_PRINTING)) {
	buffercycle (s);
	werase (Msg1w);
	wprintw (Msg1w, s);
	wrefresh (Msg1w);
    }
}

// for run on-messages -- print1 clears first....
void nprint1 (const char* s)
{
    if (!gamestatusp (SUPPRESS_PRINTING)) {
	if (bufferappend (s)) {
	    wprintw (Msg1w, s);
	    wrefresh (Msg1w);
	}
    }
}

// direct print to second msg line
void print2 (const char* s)
{
    if (!gamestatusp (SUPPRESS_PRINTING)) {
	buffercycle (s);
	werase (Msg2w);
	wprintw (Msg2w, s);
	wrefresh (Msg2w);
    }
}

// for run on-messages -- print2 clears first....
void nprint2 (const char* s)
{
    if (!gamestatusp (SUPPRESS_PRINTING)) {
	if (bufferappend (s)) {
	    wprintw (Msg2w, s);
	    wrefresh (Msg2w);
	}
    }
}

// msg line 3 is not part of the region that mprint or printm can reach
// typical use of print3 is for "you can't do that" type error messages
void print3 (const char* s)
{
    if (!gamestatusp (SUPPRESS_PRINTING)) {
	buffercycle (s);
	werase (Msg3w);
	wprintw (Msg3w, s);
	wrefresh (Msg3w);
    }
}

// for run on-messages -- print3 clears first....
void nprint3 (const char* s)
{
    if (!gamestatusp (SUPPRESS_PRINTING)) {
	if (bufferappend (s)) {
	    wprintw (Msg3w, s);
	    wrefresh (Msg3w);
	}
    }
}

// prints wherever cursor is in window, but checks to see if
// it should morewait and clear window
void mprint (const char* s)
{
    if (!gamestatusp (SUPPRESS_PRINTING)) {
	unsigned x = getcurx (Msgw);
	if (x + strlen (s) >= Level->width) {
	    buffercycle (s);
	    if (Msgw == Msg1w) {
		werase (Msg2w);
		Msgw = Msg2w;
	    } else {
		morewait();
		werase (Msg1w);
		werase (Msg2w);
		wnoutrefresh (Msg2w);
		Msgw = Msg1w;
	    }
	} else if (x > 0)
	    bufferappend (s);
	else
	    buffercycle (s);
	wprintw (Msgw, s);
	waddch (Msgw, ' ');
	wrefresh (Msgw);
    }
}

void mprintf (const char* fmt, ...)
{
    char buf [128];
    va_list args;
    va_start (args, fmt);
    vsnprintf (ArrayBlock(buf), fmt, args);
    buf[ArraySize(buf)-1] = 0;
    mprint (buf);
    va_end (args);
}

// display a file given a string name of file
void displayfile (const char* filestr)
{
    clear();
    refresh();
    const char* ld = filestr;
    char d;
    for (d = ' '; *ld && d != 'q' && d != KEY_ESCAPE;) {
	if (getcury(stdscr) > ScreenLength) {
	    standout();
	    printw ("\n-More-");
	    standend();
	    refresh();
	    d = getch();
	    clear();
	}
	addch (*ld++);
    }
    if ((char) d != 'q' && (char) d != KEY_ESCAPE) {
	standout();
	printw ("\n-Done-");
	standend();
	refresh();
	getch();
    }
    clear();
    refresh();
}

void omega_title (void)
{
    displayfile (Data_Title);
    clear();
    touchwin (stdscr);
    refresh();
}

static void clrgen_init (void)
{
    use_default_colors();
    init_pair (1, COLOR_BLACK, COLOR_DEFAULT);
    init_pair (2, COLOR_BLACK, COLOR_GREEN);
    init_pair (3, COLOR_BLACK, COLOR_RED);
    init_pair (4, COLOR_BLACK, COLOR_WHITE);
    init_pair (5, COLOR_BLACK, COLOR_YELLOW);
    init_pair (6, COLOR_BLUE, COLOR_DEFAULT);
    init_pair (7, COLOR_BLUE, COLOR_WHITE);
    init_pair (8, COLOR_CYAN, COLOR_DEFAULT);
    init_pair (9, COLOR_GREEN, COLOR_DEFAULT);
    init_pair (10, COLOR_GREEN, COLOR_BLUE);
    init_pair (11, COLOR_GREEN, COLOR_RED);
    init_pair (12, COLOR_GREEN, COLOR_YELLOW);
    init_pair (13, COLOR_MAGENTA, COLOR_DEFAULT);
    init_pair (14, COLOR_MAGENTA, COLOR_WHITE);
    init_pair (15, COLOR_RED, COLOR_DEFAULT);
    init_pair (16, COLOR_RED, COLOR_WHITE);
    init_pair (17, COLOR_WHITE, COLOR_DEFAULT);
    init_pair (18, COLOR_WHITE, COLOR_BLUE);
    init_pair (19, COLOR_WHITE, COLOR_RED);
    init_pair (20, COLOR_WHITE, COLOR_YELLOW);
    init_pair (21, COLOR_YELLOW, COLOR_DEFAULT);
    init_pair (22, COLOR_YELLOW, COLOR_BLUE);
    init_pair (23, COLOR_YELLOW, COLOR_RED);
    init_pair (24, COLOR_YELLOW, COLOR_WHITE);
    init_pair (25, COLOR_YELLOW, COLOR_YELLOW);
}

// initialize, screen, windows
void initgraf (void)
{
    initscr();
    start_color();
    clrgen_init();
    curs_set (0);
    if (LINES < 24 || COLS < 80) {
	printf ("Minimum Screen Size: 24 Lines by 80 Columns.");
	exit (0);
    }
    ScreenLength = LINES - 6;
    Msg1w = newwin (1, 80, 0, 0);
    Msg2w = newwin (1, 80, 1, 0);
    Msg3w = newwin (1, 80, 2, 0);
    Msgw = Msg1w;
    Morew = newwin (1, 15, 3, 65);
    Locw = newwin (1, 80, ScreenLength + 3, 0);
    Levelw = newwin (ScreenLength, 64, 3, 0);
    Menuw = newwin (ScreenLength, 64, 3, 0);
    Dataw = newwin (2, 80, ScreenLength + 4, 0);
    Timew = newwin (2, 15, 4, 65);
    Phasew = newwin (2, 15, 6, 65);
    Flagw = newwin (4, 15, 9, 65);
    Comwin = newwin (8, 15, 14, 65);

    noecho();
    crmode();

    clear();
    touchwin (stdscr);
}

static int lastx = -1, lasty = -1;

static void drawplayer (void)
{
    if (Current_Environment == E_COUNTRYSIDE) {
	if (inbounds (lastx, lasty) && !offscreen (lasty)) {
	    wmove (Levelw, screenmod (lasty), lastx);
	    chtype c = Country->site(lastx,lasty).showchar;
	    wattrset (Levelw, CHARATTR (c));
	    waddch (Levelw, (c & 0xff));
	}
	wmove (Levelw, screenmod (Player.y), Player.x);
	wattrset (Levelw, CHARATTR (PLAYER));
	waddch (Levelw, (PLAYER & 0xff));
    } else {
	if (inbounds (lastx, lasty) && !offscreen (lasty))
	    plotspot (lastx, lasty, (Player.status[BLINDED] > 0 ? false : true));
	wmove (Levelw, screenmod (Player.y), Player.x);
	if ((!Player.status[INVISIBLE]) || Player.status[TRUESIGHT]) {
	    wattrset (Levelw, CHARATTR (PLAYER));
	    waddch (Levelw, (PLAYER & 0xff));
	}
    }
    lastx = Player.x;
    lasty = Player.y;
}

void setlastxy (int new_x, int new_y)
{
    lastx = new_x;
    lasty = new_y;
}

void drawvision (int x, int y)
{
    static int oldx = -1, oldy = -1;
    int i, j, c;

    if (Current_Environment != E_COUNTRYSIDE) {
	if (Player.status[BLINDED]) {
	    drawspot (oldx, oldy);
	    drawspot (x, y);
	    drawplayer();
	} else {
	    if (Player.status[ILLUMINATION] > 0) {
		for (i = -2; i < 3; i++)
		    for (j = -2; j < 3; j++)
			if (inbounds (x + i, y + j))
			    if (view_los_p (x + i, y + j, Player.x, Player.y))
				dodrawspot (x + i, y + j);
	    } else {
		for (i = -1; i < 2; i++)
		    for (j = -1; j < 2; j++)
			if (inbounds (x + i, y + j))
			    dodrawspot (x + i, y + j);
	    }
	    drawplayer();
	    drawmonsters (false);	// erase all monsters
	    drawmonsters (true);	// draw those now visible
	}
	if ((!gamestatusp (FAST_MOVE)) || (!optionp (JUMPMOVE)))
	    omshowcursor (Player.x, Player.y);
	oldx = x;
	oldy = y;
    } else {
	for (i = -1; i < 2; i++) {
	    for (j = -1; j < 2; j++) {
		if (inbounds (x + i, y + j)) {
		    c_set (x + i, y + j, SEEN);
		    if (!offscreen (y + j)) {
			wmove (Levelw, screenmod (y + j), x + i);
			c = Country->site(x+i,y+j).showchar;
			wattrset (Levelw, CHARATTR (c));
			waddch (Levelw, (c & 0xff));
		    }
		}
	    }
	}
	drawplayer();
	omshowcursor (Player.x, Player.y);
    }
}

void omshowcursor (int x, int y)
{
    if (!offscreen (y)) {
	wmove (Levelw, screenmod (y), x);
	wrefresh (Levelw);
    }
}

void levelrefresh (void)
{
    wrefresh (Levelw);
}

// draws a particular spot under if in line-of-sight
void drawspot (int x, int y)
{
    chtype c;
    if (inbounds (x, y)) {
	c = getspot (x, y, false);
	if (c != Level->site(x,y).showchar)
	    if (view_los_p (Player.x, Player.y, x, y)) {
		lset (x, y, SEEN);
		Level->site(x,y).showchar = c;
		putspot (x, y, c);
	    }
    }
}

// draws a particular spot regardless of line-of-sight
void dodrawspot (int x, int y)
{
    chtype c;
    if (inbounds (x, y)) {
	c = getspot (x, y, false);
	if (c != Level->site(x,y).showchar) {
	    lset (x, y, SEEN);
	    Level->site(x,y).showchar = c;
	    putspot (x, y, c);
	}
    }
}

// write a blank to a spot if it is floor
static void blankoutspot (int i, int j)
{
    if (inbounds (i, j)) {
	lreset (i, j, LIT);
	lset (i, j, CHANGED);
	if (Level->site(i,j).locchar == FLOOR) {
	    Level->site(i,j).showchar = SPACE;
	    putspot (i, j, SPACE);
	}
    }
}

// blank out a spot regardless
static void blotspot (int i, int j)
{
    if (inbounds (i, j)) {
	lreset (i, j, SEEN);
	Level->site(i,j).showchar = SPACE;
	if (!offscreen (j)) {
	    wmove (Levelw, screenmod (j), i);
	    wattrset (Levelw, CHARATTR (SPACE));
	    waddch (Levelw, SPACE & 0xff);
	}
    }
}

// for displaying activity specifically at some point
void plotspot (int x, int y, int showmonster)
{
    if (loc_statusp (x, y, SEEN))
	putspot (x, y, getspot (x, y, showmonster));
    else
	putspot (x, y, SPACE);
}

// Puts c at x,y on screen. No fuss, no bother.
void putspot (int x, int y, chtype c)
{
    if (!offscreen (y)) {
	wmove (Levelw, screenmod (y), x);
	wattrset (Levelw, CHARATTR (c));
	waddch (Levelw, (0xff & c));
    }
}

// regardless of line of sight, etc, draw a monster
void plotmon (struct monster *m)
{
    if (!offscreen (m->y)) {
	wmove (Levelw, screenmod (m->y), m->x);
	wattrset (Levelw, CHARATTR (m->monchar));
	waddch (Levelw, (m->monchar & 0xff));
    }
}

// if display, displays monsters, otherwise erases them
void drawmonsters (int display)
{
    foreach (m, Level->mlist) {
	if (m->hp < 0) continue;
	if (display) {
	    if (view_los_p (Player.x, Player.y, m->x, m->y))
		if (Player.status[TRUESIGHT] || (!m_statusp (m, M_INVISIBLE)))
		    putspot (m->x, m->y, m->monchar);
	} else
	    erase_monster (m);
    }
}

// replace monster with what would be displayed if monster weren't there
void erase_monster (struct monster *m)
{
    if (loc_statusp (m->x, m->y, SEEN))
	putspot (m->x, m->y, getspot (m->x, m->y, false));
    else
	blotspot (m->x, m->y);
}

// find apt char to display at some location
chtype getspot (int x, int y, int showmonster)
{
    if (loc_statusp (x, y, SECRET))
	return (WALL);
    const monster* m = Level->creature(x,y);
    switch (Level->site(x,y).locchar) {
	case WATER:
	    if (m && !m_statusp (*m, SWIMMING) && showmonster)
		return (m->monchar);
	    else
		return (WATER);
	    // these sites never show anything but their location char's
	case CLOSED_DOOR:
	case LAVA:
	case FIRE:
	case ABYSS:
	    return (Level->site(x,y).locchar);
	    // rubble and hedge don't show items on their location
	case RUBBLE:
	case HEDGE:
	    if (showmonster && m) {
		if (m_statusp (*m, M_INVISIBLE) && !Player.status[TRUESIGHT])
		    return (getspot (x, y, false));
		else
		    return (m->monchar);
	    } else
		return (Level->site(x,y).locchar);
	    // everywhere else, first try to show monster, next show items, next show location char
	default: {
	    unsigned nThings = 0;
	    foreach (o, Level->things)
		nThings += (o->x == x && o->y == y);
	    if (showmonster && m) {
		if (m_statusp (*m, M_INVISIBLE) && !Player.status[TRUESIGHT])
		    return (getspot (x, y, false));
		else
		    return (m->monchar);
	    } else if (nThings == 1)
		return (Level->thing(x,y)->objchar);
	    else if (nThings > 1)
		return (PILE);
	    else
		return (Level->site(x,y).locchar);
	}
    }
}

void commanderror (void)
{
    werase (Msg3w);
    wprintw (Msg3w, "%c : unknown command", Cmd);
    wrefresh (Msg3w);
}

void timeprint (void)
{
    werase (Timew);
    wprintw (Timew, "%d:%d", showhour(), showminute());
    if (showminute() == 0)
	waddch (Timew, '0');
    wprintw (Timew, hour() > 11 ? " PM \n" : " AM \n");
    wprintw (Timew, month());
    wprintw (Timew, " the %d", day());
    wprintw (Timew, ordinal (day()));
    wrefresh (Timew);
}

void comwinprint (void)
{
    werase (Comwin);
    wprintw (Comwin, "Hit: %d  \n", Player.hit);
    wprintw (Comwin, "Dmg: %d  \n", Player.dmg);
    wprintw (Comwin, "Def: %d  \n", Player.defense);
    wprintw (Comwin, "Arm: %d  \n", Player.absorption);
    wprintw (Comwin, "Spd: %d.%d  \n", 5 / Player.speed, 500 / Player.speed % 100);
    wrefresh (Comwin);
}

void dataprint (void)
{
    werase (Dataw);
    // WDT HACK: I should make these fields spaced and appropriately justified.
    // Maybe I don't feel like it right now.
    wprintw (Dataw, "Hp:%d/%d Mana:%d/%d Au:%d Level:%d/%d Carry:%d/%d \n", Player.hp, Player.maxhp, Player.mana, Player.maxmana, Player.cash, Player.level, Player.xp, Player.itemweight, Player.maxweight);
    wprintw (Dataw, "Str:%d/%d Con:%d/%d Dex:%d/%d Agi:%d/%d Int:%d/%d Pow:%d/%d   ", Player.str, Player.maxstr, Player.con, Player.maxcon, Player.dex, Player.maxdex, Player.agi, Player.maxagi, Player.iq, Player.maxiq, Player.pow, Player.maxpow);
    wrefresh (Dataw);
}

// redraw each permanent window
void xredraw (void)
{
    touchwin (Msgw);
    touchwin (Msg3w);
    touchwin (Levelw);
    touchwin (Timew);
    touchwin (Flagw);
    touchwin (Dataw);
    touchwin (Locw);
    touchwin (Morew);
    touchwin (Phasew);
    touchwin (Comwin);
    wnoutrefresh (Msgw);
    wnoutrefresh (Msg3w);
    wnoutrefresh (Levelw);
    wnoutrefresh (Timew);
    wnoutrefresh (Flagw);
    wnoutrefresh (Dataw);
    wnoutrefresh (Locw);
    wnoutrefresh (Morew);
    wnoutrefresh (Phasew);
    wrefresh (Comwin);
}

void menuaddch (int c)
{
    waddch (Menuw, c);
    wrefresh (Menuw);
}

void morewait (void)
{
    int display = true;
    int c;
    if (gamestatusp (SUPPRESS_PRINTING))
	return;
    do {
	werase (Morew);
	if (display)
	    wprintw (Morew, "***  MORE  ***");
	else
	    wprintw (Morew, "+++  MORE  +++");
	display = !display;
	wrefresh (Morew);
	c = wgetch (Msgw);
    } while ((c != ' ') && (c != KEY_ENTER) && (c != EOF));
    werase (Morew);
    wrefresh (Morew);
}

int stillonblock (void)
{
    int display = true;
    int c;
    do {
	werase (Morew);
	if (display)
	    wprintw (Morew, "<<<STAY?>>>");
	else
	    wprintw (Morew, ">>>STAY?<<<");
	display = !display;
	wrefresh (Morew);
	c = wgetch (Msgw);
    } while ((c != ' ') && (c != KEY_ESCAPE) && (c != EOF));
    werase (Morew);
    wrefresh (Morew);
    return (c == ' ');
}

void menuclear (void)
{
    werase (Menuw);
    touchwin (Menuw);
    wrefresh (Menuw);
}

void menuprint (const char* s)
{
    if (getcury(Menuw) >= ScreenLength - 2) {
	wrefresh (Menuw);
	morewait();
	werase (Menuw);
	touchwin (Menuw);
    }
    wprintw (Menuw, s);
}

void showmenu (void)
{
    wrefresh (Menuw);
}

void endgraf (void)
{
    clear();
    touchwin (stdscr);
    refresh();
    endwin();
}

void plotchar (chtype pyx, int x, int y)
{
    if (!offscreen (y)) {
	wmove (Levelw, screenmod (y), x);
	wattrset (Levelw, CHARATTR (pyx));
	waddch (Levelw, (pyx & 0xff));
	wrefresh (Levelw);
    }
}

void draw_explosion (chtype pyx, int x, int y)
{
    int i, j;

    for (j = 0; j < 3; j++) {
	for (i = 0; i < 9; i++)
	    plotchar (pyx, x + Dirs[0][i], y + Dirs[1][i]);
	napms (150);
	for (i = 0; i < 9; i++)
	    plotchar (SPACE, x + Dirs[0][i], y + Dirs[1][i]);
	napms (150);
    }
    for (i = 0; i < 9; i++)
	plotspot (x + Dirs[0][i], y + Dirs[1][i], true);
    wrefresh (Levelw);
}

const char* msgscanstring (void)
{
    static char instring[80], byte = 'x';
    int i = 0;

    instring[0] = 0;
    curs_set (1);
    byte = mgetc();
    while (byte != '\n') {
	if ((byte == 8) || (byte == 127)) {	// ^h or delete
	    if (i > 0) {
		i--;
		dobackspace();
	    }
	    instring[i] = 0;
	} else {
	    instring[i] = byte;
	    waddch (Msgw, byte);
	    wrefresh (Msgw);
	    i++;
	    instring[i] = 0;
	}
	byte = mgetc();
    }
    curs_set (0);
    return (instring);
}

void locprint (const char* s)
{
    werase (Locw);
    wprintw (Locw, s);
    wrefresh (Locw);
}

// draw everything whether visible or not
void drawscreen (void)
{
    if (Current_Environment == E_COUNTRYSIDE)
	for (unsigned i = 0; i < Level->width; i++)
	    for (unsigned j = 0; j < Level->height; j++)
		c_set (i, j, SEEN);
    else
	for (unsigned i = 0; i < Level->width; i++)
	    for (unsigned j = 0; j < Level->height; j++)
		lset (i, j, SEEN);
    if (Current_Environment == E_CITY)
	for (unsigned i = 0; i < NUMCITYSITES; i++)
	    CitySiteList[i][0] = 1;
    show_screen();
}

//selects a number up to range

int getnumber (int range)
{
    int done = false, value = 1;
    int atom;

    if (range == 1)
	return (1);
    else {
	while (!done) {
	    clearmsg();
	    wprintw (Msg1w, "How many? Change with < or >, ESCAPE to select:");
	    mnumprint (value);
	    do
		atom = mcigetc();
	    while ((atom != '<') && (atom != '>') && (atom != KEY_ESCAPE));
	    if ((atom == '>') && (value < range))
		value++;
	    else if ((atom == '<') && (value > 1))
		value--;
	    else if (atom == KEY_ESCAPE)
		done = true;
	}
    }
    return (value);
}

// reads a positive number up to 999999
long parsenum (void)
{
    int number[8];
    int place = -1;
    int i, x, y, mult = 1;
    long num = 0;
    char byte = ' ';

    curs_set (1);
    while ((byte != KEY_ESCAPE) && (byte != '\n')) {
	byte = mgetc();
	if (byte == KEY_BACKSPACE) {
	    if (place > -1) {
		number[place] = 0;
		place--;
		getyx (Msgw, y, x);
		wmove (Msgw, y, x - 1);
		waddch (Msgw, ' ');
		wmove (Msgw, y, x - 1);
		wrefresh (Msgw);
	    }
	} else if ((byte <= '9') && (byte >= '0') && (place < 7)) {
	    place++;
	    number[place] = byte;
	    waddch (Msgw, byte);
	    wrefresh (Msgw);
	}
    }
    curs_set (0);
    waddch (Msgw, ' ');
    if (byte == KEY_ESCAPE)
	return (ABORT);
    else {
	for (i = place; i >= 0; i--) {
	    num += mult * (number[i] - '0');
	    mult *= 10;
	}
	return (num);
    }
}

void display_death (const char* source)
{
    clear();
    touchwin (stdscr);
    printw ("\n\n\n\n");
    printw ("Requiescat In Pace, ");
    printw (Player.name);
    printw (" (%ld points)", calc_points());
    strcpy (Str4, "Killed by ");
    strcat (Str4, source);
    printw ("\n");
    printw (Str4);
    printw (".");
    printw ("\n\n\n\n\nHit 'c' to continue.");
    refresh();
    while (wgetch (stdscr) != 'c');
    clear();
    touchwin (stdscr);
    refresh();
}

void display_win (void)
{
    clear();
    touchwin (stdscr);
    printw ("\n\n\n\n");
    printw (Player.name);
    if (Player.rank[ADEPT]) {
	printw (" is a total master of omega with %ld points!", FixedPoints);
	strcpy (Str4, "A total master of omega");
    } else {
	strcpy (Str4, "retired a winner");
	printw (" triumphed in omega with %ld points!", calc_points());
    }
    printw ("\n\n\n\n\nHit 'c' to continue.");
    refresh();
    while (wgetch (stdscr) != 'c');
    clear();
    touchwin (stdscr);
    refresh();
}

void display_quit (void)
{
    clear();
    touchwin (stdscr);
    printw ("\n\n\n\n");
    printw (Player.name);
    strcpy (Str4, "A quitter.");
    printw (" wimped out with %ld points!", calc_points());
    printw ("\n\n\n\n\nHit 'c' to continue.");
    refresh();
    while (wgetch (stdscr) != 'c');
    clear();
    touchwin (stdscr);
    refresh();
}

void display_bigwin (void)
{
    clear();
    touchwin (stdscr);
    printw ("\n\n\n\n");
    printw (Player.name);
    strcpy (Str4, "retired, an Adept of Omega.");
    printw (" retired, an Adept of Omega with %ld points!", FixedPoints);
    printw ("\n\n\n\n\nHit 'c' to continue.");
    refresh();
    while (wgetch (stdscr) != 'c');
    clear();
    touchwin (stdscr);
    refresh();
}

void mnumprint (int n)
{
    char numstr[20];
    sprintf (numstr, "%d", n);
    bufferappend (numstr);
    wprintw (Msgw, "%d", n);
    wrefresh (Msgw);
}

void mlongprint (long n)
{
    char numstr[20];
    sprintf (numstr, "%ld", n);
    bufferappend (numstr);
    wprintw (Msgw, "%ld", n);
    wrefresh (Msgw);
}

void menunumprint (int n)
{
    if (getcury(Menuw) >= ScreenLength - 2) {
	wrefresh (Menuw);
	morewait();
	werase (Menuw);
	touchwin (Menuw);
    }
    wprintw (Menuw, "%d", n);
}

void menulongprint (long n)
{
    if (getcury(Menuw) >= ScreenLength - 2) {
	wrefresh (Menuw);
	morewait();
	werase (Menuw);
	touchwin (Menuw);
    }
    wprintw (Menuw, "%ld", n);
}

static void dobackspace (void)
{
    int x, y;
    getyx (Msgw, y, x);
    if (x > 0) {
	waddch (Msgw, ' ');
	wmove (Msgw, y, x - 1);
	waddch (Msgw, ' ');
	wmove (Msgw, y, x - 1);
    }
    wrefresh (Msgw);
}

void showflags (void)
{
    phaseprint();
    werase (Flagw);
    if (Player.food < 0)
	wprintw (Flagw, "Starving\n");
    else if (Player.food <= 3)
	wprintw (Flagw, "Weak\n");
    else if (Player.food <= 10)
	wprintw (Flagw, "Ravenous\n");
    else if (Player.food <= 20)
	wprintw (Flagw, "Hungry\n");
    else if (Player.food <= 30)
	wprintw (Flagw, "A mite peckish\n");
    else if (Player.food <= 36)
	wprintw (Flagw, "Content\n");
    else if (Player.food <= 44)
	wprintw (Flagw, "Satiated\n");
    else
	wprintw (Flagw, "Bloated\n");

    if (Player.status[POISONED] > 0)
	wprintw (Flagw, "Poisoned\n");
    else
	wprintw (Flagw, "Vigorous\n");

    if (Player.status[DISEASED] > 0)
	wprintw (Flagw, "Diseased\n");
    else
	wprintw (Flagw, "Healthy\n");

    if (gamestatusp (MOUNTED))
	wprintw (Flagw, "Mounted\n");
    else if (Player.status[LEVITATING])
	wprintw (Flagw, "Levitating\n");
    else
	wprintw (Flagw, "Afoot\n");

    wrefresh (Flagw);
}

void drawomega (void)
{
    int i;
    clear();
    touchwin (stdscr);
    for (i = 0; i < 7; i++) {
	move (1, 1);
	wattrset (stdscr, CHARATTR (CLR_LIGHT_BLUE_BLACK));
	printw ("\n\n\n");
	printw ("\n                                    *****");
	printw ("\n                               ******   ******");
	printw ("\n                             ***             ***");
	printw ("\n                           ***                 ***");
	printw ("\n                          **                     **");
	printw ("\n                         ***                     ***");
	printw ("\n                         **                       **");
	printw ("\n                         **                       **");
	printw ("\n                         ***                     ***");
	printw ("\n                          ***                   ***");
	printw ("\n                            **                 **");
	printw ("\n                       *   ***                ***   *");
	printw ("\n                        ****                    ****");
	refresh();
	napms (200);
	move (1, 1);
	wattrset (stdscr, CHARATTR (CLR_CYAN_BLACK));
	printw ("\n\n\n");
	printw ("\n                                    +++++");
	printw ("\n                               ++++++   ++++++");
	printw ("\n                             +++             +++");
	printw ("\n                           +++                 +++");
	printw ("\n                          ++                     ++");
	printw ("\n                         +++                     +++");
	printw ("\n                         ++                       ++");
	printw ("\n                         ++                       ++");
	printw ("\n                         +++                     +++");
	printw ("\n                          +++                   +++");
	printw ("\n                            ++                 ++");
	printw ("\n                       +   +++                +++   +");
	printw ("\n                        ++++                    ++++");
	refresh();
	napms (200);
	move (1, 1);
	wattrset (stdscr, CHARATTR (CLR_BLUE_BLACK));
	printw ("\n\n\n");
	printw ("\n                                    .....");
	printw ("\n                               ......   ......");
	printw ("\n                             ...             ...");
	printw ("\n                           ...                 ...");
	printw ("\n                          ..                     ..");
	printw ("\n                         ...                     ...");
	printw ("\n                         ..                       ..");
	printw ("\n                         ..                       ..");
	printw ("\n                         ...                     ...");
	printw ("\n                          ...                   ...");
	printw ("\n                            ..                 ..");
	printw ("\n                       .   ...                ...   .");
	printw ("\n                        ....                    ....");
	refresh();
	napms (200);
    }
    wattrset (stdscr, CHARATTR (CLR_WHITE_BLACK));
}

// y is an absolute coordinate
// ScreenOffset is the upper left hand corner of the current screen in absolute coordinates
void screencheck (int y)
{
    if (y - ScreenOffset < ScreenLength / 8 || y - ScreenOffset > 7 * ScreenLength / 8) {
	ScreenOffset = y - ScreenLength / 2;
	show_screen();
	if (Current_Environment != E_COUNTRYSIDE)
	    drawmonsters (true);
	if (!offscreen (Player.y))
	    drawplayer();
    }
}

void spreadroomlight (int x, int y, int roomno)
{
    if (inbounds (x, y) && !loc_statusp (x, y, LIT) && Level->site(x,y).roomnumber == roomno) {
	lightspot (x, y);
	spreadroomlight (x + 1, y, roomno);
	spreadroomlight (x, y + 1, roomno);
	spreadroomlight (x - 1, y, roomno);
	spreadroomlight (x, y - 1, roomno);
    }
}

// illuminate one spot at x y
static void lightspot (int x, int y)
{
    chtype c;
    lset (x, y, LIT);
    lset (x, y, SEEN);
    lset (x, y, CHANGED);
    c = getspot (x, y, false);
    Level->site(x,y).showchar = c;
    putspot (x, y, c);
}

void spreadroomdark (int x, int y, int roomno)
{
    if (inbounds (x, y))
	if (loc_statusp (x, y, LIT) && (Level->site(x,y).roomnumber == roomno)) {
	    blankoutspot (x, y);
	    spreadroomdark (x + 1, y, roomno);
	    spreadroomdark (x, y + 1, roomno);
	    spreadroomdark (x - 1, y, roomno);
	    spreadroomdark (x, y - 1, roomno);
	}
}

void display_pack (void)
{
    if (Player.pack.empty()) {
	print3 ("Pack is empty.");
	return;
    }
    menuclear();
    menuprint ("Items in Pack:\n");
    for (unsigned i = 0; i < Player.pack.size(); i++) {
	sprintf (Str1, "  %c: %s\n", i + 'A', itemid (&Player.pack[i]));
	menuprint (Str1);
    }
    showmenu();
}

void display_possessions (unsigned selection)
{
    static const char _slotstr[] =
	"%c%c in air:	%s\n\0"
	"%c%c freehand:	%s\n\0"
	"%c%c weapon:	%s\n\0"
	"%c%c shoulder:	%s\n\0"
	"%c%c shoulder:	%s\n\0"
	"%c%c belt:	%s\n\0"
	"%c%c belt:	%s\n\0"
	"%c%c belt:	%s\n\0"
	"%c%c shield:	%s\n\0"
	"%c%c armor:	%s\n\0"
	"%c%c boots:	%s\n\0"
	"%c%c cloak:	%s\n\0"
	"%c%c finger:	%s\n\0"
	"%c%c finger:	%s\n\0"
	"%c%c finger:	%s\n\0"
	"%c%c finger:	%s\n";
    werase (Menuw);
    unsigned slot = 0;
    const char* slotstr = _slotstr;
    foreach (o, Player.possessions) {
	char usechar = (o->id != NO_THING && o->used) ? '>' : ')';
	char idchar = index_to_key (slot);
	if (slot == selection) wstandout(Menuw);
	wprintw (Menuw, slotstr, idchar, usechar, o->id != NO_THING ? itemid(o) : "(slot vacant)");
	if (slot == selection) wstandend(Menuw);
	++slot;
	slotstr = strnext(slotstr);
    }
    wrefresh (Menuw);
}

void display_options (unsigned selection)
{
    werase (Menuw);
    if (selection == 0) wstandout(Menuw);
    wprintw (Menuw, "Option BELLICOSE:	%s\n", optionp (BELLICOSE) ? "true" : "false");
    if (selection == 1) wstandout(Menuw); else wstandend(Menuw);
    wprintw (Menuw, "Option JUMPMOVE:	%s\n", optionp (JUMPMOVE) ? "true" : "false");
    if (selection == 2) wstandout(Menuw); else wstandend(Menuw);
    wprintw (Menuw, "Option RUNSTOP:	%s\n", optionp (RUNSTOP) ? "true" : "false");
    if (selection == 3) wstandout(Menuw); else wstandend(Menuw);
    wprintw (Menuw, "Option PICKUP:	%s\n", optionp (PICKUP) ? "true" : "false");
    if (selection == 4) wstandout(Menuw); else wstandend(Menuw);
    wprintw (Menuw, "Option CONFIRM:	%s\n", optionp (CONFIRM) ? "true" : "false");
    if (selection == 5) wstandout(Menuw); else wstandend(Menuw);
    wprintw (Menuw, "Option PACKADD:	%s\n", optionp (PACKADD) ? "true" : "false");
    if (selection == 6) wstandout(Menuw); else wstandend(Menuw);
    wprintw (Menuw, "Option COMPRESS:	%s\n", optionp (COMPRESS) ? "true" : "false");
    if (selection == 7) wstandout(Menuw); else wstandend(Menuw);
    wprintw (Menuw, "Option VERBOSITY: %s\n", (Verbosity == VERBOSE ? "Verbose" : (Verbosity == MEDIUM ? "Medium" : "Terse")));
    if (selection == 8) wstandout(Menuw); else wstandend(Menuw);
    wprintw (Menuw, "Option SEARCHNUM:	%u", Searchnum);
    wstandend(Menuw);
    wrefresh (Menuw);
}

// nya ha ha ha ha haaaa....
void deathprint (void)
{
    waddch (Msgw, 'D');
    wrefresh (Msgw);
    napms (200);
    mgetc();
    waddch (Msgw, 'e');
    wrefresh (Msgw);
    napms (200);
    waddch (Msgw, 'a');
    wrefresh (Msgw);
    napms (200);
    waddch (Msgw, 't');
    wrefresh (Msgw);
    napms (200);
    waddch (Msgw, 'h');
    wrefresh (Msgw);
    napms (200);
}

void clear_if_necessary (void)
{
    if (getcurx(Msg1w) != 0) {
	werase (Msg1w);
	wrefresh (Msg1w);
    }
    if (getcurx(Msg2w) != 0) {
	werase (Msg2w);
	wrefresh (Msg2w);
    }
    if (getcurx(Msg3w) != 0) {
	werase (Msg3w);
	wrefresh (Msg3w);
    }
}

static int bufferpos = 0;

static void buffercycle (const char* s)
{
    strcpy (Stringbuffer[bufferpos++], s);
    if (bufferpos >= STRING_BUFFER_SIZE)
	bufferpos = 0;
}

static int bufferappend (const char* s)
{
    int pos = bufferpos - 1;

    if (pos < 0)
	pos = STRING_BUFFER_SIZE - 1;
    if (strlen (Stringbuffer[pos]) + strlen (s) < 80 - 1) {
	strcat (Stringbuffer[pos], s);
	return 1;
    } else
	return 0;
}

void bufferprint (void)
{
    int i = bufferpos - 1, c, finished = 0;
    clearmsg();
    wprintw (Msg1w, "^p for previous message, ^n for next, anything else to quit.");
    wnoutrefresh (Msg1w);
    do {
	if (i >= STRING_BUFFER_SIZE)
	    i = 0;
	if (i < 0)
	    i = STRING_BUFFER_SIZE - 1;
	werase (Msg2w);
	wprintw (Msg2w, Stringbuffer[i]);
	wrefresh (Msg2w);
	c = mgetc();
	if (c == 16)		// ^p
	    i--;
	else if (c == 14)	// ^n
	    i++;
	else
	    finished = 1;
    } while (!finished);
    clearmsg();
    omshowcursor (Player.x, Player.y);
}

void clear_screen (void)
{
    clear();
    touchwin (stdscr);
    refresh();
}
