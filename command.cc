#include <unistd.h>
#include <ctype.h>
#include "glob.h"

//----------------------------------------------------------------------

static void rest(void);
static void peruse(void);
static void quaff(void);
static void activate(void);
static void eat(void);
static void search(int *searchval);
static void talk(void);
static void disarm(void);
static void give(void);
static void zapwand(void);
static void magic(void);
static void upstairs(void);
static void downstairs(void);
static void opendoor(void);
static void bash_location(void);
static void closedoor(void);
static void moveplayer(int dx, int dy);
static void movepincountry(int dx, int dy);
static void examine(void);
static void help(void);
static void version(void);
static void fire(void);
static void nap(void);
static void wizard(void);
static void vault(void);
static void tacoptions(void);
static void pickpocket(void);
static void abortshadowform(void);
static void tunnel(void);
static void hunt(int terrain);
static void city_move(void);
static void frobgamestatus(void);
static void give_money (struct monster *m);
static void drop_money (void);
static object detach_money (void);
static const char* trapid (unsigned trapno);
static void enter_site (chtype site);

//----------------------------------------------------------------------

// deal with a new player command in dungeon or city mode
void p_process (void)
{
    static int searchval = 0;

    if (Player.status[BERSERK]) {
	if (goberserk()) {
	    setgamestatus (SKIP_PLAYER);
	    drawvision (Player.x, Player.y);
	}
    }
    if (!gamestatusp (SKIP_PLAYER)) {
	if (searchval > 0) {
	    searchval--;
	    if (searchval == 0)
		resetgamestatus (FAST_MOVE);
	}
	drawvision (Player.x, Player.y);
	if (!gamestatusp (FAST_MOVE)) {
	    searchval = 0;
	    Cmd = mgetc();
	    clear_if_necessary();
	}
	Command_Duration = 0;
	switch (Cmd) {
	    case KEY_CTRL|'f':	abortshadowform(); break;
	    case KEY_CTRL|'g':	wizard(); break;
	    case KEY_CTRL|'i':	display_pack(); morewait(); xredraw(); break;
	    case KEY_CTRL|'k':	if (gamestatusp (CHEATED)) frobgamestatus();	// fallthrough
	    case KEY_CTRL|'l':	xredraw();
	    case ' ':
	    case KEY_ENTER:	setgamestatus (SKIP_MONSTERS); break;
	    case KEY_CTRL|'p':	bufferprint(); setgamestatus (SKIP_MONSTERS); break;
	    case KEY_CTRL|'r':	xredraw(); setgamestatus (SKIP_MONSTERS); break;
	    case KEY_CTRL|'w':	if (gamestatusp (CHEATED)) drawscreen(); break;
	    case KEY_CTRL|'x':
		if (gamestatusp (CHEATED) || Player.rank[ADEPT])
		    wish (1);
		Command_Duration = 5;
		break;
	    case 'a':	zapwand();	Command_Duration = Player.speed*8/5; break;
	    case 'c':	closedoor();	Command_Duration = Player.speed*2/5; break;
	    case 'd':	drop();		Command_Duration = Player.speed*5/5; break;
	    case 'e':	eat();		Command_Duration = 30; break;
	    case 'f':	fire();		Command_Duration = Player.speed*5/5; break;
	    case 'g':	pickup();	Command_Duration = Player.speed*10/5; break;
	    case 'i':	do_inventory_control(); break;
	    case 'm':	magic();	Command_Duration = 12; break;
	    case 'o':	opendoor();	Command_Duration = Player.speed*5/5; break;
	    case 'p':	pickpocket();	Command_Duration = Player.speed*20/5; break;
	    case 'q':	quaff();	Command_Duration = 10; break;
	    case 'r':	peruse();	Command_Duration = 20; break;
	    case 's':	search (&searchval); Command_Duration = 20; break;
	    case 't':	talk();		Command_Duration = 10; break;
	    case 'v':	vault();	Command_Duration = Player.speed*10/5; break;
	    case 'x':	examine();	Command_Duration = 1; break;
	    case 'z':	bash_location(); Command_Duration = Player.speed*10/5; break;
	    case 'A':	activate();	Command_Duration = 10; break;
	    case 'D':	disarm();	Command_Duration = 30; break;
	    case 'E':	dismount_steed(); Command_Duration = Player.speed*10/5; break;
	    case 'F':	tacoptions(); break;
	    case 'G':	give();		Command_Duration = 10; break;
	    case 'I':	do_inventory_control(); break;
	    case 'M':	city_move();	Command_Duration = 10; break;
	    case 'O':	setoptions(); break;
	    case 'Q':	quit(); break;
	    case 'R':	rename_player(); break;
	    case 'S':	save(); break;
	    case 'T':	tunnel();	Command_Duration = Player.speed*30/5; break;
	    case 'V':	version(); break;
	    case 'Z':	bash_item();	Command_Duration = Player.speed*10/5; break;
	    case '.':	rest();		Command_Duration = 10; break;
	    case ',':	nap();		Command_Duration = 10; break;
	    case '>':	downstairs(); break;
	    case '<':	upstairs(); break;
	    case '@':
		p_movefunction (Level->site(Player.x,Player.y).p_locf);
		Command_Duration = 5;
		break;
	    case '?':	help(); setgamestatus (SKIP_MONSTERS); break;
	    case '4':
	    case 'h':	moveplayer(-1,0);	Command_Duration = Player.speed*5/5; break;
	    case '2':
	    case 'j':	moveplayer(0,1);	Command_Duration = Player.speed*5/5; break;
	    case '8':
	    case 'k':	moveplayer(0,-1);	Command_Duration = Player.speed*5/5; break;
	    case '6':
	    case 'l':	moveplayer(1,0);	Command_Duration = Player.speed*5/5; break;
	    case '1':
	    case 'b':	moveplayer(-1,1);	Command_Duration = Player.speed*5/5; break;
	    case '3':
	    case 'n':	moveplayer(1,1);	Command_Duration = Player.speed*5/5; break;
	    case '7':
	    case 'y':	moveplayer(-1,-1);	Command_Duration = Player.speed*5/5; break;
	    case '9':
	    case 'u':	moveplayer(1,-1);	Command_Duration = Player.speed*5/5; break;
	    case '5':
		setgamestatus (SKIP_MONSTERS);	// don't do anything; a dummy turn
		Cmd = mgetc();
		while (Cmd != KEY_ESCAPE && (Cmd < '1' || Cmd > '9' || Cmd == '5')) {
		    print3 ("Run in keypad direction [ESCAPE to abort]: ");
		    Cmd = mgetc();
		}
		if (Cmd != KEY_ESCAPE)
		    setgamestatus (FAST_MOVE);
		break;
	    case 'H':
		setgamestatus (FAST_MOVE);
		Cmd = 'h';
		moveplayer (-1, 0);
		Command_Duration = Player.speed * 4 / 5;
		break;
	    case 'J':
		setgamestatus (FAST_MOVE);
		Cmd = 'j';
		moveplayer (0, 1);
		Command_Duration = Player.speed * 4 / 5;
		break;
	    case 'K':
		setgamestatus (FAST_MOVE);
		Cmd = 'k';
		moveplayer (0, -1);
		Command_Duration = Player.speed * 4 / 5;
		break;
	    case 'L':
		setgamestatus (FAST_MOVE);
		Cmd = 'l';
		moveplayer (1, 0);
		Command_Duration = Player.speed * 4 / 5;
		break;
	    case 'B':
		setgamestatus (FAST_MOVE);
		Cmd = 'b';
		moveplayer (-1, 1);
		Command_Duration = Player.speed * 4 / 5;
		break;
	    case 'N':
		setgamestatus (FAST_MOVE);
		Cmd = 'n';
		moveplayer (1, 1);
		Command_Duration = Player.speed * 4 / 5;
		break;
	    case 'Y':
		setgamestatus (FAST_MOVE);
		Cmd = 'y';
		moveplayer (-1, -1);
		Command_Duration = Player.speed * 4 / 5;
		break;
	    case 'U':
		setgamestatus (FAST_MOVE);
		Cmd = 'u';
		moveplayer (1, -1);
		Command_Duration = Player.speed * 4 / 5;
		break;
	    default:
		commanderror();
		setgamestatus (SKIP_MONSTERS);
		break;
	}
    }
    if (Current_Environment != E_COUNTRYSIDE)
	roomcheck();
    screencheck (Player.y);
}

// deal with a new player command in countryside mode
void p_country_process (void)
{
    drawvision (Player.x, Player.y);
    bool no_op;
    do {
	no_op = false;
	Cmd = mgetc();
	clear_if_necessary();
	switch (Cmd) {
	    case ' ':
	    case KEY_ENTER:	no_op = true; break;
	    case KEY_CTRL|'g':	wizard(); break;
	    case KEY_CTRL|'l':	xredraw(); no_op = true; break;
	    case KEY_CTRL|'p':	bufferprint(); no_op = true; break;
	    case KEY_CTRL|'r':	xredraw(); no_op = true; break;
	    case KEY_CTRL|'w':	if (gamestatusp (CHEATED)) drawscreen(); break;
	    case KEY_CTRL|'x':
		if (gamestatusp (CHEATED) || Player.rank[ADEPT])
		    wish (1);
		break;
	    case 'd':	drop(); break;
	    case 'e':	eat(); break;
	    case 'i':	do_inventory_control(); break;
	    case 's':	countrysearch(); break;
	    case 'x':	examine(); break;
	    case 'E':	dismount_steed(); break;
	    case 'H':	hunt (Country->site(Player.x,Player.y).showchar()); break;
	    case 'I':	do_inventory_control(); break;
	    case 'O':	setoptions(); break;
	    case 'Q':	quit(); break;
	    case 'R':	rename_player(); break;
	    case 'S':	save(); break;
	    case 'V':	version(); break;
	    case '>':	enter_site (Country->site(Player.x,Player.y).locchar); break;
	    case '?':	help(); no_op = true; break;
	    case '4':
	    case 'h':	movepincountry (-1, 0); break;
	    case '2':
	    case 'j':	movepincountry (0, 1); break;
	    case '8':
	    case 'k':	movepincountry (0, -1); break;
	    case '6':
	    case 'l':	movepincountry (1, 0); break;
	    case '1':
	    case 'b':	movepincountry (-1, 1); break;
	    case '3':
	    case 'n':	movepincountry (1, 1); break;
	    case '7':
	    case 'y':	movepincountry (-1, -1); break;
	    case '9':
	    case 'u':	movepincountry (1, -1); break;
	    default:	commanderror(); no_op = true; break;
	}
    } while (no_op);
    screencheck (Player.y);
}

// no op a turn....
static void rest (void)
{
    if (random_range(20))
	return;
    static const char _boring[] =
	"Time passes slowly....\0"
	"Tick. Tock. Tick. Tock.\0"
	"Ho Hum.\0"
	"Beauty Sleep. Well, in your case, Ugly Sleep.\0"
	"And with Strange Aeons, even Death may die.\0"
	"La Di Da.\0"
	"Time keeps on tickin' tickin' -- into the future....\0"
	"Boooring!\0"
	"You think I like watching you sleep?\0"
	"You sure have an early bedtime!";
    print3 (zstrn (_boring, random_range(10), 10));
    morewait();
}

// read a scroll, book, tome, etc.
static void peruse (void)
{
    clearmsg();
    if (Player.status[BLINDED] > 0)
	print3 ("You're blind -- you can't read!!!");
    else if (Player.status[AFRAID] > 0)
	print3 ("You are too afraid to stop to read a scroll!");
    else {
	print1 ("Read -- ");
	int iidx = getitem (SCROLL);
	if (iidx == ABORT)
	    setgamestatus (SKIP_MONSTERS);
	else {
	    object& obj = Player.possessions[iidx];
	    if (obj.objchar != SCROLL) {
		print3 ("There's nothing written on ");
		nprint3 (itemid (obj));
	    } else {
		nprint1 ("You carefully unfurl the scroll....");
		morewait();
		item_use (obj);
		Player.remove_possession (iidx, 1);
	    }
	}
    }
}

static void quaff (void)
{
    clearmsg();
    print1 ("Quaff --");
    int iidx = getitem (POTION);
    if (iidx == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else {
	object& obj = Player.possessions[iidx];
	if (obj.objchar != POTION) {
	    print3 ("You can't drink ");
	    nprint3 (itemid (obj));
	} else {
	    print1 ("You drink it down.... ");
	    item_use (obj);
	    morewait();
	    Player.remove_possession (iidx, 1);
	}
    }
}

static void activate (void)
{
    do {
	clearmsg();
	print1 ("Activate -- item [i] or artifact [a] or quit [ESCAPE]?");
	char response = mcigetc();
	int iidx = ABORT;
	if (response == 'i')
	    iidx = getitem (THING);
	else if (response == 'a')
	    iidx = getitem (ARTIFACT);
	else if (response == KEY_ESCAPE) {
	    setgamestatus (SKIP_MONSTERS);
	    break;
	} else
	    continue;
	if (iidx == ABORT) {
	    setgamestatus (SKIP_MONSTERS);
	    continue;
	} else {
	    clearmsg();
	    print1 ("You activate it.... ");
	    item_use (Player.possessions[iidx]);
	    morewait();
	    Player.remove_possession (iidx, 1);
	}
    } while (false);
}

static void eat (void)
{
    clearmsg();
    print1 ("Eat --");
    int iidx = getitem (FOOD);
    if (iidx == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else {
	object& obj = Player.possessions[iidx];
	if (obj.objchar != FOOD && obj.objchar != CORPSE) {
	    print3 ("You can't eat ");
	    nprint3 (itemid (obj));
	} else {
	    if (obj.usef == I_FOOD)
		Player.food = max (0, Player.food + obj.aux);
	    item_use (obj);
	    Player.remove_possession (iidx, 1);
	    if (Current_Dungeon == E_COUNTRYSIDE) {
		Time += 100;
		hourly_check();
	    }
	}
    }
    foodcheck();
}

// search all adjacent spots for secrecy
static void search (int *searchval)
{
    if (Player.status[AFRAID] > 0)
	print3 ("You are too terror-stricken to stop to search for anything.");
    else {
	if (!gamestatusp (FAST_MOVE)) {
	    setgamestatus (FAST_MOVE);
	    *searchval = Searchnum;
	}
	for (unsigned i = 0; i < ArraySize(Dirs[0]); i++)
	    searchat (Player.x + Dirs[0][i], Player.y + Dirs[1][i]);
	drawvision (Player.x, Player.y);
    }
}

// pick up a thing where the player is
void pickup (void)
{
    if (!Level->thing(Player.x, Player.y))
	print3 ("There's nothing there!");
    else if (Player.status[SHADOWFORM])
	print3 ("You can't really interact with the real world in your shadowy state.");
    else
	pickup_at (Player.x, Player.y);
}

void drop (void)
{
    clearmsg();
    print1 ("Drop --");
    int iidx = getitem (CASH);
    if (iidx == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else {
	if (iidx == CASHVALUE)
	    drop_money();
	else if (!Player.possessions[iidx].used || !cursed (Player.possessions[iidx])) {
	    if (Player.possessions[iidx].number == 1) {
		p_drop_at (Player.x, Player.y, Player.possessions[iidx], 1);
		Player.remove_possession (iidx, 1);
	    } else {
		int n = getnumber (Player.possessions[iidx].number);
		p_drop_at (Player.x, Player.y, Player.possessions[iidx], n);
		Player.remove_possession (iidx, n);
	    }
	} else {
	    print3 ("You can't seem to get rid of: ");
	    nprint3 (itemid (Player.possessions[iidx]));
	}
    }
    calc_melee();
}

// talk to the animals -- learn their languages....
static void talk (void)
{
    int dx, dy, iidx = 0;
    char response;

    clearmsg();

    print1 ("Talk --");
    iidx = getdir();

    if (iidx == ABORT) {
	setgamestatus (SKIP_MONSTERS);
	xredraw();
	return;
    }
    dx = Dirs[0][iidx];
    dy = Dirs[1][iidx];

    monster* m = Level->creature(Player.x+dx,Player.y+dy);
    if (!m) {
	print3 ("There's nothing there to talk to!!!");
	setgamestatus (SKIP_MONSTERS);
    } else {
	menuclear();
	strcpy (Str1, "     Talk to ");
	strcat (Str1, m->monstring);
	strcat (Str1, ":");
	menuprint (Str1);
	menuprint ("\na: Greet.");
	menuprint ("\nb: Threaten.");
	menuprint ("\nc: Surrender.");
	menuprint ("\nESCAPE: Clam up.");
	showmenu();
	do
	    response = menugetc();
	while (response != 'a' && response != 'b' && response != 'c' && response != KEY_ESCAPE);
	switch (response) {
	    case 'a':	monster_talk (m); break;
	    case 'b':	threaten (m); break;
	    case 'c':	surrender (m); break;
	    default:	setgamestatus (SKIP_MONSTERS); break;
	}
    }
    xredraw();
}

// try to deactivate a trap
static void disarm (void)
{
    clearmsg();
    print1 ("Disarm -- ");
    int iidx = getdir();
    if (iidx == ABORT) {
	setgamestatus (SKIP_MONSTERS);
	return;
    }
    int x = Dirs[0][iidx] + Player.x;
    int y = Dirs[1][iidx] + Player.y;

    if (!inbounds (x, y))
	print3 ("Whoa, off the map...");
    else if (Level->site(x,y).locchar != TRAP)
	print3 ("You can't see a trap there!");
    else {
	if (random_range (50 + difficulty() * 5) < Player.dex * 2 + Player.level * 3 + Player.rank[THIEVES] * 10) {
	    print1 ("You disarmed the trap!");
	    if (random_range (100) < Player.dex + Player.rank[THIEVES] * 10) {
		unsigned componentId = NO_THING;
		switch (Level->site(x,y).p_locf) {
		    case L_TRAP_DART: componentId = THING_DART_TRAP_COMPONENT; break;
		    case L_TRAP_ACID: componentId = THING_ACID_TRAP_COMPONENT; break;
		    case L_TRAP_SNARE: componentId = THING_SNARE_TRAP_COMPONENT; break;
		    case L_TRAP_FIRE: componentId = THING_FIRE_TRAP_COMPONENT; break;
		    case L_TRAP_TELEPORT: componentId = THING_TELEPORT_TRAP_COMPONENT; break;
		    case L_TRAP_SLEEP_GAS: componentId = THING_SLEEP_TRAP_COMPONENT; break;
		    case L_TRAP_DISINTEGRATE: componentId = THING_DISINTEGRATE_TRAP_COMPONENT; break;
		    case L_TRAP_ABYSS: componentId = THING_ABYSS_TRAP_COMPONENT; break;
		    case L_TRAP_MANADRAIN: componentId = THING_MANADRAIN_TRAP_COMPONENT; break;
		}
		if (componentId != NO_THING) {
		    print2 ("You manage to retrieve the trap components!");
		    morewait();
		    learn_object (componentId);
		    gain_item (Objects[componentId]);
		    gain_experience (25);
		}
	    }
	    Level->site(x,y).p_locf = L_NO_OP;
	    Level->site(x,y).locchar = FLOOR;
	    lset (x, y, CHANGED);
	    gain_experience (5);
	} else if (random_range (10 + difficulty() * 2) > Player.dex) {
	    print1 ("You accidentally set off the trap!");
	    Player.x = x;
	    Player.y = y;
	    p_movefunction (Level->site(x,y).p_locf);
	} else
	    print1 ("You failed to disarm the trap.");
    }
}

// is it more blessed to give, or receive?
static void give (void)
{
    clearmsg();
    print1 ("Give to monster --");
    int dindex = getdir();
    if (dindex == ABORT) {
	setgamestatus (SKIP_MONSTERS);
	return;
    }
    int dx = Dirs[0][dindex];
    int dy = Dirs[1][dindex];
    monster* m = Level->creature(Player.x+dx,Player.y+dy);
    if (!m) {
	print3 ("There's nothing there to give something to!!!");
	setgamestatus (SKIP_MONSTERS);
	return;
    }
    clearmsg();
    print1 ("Give what? ");
    int iidx = getitem (CASH);
    if (iidx == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else if (iidx == CASHVALUE)
	give_money (m);
    else if (cursed (Player.possessions[iidx])) {
	print3 ("You can't even give away: ");
	nprint3 (itemid (Player.possessions[iidx]));
    } else {
	givemonster (*m, split_item (Player.possessions[iidx], 1));
	print2 ("Given: ");
	nprint2 (itemid (Player.possessions[iidx]));
	Player.remove_possession (iidx, 1);
	morewait();
	calc_melee();
    }
}

static void give_money (struct monster *m)
{
    object cash = detach_money();
    if (!cash.basevalue)
	setgamestatus (SKIP_MONSTERS);
    else
	givemonster (*m, cash);
}

// drops money, heh heh
static void drop_money (void)
{
    object money = detach_money();
    if (money.basevalue) {
	if (Current_Environment == E_CITY) {
	    print1 ("As soon as the money leaves your hand,");
	    print2 ("a horde of scrofulous beggars snatch it up and are gone!");
	} else
	    drop_at (Player.x, Player.y, money);
    } else
	setgamestatus (SKIP_MONSTERS);
}

// returns some money from player back into "money" item.
// for giving and dropping money
static object detach_money (void)
{
    object money = make_cash (difficulty());
    int c = get_money (Player.cash);
    money.basevalue = (c == ABORT ? 0 : c);
    return (money);
}

// zap a wand, of course
static void zapwand (void)
{
    clearmsg();
    if (Player.status[AFRAID] > 0) {
	print3 ("You are so terror-stricken you can't hold a wand straight!");
	return;
    }
    print1 ("Zap --");
    int iidx = getitem (STICK);
    if (iidx == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else {
	object& obj = Player.possessions[iidx];
	if (obj.objchar != STICK) {
	    print3 ("You can't zap: ");
	    nprint3 (itemid (obj));
	} else if (obj.charge < 1)
	    print3 ("Fizz.... Pflpt. Out of charges. ");
	else {
	    --obj.charge;
	    item_use (obj);
	}
    }
}

// cast a spell
static void magic (void)
{
    int iidx, pwrdrain;
    clearmsg();
    if (Player.status[AFRAID] > 0) {
	print3 ("You are too afraid to concentrate on a spell!");
	return;
    }
    iidx = getspell();
    xredraw();
    if (iidx == ABORT) {
	setgamestatus (SKIP_MONSTERS);
	return;
    }
    pwrdrain = Spells[iidx].powerdrain;
    if (Lunarity == 1)
	pwrdrain = pwrdrain / 2;
    else if (Lunarity == -1)
	pwrdrain = pwrdrain * 2;
    if (pwrdrain > Player.mana) {
	if (Lunarity == -1 && Player.mana >= pwrdrain / 2)
	    print3 ("The contrary moon has made that spell too draining! ");
	else
	    print3 ("You lack the power for that spell! ");
    } else {
	Player.mana -= pwrdrain;
	cast_spell (iidx);
    }
    dataprint();
}

// go upstairs ('<' command)
static void upstairs (void)
{
    if (Level->site(Player.x,Player.y).locchar != STAIRS_UP)
	print3 ("Not here!");
    else if (Level->site(Player.x,Player.y).p_locf == L_ESCALATOR)
	p_movefunction (Level->site(Player.x,Player.y).p_locf);
    else {
	if (gamestatusp (MOUNTED))
	    print2 ("You manage to get your horse upstairs.");
	print1 ("You ascend a level.");
	if (Level->depth <= 1) {
	    if (Level->environment == E_SEWERS)
		change_environment (E_CITY);
	    else
		change_environment (E_COUNTRYSIDE);
	} else
	    change_level (Level->depth, Level->depth - 1, false);
	roomcheck();
    }
    setgamestatus (SKIP_MONSTERS);
}

// go downstairs ('>' command)
static void downstairs (void)
{
    if (Level->site(Player.x,Player.y).locchar != STAIRS_DOWN)
	print3 ("Not here!");
    else if (Level->site(Player.x,Player.y).p_locf == L_ENTER_CIRCLE || Level->site(Player.x,Player.y).p_locf == L_ENTER_COURT)
	p_movefunction (Level->site(Player.x,Player.y).p_locf);
    else {
	if (gamestatusp (MOUNTED))
	    print2 ("You manage to get your horse downstairs.");
	if (Current_Environment == Current_Dungeon) {
	    print1 ("You descend a level.");
	    change_level (Level->depth, Level->depth + 1, false);
	    roomcheck();
	} else if ((Current_Environment == E_CITY) || (Last_Environment == E_CITY))
	    change_environment (E_SEWERS);
	else if (Current_Environment != Current_Dungeon)
	    print3 ("This stairway is deviant. You can't use it.");
    }
    setgamestatus (SKIP_MONSTERS);
}

// set various player options
// have to redefine in odefs for next full recompile
void setoptions (void)
{
    clearmsg();
    print1 ("Currently selected option is preceded by highlit >>");
    print2 ("Move selected option with '>' and '<', ESCAPE to quit.");
    menuclear();
    int slot = 0;
    while (true) {
	display_options (slot);
	int response = mcigetc();
	switch (response) {
	    case 'j':
	    case '>':
	    case KEY_DOWN: slot = min (slot + 1, NUMOPTIONS-1); break;
	    case 'k':
	    case '<':
	    case KEY_UP: slot = max (slot - 1, 0); break;
	    case KEY_HOME: slot = 0; break;
	    case KEY_LL: slot = NUMOPTIONS - 1; break;
	    case 't':
		if (slot <= NUMTFOPTIONS)
		    optionset (pow2 (slot - 1));
		else if (slot == VERBOSITY_LEVEL)
		    Verbosity = TERSE;
		else
		    print3 ("'T' is meaningless for this option.");
		break;
	    case 'f':
		if (slot <= NUMTFOPTIONS)
		    optionreset (pow2 (slot - 1));
		else
		    print3 ("'F' is meaningless for this option.");
		break;
	    case 'm':
		if (slot == VERBOSITY_LEVEL)
		    Verbosity = MEDIUM;
		else
		    print3 ("'M' is meaningless for this option.");
		break;
	    case 'v':
		if (slot == VERBOSITY_LEVEL)
		    Verbosity = VERBOSE;
		else
		    print3 ("'V' is meaningless for this option.");
		break;
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		if (slot == SEARCH_DURATION)
		    Searchnum = response - '0';
		else
		    print3 ("A number is meaningless for this option.");
		break;
	    case KEY_ESCAPE:
		xredraw();
		return;
	    default:
		print3 ("That response is meaningless for this option.");
		break;
	}
    }
}

// open a door
static void opendoor (void)
{
    int dir;
    int ox, oy;

    clearmsg();
    print1 ("Open --");
    dir = getdir();
    if (dir == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else {
	ox = Player.x + Dirs[0][dir];
	oy = Player.y + Dirs[1][dir];
	if (Level->site(ox,oy).locchar == OPEN_DOOR) {
	    print3 ("That door is already open!");
	    setgamestatus (SKIP_MONSTERS);
	} else if (Level->site(ox,oy).locchar == PORTCULLIS) {
	    print1 ("You try to lift the massive steel portcullis....");
	    if (random_range(100) < Player.str) {
		print2 ("Incredible. You bust a gut and lift the portcullis.");
		Level->site(ox,oy).locchar = FLOOR;
		lset (ox, oy, CHANGED);
	    } else {
		print2 ("Argh. You ruptured yourself.");
		p_damage (Player.str, UNSTOPPABLE, "a portcullis");
	    }
	} else if (Level->site(ox,oy).locchar != CLOSED_DOOR || loc_statusp (ox, oy, SECRET)) {
	    print3 ("You can't open that!");
	    setgamestatus (SKIP_MONSTERS);
	} else if (Level->site(ox,oy).aux == LOCKED)
	    print3 ("That door seems to be locked.");
	else {
	    Level->site(ox,oy).locchar = OPEN_DOOR;
	    lset (ox, oy, CHANGED);
	}
    }
}

// Try to destroy some location
static void bash_location (void)
{
    int dir;
    int ox, oy;

    clearmsg();
    print1 ("Bashing --");
    dir = getdir();
    if (dir == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else {
	ox = Player.x + Dirs[0][dir];
	oy = Player.y + Dirs[1][dir];
	if ((Current_Environment == E_CITY) && (ox == 0) && (oy == 0)) {
	    print1 ("Back Door WIZARD Mode!");
	    print2 ("You will invalidate your score if you proceed.");
	    morewait();
	    print1 ("Enable WIZARD Mode? [yn] ");
	    if (ynq1() == 'y') {
		print2 ("You feel like a cheater.");
		setgamestatus (CHEATED);
	    } else
		print2 ("A sudden tension goes out of the air....");
	} else {
	    if (Level->site(ox,oy).locchar == WALL) {
		print1 ("You hurl yourself at the wall!");
		p_damage (Player.str, NORMAL_DAMAGE, "a suicidal urge");
	    } else if (Level->site(ox,oy).locchar == OPEN_DOOR) {
		print1 ("You hurl yourself through the open door!");
		print2 ("Yaaaaah! ... thud.");
		morewait();
		Player.x = ox;
		Player.y = oy;
		p_damage (3, UNSTOPPABLE, "silliness");
		p_movefunction (Level->site(Player.x,Player.y).p_locf);
		setgamestatus (SKIP_MONSTERS);	// monsters are surprised...
	    } else if (Level->site(ox,oy).locchar == CLOSED_DOOR) {
		if (loc_statusp (ox, oy, SECRET)) {
		    print1 ("You found a secret door!");
		    lreset (ox, oy, SECRET);
		    lset (ox, oy, CHANGED);
		}
		if (Level->site(ox,oy).aux == LOCKED) {
		    if (random_range (50 + difficulty() * 10) < Player.str) {
			Player.x = ox;
			Player.y = oy;
			print2 ("You blast the door off its hinges!");
			Level->site(ox,oy).locchar = FLOOR;
			lset (ox, oy, CHANGED);
			p_movefunction (Level->site(Player.x,Player.y).p_locf);
			setgamestatus (SKIP_MONSTERS);	// monsters are surprised...
		    } else {
			print1 ("Crash! The door holds.");
			if (random_range (30) > Player.str)
			    p_damage (max (1, statmod (Player.str)), UNSTOPPABLE, "a door");
		    }
		} else {
		    Player.x = ox;
		    Player.y = oy;
		    print2 ("You bash open the door!");
		    if (random_range (30) > Player.str)
			p_damage (1, UNSTOPPABLE, "a door");
		    Level->site(ox,oy).locchar = OPEN_DOOR;
		    lset (ox, oy, CHANGED);
		    p_movefunction (Level->site(Player.x,Player.y).p_locf);
		    setgamestatus (SKIP_MONSTERS);	// monsters are surprised...
		}
	    } else if (Level->site(ox,oy).locchar == STATUE) {
		statue_random (ox, oy);
	    } else if (Level->site(ox,oy).locchar == PORTCULLIS) {
		print1 ("Really, you don't have a prayer.");
		if (random_range (1000) < Player.str) {
		    print2 ("The portcullis flies backwards into a thousand fragments.");
		    print3 ("Wow. What a stud.");
		    gain_experience (100);
		    Level->site(ox,oy).locchar = FLOOR;
		    Level->site(ox,oy).p_locf = L_NO_OP;
		    lset (ox, oy, CHANGED);
		} else {
		    print2 ("You only hurt yourself on the 3'' thick steel bars.");
		    p_damage (Player.str, UNSTOPPABLE, "a portcullis");
		}
	    } else if (Level->site(ox,oy).locchar == ALTAR) {
		if ((Player.patron > 0) && (Level->site(ox,oy).aux == Player.patron)) {
		    print1 ("You have a vision! An awesome angel hovers over the altar.");
		    print2 ("The angel says: 'You twit, don't bash your own altar!'");
		    print3 ("The angel slaps you upside the head for your presumption.");
		    p_damage (Player.hp - 1, UNSTOPPABLE, "an annoyed angel");
		} else if (Level->site(ox,oy).aux == 0) {
		    print1 ("The feeble powers of the minor godling are not enough to");
		    print2 ("protect his altar! The altar crumbles away to dust.");
		    print3 ("You feel almost unbearably smug.");
		    Level->site(ox,oy).locchar = RUBBLE;
		    Level->site(ox,oy).p_locf = L_RUBBLE;
		    lset (ox, oy, CHANGED);
		    gain_experience (5);
		} else {
		    print1 ("You have successfully annoyed a major deity. Good job.");
		    print2 ("Zzzzap! A bolt of godsfire strikes!");
		    if (Player.rank[PRIESTHOOD] >= LAY)
			print3 ("Your own deity's aegis defends you from the bolt!");
		    p_damage (max (0, random_range (100) - Player.rank[PRIESTHOOD] * 20), UNSTOPPABLE, "a bolt of godsfire");
		    if (Player.rank[PRIESTHOOD] * 20 + Player.pow + Player.level > random_range (200)) {
			morewait();
			print1 ("The altar crumbles...");
			Level->site(ox,oy).locchar = RUBBLE;
			Level->site(ox,oy).p_locf = L_RUBBLE;
			lset (ox, oy, CHANGED);
			morewait();
			if (Player.rank[PRIESTHOOD]) {
			    print2 ("You sense your deity's pleasure with you.");
			    morewait();
			    print3 ("You are surrounded by a golden glow.");
			    cleanse (1);
			    heal (10);
			}
			gain_experience (500);
		    }
		}
	    } else {
		print3 ("You restrain yourself from total silliness.");
		setgamestatus (SKIP_MONSTERS);
	    }
	}
    }
}

// attempt destroy an item
void bash_item (void)
{
    clearmsg();
    print1 ("Destroy an item --");
    int item = getitem (NULL_ITEM);
    if (item == CASHVALUE)
	print3 ("Can't destroy cash!");
    else if (item != ABORT) {
	object& obj = Player.possessions[item];
	if (Player.str + random_range (20) > obj.fragility + random_range (20)) {
	    if (damage_item(&obj) && Player.alignment < 0) {
		print2 ("That was fun....");
		gain_experience (obj.level * obj.level * 5);
	    }
	} else {
	    if (obj.objchar == WEAPON) {
		print2 ("The weapon turned in your hand -- you hit yourself!");
		p_damage (random_range (obj.dmg + absv(obj.plus)), NORMAL_DAMAGE, "a failure at vandalism");
	    } else if (obj.objchar == ARTIFACT) {
		print2 ("Uh Oh -- Now you've gotten it angry....");
		p_damage (obj.level * 10, UNSTOPPABLE, "an enraged artifact");
	    } else {
		print2 ("Ouch! Damn thing refuses to break...");
		p_damage (1, UNSTOPPABLE, "a failure at vandalism");
	    }
	}
    }
}

void save (void)
{
    clearmsg();
    if (gamestatusp (ARENA_MODE))
	print3 ("Can't save the game in the arena!");
    else if (Current_Environment == E_ABYSS)
	print3 ("Can't save the game in the Adept's Challenge!");
    else if (Current_Environment == E_TACTICAL_MAP)
	print3 ("Can't save the game in the tactical map!");
    else {
	print1 ("Confirm Save? [yn] ");
	if (ynq1() == 'y' && save_game())
	    return;
    }
    setgamestatus (SKIP_MONSTERS);	// if we get here, we failed to save
}

// close a door
static void closedoor (void)
{
    int dir;
    int ox, oy;

    clearmsg();

    print1 ("Close --");
    dir = getdir();
    if (dir == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else {
	ox = Player.x + Dirs[0][dir];
	oy = Player.y + Dirs[1][dir];
	if (Level->site(ox,oy).locchar == CLOSED_DOOR) {
	    print3 ("That door is already closed!");
	    setgamestatus (SKIP_MONSTERS);
	} else if (Level->site(ox,oy).locchar != OPEN_DOOR) {
	    print3 ("You can't close that!");
	    setgamestatus (SKIP_MONSTERS);
	} else
	    Level->site(ox,oy).locchar = CLOSED_DOOR;
	lset (ox, oy, CHANGED);
    }
}

// handle a h,j,k,l, etc.
static void moveplayer (int dx, int dy)
{
    if (p_moveable (Player.x + dx, Player.y + dy)) {

	if (Player.status[IMMOBILE] > 0) {
	    resetgamestatus (FAST_MOVE);
	    print3 ("You are unable to move");
	} else if (Player.maxweight < Player.itemweight && random_range (2) && !Player.status[LEVITATING]) {
	    if (gamestatusp (MOUNTED)) {
		print1 ("Your horse refuses to carry you and your pack another step!");
		print2 ("Your steed bucks wildly and throws you off!");
		p_damage (10, UNSTOPPABLE, "a cruelly abused horse");
		resetgamestatus (MOUNTED);
		summon (-1, HORSE);
	    } else {
		p_damage (1, UNSTOPPABLE, "a rupture");
		print3 ("The weight of your pack drags you down. You can't move.");
	    }
	} else {
	    Player.x += dx;
	    Player.y += dy;
	    p_movefunction (Level->site(Player.x,Player.y).p_locf);

	    // causes moves to take effectively 30 seconds in town without
	    // monsters being sped up compared to player
	    if ((Current_Environment == E_CITY) || (Current_Environment == E_VILLAGE)) {
		twiddle = !twiddle;
		if (twiddle) {
		    Time++;
		    if (Time % 10 == 0)
			tenminute_check();
		    else
			minute_status_check();
		}
	    }

	    // this test protects against player entering countryside and still
	    // having effects from being on the Level, a kluge, but hey,...
	    if (Current_Environment != E_COUNTRYSIDE) {
		if (gamestatusp (FAST_MOVE))
		    if (Level->thing(Player.x,Player.y) || (optionp (RUNSTOP) && loc_statusp (Player.x, Player.y, STOPS)))
			resetgamestatus (FAST_MOVE);
		if (Level->thing(Player.x, Player.y) && optionp (PICKUP))
		    pickup();
	    }
	}
    } else if (gamestatusp (FAST_MOVE)) {
	drawvision (Player.x, Player.y);
	resetgamestatus (FAST_MOVE);
    }
}

// handle a h,j,k,l, etc.
static void movepincountry (int dx, int dy)
{
    bool takestime = true;
    if ((Player.maxweight < Player.itemweight) && random_range (2) && (!Player.status[LEVITATING])) {
	if (gamestatusp (MOUNTED)) {
	    print1 ("Your horse refuses to carry you and your pack another step!");
	    print2 ("Your steed bucks wildly and throws you off!");
	    p_damage (10, UNSTOPPABLE, "a cruelly abused horse");
	    resetgamestatus (MOUNTED);
	    morewait();
	    print1 ("With a shrill neigh of defiance, your former steed gallops");
	    print2 ("off into the middle distance....");
	    if (!Player.pack.empty()) {
		morewait();
		print1 ("You remember (too late) that the contents of your pack");
		print2 ("were kept in your steed's saddlebags!");
		Player.pack.clear();
		calc_melee();
	    }
	} else {
	    p_damage (1, UNSTOPPABLE, "a rupture");
	    print3 ("The weight of your pack drags you down. You can't move.");
	}
    } else {
	if (gamestatusp (LOST)) {
	    print3 ("Being lost, you strike out randomly....");
	    morewait();
	    dx = random_range (3) - 1;
	    dy = random_range (3) - 1;
	}
	if (p_country_moveable (Player.x + dx, Player.y + dy)) {
	    if (Player.status[IMMOBILE] > 0)
		print3 ("You are unable to move");
	    else {
		Player.x += dx;
		Player.y += dy;
		if (!gamestatusp (MOUNTED) && Player.has_possession(O_BOOTS)) {
		    if (Player.possessions[O_BOOTS].usef == I_BOOTS_7LEAGUE) {
			takestime = false;
			if (Player.possessions[O_BOOTS].blessing < 0) {
			    print1 ("Whooah! -- Your boots launch you into the sky....");
			    print2 ("You come down in a strange location....");
			    Player.x = random_range (Level->width);
			    Player.y = random_range (Level->height);
			    morewait();
			    clearmsg();
			    print1 ("Your boots disintegrate with a malicious giggle...");
			    Player.remove_possession (O_BOOTS);
			} else if (!object_is_known (Player.possessions[O_BOOTS])) {
			    print1 ("Wow! Your boots take you 7 leagues in a single stride!");
			    learn_object (Player.possessions[O_BOOTS]);
			}
		    }
		}
		if (gamestatusp (LOST) && (Precipitation < 1) && c_statusp (Player.x, Player.y, SEEN)) {
		    print3 ("Ah! Now you know where you are!");
		    morewait();
		    resetgamestatus (LOST);
		} else if (gamestatusp (LOST)) {
		    print3 ("You're still lost.");
		    morewait();
		}
		if (Precipitation > 0)
		    Precipitation--;
		c_set (Player.x, Player.y, SEEN);
		terrain_check (takestime);
	    }
	}
    }
}

// look at some spot
static void examine (void)
{
    int x = Player.x, y = Player.y, drewmenu = false;
    clearmsg();
    // WDT HACK: I'm not sure I buy that one shouldn't be able to examine
    // when one is blind.  However, the 'right' way to do it is certainly
    // too difficult (I would expect to be able to examine only the items
    // I actually recall).  So, for now I'll use David Given's compromise.
    // 12/30/98
    if (Player.status[BLINDED] > 0) {
	mprint ("You're blind - you can't examine things.");
	return;
    }
    setgamestatus (SKIP_MONSTERS);
    mprint ("Examine --");
    setspot (&x, &y);
    if (inbounds (x, y)) {
	clearmsg();
	if (Current_Environment == E_COUNTRYSIDE) {
	    if (!c_statusp (x, y, SEEN))
		print3 ("How should I know what that is?");
	    else {
		mprint ("That terrain is:");
		mprint (countryid (Country->site(x,y).showchar()));
	    }
	} else if (!view_los_p (Player.x, Player.y, x, y))
	    print3 ("I refuse to examine something I can't see.");
	else {
	    clearmsg();
	    monster* sm = Level->creature(x,y);
	    if (sm)
		mprint (mstatus_string (sm));
	    else if ((Player.x == x) && (Player.y == y))
		describe_player();
	    if (loc_statusp (x, y, SECRET))
		print2 ("An age-worn stone wall.");
	    else {
		switch (Level->site(x,y).locchar) {
		    case SPACE:		print2 ("An infinite void."); break;
		    case PORTCULLIS:	print2 ("A heavy steel portcullis"); break;
		    case ABYSS:		print2 ("An entrance to the infinite abyss"); break;
		    case FLOOR:
			if (Current_Dungeon == Current_Environment)
			    print2 ("A dirty stone floor.");
			else
			    print2 ("The ground.");
			break;
		    case WALL:
			if (Level->site(x,y).aux == 0)		print2 ("A totally impervious wall.");
			else if (Level->site(x,y).aux < 10)	print2 ("A pitted concrete wall.");
			else if (Level->site(x,y).aux < 30)	print2 ("An age-worn sandstone wall.");
			else if (Level->site(x,y).aux < 50)	print2 ("A smooth basalt wall.");
			else if (Level->site(x,y).aux < 70)	print2 ("A solid granite wall.");
			else if (Level->site(x,y).aux < 90)	print2 ("A wall of steel.");
			else if (Level->site(x,y).aux < 210) {
			    if (Current_Environment == E_CITY)
				print2 ("A thick wall of Rampart bluestone");
			    else
				print2 ("A magically reinforced wall.");
			} else
			    print2 ("An almost totally impervious wall.");
			break;
		    case RUBBLE:	print2 ("A dangerous-looking pile of rubble."); break;
		    case SAFE:		print2 ("A steel safe inset into the floor."); break;
		    case CLOSED_DOOR:	print2 ("A solid oaken door, now closed."); break;
		    case OPEN_DOOR:	print2 ("A solid oaken door, now open."); break;
		    case STATUE:	print2 ("A strange-looking statue."); break;
		    case STAIRS_UP:	print2 ("A stairway leading up."); break;
		    case STAIRS_DOWN:	print2 ("A stairway leading down...."); break;
		    case TRAP:		print2 (trapid (Level->site(x,y).p_locf)); break;
		    case HEDGE:
			if (Level->site(x,y).p_locf == L_EARTH_STATION)
			    print2 ("A weird fibrillation of oozing tendrils.");
			else
			    print2 ("A brambly, thorny hedge.");
			break;
		    case LAVA:		print2 ("A bubbling pool of lava."); break;
		    case LIFT:		print2 ("A strange glowing disk."); break;
		    case ALTAR:		print2 ("An altar."); break;
		    case CHAIR:		print2 ("A chair."); break;
		    case WHIRLWIND:	print2 ("A strange cyclonic electrical storm."); break;
		    case WATER:
			if (Level->site(x,y).p_locf == L_WATER)
			    print2 ("A deep pool of water.");
			else if (Level->site(x,y).p_locf == L_CHAOS)
			    print2 ("A pool of primal chaos.");
			else if (Level->site(x,y).p_locf == L_WATER_STATION)
			    print2 ("A bubbling pool of acid.");
			else
			    print2 ("An eerie pool of water.");
			break;
		    case FIRE:		print2 ("A curtain of fire."); break;
		    default:		print2 ("Wow, I haven't the faintest idea!"); break;
		}
	    }
	    if (Level->thing(x,y) && !loc_statusp (x, y, SECRET)) {
		drewmenu = true;
		menuclear();
		menuprint ("Things on floor:\n");
		foreach (o, Level->things) {
		    if (o->x == x && o->y == y) {
			menuprint ("\n");
			menuprint (itemid(o));
		    }
		}
		showmenu();
	    }
	    morewait();
	    sign_print (x, y, true);
	}
    }
    if (drewmenu)
	xredraw();
}

static void help (void)
{
    char c;
    do {
	clearmsg();
	print1 ("Please enter the letter indicating what topic you want help on.");
	menuclear();
	menuprint ("a: Overview\n");
	menuprint ("b: Characters\n");
	menuprint ("c: Inventories\n");
	menuprint ("d: Movement\n");
	menuprint ("e: Combat\n");
	menuprint ("f: Bugs\n");
	menuprint ("g: Magic\n");
	menuprint ("h: The Countryside\n");
	menuprint ("i: The Screen Display\n");
	menuprint ("j: Saving and Restoring\n");
	menuprint ("k: Options Settings\n");
	menuprint ("l: Dungeon/City/Other Command List\n");
	menuprint ("m: Countryside Command List\n");
	menuprint ("ESCAPE: Forget the whole thing.");
	showmenu();
	c = mcigetc();
	if (c >= 'a' && c <= 'm') {
	    static const char* _helps[] = {
		Help_Overview, Help_Character, Help_Inventory, Help_Movement,
		Help_Combat, Help_Bugs, Help_Magic, Help_Countryside, Help_Screen,
		Help_Saving, Help_Options, Help_CityCommands, Help_CountryCommands
	    };
	    displayfile (_helps[c-'a']);
	}
    } while (c != KEY_ESCAPE);
    xredraw();
}

static void version (void)
{
    setgamestatus (SKIP_MONSTERS);
    print3 (OMEGA_NAME " " OMEGA_VERSTRING);
}

static void fire (void)
{
    clearmsg();
    print1 ("Fire/Throw --");
    int ii = getitem (NULL_ITEM);
    if (ii == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else if (ii == CASHVALUE)
	print3 ("Can't fire money at something!");
    else if (cursed (Player.possessions[ii]) == true+true)
	print3 ("You can't seem to get rid of it!");
    else if (Player.possessions[O_WEAPON_HAND].id == WEAPON_CROSSBOW &&
	     Player.possessions[O_WEAPON_HAND].aux != LOADED && Player.possessions[ii].id == WEAPON_BOLT) {
	mprint ("You crank back the crossbow and load a bolt.");
	Player.possessions[O_WEAPON_HAND].aux = LOADED;
	Player.remove_possession (ii, 1);
    } else {
	object& obj = Player.possessions[ii];
	int x1, y1, x2, y2;
	x1 = x2 = Player.x;
	y1 = y2 = Player.y;
	setspot (&x2, &y2);
	if (x2 == Player.x && y2 == Player.y) {
	    mprint ("You practice juggling for a moment or two.");
	    p_drop_at (x2, y2, obj, 1);
	} else {
	    do_object_los (obj.objchar, &x1, &y1, x2, y2);
	    monster *m;
	    if ((m = Level->creature(x1,y1))) {
		if (obj.dmg == 0) {
		    if (m->treasure > 0) {	// the monster can have treasure/objects
			mprint ("Your gift is caught!");
			givemonster (*m, split_item (obj, 1));
		    } else {
			mprint ("Your thrown offering is ignored.");
			setgamestatus (SUPPRESS_PRINTING);
			p_drop_at (x1, y1, obj, 1);
			resetgamestatus (SUPPRESS_PRINTING);
		    }
		} else if (obj.aux == I_SCYTHE) {
		    mprint ("It isn't very aerodynamic... you miss.");
		    setgamestatus (SUPPRESS_PRINTING);
		    p_drop_at (x1, y1, obj, 1);
		    resetgamestatus (SUPPRESS_PRINTING);
		} else if (hitp (Player.hit, m->ac)) {	// ok already, hit the damn thing
		    weapon_use (2 * statmod (Player.str), &obj, m);
		    if ((obj.id != WEAPON_ARROW && obj.id != WEAPON_BOLT) || random_range(4)) {
			setgamestatus (SUPPRESS_PRINTING);
			p_drop_at (x1, y1, obj, 1);
			resetgamestatus (SUPPRESS_PRINTING);
		    }
		} else {
		    mprint ("You miss it.");
		    setgamestatus (SUPPRESS_PRINTING);
		    p_drop_at (x1, y1, obj, 1);
		    resetgamestatus (SUPPRESS_PRINTING);
		}
	    } else {
		setgamestatus (SUPPRESS_PRINTING);
		p_drop_at (x1, y1, obj, 1);
		resetgamestatus (SUPPRESS_PRINTING);
	    }
	}
	Player.remove_possession (ii, 1);
    }
}

void quit (void)
{
    clearmsg();
    mprint ("Quit: Are you sure? [yn] ");
    if (ynq() == 'y') {
	if (Player.rank[ADEPT])
	    display_bigwin();
	endgraf();
	exit (0);
    } else
	resetgamestatus (SKIP_MONSTERS);
}

// rest in 10 second segments so if woken up by monster won't die automatically....
static void nap (void)
{
    static int naptime;
    if (gamestatusp (FAST_MOVE)) {
	if (naptime-- < 1) {
	    clearmsg();
	    mprint ("Yawn. You wake up.");
	    resetgamestatus (FAST_MOVE);
	    drawvision (Player.x, Player.y);
	}
    } else {
	clearmsg();
	mprint ("Rest for how long? (in minutes) ");
	naptime = (int) parsenum();
	if (naptime > 600) {
	    print3 ("You can only sleep up to 10 hours (600 minutes)");
	    naptime = 3600;
	} else
	    naptime *= 6;
	if (naptime > 1) {
	    clearmsg();
	    setgamestatus (FAST_MOVE);
	    mprint ("Resting.... ");
	}
    }
}

static void wizard (void)
{
    setgamestatus (SKIP_MONSTERS);
    if (gamestatusp (CHEATED))
	mprint ("You're already in wizard mode!");
    else {
	clearmsg();
	mprint ("Really try to enter wizard mode? [yn] ");
	if (ynq() == 'y') {
	    setgamestatus (CHEATED);
	    mprint ("Wizard mode set.");
	}
    }
}

// Jump, that is
static void vault (void)
{
    int x = Player.x, y = Player.y, jumper = 0;
    clearmsg();
    if (Player.has_possession(O_BOOTS) && Player.possessions[O_BOOTS].usef == I_BOOTS_JUMPING)
	jumper = 2;
    if (Player.status[IMMOBILE] > 0) {
	resetgamestatus (FAST_MOVE);
	print3 ("You are unable to move");
    } else {
	setgamestatus (SKIP_MONSTERS);
	mprint ("Jump where?");
	setspot (&x, &y);
	if (!los_p (Player.x, Player.y, x, y))
	    print3 ("The way is obstructed.");
	else if (Player.itemweight > Player.maxweight)
	    print3 ("You are too burdened to jump anywhere.");
	else if (distance (x, y, Player.x, Player.y) > max (2, statmod (Player.agi) + 2) + jumper)
	    print3 ("The jump is too far for you.");
	else if (Level->creature(x,y))
	    print3 ("You can't jump on another creature.");
	else if (!p_moveable (x, y))
	    print3 ("You can't jump there.");
	else {
	    resetgamestatus (SKIP_MONSTERS);
	    Player.x = x;
	    Player.y = y;
	    if ((!jumper) && (random_range (30) > Player.agi)) {
		mprint ("Oops -- took a tumble.");
		setgamestatus (SKIP_PLAYER);
		p_damage ((Player.itemweight / 250), UNSTOPPABLE, "clumsiness");
	    }
	    p_movefunction (Level->site(Player.x,Player.y).p_locf);
	    if (Current_Environment != E_COUNTRYSIDE)
		if (Level->thing(Player.x,Player.y) && optionp (PICKUP))
		    pickup();
	}
    }
}

// Sets sequence of combat maneuvers.
static void tacoptions (void)
{
    unsigned actionsleft, done, place;
    char defatt;
    const char *attstr;
    const char *defstr;	// for the default setting
    int draw_again = 1;

    setgamestatus (SKIP_MONSTERS);

    done = false;
    actionsleft = maneuvers();
    place = 0;
    do {
	if (draw_again) {
	    menuclear();
	    menuprint ("Enter a combat maneuvers sequence.\n");
	    menuprint ("? for help, ! for default, backspace to start again,\n");
	    menuprint (" RETURN to save sequence\n");
	    showmenu();
	    draw_again = 0;
	}
	clearmsg();
	mprint ("Maneuvers Left:");
	mnumprint (actionsleft);
	switch (mgetc()) {
	    case '?':
		displayfile (Help_Combat);
		draw_again = 1;
		break;
	    case 'a':
	    case 'A':
		if (actionsleft < 1)
		    print3 ("No more maneuvers!");
		else {
		    if (!Player.has_possession(O_WEAPON_HAND)) {
			Player.meleestr[place] = 'C';
			menuprint ("\nPunch:");
		    } else if (Player.possessions[O_WEAPON_HAND].type == THRUSTING) {
			Player.meleestr[place] = 'T';
			menuprint ("\nThrust:");
		    } else if (Player.possessions[O_WEAPON_HAND].type == STRIKING) {
			Player.meleestr[place] = 'C';
			menuprint ("\nStrike:");
		    } else {
			menuprint ("\nCut:");
			Player.meleestr[place] = 'C';
		    }
		    place++;
		    Player.meleestr[place] = getlocation();
		    place++;
		    actionsleft--;
		}
		break;
	    case 'b':
	    case 'B':
		if (actionsleft < 1)
		    print3 ("No more maneuvers!");
		else {
		    Player.meleestr[place] = 'B';
		    if (!Player.has_possession(O_WEAPON_HAND))
			menuprint ("\nDodge (from):");
		    else if (Player.possessions[O_WEAPON_HAND].type == THRUSTING)
			menuprint ("\nParry:");
		    else
			menuprint ("\nBlock:");
		    place++;
		    Player.meleestr[place] = getlocation();
		    place++;
		    actionsleft--;
		}
		break;
	    case 'l':
	    case 'L':
		if (actionsleft < 2)
		    print3 ("Not enough maneuvers to lunge!");
		else {
		    if (Player.has_possession(O_WEAPON_HAND)) {
			if (Player.possessions[O_WEAPON_HAND].type != MISSILE) {
			    menuprint ("\nLunge:");
			    Player.meleestr[place] = 'L';
			    place++;
			    Player.meleestr[place] = getlocation();
			    place++;
			    actionsleft -= 2;
			} else {
			    print3 ("Can't lunge with a missile weapon!");
			    morewait();
			}
		    } else {
			print3 ("Can't lunge without a weapon!");
			morewait();
		    }
		}
		break;
	    case 'r':
	    case 'R':
		if (actionsleft < 2)
		    print3 ("Not enough maneuvers to riposte!");
		else {
		    if (Player.has_possession(O_WEAPON_HAND)) {
			if (Player.possessions[O_WEAPON_HAND].type == THRUSTING) {
			    Player.meleestr[place++] = 'R';
			    menuprint ("\nRiposte:");
			    Player.meleestr[place++] = getlocation();
			    actionsleft -= 2;
			} else {
			    print3 ("Can't riposte without a thrusting weapon!");
			    morewait();
			}
		    } else {
			print3 ("Can't riposte without a thrusting weapon!");
			morewait();
		    }
		}
		break;
	    case KEY_BACKSPACE:
		place = 0;
		actionsleft = maneuvers();
		draw_again = 1;
		break;
	    case '!':
		if (!Player.has_possession(O_WEAPON_HAND)) {
		    defatt = 'C';
		    attstr = "Punch";
		} else if (Player.possessions[O_WEAPON_HAND].type == THRUSTING) {
		    defatt = 'T';
		    attstr = "Thrust";
		} else if (Player.possessions[O_WEAPON_HAND].type == STRIKING) {
		    defatt = 'C';
		    attstr = "Strike";
		} else {
		    defatt = 'C';
		    attstr = "Cut";
		}
		if (!Player.has_possession(O_WEAPON_HAND))
		    defstr = "Dodge";
		else if (Player.possessions[O_WEAPON_HAND].type == THRUSTING)
		    defstr = "Parry";
		else
		    defstr = "Block";
		menuclear();
		menuprint ("Enter a combat maneuvers sequence.\n");
		menuprint ("? for help, ! for default, backspace to start again,\n");
		menuprint (" RETURN to save sequence\n\n");
		for (place = 0; place < maneuvers(); place++)
		    if (place & 1) {	// every 2nd time around
			Player.meleestr[place * 2] = 'B';
			Player.meleestr[(place * 2) + 1] = 'C';
			menuprint (defstr);
			menuprint (" Center.\n");
		    } else {
			Player.meleestr[place * 2] = defatt;
			Player.meleestr[(place * 2) + 1] = 'C';
			menuprint (attstr);
			menuprint (" Center.\n");
		    }
		actionsleft = 0;
		showmenu();
		Player.meleestr[place * 2] = '\0';
		break;
	    case KEY_ENTER:
	    case KEY_ESCAPE:
		done = true;
		break;
	}
	//    if (actionsleft < 1) morewait();
    } while (!done);
    xredraw();
    Player.meleestr[place] = 0;
}

// Do the Artful Dodger trick
static void pickpocket (void)
{
    clearmsg();
    mprint ("Pickpocketing --");
    clearmsg();
    int ii = getdir();
    if (ii == ABORT) {
	setgamestatus (SKIP_MONSTERS);
	return;
    }
    int dx = Dirs[0][ii];
    int dy = Dirs[1][ii];

    monster* m = Level->creature(Player.x + dx, Player.y + dy);
    if (!m) {
	print3 ("There's nothing there to steal from!!!");
	setgamestatus (SKIP_MONSTERS);
	return;
    }
    if (m->id == GUARD) {
	mprint ("Trying to steal from a guardsman, eh?");
	mprint ("Not a clever idea.");
	if (Player.cash > 0) {
	    mprint ("As a punitive fine, the guard takes all your money.");
	    Player.cash = 0;
	    dataprint();
	} else {
	    mprint ("The guardsman places you under arrest.");
	    morewait();
	    send_to_jail();
	}
    } else if (m->possessions.empty()) {
	mprint ("You couldn't find anything worth taking!");
	mprint ("But you managed to annoy it...");
	m_status_set (m, HOSTILE);
    } else if (Player.dex * 5 + Player.rank[THIEVES] * 20 + random_range (100) > random_range (100) + m->level * 20) {
	mprint ("You successfully complete your crime!");
	mprint ("You stole:");
	mprint (itemid (&m->possessions[0]));
	--Player.alignment;
	gain_experience (m->level * m->level);
	gain_item (m->possessions[0]);
	m->possessions.erase (m->possessions.begin());
    } else
	mprint ("Pickpocketing failed. This is just not your day.");
}

void rename_player (void)
{
    setgamestatus (SKIP_MONSTERS);
    clearmsg();
    mprint ("Rename Character: ");
    strcpy (Str1, msgscanstring());
    if (strlen (Str1) == 0)
	mprint (Player.name);
    else {
	if (Str1[0] >= 'a' && Str1[0] <= 'z')
	    Str1[0] += 'A' - 'a';
	Player.name = Str1;
    }
    sprintf (Str1, "Henceforth, you shall be known as %s", Player.name.c_str());
    print2 (Str1);
}

static void abortshadowform (void)
{
    setgamestatus (SKIP_MONSTERS);
    if (Player.status[SHADOWFORM] && (Player.status[SHADOWFORM] < 1000)) {
	mprint ("You abort your spell of Shadow Form.");
	Player.immunity[NORMAL_DAMAGE]--;
	Player.immunity[ACID]--;
	Player.immunity[THEFT]--;
	Player.immunity[INFECTION]--;
	mprint ("You feel less shadowy now.");
	Player.status[SHADOWFORM] = 0;
    }
}

static void tunnel (void)
{
    clearmsg();
    mprint ("Tunnel -- ");
    int dir = getdir();
    if (dir == ABORT)
	setgamestatus (SKIP_MONSTERS);
    else {
	int ox = Player.x + Dirs[0][dir];
	int oy = Player.y + Dirs[1][dir];
	if (loc_statusp (ox, oy, SECRET))
	    mprint ("You have no success as yet.");
	else if (Level->site(ox,oy).locchar != WALL) {
	    print3 ("You can't tunnel through that!");
	    setgamestatus (SKIP_MONSTERS);
	} else {
	    int aux = Level->site(ox,oy).aux;
	    if (!random_range(20)) {
		if (!Player.has_possession(O_WEAPON_HAND)) {
		    mprint ("Ouch! broke a fingernail...");
		    p_damage (Player.str / 6, UNSTOPPABLE, "a broken fingernail");
		} else if (Player.possessions[O_WEAPON_HAND].type == THRUSTING || (Player.possessions[O_WEAPON_HAND].type != STRIKING && Player.possessions[O_WEAPON_HAND].fragility < random_range (20))) {
		    mprint ("Clang! Uh oh...");
		    damage_item (&Player.possessions[O_WEAPON_HAND]);
		} else
		    mprint ("Your digging implement shows no sign of breaking.");
	    }
	    if (!Player.has_possession(O_WEAPON_HAND)) {
		if ((aux > 0) && ((Player.str / 3) + random_range (100) > aux)) {
		    mprint ("You carve a tunnel through the stone!");
		    Level->tunnelcheck();
		    Level->site(ox,oy).locchar = RUBBLE;
		    Level->site(ox,oy).p_locf = L_RUBBLE;
		    lset (ox, oy, CHANGED);
		} else
		    mprint ("No joy.");
	    } else if (Player.possessions[O_WEAPON_HAND].type == THRUSTING) {
		if (aux > 0 && Player.possessions[O_WEAPON_HAND].dmg * 2 + random_range (100) > aux) {
		    mprint ("You carve a tunnel through the stone!");
		    Level->tunnelcheck();
		    Level->site(ox,oy).locchar = RUBBLE;
		    Level->site(ox,oy).p_locf = L_RUBBLE;
		    lset (ox, oy, CHANGED);
		} else
		    mprint ("No luck.");
	    } else if (aux > 0 && Player.possessions[O_WEAPON_HAND].dmg + random_range (100) > aux) {
		mprint ("You carve a tunnel through the stone!");
		Level->tunnelcheck();
		Level->site(ox,oy).locchar = RUBBLE;
		Level->site(ox,oy).p_locf = L_RUBBLE;
		lset (ox, oy, CHANGED);
	    } else
		mprint ("You have no success as yet.");
	}
    }
}

static void hunt (int terrain)
{
    int fertility = 0;
    switch (terrain) {
	case SWAMP:
	    mprint ("You hesitate to hunt for food in the marshy wasteland.");
	    break;
	case VOLCANO:
	case CASTLE:
	case TEMPLE:
	case CAVES:
	case STARPEAK:
	case MAGIC_ISLE:
	case DRAGONLAIR:
	    mprint ("There is nothing alive here (or so it seems)");
	    break;
	case VILLAGE:
	case CITY:
	    mprint ("You can find no food here; perhaps if you went inside....");
	    break;
	case ROAD:
	    mprint ("You feel it would be a better idea to hunt off the road.");
	    break;
	case CHAOS_SEA:
	    mprint ("Food in the Sea of Chaos? Go on!");
	    break;
	case DESERT:
	    mprint ("You wander off into the trackless desert in search of food...");
	    Time += 100;
	    hourly_check();
	    fertility = 10;
	    break;
	case JUNGLE:
	    mprint ("You search the lush and verdant jungle for game....");
	    Time += 100;
	    hourly_check();
	    fertility = 80;
	    break;
	case PLAINS:
	    mprint ("You set off through the tall grass; the game is afoot.");
	    Time += 100;
	    hourly_check();
	    fertility = 50;
	    break;
	case TUNDRA:
	    mprint ("You blaze a trail through the frozen wasteland....");
	    Time += 100;
	    hourly_check();
	    fertility = 30;
	    break;
	case FOREST:
	    mprint ("You try to follow the many tracks through the forest loam....");
	    Time += 100;
	    hourly_check();
	    fertility = 70;
	    break;
	case MOUNTAINS:
	case PASS:
	    mprint ("You search the cliff walls looking for something to eat....");
	    Time += 100;
	    hourly_check();
	    fertility = 30;
	    break;
	case RIVER:
	    mprint ("The halcyon river is your hopeful food source...");
	    Time += 100;
	    hourly_check();
	    fertility = 80;
	    break;
    }
    if ((Date % 360 < 60 || Date % 360 > 300) && terrain != DESERT && terrain != JUNGLE) {
	mprint ("The cold weather impedes your hunt....");
	fertility = fertility / 2;
    }
    if (fertility > random_range (100)) {
	mprint ("You have an encounter...");
	change_environment (E_TACTICAL_MAP);
    } else
	mprint ("Your hunt is fruitless.");
}

void dismount_steed (void)
{
    if (!gamestatusp (MOUNTED))
	print3 ("You're on foot already!");
    else if (Current_Environment == E_COUNTRYSIDE) {
	mprint ("If you leave your steed here he will wander away!");
	mprint ("Do it anyway? [yn] ");
	if (ynq() == 'y')
	    resetgamestatus (MOUNTED);
    } else {
	resetgamestatus (MOUNTED);;
	monster& m = make_site_monster (Player.x, Player.y, HORSE);
	m.status = MOBILE + SWIMMING;
    }
    calc_melee();
}

static void city_move (void)
{
    int site, x = Player.x, y = Player.y, toggle = false;
    clearmsg();
    if (Current_Environment != E_CITY) {
	print3 ("This command only works in the city!");
	setgamestatus (SKIP_MONSTERS);
    } else if (Player.status[IMMOBILE] > 0)
	print3 ("You can't even move!");
    else if (hostilemonstersnear()) {
	setgamestatus (SKIP_MONSTERS);
	print3 ("You can't move this way with hostile monsters around!");
    } else if (Level->site(Player.x,Player.y).aux == NOCITYMOVE)
	print3 ("You can't use the 'M' command from this location.");
    else {
	print1 ("Move to which establishment [? for help, ESCAPE to quit]");
	site = parsecitysite();
	if (site != ABORT) {
	    mprint ("You're on your way...");
	    morewait();
	    while ((x != CitySiteList[site][1]) || (y != CitySiteList[site][2])) {
		toggle = !toggle;
		if (toggle) {
		    Time++;
		    if (Time % 10 == 0)
			tenminute_check();
		    else
			minute_status_check();
		}
		x += sign (CitySiteList[site][1] - x);
		y += sign (CitySiteList[site][2] - y);
		screencheck (y);
		omshowcursor (x, y);
	    }
	    Player.x = x;
	    Player.y = y;
	    screencheck (Player.y);
	    mprint ("Made it!");
	    drawvision (Player.x, Player.y);
	    morewait();
	    p_movefunction (Level->site(x,y).p_locf);
	}
    }
}

static void frobgamestatus (void)
{
    char response;
    long num;
    mprint ("Set or Reset or Forget it [s,r,ESCAPE]:");
    do
	response = (char) mcigetc();
    while ((response != 'r') && (response != 's') && (response != KEY_ESCAPE));
    if (response != KEY_ESCAPE) {
	mprint ("Enter log2 of flag:");
	num = (int) parsenum();
	if (num > -1) {
	    num = pow2 (num);
	    if (num == CHEATED) {
		mprint ("Can't alter Wizard flag.");
		return;
	    }
	    if (response == 's')
		setgamestatus (num);
	    else
		resetgamestatus (num);
	    mprint ("Done....");
	}
    }
}

// identifies a trap for examine() by its aux value
static const char* trapid (unsigned trapno)
{
    static const char c_TrapNames[] =
	"A dart trap\0"
	"A pit\0"
	"A trap door\0"
	"A snare\0"
	"A blade trap\0"
	"A fire trap\0"
	"A teleport trap\0"
	"A disintegration trap\0"
	"A snare\0"
	"An acid shower trap\0"
	"A manadrain trap\0"
	"A concealed entrance to the abyss\0"
	"A siren trap\0"
	"A completely inoperative trap";
    return (zstrn (c_TrapNames, trapno-L_TRAP_DART, L_TRAP_SIREN-L_TRAP_DART+1));
}

static void enter_site (chtype site)
{
    static const EObjchar c_Sitechar[] =
	{ CITY, VILLAGE, CAVES, CASTLE, VOLCANO, TEMPLE, DRAGONLAIR, STARPEAK, MAGIC_ISLE };
    static const uint8_t c_DestEnv [ArraySize(c_Sitechar)] =
	{ E_CITY, E_VILLAGE, E_CAVES, E_CASTLE, E_VOLCANO, E_TEMPLE, E_DLAIR, E_STARPEAK, E_MAGIC_ISLE };
    for (unsigned i = 0; i < ArraySize(c_Sitechar); ++i)
	if (c_Sitechar[i] == site)
	    return (change_environment ((EEnvironment) c_DestEnv[i]));
    print3 ("There's nothing to enter here!");
}
