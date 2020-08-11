// Omega is free software, distributed under the ISC license

#include "glob.h"

// various traps

// note special function on different levels
void l_trap_siren (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    mprint ("A klaxon goes off!");
    mprint ("'Intruder Alert -- Intruder Alert -- Intruder Alert'");
    mprint ("You have the feeling you have been discovered....");
    morewait();
    clearmsg();
    if (Level->environment == E_HOUSE || Level->environment == E_MANSION) {
	if (gamestatusp (DESTROYED_ORDER))
	    mprint ("Nobody answers the alarm.");
	else {
	    mprint ("The city guard arrives!");
	    mprint ("You are apprehended....");
	    morewait();
	    send_to_jail();
	}
    } else if (Level->environment == E_HOVEL)
	mprint ("Nobody answers the alarm.");
    else {
	if (Level->environment == E_CIRCLE) {
	    summon (-1, DEMON_PRINCE);	// prime circle demon
	    summon (-1, DEMON_PRINCE);
	    summon (-1, DEMON_PRINCE);
	}
	foreach (m, Level->mlist) {
	    m_status_reset (*m, ASLEEP);
	    m->sense *= 2;
	    if (Level->environment == E_CIRCLE || (m->id == GUARD && (Level->environment == E_VILLAGE || Level->environment == E_CITY)))
		m_status_set (*m, HOSTILE);
	}
    }
}

void l_trap_dart (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    if (gamestatusp (MOUNTED)) {
	mprint ("A dart annoys your horse....");
    } else if (random_range (100) < Player.absorption)
	mprint ("A dart plinks off your armor");
    else {
	mprint ("You were hit by a dart!");
	p_damage (difficulty(), NORMAL_DAMAGE, "a dart trap");
	if (random_range (3) == 1) {
	    mprint ("It was poisoned!");
	    morewait();
	    p_poison (difficulty());
	}
    }
}

void l_trap_pit (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    if (gamestatusp (MOUNTED)) {
	mprint ("Your horse stumbles into a pit and breaks a leg!");
	morewait();
	mprint ("You are forced to put it out of its misery.");
	morewait();
	resetgamestatus (MOUNTED);
	showflags();
    } else if (Player.itemweight < ((int) (Player.maxweight / 2)))
	mprint ("You nimbly dodge a pit trap.");
    else {
	mprint ("You fell into a pit!");
	if (random_range (3) == 1) {
	    mprint ("And were impaled by spikes!");
	    morewait();
	    p_damage (difficulty() * 5, NORMAL_DAMAGE, "a spiked pit");
	} else
	    p_damage (difficulty() * 2, NORMAL_DAMAGE, "a pit");
	Player.status[IMMOBILE]++;
    }
}

void l_trap_door (void)
{
    if (!Level->IsDungeon())
	mprint ("You feel oddly lucky.");
    else {
	Level->site(Player.x,Player.y).locchar = TRAP;
	lset (Player.x, Player.y, CHANGED);
	if (gamestatusp (MOUNTED)) {
	    mprint ("You and your horse fall through a trap door!");
	    morewait();
	    mprint ("Your horse breaks its back. Snif.");
	    morewait();
	    if (Level->site(Player.x,Player.y).aux != S_DISINTEGRATE) {
		mprint ("You're hit by a rockslide!");
		morewait();
		p_damage (Level->depth * difficulty(), UNSTOPPABLE, "a rockslide");
	    }
	    change_level (Level->depth, Level->depth + 1, false);
	    resetgamestatus (MOUNTED);
	    showflags();
	    roomcheck();
	} else if (random_range (100) < Player.agi)
	    mprint ("You leap over a trap door.");
	else {
	    mprint ("You fell through a trap door!");
	    morewait();
	    p_damage (difficulty(), NORMAL_DAMAGE, "a trap door");
	    if (Level->site(Player.x,Player.y).aux != S_DISINTEGRATE) {
		mprint ("You're hit by a rockslide!");
		morewait();
		p_damage (Level->depth * difficulty(), UNSTOPPABLE, "a rockslide");
	    }
	    change_level (Level->depth, Level->depth + 1, false);
	    roomcheck();
	}
    }
}

void l_trap_snare (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    if (gamestatusp (MOUNTED))
	mprint ("Your horse steps out of a snare trap.");
    else if (random_range (100) < Player.agi)
	mprint ("You agilely avoid a snare.");
    else {
	mprint ("You were caught in a snare!");
	Player.status[IMMOBILE] += difficulty();
    }
}

void l_trap_blade (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    if (random_range (30) < Player.agi + Player.level)
	mprint ("You duck under a scything blade!");
    else {
	if (gamestatusp (MOUNTED)) {
	    mprint ("Your horse is struck by a scything blade and killed instantly.");
	    morewait();
	    resetgamestatus (MOUNTED);
	    showflags();
	} else {
	    mprint ("A heavy blade scythes across the room and hits you!");
	    morewait();
	    p_damage (random_range (difficulty() * 3) + difficulty() - Player.absorption, NORMAL_DAMAGE, "a blade trap");
	}
    }
}

void l_trap_fire (void)
{
    if (gamestatusp (MOUNTED)) {
	mprint ("Your horse is struck by a blast of fire and is charbroiled!");
	morewait();
	resetgamestatus (MOUNTED);
	showflags();
    } else if (random_range (50) < Player.agi + Player.level)
	mprint ("You dodge a pillar of fire!");
    else {
	mprint ("You were blasted by a fire trap!");
	morewait();
	p_damage (random_range ((difficulty() + 1) * 5), FLAME, "a fire trap");
    }
    Level->site(Player.x,Player.y).locchar = FIRE;
    Level->site(Player.x,Player.y).p_locf = L_FIRE;
    lset (Player.x, Player.y, CHANGED);
}

void l_trap_teleport (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    mprint ("You experience a momentary disorientation....");
    morewait();
    if (random_range (10000) > difficulty() * difficulty())
	p_teleport (0);
    else
	p_teleport (-1);
}

void l_trap_disintegrate (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    mprint ("Oh, no! A disintegration trap!");
    morewait();
    if (gamestatusp (MOUNTED)) {
	mprint ("Your horse falls apart into its component atoms...");
	morewait();
	resetgamestatus (MOUNTED);
	showflags();
    }
    disintegrate (Player.x, Player.y);
}

void l_trap_sleepgas (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    mprint ("Hsssssssss....");
    morewait();
    mprint ("You detect a strange odor....");
    morewait();
    sleep_player ((difficulty() / 5) + 1);
}

void l_trap_acid (void)
{
    int i, k, itemdamage;
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    if (Player.agi + Player.level < random_range (100)) {
	mprint ("You are drenched by a spray of acid!");
	morewait();
	if (gamestatusp (MOUNTED)) {
	    mprint ("Your horse dies unpleasantly.");
	    morewait();
	    resetgamestatus (MOUNTED);
	    showflags();
	}
	p_damage (random_range (difficulty() * 5), ACID, "an acid trap");
	if (!Player.immune_to (ACID)) {
	    mprint ("The acid seeps over your possessions...");
	    morewait();
	    itemdamage = random_range (5);
	    for (i = k = 0; ((i < MAXITEMS) && (k < itemdamage)); i++) {
		if (Player.has_possession(i)) {
		    k++;
		    damage_item (&Player.possessions[i]);
		}
	    }
	}
    } else
	mprint ("You somehow dodge a shower of hydroflouric acid!");
}

void l_trap_abyss (void)
{
    Level->site(Player.x,Player.y).locchar = ABYSS;
    lset (Player.x, Player.y, CHANGED);
    if (gamestatusp (MOUNTED)) {
	mprint ("You and your horse fall into the infinite abyss!");
	morewait();
	l_abyss();
    }
    if (Player.dex + Player.level < random_range (100)) {
	mprint ("You stumble over a concealed entrance to the abyss!");
	morewait();
	l_abyss();
    } else
	mprint ("You gingerly step around a concealed entrance to the abyss!");
}

void l_trap_manadrain (void)
{
    Level->site(Player.x,Player.y).locchar = TRAP;
    lset (Player.x, Player.y, CHANGED);
    if (Player.mana == 0) {
	if (random_range (10) == 3) {
	    mprint ("The manadrain trap overloads -- positive feedback....");
	    mprint ("That's strange.... You feel repowered!");
	    morewait();
	    Level->site(Player.x,Player.y).locchar = FLOOR;
	    Level->site(Player.x,Player.y).p_locf = L_NO_OP;
	    lset (Player.x, Player.y, CHANGED);
	    Player.mana = Player.calcmana();
	} else
	    mprint ("You feel strangely unaffected by the manadrain trap.");
    } else {
	mprint ("A weird rainbow light seems to play over you....");
	mprint ("You feel drained.");
	morewait();
	while (Player.mana > 1) {
	    Player.mana /= 2;
	    dataprint();
	}
	Player.mana = 0;
	dispel (-1);
    }
}
