/* File: xtra1.c */

/* Purpose: misc code */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "script.h"
#include "button.h"


/*
 * Modify a stat value by a "modifier", return new value
 *
 * Stats go up: 3,4,...,17,18,18/10,18/20,...,18/220
 * Or even: 18/13, 18/23, 18/33, ..., 18/220
 *
 * Stats go down: 18/220, 18/210,..., 18/10, 18, 17, ..., 3
 * Or even: 18/13, 18/03, 18, 17, ..., 3
 */
s16b modify_stat_value(int value, int amount)
{
    value += amount * 10;

    if (value < 30) value = 30;

	/* Return new value */
	return (value);
}


/*
 * Print character info at given row, column in a 13 char field
 */
static void prt_field(cptr info, int col, int row)
{
	/* Dump 13 spaces to clear */
	put_fstr(col, row, "             ");

	/* Dump the info itself */
	put_fstr(col, row, CLR_L_BLUE "%s", info);
}

/*
 * Returns a formatted string in the buffer of
 * the stat value which is the first parameter
 * in the va_list.
 */
void stat_format(char *buf, uint max, cptr fmt, va_list *vp)
{
    int arg;

	/* Unused parameter */
	(void)fmt;

	/* Get the argument */
	arg = va_arg(*vp, int);

	/* Format the number for the stat */
	if (arg >= 400)
        strnfmt(buf, max, "  40+ ");
    else
        strnfmt(buf, max, "  %2d.%d", arg / 10, arg % 10);
}


/*
 * Print character stat in given row, column
 */
static void prt_stat(int stat)
{
	/* Display "injured" stat */
	if (p_ptr->stat[stat].cur < p_ptr->stat[stat].max)
	{
		put_fstr(COL_STAT, ROW_STAT + stat, "%5s" CLR_YELLOW " %v",
				 stat_names_reduced[stat],
				 stat_format, p_ptr->stat[stat].use);
	}

	/* Display "healthy" stat */
	else
	{
		put_fstr(COL_STAT, ROW_STAT + stat, "%5s" CLR_L_GREEN " %v",
				 stat_names[stat],
				 stat_format, p_ptr->stat[stat].use);
	}

	/* Indicate natural maximum */
	if (p_ptr->stat[stat].max == stat_cap(stat))
	{
		put_fstr(COL_STAT + 3, ROW_STAT + stat, "!");
	}
}

static int bar_count = 0;

static void clear_status_bar(void)
{
	put_fstr(COL_STATBAR, ROW_STATBAR, "            ");
}


static void show_status_bar(cptr *letter, int num)
{
	int i;

	if (num <= 12)
	{
		/* Reset everything */
		bar_count = 0;
		clear_status_bar();

		/* Display the flags */
		for (i = 0; i < num; i++)
		{
			put_fstr(COL_STATBAR + i, ROW_STATBAR, letter[i]);
		}
	}
	else
	{
		/* increment the count (scroll the flags) */
		bar_count++;

		if (bar_count >= num) bar_count = 0;

		if (bar_count + 12 < num)
		{
			/* Simple case - all in a row */
			for (i = 0; i < 12; i++)
			{
				put_fstr(COL_STATBAR + i, ROW_STATBAR, letter[i + bar_count]);
			}
		}
		else
		{
			/* Split over boundary */
			for (i = 0; i < num - bar_count; i++)
			{
				put_fstr(COL_STATBAR + i, ROW_STATBAR, letter[i + bar_count]);
			}
			for (i = 0; i < 12 + bar_count - num; i++)
			{
				put_fstr(COL_STATBAR + i + num - bar_count, ROW_STATBAR,
						   letter[i]);
			}
		}
	}
}


/*
 * Show status bar
 */
static void prt_status(void)
{
	int num = 0;
	cptr letter[30];

	/* Collate active flags */

	/* Hack -- Hallucinating */
	if (query_timed(TIMED_IMAGE))
	{
		letter[num] = CLR_VIOLET "H";
		num++;
	}

	/* Blindness */
	if (query_timed(TIMED_BLIND))
	{
		letter[num] = CLR_L_DARK "B";
		num++;
	}

	/* Times see-invisible */
	if (query_timed(TIMED_SEE_INVIS))
	{
		letter[num] = CLR_L_BLUE "I";
		num++;
	}

	/* Timed esp */
	if (query_timed(TIMED_ESP))
	{
		letter[num] = CLR_ORANGE "E";
		num++;
	}

	/* Timed infra-vision */
	if (query_timed(TIMED_INFRA))
	{
		letter[num] = CLR_L_RED "I";
		num++;
	}

	/* Paralysis */
	if (query_timed(TIMED_PARALYZED))
	{
		letter[num] = CLR_RED "P";
		num++;
	}

	/* Confusion */
	if (query_timed(TIMED_CONFUSED))
	{
		letter[num] = CLR_VIOLET "C";
		num++;
	}

	/* Fast */
	if (query_timed(TIMED_FAST) || query_timed(TIMED_XTRA_FAST))
	{
		letter[num] = CLR_GREEN "S";
		num++;
	}

	/* Slow */
	if (query_timed(TIMED_SLOW))
	{
		letter[num] = CLR_RED "S";
		num++;
	}

	/* Protection from evil */
	if (query_timed(TIMED_PROTEVIL))
	{
		letter[num] = CLR_L_DARK "E";
		num++;
	}

	/* Invulnerability */
	if (query_timed(TIMED_INVULN))
	{
		letter[num] = CLR_YELLOW "I";
		num++;
	}

	/* Wraith form */
	if (query_timed(TIMED_WRAITH_FORM))
	{
		letter[num] = CLR_L_DARK "W";
		num++;
	}

	if (query_timed(TIMED_ETHEREALNESS))
	{
		letter[num] = CLR_L_DARK "E";
		num++;
	}

	if (query_timed(TIMED_LUMINOSITY))
	{
		letter[num] = CLR_YELLOW "*";
		num++;
	}

	/* Heroism */
	if (query_timed(TIMED_HERO))
	{
		letter[num] = CLR_WHITE "H";
		num++;
	}

	/* Super Heroism / berserk */
	if (query_timed(TIMED_SHERO))
	{
		letter[num] = CLR_RED "B";
		num++;
	}

	if (query_timed(TIMED_STR))
	{
		letter[num] = CLR_YELLOW "S";
		num++;
	}

	if (query_timed(TIMED_CHR))
	{
		letter[num] = CLR_YELLOW "C";
		num++;
	}

	if (query_timed(TIMED_SUST_ALL))
	{
		letter[num] = CLR_BLUE "A";
		num++;
	}

	/* Blessed */
	if (query_timed(TIMED_BLESSED))
	{
		letter[num] = CLR_WHITE "B";
		num++;
	}

	/* Shield */
	if (query_timed(TIMED_SHIELD))
	{
		letter[num] = CLR_WHITE "S";
		num++;
	}

	/* Oppose Confusion */
	if (query_timed(TIMED_OPPOSE_CONF))
	{
		letter[num] = CLR_BLUE "C";
		num++;
	}

	/* Oppose Blindness */
	if (query_timed(TIMED_OPPOSE_BLIND))
	{
		letter[num] = CLR_BLUE "B";
		num++;
	}

	/* Oppose Acid */
	if (query_timed(TIMED_IMMUNE_ACID))
	{
		letter[num] = CLR_WHITE "A";
		num++;
	}
	else if (query_timed(TIMED_OPPOSE_ACID))
	{
		letter[num] = CLR_GREEN "A";
		num++;
	}

	/* Oppose Lightning */
	if (query_timed(TIMED_IMMUNE_ELEC))
	{
		letter[num] = CLR_WHITE "E";
		num++;
	}
	else if (query_timed(TIMED_OPPOSE_ELEC))
	{
		letter[num] = CLR_BLUE "E";
		num++;
	}

	/* Oppose Fire */
	if (query_timed(TIMED_IMMUNE_FIRE))
	{
		letter[num] = CLR_WHITE "F";
		num++;
	}
	else if (query_timed(TIMED_OPPOSE_FIRE))
	{
		letter[num] = CLR_RED "F";
		num++;
	}

	/* Oppose Cold */
	if (query_timed(TIMED_IMMUNE_COLD))
	{
		letter[num] = CLR_WHITE "C";
		num++;
	}
	else if (query_timed(TIMED_OPPOSE_COLD))
	{
		letter[num] = CLR_WHITE "C";
		num++;
	}

	/* Oppose Poison */
	if (query_timed(TIMED_OPPOSE_POIS))
	{
		letter[num] = CLR_GREEN "P";
		num++;
	}

	/* Word of Recall */
	if (query_timed(TIMED_WORD_RECALL))
	{
		letter[num] = CLR_WHITE "W";
		num++;
	}

	/* Aura of Fire */
	if (query_timed(TIMED_SH_FIRE))
	{
		letter[num] = CLR_RED "*";
		num++;
	}

	/* Aura of Acid */
	if (query_timed(TIMED_SH_ACID))
	{
		letter[num] = CLR_GREEN "*";
		num++;
	}

	/* Aura of Electricity */
	if (query_timed(TIMED_SH_ELEC))
	{
		letter[num] = CLR_BLUE "*";
		num++;
	}

	/* Aura of Cold */
	if (query_timed(TIMED_SH_COLD))
	{
		letter[num] = CLR_WHITE "*";
		num++;
	}

	/* Aura of Cold */
	if (query_timed(TIMED_SH_FEAR))
	{
		letter[num] = CLR_L_DARK "*";
		num++;
	}

	/* Confusing Hands */
	if (p_ptr->state.confusing)
	{
		letter[num] = CLR_RED "C";
		num++;
	}

	/* Invisibility */
	if (query_timed(TIMED_INVIS) || query_timed(TIMED_XTRA_INVIS))
	{
		letter[num] = CLR_VIOLET "I";
		num++;
	}

	if (num)
	{
		/* Display the status bar if there are flags set */
		show_status_bar(letter, num);
	}
	else
		clear_status_bar();
}


/*
 * Prints "title", including "wizard" or "winner" as needed.
 */
static void prt_title(void)
{
	cptr p;

	/* Wizard */
	if (p_ptr->state.wizard)
	{
		p = "[=-WIZARD-=]";
	}

	/* Winner */
	else if (p_ptr->state.total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		p = "***WINNER***";
	}

	/* Normal */
	else
	{
		p = player_title[p_ptr->rp.pclass][(p_ptr->lev - 1) / 5];

	}

	prt_field(p, COL_TITLE, ROW_TITLE);
}


/*
 * Prints level
 */
static void prt_level(void)
{
	if (p_ptr->lev >= p_ptr->max_lev)
	{
		put_fstr(COL_LEVEL, ROW_LEVEL, "LEVEL " CLR_L_GREEN "%6d", p_ptr->lev);
	}
	else
	{
		put_fstr(COL_LEVEL, ROW_LEVEL, "Level " CLR_YELLOW "%6d", p_ptr->lev);
	}
}


/*
 * Display the experience
 */
static void prt_exp(void)
{
	cptr attr;

	if (p_ptr->exp >= p_ptr->max_exp)
	{
		attr = CLR_L_GREEN;
	}
	else
	{
		attr = CLR_YELLOW;
	}

	if (toggle_xp)
	{
		if (p_ptr->lev >= PY_MAX_LEVEL)
		{
			put_fstr(COL_EXP, ROW_EXP, "NEED %s*******", attr);
		}
		else
		{
			/* Print the amount of experience to go until the next level */
			put_fstr(COL_EXP, ROW_EXP, "NEED%s%8ld", attr,
						  (long)(player_exp[p_ptr->lev - 1] * p_ptr->expfact /
								 100L) - (long)p_ptr->exp);
		}
	}
	else
	{
		/* Use the 'old' experience display */
		put_fstr(COL_EXP, ROW_EXP, "EXP %s%8ld", attr, (long)p_ptr->exp);
	}
}


/*
 * Prints current gold
 */
static void prt_gold(void)
{
	put_fstr(COL_GOLD, ROW_GOLD, "AU " CLR_L_GREEN "%9ld", (long)p_ptr->au);
}



/*
 * Prints current AC
 */
static void prt_ac(void)
{
	put_fstr(COL_AC, ROW_AC, "Cur AC " CLR_L_GREEN "%5d",
				p_ptr->dis_ac + p_ptr->dis_to_a);
}


/*
 * Prints Cur/Max hit points
 */
static void prt_hp(void)
{
	cptr color;
	byte color_player;

#ifndef VARIABLE_PLAYER_GRAPH

	monster_race *r_ptr = &r_info[0];
	byte old_attr = r_ptr->x_attr;

#endif /* !VARIABLE_PLAYER_GRAPH */

	color = CLR_L_GREEN;
	color_player = TERM_WHITE;

	if ((p_ptr->mhp > 0) && (p_ptr->chp < p_ptr->mhp)) {
		switch ((p_ptr->chp * 10) / p_ptr->mhp) {
		case 9:
		{
			color = CLR_L_WHITE;
			color_player = TERM_L_WHITE;
			break;
		}
		case 8:
		case 7:
		{
			color = CLR_YELLOW;
			color_player = TERM_YELLOW;
			break;
		}
		case 6:
		case 5:
		{
			color = CLR_ORANGE;
			color_player = TERM_ORANGE;
			break;
		}
		case 4:
		case 3:
		{
			color = CLR_L_RED;
			color_player = TERM_L_RED;
			break;
		}
		default:
		{
			color = CLR_RED;
			color_player = TERM_RED;
			break;
		}
		}
	}

	put_fstr(COL_HP, ROW_HP, "HP %s%4d" CLR_WHITE "/" CLR_L_GREEN "%4d", color, p_ptr->chp, p_ptr->mhp);
	//put_fstr(COL_MAXHP, ROW_MAXHP, "Max HP " CLR_L_GREEN "%5d", p_ptr->mhp);
	//put_fstr(COL_CURHP, ROW_CURHP, "Cur HP %s%5d", color, p_ptr->chp);


#ifndef VARIABLE_PLAYER_GRAPH


	/* Hack - only change the colour if in character mode */
	if (r_ptr->x_char != '@') return;

	/* Only change colour if asked */
	if (!view_player_colour)
	{
		/* Normal colour is white */
		color_player = TERM_WHITE;
	}

	/* Redraw the player ? */
	if (old_attr != color_player)
	{
		/* Change the player colour */
		r_ptr->x_attr = color_player;

		/* Show the change */
		if (character_dungeon) lite_spot(p_ptr->px, p_ptr->py);
	}
#endif /* !VARIABLE_PLAYER_GRAPH */
}


/*
 * Prints players max/cur spell points
 */
static void prt_sp(void)
{
	cptr color;


	/* Do not show mana unless it matters */
	if (!mp_ptr->spell_book) return;

	color = CLR_L_GREEN;

	if ((p_ptr->msp > 0) && (p_ptr->csp < p_ptr->msp)) {
		switch ((p_ptr->csp * 10) / p_ptr->msp) {
		case 9:
		{
			color = CLR_L_WHITE;
			break;
		}
		case 8:
		case 7:
		{
			color = CLR_YELLOW;
			break;
		}
		case 6:
		case 5:
		{
			color = CLR_ORANGE;
			break;
		}
		case 4:
		case 3:
		{
			color = CLR_L_RED;
			break;
		}
		default:
		{
			color = CLR_RED;
			break;
		}
		}
	}

	/* Show mana */
	put_fstr(COL_SP, ROW_SP, "SP %s%4d" CLR_WHITE "/" CLR_L_GREEN "%4d", color, p_ptr->csp, p_ptr->msp);
	//put_fstr(COL_CURSP, ROW_CURSP, "Cur SP %s%5d", color, p_ptr->csp);
	//put_fstr(COL_MAXSP, ROW_MAXSP, "Max SP " CLR_L_GREEN "%5d", p_ptr->msp);
}


/*
 * Prints depth in stat area
 */
static void prt_depth(void)
{
	if (!p_ptr->depth)
	{
		if (p_ptr->place_num)
		{
			int q_num = place[p_ptr->place_num].quest_num;

			/* Is there a quest here? */
			if (q_num && q_num < z_info->q_max)
			{
				/* Is this a quest to find a ruin? */
				if (quest[q_num].type == QUEST_TYPE_FIND_PLACE)
				{
					prtf(COL_DEPTH, Term->hgt - 1, "     Ruin        ");
				}
				/* then it is this a wilderness quest */
				else if (quest[q_num].type == QUEST_TYPE_WILD)
				{
					prtf(COL_DEPTH, Term->hgt - 1, "      Quest      ");
				}
				/* fixed quest.  Only alert if there are stairs there. */
				else
				{
					if (quest_stairs_active(p_ptr->place_num))
						prtf(COL_DEPTH, Term->hgt -1, "      Quest      ");
					else
						prtf(COL_DEPTH, Term->hgt -1, "    Wilderness   ");
				}

			}
			else
			{
				prtf(COL_DEPTH, Term->hgt - 1, "%17s", place[p_ptr->place_num].name);
			}
		}
		else
		{
			prtf(COL_DEPTH, Term->hgt - 1, "    Wilderness   ");
		}
	}
	else if (depth_in_feet)
	{
		prtf(COL_DEPTH, Term->hgt - 1, "     %4d ft     ", p_ptr->depth * 50);
	}
	else
	{
		prtf(COL_DEPTH, Term->hgt - 1, "     Lev %d     ", p_ptr->depth);
	}
}

/*
 * Prints time of day after the depth
 */
static void prt_time(void)
{
  if (Term->wid > COL_DEPTH + 30) {
    int hour = (turn % TOWN_DAY) / TOWN_HOUR;
	  if (p_ptr->depth) {
      if (hour >= 22) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "   Morning");
      } else
      if (hour >= 16) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "     Night");
      } else
      if (hour >= 10) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "   Evening");
      } else
      if (hour >= 4) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "    Midday");
      } else
      {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "   Morning");
      }
    } else {
      if (hour >= 23) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "   Sunrise");
      } else
      if (hour >= 21) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "   Predawn");
      } else
      if (hour >= 19) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "Late Night");
      } else
      if (hour >= 17) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "  Midnight");
      } else
      if (hour >= 15) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "Late Evening");
      } else
      if (hour >= 13) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "   Evening");
      } else
      if (hour >= 11) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "    Sunset");
      } else
      if (hour >= 9) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "Late Afternoon");
      } else
      if (hour >= 7) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, " Afternoon");
      } else
      if (hour >= 5) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "      Noon");
      } else
      if (hour >= 3) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "Late Morning");
      } else
      if (hour >= 1) {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "   Morning");
      } else
      {
			  prtf(COL_DEPTH + 20, Term->hgt - 1, "   Sunrise");
      }
    }
  }
}


/*
 * Prints status of hunger
 */
static void prt_hunger(void)
{
	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		put_fstr(COL_HUNGRY, Term->hgt - 1, CLR_RED "Weak  ");
	}

	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		put_fstr(COL_HUNGRY, Term->hgt - 1, CLR_ORANGE "Weak  ");
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		put_fstr(COL_HUNGRY, Term->hgt - 1, CLR_YELLOW "Hungry");
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		put_fstr(COL_HUNGRY, Term->hgt - 1, CLR_WHITE "Satiated");
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		put_fstr(COL_HUNGRY, Term->hgt - 1, CLR_L_GREEN "Full  ");
	}

	/* Gorged */
	else
	{
		put_fstr(COL_HUNGRY, Term->hgt - 1, CLR_GREEN "Gorged");
	}
}


/*
 * Prints Blind status
 */
static void prt_blind(void)
{
	if (query_timed(TIMED_BLIND))
	{
		put_fstr(COL_BLIND, Term->hgt - 1, CLR_ORANGE "Blind");
	}
	else
	{
		pcave_type *pc_ptr = parea(p_ptr->px, p_ptr->py);
		if (pc_ptr->player & GRID_DTCT) {
			put_fstr(COL_BLIND, Term->hgt - 1, CLR_L_GREEN "DTrap");
		} else {
			put_fstr(COL_BLIND, Term->hgt - 1, "     ");
		}
	}
}


/*
 * Prints Confusion status
 */
static void prt_confused(void)
{
	if (query_timed(TIMED_CONFUSED))
	{
		put_fstr(COL_CONFUSED, Term->hgt - 1, CLR_ORANGE "Confused");
	}
	else
	{
		put_fstr(COL_CONFUSED, Term->hgt - 1, "        ");
	}
}


/*
 * Prints Fear status
 */
static void prt_afraid(void)
{
	if (query_timed(TIMED_AFRAID))
	{
		put_fstr(COL_AFRAID, Term->hgt - 1, CLR_ORANGE "Afraid");
	}
	else
	{
		put_fstr(COL_AFRAID, Term->hgt - 1, "      ");
	}
}


/*
 * Prints Poisoned status
 */
static void prt_poisoned(void)
{
	if (query_timed(TIMED_POISONED))
	{
		put_fstr(COL_POISONED, Term->hgt - 1, CLR_ORANGE "Poisoned");
	}
	else
	{
		put_fstr(COL_POISONED, Term->hgt - 1, "        ");
	}
}


/*
 * Prints Searching, Resting, or 'count' status
 *
 * This function was a major bottleneck when resting, so a lot of
 * the text formatting code was optimized in place below.
 */
static void prt_state(void)
{
	char text[16];

	/* Resting */
	if (p_ptr->state.resting)
	{
		int i;

		/* Start with "Rest" */
		strcpy(text, "R     ");

		/* Extensive (timed) rest */
		if (p_ptr->state.resting >= 1000)
		{
			i = p_ptr->state.resting / 100;
			text[5] = '0';
			text[4] = '0';
			text[3] = '0' + (i % 10);
			if (i >= 10)
			{
				i = i / 10;
				text[2] = '0' + (i % 10);
				if (i >= 10)
				{
					text[1] = '0' + (i / 10);
				}
			}
		}

		/* Long (timed) rest */
		else if (p_ptr->state.resting >= 100)
		{
			i = p_ptr->state.resting;
			text[5] = '0' + (i % 10);
			i = i / 10;
			text[4] = '0' + (i % 10);
			text[3] = '0' + (i / 10);
		}

		/* Medium (timed) rest */
		else if (p_ptr->state.resting >= 10)
		{
			i = p_ptr->state.resting;
			text[5] = '0' + (i % 10);
			text[4] = '0' + (i / 10);
		}

		/* Short (timed) rest */
		else if (p_ptr->state.resting > 0)
		{
			i = p_ptr->state.resting;
			text[5] = '0' + (i);
		}

		/* Rest until healed */
		else if (p_ptr->state.resting == -1)
		{
			text[1] = text[2] = text[3] = text[4] = text[5] = '*';
		}

		/* Rest until done */
		else if (p_ptr->state.resting == -2)
		{
			text[1] = text[2] = text[3] = text[4] = text[5] = '&';
		}

		/* Display the info (or blanks) */
		put_fstr(COL_STATE, Term->hgt - 1, text);
	}

	/* Repeating */
	else if (p_ptr->cmd.rep)
	{
		if (p_ptr->cmd.rep > 999)
		{
			put_fstr(COL_STATE, Term->hgt - 1, "C%3d00", p_ptr->cmd.rep / 100);
		}
		else
		{
			put_fstr(COL_STATE, Term->hgt - 1, "C  %3d", p_ptr->cmd.rep);
		}
	}

	/* Searching */
	else if (p_ptr->state.searching == SEARCH_MODE_SEARCH)
	{
		put_fstr(COL_STATE, Term->hgt - 1, "Search ");
	}
	else if (p_ptr->state.searching == SEARCH_MODE_SWING)
	{
		put_fstr(COL_STATE, Term->hgt - 1, "Swing  ");
	}
	else if (p_ptr->state.searching == SEARCH_MODE_STEALTH)
	{
		put_fstr(COL_STATE, Term->hgt - 1, "Stealth");
	}

	/* Nothing interesting */
	else
	{
		put_fstr(COL_STATE, Term->hgt - 1, CLR_L_DARK "Search ");
		//put_fstr(COL_STATE, Term->hgt - 1, "       ");
	}
}


/*
 * Prints the speed or paralysis of a character.
 *
 * Note that the strings must be exactly 10 chars long.
 */
static void prt_speed(void)
{
	int i = p_ptr->pspeed;

	/* Hack -- Visually "undo" the Search Mode Slowdown */
	if (p_ptr->state.searching) i += 10;

	/* Paralysis */
	if (query_timed(TIMED_PARALYZED))
	{
		put_fstr(COL_SPEED, Term->hgt - 1, CLR_RED "Paralyzed!");
	}

	/* Fast */
	else if (i > 110)
	{
		if (i <= 110 + 9)
		{
			/* One digit */
			put_fstr(COL_SPEED, Term->hgt - 1, CLR_L_GREEN "Fast (+%d) ", (i - 110));
		}
		else if (i <= 110 + 99)
		{
			/* Two digits */
			put_fstr(COL_SPEED, Term->hgt - 1, CLR_L_GREEN "Fast (+%d)", (i - 110));
		}
		else
		{
			/* Hack - save space */
			put_fstr(COL_SPEED, Term->hgt - 1, CLR_L_GREEN "Fast (***)");
		}
	}

	/* Slow */
	else if (i < 110)
	{
		if (i >= 110 - 9)
		{
			/* One digit */
			put_fstr(COL_SPEED, Term->hgt - 1, CLR_L_UMBER "Slow (-%d) ", (110 - i));
		}
		else if (i >= 110 - 99)
		{
			/* Two digits */
			put_fstr(COL_SPEED, Term->hgt - 1, CLR_L_UMBER "Slow (-%d)", (110 - i));
		}
		else
		{
			/* Hack - save space */
			put_fstr(COL_SPEED, Term->hgt - 1, CLR_L_UMBER "Slow (***)");
		}
	}
	else
	{
		/* Nothing to print */
		put_fstr(COL_SPEED, Term->hgt - 1, CLR_L_UMBER "          ");
	}
}


static void prt_study(void)
{
	if (p_ptr->new_spells) {
		put_fstr(COL_STUDY, Term->hgt - 1, "Study");
	} else
	if (p_ptr->spell.realm[0]) {
		put_fstr(COL_STUDY, Term->hgt - 1, CLR_L_DARK "Study");
	} else
	{
		put_fstr(COL_STUDY, Term->hgt - 1, "     ");
	}
}


static void prt_cut(void)
{
	int c = query_timed(TIMED_CUT);

	if (c > 1000)
	{
		put_fstr(COL_CUT, ROW_CUT, CLR_L_RED "Mortal wound");
	}
	else if (c > 200)
	{
		put_fstr(COL_CUT, ROW_CUT, CLR_RED "Deep gash   ");
	}
	else if (c > 100)
	{
		put_fstr(COL_CUT, ROW_CUT, CLR_RED "Severe cut  ");
	}
	else if (c > 50)
	{
		put_fstr(COL_CUT, ROW_CUT, CLR_ORANGE "Nasty cut   ");
	}
	else if (c > 25)
	{
		put_fstr(COL_CUT, ROW_CUT, CLR_ORANGE "Bad cut     ");
	}
	else if (c > 10)
	{
		put_fstr(COL_CUT, ROW_CUT, CLR_YELLOW "Light cut   ");
	}
	else if (c)
	{
		put_fstr(COL_CUT, ROW_CUT, CLR_YELLOW "Graze       ");
	}
	else
	{
		put_fstr(COL_CUT, ROW_CUT, "            ");
	}
}


static void prt_stun(void)
{
	int s = query_timed(TIMED_STUN);

	if (s > 100)
	{
		put_fstr(COL_STUN, ROW_STUN, CLR_RED "Knocked out ");
	}
	else if (s > 50)
	{
		put_fstr(COL_STUN, ROW_STUN, CLR_ORANGE "Heavy stun  ");
	}
	else if (s)
	{
		put_fstr(COL_STUN, ROW_STUN, CLR_ORANGE "Stun        ");
	}
	else
	{
		put_fstr(COL_STUN, ROW_STUN, "            ");
	}
}



/*
 * Redraw the "monster health bar"	-DRS-
 * Rather extensive modifications by	-BEN-
 *
 * The "monster health bar" provides visual feedback on the "health"
 * of the monster currently being "tracked".  There are several ways
 * to "track" a monster, including targeting it, attacking it, and
 * affecting it (and nobody else) with a ranged attack.
 *
 * Display the monster health bar (affectionately known as the
 * "health-o-meter").  Clear health bar if nothing is being tracked.
 * Auto-track current target monster when bored.  Note that the
 * health-bar stops tracking any monster that "disappears".
 */
/*static void health_redraw(void) - not static because need this in a context menu*/
void health_redraw(void)
{
	Term_erase(COL_TARGET_NAME, ROW_TARGET_NAME, 12);

	/* Not tracking */
	if (!p_ptr->health_who)
	{
		/* Erase the health bar */
		Term_erase(COL_INFO, ROW_INFO, 12);

		/* if we are targeting a position, display to position offset */
		if (p_ptr->target_who < 0) {
			int x, y;
			char cx,cy;

			x = p_ptr->target_col - p_ptr->px;
			y = p_ptr->target_row - p_ptr->py;

			if (x < 0) {
				x = -x;
				cx = 'W';
			} else {
				cx = 'E';
			}
			if (y < 0) {
				y = -y;
				cy = 'N';
			} else {
				cy = 'S';
			}

			/* Dump the offset of the target spot */
			put_fstr(COL_TARGET_NAME, ROW_TARGET_NAME, "( %2d%c %2d%c )", x,cx,y,cy);
		}
	}

	/* Tracking an unseen monster */
	else if (!m_list[p_ptr->health_who].ml)
	{
		/* Indicate that the monster health is "unknown" */
		put_fstr(COL_INFO, ROW_INFO, "[----------]");
		put_fstr(COL_TARGET_NAME, ROW_TARGET_NAME, "Unknown");
	}

	/* Tracking a hallucinatory monster */
	else if (query_timed(TIMED_IMAGE))
	{
		/* Indicate that the monster health is "unknown" */
		put_fstr(COL_INFO, ROW_INFO, "[----------]");
		put_fstr(COL_TARGET_NAME, ROW_TARGET_NAME, "Unknown");
	}

	/* Tracking a dead monster ??? */
	else if (!m_list[p_ptr->health_who].hp < 0)
	{
		/* Indicate that the monster health is "unknown" */
		put_fstr(COL_INFO, ROW_INFO, "[----------]");
		//put_fstr(COL_TARGET_NAME, ROW_TARGET_NAME, r_info[m_list[p_ptr->health_who].r_idx].name);
	}

	/* Tracking a visible monster */
	else
	{
		int pct, len;

		monster_type *m_ptr = &m_list[p_ptr->health_who];

		/* Default to almost dead */
		cptr attr = CLR_RED;

		/* Extract the "percent" of health */
		pct = 100L * m_ptr->hp / m_ptr->maxhp;

		/* Badly wounded */
		if (pct >= 10) attr = CLR_L_RED;

		/* Wounded */
		if (pct >= 25) attr = CLR_ORANGE;

		/* Somewhat Wounded */
		if (pct >= 60) attr = CLR_YELLOW;

		/* Healthy */
		if (pct >= 100) attr = CLR_L_GREEN;

		/* Afraid */
		if (m_ptr->monfear) attr = CLR_VIOLET;

		/* Asleep */
		if (m_ptr->csleep) attr = CLR_BLUE;

		/* Invulnerable */
		if (m_ptr->invulner) attr = CLR_WHITE;

		/* Imprisoned */
		if (m_ptr->imprisoned) attr = CLR_L_DARK;

		/* Convert percent into "health" */
		len = (pct < 10) ? 1 : (pct < 90) ? (pct / 10 + 1) : 10;

		/* Default to "unknown" */
		put_fstr(COL_INFO, ROW_INFO, "[----------]");

		/* Dump the current "health" (use '*' symbols) */
		put_fstr(COL_INFO + 1, ROW_INFO, "%s%.*s", attr, len, "**********");

		/* Dump the name of the monster */
		put_fstr(COL_TARGET_NAME, ROW_TARGET_NAME, "%.*s", 12, mon_race_name(monst_race(m_list[p_ptr->health_who].r_idx)));
	}
}



/*
 * Display basic info (mostly left of map)
 */
static void prt_frame_basic(void)
{
	int i;

	/* Race and Class */
	prt_field(format("%s%s", p_ptr->state.lich ? "Lich " : "", rp_ptr->title), COL_RACE, ROW_RACE);
	prt_field(cp_ptr->title, COL_CLASS, ROW_CLASS);

	/* Title */
	prt_title();

	/* Level/Experience */
	prt_level();
	prt_exp();

	/* All Stats */
	for (i = 0; i < A_MAX; i++) prt_stat(i);

	/* Status Bar */
	prt_status();

	/* Armor */
	prt_ac();

	/* Hitpoints */
	prt_hp();

	/* Spellpoints */
	prt_sp();

	/* Gold */
	prt_gold();

	/* Current depth */
	prt_depth();

	/* Special */
	health_redraw();
}


/*
 * Display extra info (mostly below map)
 */
static void prt_frame_extra(void)
{
	/* Cut/Stun */
	prt_cut();
	prt_stun();

	/* Food */
	prt_hunger();

	/* Various */
	prt_blind();
	prt_confused();
	prt_afraid();
	prt_poisoned();

	/* State */
	prt_state();

	/* Speed */
	prt_speed();

	/* Study spells */
	prt_study();
}


/*
 * Hack -- display inventory in sub-windows
 */
static void fix_inven(void)
{
	int j;

	/* Update inventory information */
	Term_write_list(p_ptr->inventory, LIST_INVEN);

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_INVEN))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display inventory */
		display_inven();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}



/*
 * Hack -- display equipment in sub-windows
 */
static void fix_equip(void)
{
	int j;

	/* Update equipment information */
	Term_write_equipment();

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_EQUIP))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display equipment */
		display_equip();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display equipment in sub-windows
 */
static void fix_spell(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_SPELL))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display spell list */
		display_spell_list();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display character in sub-windows
 */
static void fix_player(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_PLAYER))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display player */
		display_player(DISPLAY_PLAYER_STANDARD);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}



/*
 * Hack -- display recent messages in sub-windows
 *
 * XXX XXX XXX Adjust for width and split messages
 */
static void fix_message(void)
{
	int j, i;
	int w, h;
	int x, y;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_MESSAGE))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Get size */
		Term_get_size(&w, &h);

		/* Dump messages */
		for (i = 0; i < h; i++)
		{
			cptr attr = color_seq[message_color((s16b)i)];

			/* Dump the message on the appropriate line */
			put_fstr(0, (h - 1) - i, "%s%s", attr, message_str((s16b)i));

			/* Cursor */
			(void)Term_locate(&x, &y);

			/* Clear to end of line */
			Term_erase(x, y, 255);
		}

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display overhead view in sub-windows
 *
 * Note that the "player" symbol does NOT appear on the map.
 */
static void fix_overhead(void)
{
	int j;

	int cy, cx;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_OVERHEAD))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* No offset from player */
		cx = 0;
		cy = 0;

		/* Redraw map */
		display_map(&cx, &cy);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display dungeon view in sub-windows
 */
static void fix_dungeon(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_DUNGEON))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Redraw dungeon view */
		display_dungeon();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display monster recall in sub-windows
 */
static void fix_monster(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_MONSTER))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display monster race info */
		if (p_ptr->monster_race_idx) display_roff_mon(p_ptr->monster_race_idx);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Hack -- display visible monster list in sub-windows
 */
static void fix_visible(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_VISIBLE))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display monster list */
		display_visible();

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}



/*
 * Hack -- display object recall in sub-windows
 */
static void fix_object(void)
{
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *old = Term;

		/* No window */
		if (!angband_term[j]) continue;

		/* No relevant flags */
		if (!(window_flag[j] & (PW_OBJECT))) continue;

		/* Activate */
		Term_activate(angband_term[j]);

		/* Display monster race info */
		if (p_ptr->object_kind_idx) display_koff(p_ptr->object_kind_idx);

		/* Fresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Calculate number of spells player should have, and forget,
 * or remember, spells until that number is properly reflected.
 *
 * Note that this function induces various "status" messages,
 * which must be bypasses until the character is created.
 */
static void calc_spells(void)
{
	int i, j, n, levels, r;
	int num_allowed, num_known, lev;
	bool new[PY_MAX_SPELLS][2];
	byte forgot_flg[2];
	byte present_flg[2];
	spell_external sp[2];
	spell_external sp_e;
	bool done2[NUM_SPELLS];

	int realm;

	/* Save the current number of spells to learn */
	s16b old_spells = p_ptr->new_spells;

	cptr p = ((mp_ptr->spell_book == TV_SORCERY_BOOK) ? "spell" : "prayer");

	/* Realms are constant */
	sp[0].r = p_ptr->spell.realm[0]-1;
	sp[1].r = p_ptr->spell.realm[1]-1;


	forgot_flg[0] = SP_FORGOTTEN_1;
	forgot_flg[1] = SP_FORGOTTEN_2;
	present_flg[0] = SP_PRESENT_1;
	present_flg[1] = SP_PRESENT_2;

	/* Hack -- must be literate */
	if (!mp_ptr->spell_book) return;

	/* Hack -- wait for creation */
	if (!character_generated) return;

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Determine the number of spells allowed */
	levels = p_ptr->lev - mp_ptr->spell_first + 1;

	/* Hack -- no negative spells */
	if (levels < 0) levels = 0;

	/* Extract total allowed spells */
	num_allowed =
		(adj_mag_study[p_ptr->stat[mp_ptr->spell_stat].ind] * levels / PY_MAX_LEVEL);

	/* Extract maximum slots by tier */
	n = 0;
	for (i = 0; i < SPELL_LAYERS; i++)
	{
		/* Can only use slots in a tier if we are high enough level. */
		if (p_ptr->lev >= i*(PY_MAX_LEVEL/SPELL_LAYERS) + mp_ptr->spell_first)
			n += mp_ptr->max_spells[i];
		else break;
	}

	/* Use maximum */
	num_allowed = MIN(num_allowed, n);

	/* Extract maximum learnable slots */
	n = 0;

	/* Two realms */
	C_WIPE(done2, NUM_SPELLS, bool);

	for (r = 0; r < 2; r++)
	{
		/* Only check realms we have */
		if (!p_ptr->spell.realm[r]) break;

		/* Scan the spells */
		for (i = 0; i < NUM_SPELLS; i++)
		{
			/* Set the realm  and spell */
			sp_e.r = p_ptr->spell.realm[r]-1;
			sp_e.s = i;

			/* Skip non-spells */
			if (s_info[sp_e.r][sp_e.s].sval == 99) continue;

			/* In realm 2, skip spells already done. */
			if (r == 1 && done2[i]) continue;

			/* In realm 1, search for the spell being in both realms. */
			if (r == 0 && p_ptr->spell.realm[1])
			{
				for (j = 0; j < NUM_SPELLS; j++)
				{
					if (s_info[p_ptr->spell.realm[1]-1][j].s_idx !=
						s_info[sp_e.r][i].s_idx) continue;
					if (s_info[p_ptr->spell.realm[1]-1][j].realm !=
						s_info[sp_e.r][i].s_idx) continue;

					/* Found it */
					done2[j] = TRUE;

					/* Use the lower level one */
					if (s_info[p_ptr->spell.realm[1]-1][j].info[p_ptr->rp.pclass].slevel <
						s_info[sp_e.r][sp_e.s].info[p_ptr->rp.pclass].slevel)
					{
						sp_e.r = p_ptr->spell.realm[1]-1;
						sp_e.s = j;
					}

					break;
				}
			}

			/* Don't count spells that are too high level to learn */
			if (p_ptr->lev < s_info[sp_e.r][sp_e.s].info[p_ptr->rp.pclass].slevel) continue;

			/* Priestly classes can't focus, just add 1 */
			if (mp_ptr->spell_book == TV_LIFE_BOOK)
			{
				n++;
			}
			/* Non-priestly classes: take into account how much focussing we can do */
			else
			{
				n += 1 + ((p_ptr->lev - s_info[sp_e.r][sp_e.s].info[p_ptr->rp.pclass].slevel)/mp_ptr->focus_offset);
			}
		}
	}

	/* Use maximum */
	num_allowed = MIN(num_allowed, n);

	/* Number known  */
	num_known = p_ptr->spell.spell_max;

	/* Subtract off spells we've forgotten */
	for (i = 0; i < p_ptr->spell.spell_max; i++)
	{
		if (p_ptr->spell.data[i].flags & SP_PRESENT_1 &&
			p_ptr->spell.data[i].flags & SP_PRESENT_2)
		{
			if (p_ptr->spell.data[i].flags & SP_FORGOTTEN_1 &&
				p_ptr->spell.data[i].flags & SP_FORGOTTEN_2)
				num_known--;
		}
		else if (p_ptr->spell.data[i].flags & SP_PRESENT_1)
		{
			if (p_ptr->spell.data[i].flags & SP_FORGOTTEN_1)
				num_known--;
		}
		/* Must be present in 2nd realm */
		else
		{
			if (p_ptr->spell.data[i].flags & SP_FORGOTTEN_2)
				num_known--;
		}
	}

	/* See how many spells we must forget or may learn */
	p_ptr->new_spells = num_allowed - num_known;

	/* Check spells against the proper level for learning them */
	for (i = p_ptr->spell.spell_max - 1; i >= 0; i--)
	{
		r = p_ptr->spell.data[i].realm;

		new[i][0] = new[i][1] = FALSE;

		/* Spell exists in both realms */
		if (p_ptr->spell.data[i].flags & SP_PRESENT_1 &&
			p_ptr->spell.data[i].flags & SP_PRESENT_2)
		{
			bool forgot[2];

			forgot[0] = forgot[1] = FALSE;

			for (r = 0; r < 2; r++)
			{
				realm = p_ptr->spell.realm[r]-1;

				/* Calculate the spell's "level" */
				lev = s_info[realm][p_ptr->spell.data[i].spell[r]].info[p_ptr->rp.pclass].slevel +
						(p_ptr->spell.data[i].focus-1)*(mp_ptr->focus_offset);

				/* Skip spells we shouldn't forget yet */
				if (lev <= p_ptr->lev)
					continue;

				/* Do we need to mention it? */
				if (!(p_ptr->spell.data[i].flags & forgot_flg[r]))
				{
					new[i][r] = TRUE;
				}

				/* Mark as forgotten in this realm */
				p_ptr->spell.data[i].flags |= forgot_flg[r];
				forgot[r] = TRUE;
			}

			/* One more spell can be learned, if we forgot this spell
				in both realms */
			if (forgot[0] && forgot[1])
				p_ptr->new_spells++;
		}
		else if (p_ptr->spell.data[i].flags & SP_PRESENT_1 ||
			p_ptr->spell.data[i].flags & SP_PRESENT_2)
		{
			r = ((p_ptr->spell.data[i].flags & SP_PRESENT_1) ? 0 : 1);
			realm = p_ptr->spell.realm[r]-1;

			/* Calculate the spell's "level" */
			lev = s_info[realm][p_ptr->spell.data[i].spell[r]].info[p_ptr->rp.pclass].slevel +
					(p_ptr->spell.data[i].focus-1)*(mp_ptr->focus_offset);

			/* Skip spells we shouldn't forget yet */
			if (lev <= p_ptr->lev)
				continue;

			/* Do we need to mention it? */
			if (!(p_ptr->spell.data[i].flags & forgot_flg[r]))
			{
				new[i][r] = TRUE;
			}

			/* Mark as forgotten in this realm */
			p_ptr->spell.data[i].flags |= forgot_flg[r];

			/* One more spell can be learned */
			p_ptr->new_spells++;
		}

	}


	/* Forget spells if we know too many spells */
	for (i = p_ptr->spell.spell_max - 1; i >= 0; i--)
	{
		bool forgot = FALSE;

		/* Stop when possible */
		if (p_ptr->new_spells >= 0) break;

		for (r = 0; r < 2; r++)
		{
			/* Skip when this realm doesn't have this spell, or we've already
				forgotton this one. */
			if (!(p_ptr->spell.data[i].flags & present_flg[r]) || new[i][r]) continue;

			if (!(p_ptr->spell.data[i].flags & forgot_flg[r]))
			{
				new[i][r] = TRUE;
				forgot = TRUE;
				p_ptr->spell.data[i].flags |= forgot_flg[r];
			}
		}

		/* Did we forget the spell? */
		if (forgot)
			/* Can learn one more spell. */
			p_ptr->new_spells++;
	}

	/* Mention forgotten spells */
	for (i = p_ptr->spell.spell_max - 1; i >= 0; i--)
	{
		sp[0].s = p_ptr->spell.data[i].spell[0];
		sp[1].s = p_ptr->spell.data[i].spell[1];

		if (new[i][0] && new[i][1])
		{

			/* Mention we are forgtting both */
			if (p_ptr->spell.data[i].focus == 1)
				msgf("You have forgotten the %s of %s/%s.", p,
						   	spell_name(sp[0]), spell_name(sp[1]));
			else
				msgf("You have forgotten how to %s the %s of %s/%s.",
							focus_description[p_ptr->spell.data[i].focus-1], p,
						   	spell_name(sp[0]), spell_name(sp[1]));
		}
		else if (new[i][0] || new[i][1])
		{
			r = (new[i][0] ? 0 : 1);

			/* Mention we are forgetting it */
			if (p_ptr->spell.data[i].focus == 1)
				msgf("You have forgotten the %s of %s.", p,
						   spell_name(sp[r]));
			else
				msgf("You have forgotten how to %s the %s of %s.",
							focus_description[p_ptr->spell.data[i].focus-1], p,
						   spell_name(sp[r]));
		}
	}

	/* Check for spells to remember */
	for (i = 0; i < PY_MAX_SPELLS; i++)
	{
		bool counts = FALSE;
		bool both = FALSE;
		bool done = FALSE;
		byte flags = p_ptr->spell.data[i].flags;

		new[i][0] = new[i][1] = FALSE;

		/* None left to remember */
		if (p_ptr->new_spells <= 0) break;

		/* If the current spell is not at all forgotten, skip it. */
		if (!(flags & (forgot_flg[0] | forgot_flg[1])))
			continue;

		/* If the current spell is partially forgotten, perhaps restore it
		   to total remembrance, but this won't count as a spell remembered. */

		/* Coutns if forgotten in both realms */
		if ((flags & forgot_flg[0]) &&	(flags & forgot_flg[1]))
			counts = both = TRUE;
		/* Counts if forgotten in one realm and absent in the other */
		else if (((flags & forgot_flg[0]) && !(flags & present_flg[1])) ||
				 ((flags & forgot_flg[1]) && !(flags & present_flg[0])))
			counts = TRUE;
		else
			counts = FALSE;

		/* If in both realms, might only remember it in one. */
		if ((flags & present_flg[1]) && (flags & present_flg[0]))
		{
			for (r = 0; r < 2; r++)
			{
				realm = p_ptr->spell.realm[r]-1;

				/* Calculate the spell's "level" */
				lev = s_info[realm][p_ptr->spell.data[i].spell[r]].info[p_ptr->rp.pclass].slevel +
					(p_ptr->spell.data[i].focus-1)*(mp_ptr->focus_offset);

				/* Skip spells we cannot remember */
				if (lev > p_ptr->lev)
				{
					/* Would only be a partial remember, so doesn't "count." */
					if (both) counts = FALSE;
					continue;
				}
				/* Going to remember something */
				done = TRUE;

				/* Mark the spell as remembered */
				if (flags & forgot_flg[r])
					new[i][r] = TRUE;

				p_ptr->spell.data[i].flags &= ~forgot_flg[r];
				flags = p_ptr->spell.data[i].flags;
			}

			/* If can't remember, skip it. */
			if (!done) continue;
		}
		else
		{
			/* Get the realm the spell is in */
			r = (flags & present_flg[0] ? 0 : 1);

			realm = p_ptr->spell.realm[r]-1;

			/* Calculate the spell's "level" */
			lev = s_info[realm][p_ptr->spell.data[i].spell[r]].info[p_ptr->rp.pclass].slevel +
				(p_ptr->spell.data[i].focus-1)*(mp_ptr->focus_offset);

			/* Skip spells we can't remember */
			if (lev > p_ptr->lev) continue;

			/* Mark the spell as remembered */
			if (flags & forgot_flg[r])
				new[i][r] = TRUE;

			p_ptr->spell.data[i].flags &= ~forgot_flg[r];
			flags = p_ptr->spell.data[i].flags;
		}

		/* Prepare for messages */
		sp[0].s = p_ptr->spell.data[i].spell[0];
		sp[1].s = p_ptr->spell.data[i].spell[1];


		/* Message, if the player remembered anything */
		if (new[i][0] && new[i][1])
		{
			/* Newly remembered both */
			if (p_ptr->spell.data[i].focus == 1)
				msgf("You have remembered the %s of %s/%s.", p,
							spell_name(sp[0]), spell_name(sp[1]));
			else
				msgf("You have remembered how to %s the %s of %s/%s.",
							focus_description[p_ptr->spell.data[i].focus-1], p,
							spell_name(sp[0]), spell_name(sp[1]));
		}
		else if (new[i][0] || new[i][1])
		{
			r = (new[i][0] ? 0 : 1);

			/* Newly remembered one */
			if (p_ptr->spell.data[i].focus == 1)
				msgf("You have remembered the %s of %s.", p, spell_name(sp[r]));
			else
				msgf("You have remembered how to %s the %s of %s.",
							focus_description[p_ptr->spell.data[i].focus-1], p, spell_name(sp[r]));
		}
		else
			/* Paranoia - didn't remember either, shouldn't re-count spells */
			continue;

		if (counts)
		{
			/* One less can be learned */
			p_ptr->new_spells--;
		}
	}

	/* Spell count changed */
	if (old_spells != p_ptr->new_spells)
	{
		/* Message if needed */
		if (p_ptr->new_spells)
		{
			/* Message */
			msgf("You can learn %d more %s%s.",
					   p_ptr->new_spells, p,
					   (p_ptr->new_spells != 1) ? "s" : "");
		}

		/* Redraw Study Status */
		p_ptr->redraw |= (PR_STUDY);
	}
}


/*
 * Calculate maximum mana.  You do not need to know any spells.
 * Note that mana is lowered by heavy (or inappropriate) armor.
 *
 * This function induces status messages.
 */
static void calc_mana(void)
{
	int msp, levels, cur_wgt, max_wgt;

	object_type *o_ptr;

	bool old_cumber_glove = p_ptr->state.cumber_glove;
	bool old_cumber_armor = p_ptr->state.cumber_armor;


	/* Hack -- Must be literate */
	if (!mp_ptr->spell_book) return;

	if (p_ptr->rp.pclass == CLASS_MINDCRAFTER)
	{
		levels = p_ptr->lev;
	}
	else
	{
		/* Extract "effective" player level */
		levels = (p_ptr->lev - mp_ptr->spell_first) + 1;
	}


	/* Hack -- no negative mana */
	if (levels < 0) levels = 0;

	/* Extract total mana */
	msp = (adj_mag_mana[p_ptr->stat[mp_ptr->spell_stat].ind] * levels / 25);
	if (levels > 0)
		msp += (adj_mag_mana[p_ptr->stat[mp_ptr->spell_stat].ind] / 20);

	/* Hack -- usually add one mana */
	if (msp) msp++;

	/* Hack: High mages have a 25% mana bonus */
	if (msp && (p_ptr->rp.pclass == CLASS_HIGH_MAGE)) msp += msp / 4;


	/* Only mages are affected */
	if (mp_ptr->spell_book == TV_SORCERY_BOOK)
	{
		/* Assume player is not encumbered by gloves */
		p_ptr->state.cumber_glove = FALSE;

		/* Get the gloves */
		o_ptr = &p_ptr->equipment[EQUIP_HANDS];

		/* Normal gloves hurt mage-type spells */
		if (o_ptr->k_idx &&
			!((FLAG(o_ptr, TR_FREE_ACT)) || (FLAG(o_ptr, TR_DEX) && (o_ptr->pval > 0))))
		{
			/* Encumbered */
			p_ptr->state.cumber_glove = TRUE;

			/* Reduce mana */
			msp = (3 * msp) / 4;
		}
	}


	/* Assume player not encumbered by armor */
	p_ptr->state.cumber_armor = FALSE;

	/* Weigh the armor */
	cur_wgt = 0;
	cur_wgt += p_ptr->equipment[EQUIP_BODY].weight;
	cur_wgt += p_ptr->equipment[EQUIP_HEAD].weight;
	cur_wgt += p_ptr->equipment[EQUIP_ARM].weight;
	cur_wgt += p_ptr->equipment[EQUIP_OUTER].weight;
	cur_wgt += p_ptr->equipment[EQUIP_HANDS].weight;
	cur_wgt += p_ptr->equipment[EQUIP_FEET].weight;

	/* Determine the weight allowance */
	max_wgt = mp_ptr->spell_weight;

	/* Heavy armor penalizes mana by a percentage.  -LM- */
	if (((cur_wgt - max_wgt) / 10) > 0)
	{
		/* Encumbered */
		p_ptr->state.cumber_armor = TRUE;

		/*
		 * Subtract a percentage of maximum mana.
		 * The addition of one is to make sure the
		 * mana total is decreased by some amount.
		 */
		switch (p_ptr->rp.pclass)
		{
			case CLASS_MAGE:
			case CLASS_HIGH_MAGE:
			{
				/*
				 * For these classes, mana is halved if armour
				 * is 30 pounds over their weight limit.
				 */
				msp -= msp * (cur_wgt - max_wgt) / 700 + 1;
				break;
			}

			case CLASS_PRIEST:
			case CLASS_MINDCRAFTER:
			{
				/* Mana halved if armour is 40 pounds over weight limit. */
				msp -= msp * (cur_wgt - max_wgt) / 900 + 1;
				break;
			}

			case CLASS_ROGUE:
			case CLASS_RANGER:
			case CLASS_MONK:
			{
				/* Mana halved if armour is 50 pounds over weight limit. */
				msp -= msp * (cur_wgt - max_wgt) / 1100 + 1;
				break;
			}

			case CLASS_PALADIN:
			case CLASS_CHAOS_WARRIOR:
			case CLASS_WARRIOR_MAGE:
			{
				/* Mana halved if armour is 60 pounds over weight limit. */
				msp -= msp * (cur_wgt - max_wgt) / 1300 + 1;
				break;
			}

			default:
			{
				/* For new classes, but not yet added to this formula. */
				msp -= msp * (cur_wgt - max_wgt) / 800 + 1;
				break;
			}
		}
	}


	/* Add bonus mana (not affected by encumberance or gloves) */
	msp += p_ptr->sp_bonus * levels;

	/* Mana can never be negative */
	if (msp < 0) msp = 0;

	/* Handle lich */
	if (p_ptr->state.lich)
	{
		int bonus, mhp;

		/* Un-inflate "half-hitpoint bonus per level" value */
		bonus = ((int)(adj_con_mhp[p_ptr->stat[A_CON].ind]) - 128);

		/* Calculate hitpoints */
		mhp = p_ptr->player_hp[p_ptr->lev - 1] + (bonus * p_ptr->lev / 2);

		/* Always have at least one hitpoint per level */
		if (mhp < p_ptr->lev + 5) mhp = p_ptr->lev + 5;

		/* Factor in the hero / superhero settings */
		if (query_timed(TIMED_HERO)) mhp += 10;
		if (query_timed(TIMED_SHERO)) mhp += 30;

		/* New maximum SP */
		msp = MAX(msp+(mhp/3), mhp+(msp/3));
	}

	/* Maximum mana has changed */
	if (p_ptr->msp != msp)
	{
		/* Enforce maximum */
		if (p_ptr->csp >= msp)
		{
			p_ptr->csp = msp;
			p_ptr->csp_frac = 0;
		}

		/* Save new mana */
		p_ptr->msp = msp;

		/* Display mana later */
		p_ptr->redraw |= (PR_MANA);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
		p_ptr->window |= (PW_SPELL);
	}


	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "glove state" changes */
	if (old_cumber_glove != p_ptr->state.cumber_glove)
	{
		/* Message */
		if (p_ptr->state.cumber_glove)
		{
			msgf("Your covered hands feel unsuitable for spellcasting.");
		}
		else
		{
			msgf("Your hands feel more suitable for spellcasting.");
		}
	}


	/* Take note when "armor state" changes */
	if (old_cumber_armor != p_ptr->state.cumber_armor)
	{
		/* Message */
		if (p_ptr->state.cumber_armor)
		{
			msgf("The weight of your armor encumbers your movement.");
		}
		else
		{
			msgf("You feel able to move more freely.");
		}
	}
}



/*
 * Calculate the players (maximal) hit points
 * Adjust current hitpoints if necessary
 */
static void calc_hitpoints(void)
{
	int bonus, mhp;

	/* Un-inflate "half-hitpoint bonus per level" value */
	bonus = ((int)(adj_con_mhp[p_ptr->stat[A_CON].ind]) - 128);

	/* Calculate hitpoints */
	mhp = p_ptr->player_hp[p_ptr->lev - 1] + (bonus * p_ptr->lev / 2);

	/* Always have at least one hitpoint per level */
	if (mhp < p_ptr->lev + 5) mhp = p_ptr->lev + 5;

	/* Factor in the hero / superhero settings */
	if (query_timed(TIMED_HERO)) mhp += 10;
	if (query_timed(TIMED_SHERO)) mhp += 30;

	/* New maximum hitpoints */
	if (p_ptr->mhp != mhp)
	{
		/* Enforce maximum */
		if (p_ptr->chp >= mhp)
		{
			p_ptr->chp = mhp;
			p_ptr->chp_frac = 0;
		}

		/* Save the new max-hitpoints */
		p_ptr->mhp = mhp;

		/* Display hitpoints (later) */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}

	/* Handle lich */
	if (p_ptr->state.lich)
	{
		p_ptr->mhp = 0;
		p_ptr->chp = 0;

		/* Display hitpoints (later) */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}
}



/*
 * Extract and set the current "lite radius"
 */
static void calc_torch(void)
{
	int i;
	object_type *o_ptr;

	s16b old_lite = p_ptr->cur_lite;

	/* Assume no light */
	p_ptr->cur_lite = 0;

	/* Loop through all wielded items */
	for (i = 0; i < EQUIP_MAX; i++)
	{
		o_ptr = &p_ptr->equipment[i];

		/* Examine actual lites */
		if ((i == EQUIP_LITE) && (o_ptr->k_idx) && (o_ptr->tval == TV_LITE))
		{
			/* Artifact Lites provide permanent, bright, lite */
			if (FLAG(o_ptr, TR_INSTA_ART))
			{
				p_ptr->cur_lite += 4;
				continue;
			}

			/* Lanterns (with fuel) provide more lite */
			if ((o_ptr->sval == SV_LITE_LANTERN) && (o_ptr->timeout > 0))
			{
				p_ptr->cur_lite += 3;
				continue;
			}

			/* Torches (with fuel) provide some lite */
			if ((o_ptr->sval == SV_LITE_TORCH) && (o_ptr->timeout > 0))
			{
				p_ptr->cur_lite += 2;
				continue;
			}
		}
		else
		{
			/* Skip empty slots */
			if (!o_ptr->k_idx) continue;

			/* does this item glow? */
			if (FLAG(o_ptr, TR_LITE)) p_ptr->cur_lite++;
		}
	}

	/*
	 * Check if the player doesn't have a lite source,
	 * but does glow as an intrinsic.
	 */
	if ((p_ptr->cur_lite == 0) && (FLAG(p_ptr, TR_LITE)))
	{
		p_ptr->cur_lite = 1;
	}

	/*
	 * Apply luminosity
	 */
	if (query_timed(TIMED_LUMINOSITY))
	{
		/* Only has an effect if the player
		   has light */
		if (p_ptr->cur_lite)
			p_ptr->cur_lite += 4;
	}


	/* Adjust radius for pre-dawn / dusk */
	if (!p_ptr->depth)
	{
		s32b plrtime = turn % TOWN_DAY;
		/* Calculate the half hour, from 0 to 48.  The beginning
		   of 0 is dawn, the beginning of 24 is dusk. */
		int hour = (int) (plrtime / TOWN_HALF_HOUR);

		/* Radius starts at minimum 8 within 30 minutes of dawn / dusk,
		   half that for every 30 minutes further away. */
		if (hour == 24 || hour == 47)
			p_ptr->cur_lite = MAX(p_ptr->cur_lite, 8);
		else if (hour == 25 || hour == 46)
			p_ptr->cur_lite = MAX(p_ptr->cur_lite, 4);
		else if (hour == 26 || hour == 45)
			p_ptr->cur_lite = MAX(p_ptr->cur_lite, 2);
		else if (hour == 27 || hour == 44)
			p_ptr->cur_lite = MAX(p_ptr->cur_lite, 1);
	}

	/*
	 * Hack - blindness gives a torch radius of zero.
	 * This speeds up the map_info() function.
	 */
	if (query_timed(TIMED_BLIND))
	{
		/* No light */
		p_ptr->cur_lite = 0;
	}

	/* Notice changes in the "lite radius" */
	if (old_lite != p_ptr->cur_lite)
	{
		/* Update the view */
		p_ptr->update |= (PU_VIEW);

		/* Update the monsters */
		p_ptr->update |= (PU_MONSTERS);

		/* Update the monster lighting */
		p_ptr->update |= (PU_MON_LITE);

		/* Redraw the map */
		p_ptr->redraw |= (PR_MAP);
	}
}

/*
 * Recalculate the inventory and equipment weight
 */
static void calc_weight(void)
{
	object_type *o_ptr;

	int i;

	/* No weight yet */
	p_ptr->total_weight = 0;

	OBJ_ITT_START (p_ptr->inventory, o_ptr)
	{
		/* Increase the weight */
		p_ptr->total_weight += object_weight(o_ptr);
	}
	OBJ_ITT_END;

	for (i = 0; i < EQUIP_MAX; i++)
	{
		o_ptr = &p_ptr->equipment[i];

		/* Need valid items */
		if (!o_ptr->k_idx) continue;

		/* Increase the weight */
		p_ptr->total_weight += o_ptr->weight;
	}
}

/*
 * Computes current weight limit.
 */
static int weight_limit(void)
{
	int i;

	/* Weight limit based only on strength */
	i = adj_str_wgt[p_ptr->stat[A_STR].ind] * 100;

	/* Return the result */
	return (i);
}

/* Calculate all class-based bonuses and penalties to melee Skill.  Oangband
 * recognizes that it takes a great deal of training to get critical hits with
 * a large, heavy weapon - training that many classes simply do not have the
 * time or inclination for.  -LM-
 */
static sint add_special_melee_skill(byte pclass, object_type *o_ptr)
{
	int add_skill = 0;
	s16b weight = o_ptr->weight;

	switch (pclass)
	{
		case CLASS_WARRIOR:
		{
			/*
			 * Warrior.
			 * Can use 15 lb weapons without penalty at level 1,
			 * and 45 lb weapons without penalty at 50th level.
			 */
			add_skill = 30 + p_ptr->lev - (weight / 6);
			if (add_skill > 0) add_skill = 0;
			if (add_skill < -10) add_skill = -10;
			break;
		}

		case CLASS_MAGE:
		case CLASS_HIGH_MAGE:
		{
			/*
			 * Mage/High Mage.
			 * Can use 6 lb weapons without penalty at level 1,
			 * and 16 lb weapons without penalty at 50th level.
			 */
			add_skill = 25 + (2 * p_ptr->lev / 3) - (weight / 3);
			if (add_skill > 0) add_skill = 0;
			if (add_skill < -30) add_skill = -30;
			break;
		}

		case CLASS_PRIEST:
		{
			/*
			 * Priest.
			 * Can use 12 lb weapons without penalty at level 1,
			 * and 22 lb weapons without penalty at 50th level.
			 */
			add_skill = 35 + (1 * p_ptr->lev / 2) - (weight / 4);
			if (add_skill > 0) add_skill = 0;
			if (add_skill < -25) add_skill = -25;
			break;
		}

		case CLASS_ROGUE:
		{
			/*
			 * Rogue.
			 * Can use 10 lb weapons without penalty at level 1,
			 * and 20 lb weapons without penalty at 50th level.
			 * Can get a bonus for using light weapons.
			 */
			if (!o_ptr->k_idx)
			{
				add_skill = 0;
			}
			else
			{
				add_skill = 35 + (2 * p_ptr->lev / 3) - (weight / 4);
				if (add_skill > 0) add_skill = add_skill / 2;
				if (add_skill > 15) add_skill = 15;
				if (add_skill < -25) add_skill = -25;
			}
			break;
		}

		case CLASS_RANGER:
		{
			/*
			 * Ranger.
			 * Can use 12 lb weapons without penalty at level 1,
			 * and 25 lb weapons without penalty at 50th level.
			 */
			add_skill = 30 + (1 * p_ptr->lev / 2) - (weight / 5);
			if (add_skill > 0) add_skill = 0;
			if (add_skill < -20) add_skill = -20;
			break;
		}

		case CLASS_PALADIN:
		case CLASS_CHAOS_WARRIOR:
		case CLASS_WARRIOR_MAGE:
		{
			/*
			 * Paladin/Chaos warrior/Warrior mage.
			 * Can use 15 lb weapons without penalty at level 1,
			 * and 45 lb weapons without penalty at 50th level.
			 */
			add_skill = 30 + p_ptr->lev - (weight / 6);
			if (add_skill > 0) add_skill = 0;
			if (add_skill < -10) add_skill = -10;
			break;
		}

		case CLASS_MONK:
		{
			/*
			 * Monk.
			 * Can use 5 lb weapons without penalty at level 1,
			 * and slightly over 12 lb weapons without penalty at 50th level.
			 * Much prefers to use hands and feet.
			 */
			if (!o_ptr->k_idx)
			{
				add_skill = 14 + (p_ptr->lev);
			}
			else
			{
				add_skill = 25 + (p_ptr->lev / 2) - (weight / 3);
				if (add_skill > 0) add_skill = 0;
				if (add_skill < -30) add_skill = -30;
			}
			break;
		}

		case CLASS_MINDCRAFTER:
		{
			/*
			 * Mindcrafter.
			 * Can use 6 lb weapons without penalty at level 1,
			 * and 16 lb weapons without penalty at 50th level.
			 */
			add_skill = 25 + (2 * p_ptr->lev / 3) - (weight / 3);
			if (add_skill > 0) add_skill = 0;
			if (add_skill < -30) add_skill = -30;
			break;
		}
	}

	return (add_skill);
}


/*
 * Calculate all class and race-based bonuses and penalties to missile skill
 *			 -LM-
 */
static sint add_special_missile_skill(byte pclass)
{
	int add_skill = 0;

	switch (pclass)
	{
		case CLASS_ROGUE:
		{
			/* Rogues are good with slings. */
			if (p_ptr->ammo_tval == TV_SHOT)
			{
				add_skill = 3 + p_ptr->lev / 4;
			}
			break;
		}

		case CLASS_RANGER:
		{
			/*
			 * Rangers have a high missile skill,
			 * but they are not supposed to
			 * be great with xbows and slings.
			 */
			if (p_ptr->ammo_tval == TV_SHOT)
			{
				add_skill = 2 - p_ptr->lev / 7;
			}
			if (p_ptr->ammo_tval == TV_BOLT)
			{
				add_skill = 2 - p_ptr->lev / 7;
			}
			break;
		}

		case CLASS_MONK:
		{
			/* Monks get a small bonus with slings. */
			if (p_ptr->ammo_tval == TV_SHOT)
			{
				add_skill = 1 - p_ptr->lev / 7;
			}
		}
	}

	return (add_skill);
}


void object_bonuses(const object_type *o_ptr, bonuses_type *b)
{
	/* Zero the bonuses */
	memset(b, 0, sizeof(bonuses_type));

	/* Affect stats */
	if (FLAG(o_ptr, TR_STR)) b->stat[A_STR] = o_ptr->pval;
	if (FLAG(o_ptr, TR_INT)) b->stat[A_INT] = o_ptr->pval;
	if (FLAG(o_ptr, TR_WIS)) b->stat[A_WIS] = o_ptr->pval;
	if (FLAG(o_ptr, TR_DEX)) b->stat[A_DEX] = o_ptr->pval;
	if (FLAG(o_ptr, TR_CON)) b->stat[A_CON] = o_ptr->pval;
	if (FLAG(o_ptr, TR_CHR)) b->stat[A_CHR] = o_ptr->pval;

	/* Affect mana */
	if (FLAG(o_ptr, TR_SP)) b->sp_bonus = o_ptr->pval;

	/* Affect stealth */
	if (FLAG(o_ptr, TR_STEALTH)) b->skills[SKILL_STL] = o_ptr->pval;

	/* Affect sensing ability (factor of five) */
	if (FLAG(o_ptr, TR_SEARCH)) b->skills[SKILL_SNS] = (o_ptr->pval * 5);

	/* Affect searching frequency (factor of five) */
	if (FLAG(o_ptr, TR_SEARCH)) b->skills[SKILL_FOS] = (o_ptr->pval * 5);

	/* Affect infravision */
	if (FLAG(o_ptr, TR_INFRA)) b->see_infra = o_ptr->pval;

	/* Affect digging (factor of 20) */
	if (FLAG(o_ptr, TR_TUNNEL)) b->skills[SKILL_DIG] = (o_ptr->pval * 20);

	/* Affect speed */
	if (FLAG(o_ptr, TR_SPEED)) b->pspeed = o_ptr->pval;

	/* Affect blows */
	if (FLAG(o_ptr, TR_BLOWS)) b->extra_blows = o_ptr->pval;

	/* Boost shots */
	if (FLAG(o_ptr, TR_XTRA_SHOTS)) b->extra_shots = 1;

	/* Boost saving throws */
	if (FLAG(o_ptr, TR_LUCK_10)) b->skills[SKILL_SAV] = 10;

	/* Apply special bonuses */
	apply_object_trigger(TRIGGER_BONUS, (object_type *)o_ptr, "p", "b", "bonuses_type", b);
}


void object_bonuses_known(const object_type *o_ptr, bonuses_type *b)
{
	/* Zero the bonuses */
	memset(b, 0, sizeof(bonuses_type));

	/* Affect stats */
	if (KN_FLAG(o_ptr, TR_STR)) b->stat[A_STR] = o_ptr->pval;
	if (KN_FLAG(o_ptr, TR_INT)) b->stat[A_INT] = o_ptr->pval;
	if (KN_FLAG(o_ptr, TR_WIS)) b->stat[A_WIS] = o_ptr->pval;
	if (KN_FLAG(o_ptr, TR_DEX)) b->stat[A_DEX] = o_ptr->pval;
	if (KN_FLAG(o_ptr, TR_CON)) b->stat[A_CON] = o_ptr->pval;
	if (KN_FLAG(o_ptr, TR_CHR)) b->stat[A_CHR] = o_ptr->pval;

	/* Affect mana */
	if (KN_FLAG(o_ptr, TR_SP)) b->sp_bonus = o_ptr->pval;

	/* Affect stealth */
	if (KN_FLAG(o_ptr, TR_STEALTH)) b->skills[SKILL_STL] = o_ptr->pval;

	/* Affect sensing ability (factor of five) */
	if (KN_FLAG(o_ptr, TR_SEARCH)) b->skills[SKILL_SNS] = (o_ptr->pval * 5);

	/* Affect searching frequency (factor of five) */
	if (KN_FLAG(o_ptr, TR_SEARCH)) b->skills[SKILL_FOS] = (o_ptr->pval * 5);

	/* Affect infravision */
	if (KN_FLAG(o_ptr, TR_INFRA)) b->see_infra = o_ptr->pval;

	/* Affect digging (factor of 20) */
	if (KN_FLAG(o_ptr, TR_TUNNEL)) b->skills[SKILL_DIG] = (o_ptr->pval * 20);

	/* Affect speed */
	if (KN_FLAG(o_ptr, TR_SPEED)) b->pspeed = o_ptr->pval;

	/* Affect blows */
	if (KN_FLAG(o_ptr, TR_BLOWS)) b->extra_blows = o_ptr->pval;

	/* Boost shots */
	if (KN_FLAG(o_ptr, TR_XTRA_SHOTS)) b->extra_shots = 1;

	/* Boost saving throws */
	if (KN_FLAG(o_ptr, TR_LUCK_10)) b->skills[SKILL_SAV] = 10;

	/* Apply special bonuses */
	if (object_known_full(o_ptr))
		apply_object_trigger(TRIGGER_BONUS, (object_type *)o_ptr, "p", "b", "bonuses_type", b);
}


/*
 * Calculate the players current "state", taking into account
 * not only race/class intrinsics, but also objects being worn
 * and temporary spell effects.
 *
 * See also calc_mana() and calc_hitpoints().
 *
 * Take note of the new "speed code", in particular, a very strong
 * player will start slowing down as soon as he reaches 150 pounds,
 * but not until he reaches 450 pounds will he be half as fast as
 * a normal kobold.  This both hurts and helps the player, hurts
 * because in the old days a player could just avoid 300 pounds,
 * and helps because now carrying 300 pounds is not very painful.
 *
 * The "weapon" and "bow" do *not* add to the bonuses to hit or to
 * damage, since that would affect non-combat things.  These values
 * are actually added in later, at the appropriate place.
 *
 * This function induces various "status" messages.
 */
static void calc_bonuses(void)
{
	int i, j, hold;
	int old_speed;
	bool old_telepathy;
	bool old_see_inv;
	int old_dis_ac;
	int old_dis_to_a;
	int extra_blows;
	int extra_shots;
	object_type *o_ptr;

	bool old_heavy_wield = p_ptr->state.heavy_wield;
	bool old_heavy_shoot = p_ptr->state.heavy_shoot;
	bool old_icky_wield = p_ptr->state.icky_wield;
	bool old_monk_armour = p_ptr->state.monk_armour_stat;

	object_flags oflags;
	object_flags *of_ptr = &oflags;

	/* Save the old speed */
	old_speed = p_ptr->pspeed;

	/* Save the old vision stuff */
	old_telepathy = FLAG(p_ptr, TR_TELEPATHY) ? TRUE : FALSE;
	old_see_inv = FLAG(p_ptr, TR_SEE_INVIS) ? TRUE : FALSE;

	/* Save the old armor class */
	old_dis_ac = p_ptr->dis_ac;
	old_dis_to_a = p_ptr->dis_to_a;


	/* Clear extra blows/shots */
	extra_blows = extra_shots = 0;

	/* Calculate monk armour status */
	if (p_ptr->rp.pclass == CLASS_MONK)
	{
		u16b monk_arm_wgt = 0;

		/* Weigh the armor */
		monk_arm_wgt += p_ptr->equipment[EQUIP_BODY].weight;
		monk_arm_wgt += p_ptr->equipment[EQUIP_HEAD].weight;
		monk_arm_wgt += p_ptr->equipment[EQUIP_ARM].weight;
		monk_arm_wgt += p_ptr->equipment[EQUIP_OUTER].weight;
		monk_arm_wgt += p_ptr->equipment[EQUIP_HANDS].weight;
		monk_arm_wgt += p_ptr->equipment[EQUIP_FEET].weight;

		if (monk_arm_wgt > (150 + (p_ptr->lev * 4)))
		{
			/* Burdened */
			p_ptr->state.monk_armour_stat = TRUE;
		}
		else
		{
			/* Not burdened */
			p_ptr->state.monk_armour_stat = FALSE;
		}
	}

	/* Clear the stat modifiers */
	for (i = 0; i < A_MAX; i++) p_ptr->stat[i].add = 0;


	/* Clear the Displayed/Real armor class */
	p_ptr->dis_ac = p_ptr->ac = 0;

	/* Clear the Displayed/Real Bonuses */
	p_ptr->dis_to_h = p_ptr->to_h = 0;
	p_ptr->dis_to_d = p_ptr->to_d = 0;
	p_ptr->dis_to_a = p_ptr->to_a = 0;

	/* Start with "normal" speed */
	p_ptr->pspeed = 110;

	/* Start with "normal" mana */
	p_ptr->sp_bonus = 0;

	/* Start with a single blow per turn */
	p_ptr->num_blow = 1;

	/* Start with a single shot per turn */
	p_ptr->num_fire = 1;

	/* Reset the "ammo" tval */
	p_ptr->ammo_tval = 0;

	/* Clear all the flags */
	p_ptr->flags[0] = 0;
	p_ptr->flags[1] = 0;
	p_ptr->flags[2] = 0;
	p_ptr->flags[3] = 0;

	/* Base infravision (purely racial) */
	p_ptr->see_infra = rp_ptr->infra;


	/* Base skill -- disarming */
	p_ptr->skills[SKILL_DIS] = rp_ptr->r_dis + cp_ptr->c_dis;

	/* Base skill -- magic devices */
	p_ptr->skills[SKILL_DEV] = rp_ptr->r_dev + cp_ptr->c_dev;

	/* Base skill -- saving throw */
	p_ptr->skills[SKILL_SAV] = rp_ptr->r_sav + cp_ptr->c_sav;

	/* Base skill -- stealth */
	p_ptr->skills[SKILL_STL] = rp_ptr->r_stl + cp_ptr->c_stl;

	/* Base skill -- sensing ability */
	p_ptr->skills[SKILL_SNS] = rp_ptr->r_sns + cp_ptr->c_sns;

	/* Base skill -- searching frequency */
	p_ptr->skills[SKILL_FOS] = rp_ptr->r_fos + cp_ptr->c_fos;

	/* Base skill -- combat (normal) */
	p_ptr->skills[SKILL_THN] = rp_ptr->r_thn + cp_ptr->c_thn;

	/* Base skill -- combat (shooting) */
	p_ptr->skills[SKILL_THB] = rp_ptr->r_thb + cp_ptr->c_thb;

	/* Base skill -- combat (throwing) */
	p_ptr->skills[SKILL_THT] = rp_ptr->r_thb + cp_ptr->c_thb;

	/* Base skill -- digging */
	p_ptr->skills[SKILL_DIG] = 0;

	/* Get the player racial/class flags (including some mutations) */
	player_flags(of_ptr);

	/* Hack - handle speed from monk/sprite/klackon here */
	if (FLAG(of_ptr, TR_SPEED))
	{
		/* Being both should give you both bonuses */
		if ((p_ptr->rp.prace == RACE_KLACKON || p_ptr->rp.prace == RACE_SPRITE) && p_ptr->rp.pclass == CLASS_MONK)
			p_ptr->pspeed += p_ptr->lev / 10;
		p_ptr->pspeed += (p_ptr->lev) / 10;
		of_ptr->flags[0] &= ~(TR0_SPEED);
	}

	p_ptr->flags[0] |= of_ptr->flags[0];
	p_ptr->flags[1] |= of_ptr->flags[1];
	p_ptr->flags[2] |= of_ptr->flags[2];
	p_ptr->flags[3] |= of_ptr->flags[3];

	/* Effects of constantly acting mutations */
	if (p_ptr->muta3)
	{
		mutation_effect();
	}

	/* Scan the usable inventory */
	for (i = 0; i < EQUIP_MAX; i++)
	{
		bonuses_type b;

		o_ptr = &p_ptr->equipment[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		p_ptr->flags[0] |= o_ptr->flags[0];
		p_ptr->flags[1] |= o_ptr->flags[1];
		p_ptr->flags[2] |= o_ptr->flags[2];
		p_ptr->flags[3] |= o_ptr->flags[3];

		/* Calculate bonuses from object */
		object_bonuses(o_ptr, &b);

		/* Modify the base armor class */
		p_ptr->ac += o_ptr->ac;

		/* The base armor class is always known */
		p_ptr->dis_ac += o_ptr->ac;

		/* Apply bonuses to stats */
		for (j = 0; j < A_MAX; j++)
		{
			p_ptr->stat[j].add += b.stat[j];
		}

		p_ptr->sp_bonus += b.sp_bonus;
		p_ptr->see_infra += b.see_infra;
		p_ptr->pspeed += b.pspeed;
		extra_blows += b.extra_blows;
		extra_shots += b.extra_shots;

		/* Apply bonuses to skills */
		for (j = 0; j < MAX_SKILL; j++)
		{
			p_ptr->skills[j] += b.skills[j];
		}

		/* Apply the bonuses to armor class */
		p_ptr->to_a += o_ptr->to_a;

		/* Apply the mental bonuses to armor class, if known */
		if (object_known_p(o_ptr)) p_ptr->dis_to_a += o_ptr->to_a;

		/* Hack -- do not apply "weapon" bonuses */
		if (i == EQUIP_WIELD) continue;

		/* Hack -- do not apply "bow" bonuses */
		if (i == EQUIP_BOW) continue;

		/* Apply the bonuses to hit/damage */
		p_ptr->to_h += o_ptr->to_h;
		p_ptr->to_d += o_ptr->to_d;

		/* Apply the mental bonuses tp hit/damage, if known */
		if (object_known_p(o_ptr)) p_ptr->dis_to_h += o_ptr->to_h;
		if (object_known_p(o_ptr)) p_ptr->dis_to_d += o_ptr->to_d;

	}

	/* Monks get extra ac for armour _not worn_ */
	if ((p_ptr->rp.pclass == CLASS_MONK) && (!p_ptr->state.monk_armour_stat))
	{
		if (!(p_ptr->equipment[EQUIP_BODY].k_idx))
		{
			p_ptr->to_a += (p_ptr->lev * 3) / 2;
			p_ptr->dis_to_a += (p_ptr->lev * 3) / 2;
		}
		if (!(p_ptr->equipment[EQUIP_OUTER].k_idx) && (p_ptr->lev > 15))
		{
			p_ptr->to_a += ((p_ptr->lev - 13) / 3);
			p_ptr->dis_to_a += ((p_ptr->lev - 13) / 3);
		}
		if (!(p_ptr->equipment[EQUIP_ARM].k_idx) && (p_ptr->lev > 10))
		{
			p_ptr->to_a += ((p_ptr->lev - 8) / 3);
			p_ptr->dis_to_a += ((p_ptr->lev - 8) / 3);
		}
		if (!(p_ptr->equipment[EQUIP_HEAD].k_idx) && (p_ptr->lev > 4))
		{
			p_ptr->to_a += (p_ptr->lev - 2) / 3;
			p_ptr->dis_to_a += (p_ptr->lev - 2) / 3;
		}
		if (!(p_ptr->equipment[EQUIP_HANDS].k_idx))
		{
			p_ptr->to_a += (p_ptr->lev / 2);
			p_ptr->dis_to_a += (p_ptr->lev / 2);
		}
		if (!(p_ptr->equipment[EQUIP_FEET].k_idx))
		{
			p_ptr->to_a += (p_ptr->lev / 3);
			p_ptr->dis_to_a += (p_ptr->lev / 3);
		}
	}

	/* Hack -- aura of fire also provides light */
	if (FLAG(p_ptr, TR_SH_FIRE)) SET_FLAG(p_ptr, TR_LITE);

	/* Golems also get an intrinsic AC bonus */
	if (p_ptr->rp.prace == RACE_GOLEM)
	{
		p_ptr->to_a += 20 + (p_ptr->lev / 5);
		p_ptr->dis_to_a += 20 + (p_ptr->lev / 5);
	}

	/* Temporary strength */
	if (query_timed(TIMED_STR))
	{
		p_ptr->stat[A_STR].add += MAX(2, p_ptr->lev / 10);
	}

	/* Temporary charisma */
	if (query_timed(TIMED_CHR))
	{
		p_ptr->stat[A_CHR].add += 10;
	}

	/* Calculate stats */
	for (i = 0; i < A_MAX; i++)
	{
		int top, use, ind;

		/* Extract the new "stat_use" value for the stat */
		top = modify_stat_value(p_ptr->stat[i].max, p_ptr->stat[i].add);

		/* Notice changes */
		if (p_ptr->stat[i].top != top)
		{
			/* Save the new value */
			p_ptr->stat[i].top = top;

			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_STATS);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}


		/* Extract the new "stat_use" value for the stat */
		use = modify_stat_value(p_ptr->stat[i].cur, p_ptr->stat[i].add);

		if ((i == A_CHR) && (p_ptr->muta3 & MUT3_ILL_NORM))
		{
			int floor = 8 + 2 * p_ptr->lev;

			if (floor <= 18)
				floor *= 10;
			else
				floor += 180-18;

			/* 10 to 18/90 charisma, guaranteed, based on level */
			if (use < floor)
			{
				use = floor;
			}
		}

		/* Notice changes */
		if (p_ptr->stat[i].use != use)
		{
			/* Save the new value */
			p_ptr->stat[i].use = use;

			/* Redisplay the stats later */
			p_ptr->redraw |= (PR_STATS);

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}

		/* Find index into various tables */
		if (use < 400)
			ind = use / 10 - 3;
		else
			ind = 37;

		/* Notice changes */
		if (p_ptr->stat[i].ind != ind)
		{
			/* Save the new index */
			p_ptr->stat[i].ind = ind;

			/* Change in CON affects Hitpoints */
			if (i == A_CON)
			{
				p_ptr->update |= (PU_HP);
			}

			/* Change in INT may affect Mana/Spells */
			else if (i == A_INT)
			{
				if (mp_ptr->spell_stat == A_INT)
				{
					p_ptr->update |= (PU_MANA | PU_SPELLS);
				}
			}

			/* Change in WIS may affect Mana/Spells */
			else if (i == A_WIS)
			{
				if (mp_ptr->spell_stat == A_WIS)
				{
					p_ptr->update |= (PU_MANA | PU_SPELLS);
				}
			}

			/* Window stuff */
			p_ptr->window |= (PW_PLAYER);
		}
	}


	/* Apply temporary "stun" */
	if (query_timed(TIMED_STUN) > 50)
	{
		p_ptr->to_h -= 20;
		p_ptr->dis_to_h -= 20;
		p_ptr->to_d -= 20;
		p_ptr->dis_to_d -= 20;
	}
	else if (query_timed(TIMED_STUN))
	{
		p_ptr->to_h -= 5;
		p_ptr->dis_to_h -= 5;
		p_ptr->to_d -= 5;
		p_ptr->dis_to_d -= 5;
	}

	/* Invulnerability */
	if (query_timed(TIMED_INVULN))
	{
		p_ptr->to_a += 100;
		p_ptr->dis_to_a += 100;
	}

	/* wraith_form */
	if (query_timed(TIMED_WRAITH_FORM))
	{
		p_ptr->to_a += 100;
		p_ptr->dis_to_a += 100;
		SET_FLAG(p_ptr, TR_REFLECT);
	}

	/* etherealness */
	if (query_timed(TIMED_ETHEREALNESS))
	{
		p_ptr->to_a += 20;
		p_ptr->dis_to_a += 20;
		SET_FLAG(p_ptr, TR_PASS_WALL);
	}

	/* Temporary blessing */
	if (query_timed(TIMED_BLESSED))
	{
		p_ptr->to_a += 5;
		p_ptr->dis_to_a += 5;
		p_ptr->to_h += 10;
		p_ptr->dis_to_h += 10;
		p_ptr->to_d += 3;
		p_ptr->dis_to_d += 3;
	}

	/* Temporary shield */
	if (query_timed(TIMED_SHIELD))
	{
		p_ptr->to_a += 50;
		p_ptr->dis_to_a += 50;
	}

	/* Temporary "Hero" */
	if (query_timed(TIMED_HERO))
	{
		p_ptr->to_h += 12;
		p_ptr->dis_to_h += 12;
		p_ptr->to_d += 4;
		p_ptr->dis_to_d += 4;
		SET_FLAG(p_ptr, TR_RES_FEAR);
	}

	/* Temporary "Beserk" */
	if (query_timed(TIMED_SHERO))
	{
		p_ptr->to_h += 24;
		p_ptr->dis_to_h += 24;
		p_ptr->to_a -= 10;
		p_ptr->dis_to_a -= 10;
		p_ptr->to_d += 6;
		p_ptr->dis_to_d += 6;
		SET_FLAG(p_ptr, TR_RES_FEAR);
	}

	/* Temporary "fast" */
	if (query_timed(TIMED_FAST))
	{
		p_ptr->pspeed += 10;
	}

	/* Temporary "extra fast" */
	if (query_timed(TIMED_XTRA_FAST))
	{
		p_ptr->pspeed += 5;
	}

	/* Temporary stat sustain */
	if (query_timed(TIMED_SUST_ALL))
	{
		SET_FLAG(p_ptr, TR_SUST_STR);
		SET_FLAG(p_ptr, TR_SUST_INT);
		SET_FLAG(p_ptr, TR_SUST_WIS);
		SET_FLAG(p_ptr, TR_SUST_DEX);
		SET_FLAG(p_ptr, TR_SUST_CON);
		SET_FLAG(p_ptr, TR_SUST_CHR);
	}

	if (query_timed(TIMED_OPPOSE_BLIND))
	{
		SET_FLAG(p_ptr, TR_RES_BLIND);
	}

	if (query_timed(TIMED_OPPOSE_CONF))
	{
		SET_FLAG(p_ptr, TR_RES_CONF);
	}


	/* Temporary "slow" */
	if (query_timed(TIMED_SLOW))
	{
		p_ptr->pspeed -= 7;
	}

	/* Temporary "telepathy" */
	if (query_timed(TIMED_ESP))
	{
		SET_FLAG(p_ptr, TR_TELEPATHY);
	}

	/* Temporary see invisible */
	if (query_timed(TIMED_SEE_INVIS))
	{
		SET_FLAG(p_ptr, TR_SEE_INVIS);
	}

	/* Temporary infravision boost */
	if (query_timed(TIMED_INFRA))
	{
		p_ptr->see_infra += 4;
	}

	/* Temporary immunity */
	if (query_timed(TIMED_IMMUNE_ACID))
	{
		SET_FLAG(p_ptr, TR_IM_ACID);
	}
	if (query_timed(TIMED_IMMUNE_ELEC))
	{
		SET_FLAG(p_ptr, TR_IM_ELEC);
	}
	if (query_timed(TIMED_IMMUNE_FIRE))
	{
		SET_FLAG(p_ptr, TR_IM_FIRE);
	}
	if (query_timed(TIMED_IMMUNE_COLD))
	{
		SET_FLAG(p_ptr, TR_IM_COLD);
	}

	/* Temporary auras */
	if (query_timed(TIMED_SH_ACID))
	{
		SET_FLAG(p_ptr, TR_SH_ACID);
	}
	if (query_timed(TIMED_SH_ELEC))
	{
		SET_FLAG(p_ptr, TR_SH_ELEC);
	}
	if (query_timed(TIMED_SH_FIRE))
	{
		SET_FLAG(p_ptr, TR_SH_FIRE);
	}
	if (query_timed(TIMED_SH_COLD))
	{
		SET_FLAG(p_ptr, TR_SH_COLD);
	}


	if (query_timed(TIMED_SH_FEAR))
	{
		SET_FLAG(p_ptr, TR_SH_FEAR);
	}

	/* Temporary invisibility */
	if (query_timed(TIMED_INVIS))
	{
		p_ptr->skills[SKILL_STL] += 4;
	}
	if (query_timed(TIMED_XTRA_INVIS))
	{
		p_ptr->skills[SKILL_STL] += 3;
	}


	/* Hack -- Telepathy Change */
	if (FLAG(p_ptr, TR_TELEPATHY) != old_telepathy)
	{
		p_ptr->update |= (PU_MONSTERS);
	}

	/* Hack -- See Invis Change */
	if (((FLAG(p_ptr, TR_SEE_INVIS)) ? TRUE : FALSE) != old_see_inv)
	{
		p_ptr->update |= (PU_MONSTERS);
	}


	/* Extract the current weight (in tenth pounds) */
	j = p_ptr->total_weight;

	/* Extract the "weight limit" (in tenth pounds) */
	i = weight_limit();

	/* XXX XXX XXX Apply "encumbrance" from weight */
	if (j > i / 2) p_ptr->pspeed -= ((j - (i / 2)) / (i / 10));

	/* Bloating slows the player down (a little) */
	if (p_ptr->food >= PY_FOOD_MAX) p_ptr->pspeed -= 10;

	/* Searching slows the player down */
	if (p_ptr->state.searching) p_ptr->pspeed -= 10;

	/* Display the speed (if needed) */
	if (p_ptr->pspeed != old_speed) p_ptr->redraw |= (PR_SPEED);


	/* Actual Modifier Bonuses (Un-inflate stat bonuses) */
	p_ptr->to_a += ((int)(adj_dex_ta[p_ptr->stat[A_DEX].ind]) - 128);
	p_ptr->to_d += ((int)(adj_str_td[p_ptr->stat[A_STR].ind]) - 128);
	p_ptr->to_h += ((int)(adj_dex_th[p_ptr->stat[A_DEX].ind]) - 128);

	/* Displayed Modifier Bonuses (Un-inflate stat bonuses) */
	p_ptr->dis_to_a += ((int)(adj_dex_ta[p_ptr->stat[A_DEX].ind]) - 128);
	p_ptr->dis_to_d += ((int)(adj_str_td[p_ptr->stat[A_STR].ind]) - 128);
	p_ptr->dis_to_h += ((int)(adj_dex_th[p_ptr->stat[A_DEX].ind]) - 128);


	/* Redraw armor (if needed) */
	if ((p_ptr->dis_ac != old_dis_ac) || (p_ptr->dis_to_a != old_dis_to_a))
	{
		/* Redraw */
		p_ptr->redraw |= (PR_ARMOR);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);
	}


	/* Obtain the "hold" value */
	hold = adj_str_hold[p_ptr->stat[A_STR].ind];


	/* Examine the "current bow" */
	o_ptr = &p_ptr->equipment[EQUIP_BOW];


	/* Assume not heavy */
	p_ptr->state.heavy_shoot = FALSE;

	/* It is hard to carry a heavy bow */
	if (hold < o_ptr->weight / 10)
	{
		/* Hard to wield a heavy bow */
		p_ptr->to_h += 2 * (hold - o_ptr->weight / 10);
		p_ptr->dis_to_h += 2 * (hold - o_ptr->weight / 10);

		/* Heavy Bow */
		p_ptr->state.heavy_shoot = TRUE;
	}


	/* Compute "extra shots" if needed */
	if (o_ptr->k_idx)
	{
		/* Analyze the launcher */
		switch (o_ptr->sval)
		{
			case SV_SLING:
			{
				p_ptr->ammo_tval = TV_SHOT;
				p_ptr->ammo_mult = 2;
				p_ptr->bow_energy = 50;
				break;
			}

			case SV_SHORT_BOW:
			{
				p_ptr->ammo_tval = TV_ARROW;
				p_ptr->ammo_mult = 2;
				p_ptr->bow_energy = 100;
				break;
			}
			case SV_LONG_BOW:
			{
				p_ptr->ammo_tval = TV_ARROW;

				if (p_ptr->stat[A_STR].use >= 160)
				{
					p_ptr->ammo_mult = 3;
				}
				else
				{
					/* weak players cannot use a longbow well */
					p_ptr->ammo_mult = 2;
				}

				p_ptr->bow_energy = 100;
				break;
			}

			case SV_LIGHT_XBOW:
			{
				p_ptr->ammo_tval = TV_BOLT;
				p_ptr->ammo_mult = 4;
				p_ptr->bow_energy = 120;
				break;
			}

			case SV_HEAVY_XBOW:
			{
				p_ptr->ammo_tval = TV_BOLT;

				p_ptr->ammo_mult = 5;
				if (p_ptr->stat[A_DEX].use >= 160)
				{
					p_ptr->bow_energy = 150;
				}
				else
				{
					/* players with low dex will take longer to load */
					p_ptr->bow_energy = 200;
				}
				break;
			}
      default:
      {
        if (o_ptr->sval < 10) {
          p_ptr->ammo_tval = TV_SHOT;
  			  p_ptr->ammo_mult = (o_ptr->sval%10);
					p_ptr->bow_energy = 50;
        } else
        if (o_ptr->sval < 20) {
          p_ptr->ammo_tval = TV_ARROW;
          if (p_ptr->stat[A_STR].use >= o_ptr->weight*3/2) {
    			  p_ptr->ammo_mult = (o_ptr->sval%10)+2;
          } else {
    			  p_ptr->ammo_mult = (o_ptr->sval%10)+1;
          }
					p_ptr->bow_energy = 100;
        } else
        if (o_ptr->sval < 30) {
          p_ptr->ammo_tval = TV_BOLT;
  			  p_ptr->ammo_mult = (o_ptr->sval%10)+2;
          if (p_ptr->stat[A_DEX].use >= o_ptr->weight) {
					  p_ptr->bow_energy = 200 -(20*(p_ptr->stat[A_DEX].use/o_ptr->weight));
          } else {
					  p_ptr->bow_energy = 160;
          }
        } else
        {
          p_ptr->ammo_tval = TV_BOLT;
  			  p_ptr->ammo_mult = (o_ptr->sval%10)+(o_ptr->sval/10);
					p_ptr->bow_energy = 160;
        }
        if (p_ptr->bow_energy < 50) p_ptr->bow_energy = 50;
        if (p_ptr->ammo_mult < 2) p_ptr->ammo_mult = 2;
      }
		}
    if ((o_ptr->dd ==0) && (o_ptr->ds > 0)) {
				p_ptr->ammo_mult = o_ptr->ds;
    }

		/* Apply special flags */
		if (o_ptr->k_idx && !p_ptr->state.heavy_shoot)
		{
			/* Extra shots */
			p_ptr->num_fire += extra_shots;

			/* Hack -- Rangers love Bows */
			if ((p_ptr->rp.pclass == CLASS_RANGER) &&
				(p_ptr->ammo_tval == TV_ARROW))
			{
				/* Extra shot at level 15 */
				if (p_ptr->lev >= 15) p_ptr->num_fire++;

				/* Extra shot at level 30 */
				if (p_ptr->lev >= 30) p_ptr->num_fire++;

				/* Extra shot at level 45 */
				if (p_ptr->lev >= 45) p_ptr->num_fire++;
			}

			/* Hack -- Rangers can use XBows as well */
			if ((p_ptr->rp.pclass == CLASS_RANGER) &&
				(p_ptr->ammo_tval == TV_BOLT))
			{
				/* Extra shot at level 30 */
				if (p_ptr->lev >= 30) p_ptr->num_fire++;
			}

			/* Hack -- Rogues love Slings */
			if ((p_ptr->rp.pclass == CLASS_ROGUE) && (p_ptr->ammo_tval == TV_SHOT))
			{
				/* Extra shot at level 20 */
				if (p_ptr->lev >= 20) p_ptr->num_fire++;

				/* Extra shot at level 40 */
				if (p_ptr->lev >= 40) p_ptr->num_fire++;
			}

			/*
			 * Addendum -- also "Reward" high level warriors,
			 * with _any_ missile weapon -- TY
			 */
			if (p_ptr->rp.pclass == CLASS_WARRIOR &&
				(p_ptr->ammo_tval <= TV_BOLT) && (p_ptr->ammo_tval >= TV_SHOT))
			{
				/* Extra shot at level 40 */
				if (p_ptr->lev >= 40) p_ptr->num_fire++;

			}
		}
	}
	/* Add all class and race-specific adjustments to missile Skill. -LM- */
	p_ptr->skills[SKILL_THB] += add_special_missile_skill(p_ptr->rp.pclass);

	/* Examine the "main weapon" */
	o_ptr = &p_ptr->equipment[EQUIP_WIELD];

	/* Add all other class-specific adjustments to melee Skill. -LM- */
	p_ptr->skills[SKILL_THN] += add_special_melee_skill(p_ptr->rp.pclass, o_ptr);

	/* Assume okay */
	p_ptr->state.icky_wield = FALSE;

	/* Extra bonus for warriors... */
	if (p_ptr->rp.pclass == CLASS_WARRIOR)
	{
		p_ptr->to_h += (p_ptr->lev / 5);
		p_ptr->to_d += (p_ptr->lev / 5);

		p_ptr->dis_to_h += (p_ptr->lev / 5);
		p_ptr->dis_to_d += (p_ptr->lev / 5);
	}

	/* Priest weapon penalty for non-blessed edged weapons */
	if ((p_ptr->rp.pclass == CLASS_PRIEST) &&
		(!(FLAG(p_ptr, TR_BLESSED))) &&
		((o_ptr->tval == TV_SWORD) || (o_ptr->tval == TV_POLEARM)))
	{
		/* Reduce the real bonuses */
		p_ptr->to_h -= (p_ptr->lev / 5);
		p_ptr->to_d -= (p_ptr->lev / 5);

		/* Reduce the mental bonuses */
		p_ptr->dis_to_h -= (p_ptr->lev / 5);
		p_ptr->dis_to_d -= (p_ptr->lev / 5);

		/* Icky weapon */
		p_ptr->state.icky_wield = TRUE;
	}

	/* Affect Skill -- stealth (bonus one) */
	p_ptr->skills[SKILL_STL] += 1;

	/* Affect Skill -- disarming (DEX and INT) */
	p_ptr->skills[SKILL_DIS] += adj_dex_dis[p_ptr->stat[A_DEX].ind];
	p_ptr->skills[SKILL_DIS] += adj_int_dis[p_ptr->stat[A_INT].ind];

	/* Affect Skill -- magic devices (INT) */
	p_ptr->skills[SKILL_DEV] += adj_int_dev[p_ptr->stat[A_INT].ind];

	/* Affect Skill -- saving throw (WIS) */
	p_ptr->skills[SKILL_SAV] += (adj_wis_sav[p_ptr->stat[A_WIS].ind] - 128);

	/* Affect Skill -- digging (STR) */
	p_ptr->skills[SKILL_DIG] += adj_str_dig[p_ptr->stat[A_STR].ind];

	/* Affect Skill -- disarming (Level, by Class) */
	p_ptr->skills[SKILL_DIS] += (cp_ptr->x_dis * p_ptr->lev / 10);

	/* Affect Skill -- magic devices (Level, by Class) */
	p_ptr->skills[SKILL_DEV] += (cp_ptr->x_dev * p_ptr->lev / 10);

	/* Affect Skill -- saving throw (Level, by Class) */
	p_ptr->skills[SKILL_SAV] += (cp_ptr->x_sav * p_ptr->lev / 10);

	/* Affect Skill -- stealth (Level, by Class) */
	p_ptr->skills[SKILL_STL] += (cp_ptr->x_stl * p_ptr->lev / 10);

	/* Affect Skill -- sensing ability (Level, by Class) */
	p_ptr->skills[SKILL_SNS] += (cp_ptr->x_sns * p_ptr->lev / 10);

	/* Affect Skill -- search frequency (Level, by Class) */
	p_ptr->skills[SKILL_FOS] += (cp_ptr->x_fos * p_ptr->lev / 10);

	/* Affect Skill -- combat (normal) (Level, by Class) */
	p_ptr->skills[SKILL_THN] += (cp_ptr->x_thn * p_ptr->lev / 50);

	/* Affect Skill -- combat (shooting) (Level, by Class) */
	p_ptr->skills[SKILL_THB] += (cp_ptr->x_thb * p_ptr->lev / 50);

	/* Affect Skill -- combat (throwing) (Level, by Class) */
	p_ptr->skills[SKILL_THT] += (cp_ptr->x_thb * p_ptr->lev / 50);

	/* Affect Skill -- apply stealth mode bunus */
	if (p_ptr->state.searching == SEARCH_MODE_STEALTH) {
		if (p_ptr->rp.pclass == CLASS_ROGUE) {
			/* Apply an additional bonus for rogues */
			p_ptr->skills[SKILL_STL] += (p_ptr->skills[SKILL_STL] / 7) + 1;
		} else
		if (p_ptr->rp.pclass == CLASS_RANGER) {
			/* Apply an additional bonus for rangers */
			p_ptr->skills[SKILL_STL] += (p_ptr->skills[SKILL_STL] / 10) + 1;
		} else 
		{
			p_ptr->skills[SKILL_STL] += (p_ptr->skills[SKILL_STL] / 13) + 1;
		}
	}

	/* Limit Skill -- digging from 1 up */
	if (p_ptr->skills[SKILL_DIG] < 1) p_ptr->skills[SKILL_DIG] = 1;

	/* Limit Skill -- saving throw from 1 up */
	if (p_ptr->skills[SKILL_SAV] < 1) p_ptr->skills[SKILL_SAV] = 1;

	/* Limit Skill -- stealth from 0 to 30 */
	if (p_ptr->skills[SKILL_STL] > 30) p_ptr->skills[SKILL_STL] = 30;
	if (p_ptr->skills[SKILL_STL] < 0) p_ptr->skills[SKILL_STL] = 0;

	/* Apply Skill -- Extract noise from stealth */
	p_ptr->noise = (1L << (30 - p_ptr->skills[SKILL_STL]));

	if ((FLAG(p_ptr, TR_NO_MAGIC)) && (p_ptr->skills[SKILL_SAV] < p_ptr->lev * 2 + 85))
		 p_ptr->skills[SKILL_SAV] = p_ptr->lev * 2 + 85;

	/* Assume not heavy */
	p_ptr->state.heavy_wield = FALSE;

	/* Are we using a weapon? */
	if (o_ptr->k_idx)
	{
		/* It is hard to hold a heavy weapon */
		if (hold < o_ptr->weight / 10)
		{
			/* Hard to wield a heavy weapon */
			p_ptr->to_h += 2 * (hold - o_ptr->weight / 10);
			p_ptr->dis_to_h += 2 * (hold - o_ptr->weight / 10);

			/* Heavy weapon */
			p_ptr->state.heavy_wield = TRUE;

			/* The player gets to swing a heavy weapon only once. -LM- */
			p_ptr->num_blow = 1;
		}

		/* Normal weapons */
		else
		{
			int str_index, dex_index;

			int effective_weight = 0, mul = 6;

			int skill;

			/* Enforce a minimum weight of three pounds. */
			effective_weight = (o_ptr->weight < 30 ? 30 : o_ptr->weight);

			/* Compare strength and weapon weight. */
			str_index = mul * adj_str_blow[p_ptr->stat[A_STR].ind] /
				effective_weight;

			/* Maximal value */
			if (str_index > 11) str_index = 11;

			/* Index by dexterity */
			dex_index = (adj_dex_blow[p_ptr->stat[A_DEX].ind]);

			/* Maximal value */
			if (dex_index > 11) dex_index = 11;


			/* Use the blows table */
			p_ptr->num_blow = blows_table[str_index][dex_index];

			/* Get weapon skill (not including magical enhancments) */
			skill = p_ptr->skills[SKILL_THN];

			/* Require high skill to get large number of blows */
			if (p_ptr->num_blow > 1 + skill / 15)
				p_ptr->num_blow = 1 + skill / 15;

			/* Paranoia - require at least one blow */
			if (p_ptr->num_blow < 1) p_ptr->num_blow = 1;

			/* Add in extra blows */
			p_ptr->num_blow += extra_blows;


			/* Boost digging skill by weapon weight */
			p_ptr->skills[SKILL_DIG] += (o_ptr->weight / 10);
		}
	}

	/* No weapon */
	else
	{
		/* Different calculation for monks with empty hands */
		if (p_ptr->rp.pclass == CLASS_MONK)
		{
			p_ptr->num_blow = 2;

			if (p_ptr->lev > 9) p_ptr->num_blow++;
			if (p_ptr->lev > 14) p_ptr->num_blow++;
			if (p_ptr->lev > 24) p_ptr->num_blow++;
			if (p_ptr->lev > 34) p_ptr->num_blow++;
			if (p_ptr->lev > 44) p_ptr->num_blow++;
			if (p_ptr->lev > 49) p_ptr->num_blow++;

			if (p_ptr->state.monk_armour_stat)
			{
				p_ptr->num_blow /= 2;
			}
			else
			{
				p_ptr->to_h += (p_ptr->lev / 3);
				p_ptr->to_d += (p_ptr->lev / 3);

				p_ptr->dis_to_h += (p_ptr->lev / 3);
				p_ptr->dis_to_d += (p_ptr->lev / 3);
			}

			p_ptr->num_blow += extra_blows;
		}
		else
		{
			/* Everyone gets two blows if not wielding a weapon. -LM- */
			p_ptr->num_blow = 2;
		}
	}

	/* Hack -- handle "xtra" mode */
	if (character_xtra) return;

	/* Take note when "heavy bow" changes */
	if (old_heavy_shoot != p_ptr->state.heavy_shoot)
	{
		/* Message */
		if (p_ptr->state.heavy_shoot)
		{
			msgf("You have trouble wielding such a heavy bow.");
		}
		else if (p_ptr->equipment[EQUIP_BOW].k_idx)
		{
			msgf("You have no trouble wielding your bow.");
		}
		else
		{
			msgf("You feel relieved to put down your heavy bow.");
		}
	}


	/* Take note when "heavy weapon" changes */
	if (old_heavy_wield != p_ptr->state.heavy_wield)
	{
		/* Message */
		if (p_ptr->state.heavy_wield)
		{
			msgf("You have trouble wielding such a heavy weapon.");
		}
		else if (p_ptr->equipment[EQUIP_WIELD].k_idx)
		{
			msgf("You have no trouble wielding your weapon.");
		}
		else
		{
			msgf("You feel relieved to put down your heavy weapon.");
		}
	}


	/* Take note when "illegal weapon" changes */
	if (old_icky_wield != p_ptr->state.icky_wield)
	{
		/* Message */
		if (p_ptr->state.icky_wield)
		{
			msgf("You do not feel comfortable with your weapon.");
		}
		else if (p_ptr->equipment[EQUIP_WIELD].k_idx)
		{
			msgf("You feel comfortable with your weapon.");
		}
		else
		{
			msgf("You feel more comfortable after removing your weapon.");
		}
	}

	if (p_ptr->rp.pclass == CLASS_MONK &&
		(p_ptr->state.monk_armour_stat != old_monk_armour))
	{
		if (p_ptr->state.monk_armour_stat)
		{
			msgf("The weight of your armor disrupts your balance.");
		}
		else
			msgf("You regain your balance.");
	}

	p_ptr->align = friend_align;
}



/*
 * Handle "p_ptr->notice"
 */
void notice_stuff(void)
{
	/* Notice stuff */
	if (!p_ptr->notice) return;


	/* Combine the pack */
	if (p_ptr->notice & (PN_COMBINE))
	{
		p_ptr->notice &= ~(PN_COMBINE);
		combine_pack();
	}

	/* Reorder the pack */
	if (p_ptr->notice & (PN_REORDER))
	{
		p_ptr->notice &= ~(PN_REORDER);
		reorder_pack();
	}
}


/*
 * Handle "p_ptr->update"
 */
void update_stuff(void)
{
	/* Update stuff */
	if (!p_ptr->update) return;

	if (p_ptr->update & (PU_WEIGHT))
	{
		p_ptr->update &= ~(PU_WEIGHT);
		calc_weight();
	}

	if (p_ptr->update & (PU_BONUS))
	{
		p_ptr->update &= ~(PU_BONUS);
		calc_bonuses();
	}

	if (p_ptr->update & (PU_TORCH))
	{
		p_ptr->update &= ~(PU_TORCH);
		calc_torch();
	}

	if (p_ptr->update & (PU_HP))
	{
		p_ptr->update &= ~(PU_HP);
		calc_hitpoints();
	}

	if (p_ptr->update & (PU_MANA))
	{
		p_ptr->update &= ~(PU_MANA);
		calc_mana();
	}

	if (p_ptr->update & (PU_SPELLS))
	{
		p_ptr->update &= ~(PU_SPELLS);
		calc_spells();
	}

	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;

	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;

	/* Do not update map, it doesn't exist */
	if (!character_dungeon) return;

	if (p_ptr->update & (PU_MAP))
	{
		p_ptr->update &= ~(PU_MAP);
		map_panel_size();
	}

	if ((p_ptr->update & (PU_MON_LITE)) && monster_light)
	{
		p_ptr->update &= ~(PU_MON_LITE);
		update_mon_lite();

		/*
		 * Hack - the odds are that since monsters moved,
		 * we need to redraw the map.
		 */
		p_ptr->redraw |= (PR_MAP);
		p_ptr->update |= (PU_VIEW);
	}

	if (p_ptr->update & (PU_VIEW))
	{
		p_ptr->update &= ~(PU_VIEW);
		update_view();
	}

	if (p_ptr->update & (PU_FLOW))
	{
		p_ptr->update &= ~(PU_FLOW);
		update_flow();
	}

	if (p_ptr->update & (PU_DISTANCE))
	{
		p_ptr->update &= ~(PU_DISTANCE);
		p_ptr->update &= ~(PU_MONSTERS);
		p_ptr->update &= ~(PU_OBJECTS);
		update_monsters(TRUE);
	}

	if (p_ptr->update & (PU_MONSTERS))
	{
		p_ptr->update &= ~(PU_MONSTERS);
		p_ptr->update &= ~(PU_OBJECTS);
		update_monsters(FALSE);
	}

	if (p_ptr->update & (PU_OBJECTS)) {
		p_ptr->update &= ~(PU_OBJECTS);
		update_monsters(FALSE);
	}
}


/*
 * Handle "p_ptr->redraw"
 */
void redraw_stuff(void)
{
	int i;

	/* Redraw stuff */
	if (!p_ptr->redraw) return;


	/* Character is not ready yet, no screen updates */
	if (!character_generated) return;


	/* Character is in "icky" mode, no screen updates */
	if (character_icky) return;

	/* Hack -- clear the screen */
	if (p_ptr->redraw & (PR_WIPE))
	{
		p_ptr->redraw &= ~(PR_WIPE);
		message_flush();
		Term_clear();
	}


	if (p_ptr->redraw & (PR_BASIC))
	{
		p_ptr->redraw &= ~(PR_BASIC);
		p_ptr->redraw &= ~(PR_MISC | PR_TITLE | PR_STATS | PR_STATUS);
		p_ptr->redraw &= ~(PR_LEV | PR_EXP | PR_GOLD);
		p_ptr->redraw &= ~(PR_ARMOR | PR_HP | PR_MANA);
		p_ptr->redraw &= ~(PR_DEPTH | PR_HEALTH);
		prt_frame_basic();
	}

	if (p_ptr->redraw & (PR_EQUIPPY))
	{
		p_ptr->redraw &= ~(PR_EQUIPPY);
		print_equippy();		/* To draw / delete equippy chars */
	}

	if (p_ptr->redraw & (PR_MISC))
	{
		p_ptr->redraw &= ~(PR_MISC);
		prt_field(rp_ptr->title, COL_RACE, ROW_RACE);
		prt_field(cp_ptr->title, COL_CLASS, ROW_CLASS);
	}

	if (p_ptr->redraw & (PR_TITLE))
	{
		p_ptr->redraw &= ~(PR_TITLE);
		prt_title();
	}

	if (p_ptr->redraw & (PR_LEV))
	{
		p_ptr->redraw &= ~(PR_LEV);
		prt_level();
	}

	if (p_ptr->redraw & (PR_EXP))
	{
		p_ptr->redraw &= ~(PR_EXP);
		prt_exp();
	}

	if (p_ptr->redraw & (PR_STATS))
	{
		p_ptr->redraw &= ~(PR_STATS);

		/* All Stats */
		for (i = 0; i < A_MAX; i++) prt_stat(i);
	}

	if (p_ptr->redraw & (PR_STATUS))
	{
		p_ptr->redraw &= ~(PR_STATUS);
		prt_status();
	}

	if (p_ptr->redraw & (PR_ARMOR))
	{
		p_ptr->redraw &= ~(PR_ARMOR);
		prt_ac();
	}

	if (p_ptr->redraw & (PR_HP))
	{
		p_ptr->redraw &= ~(PR_HP);
		prt_hp();
	}

	if (p_ptr->redraw & (PR_MANA))
	{
		p_ptr->redraw &= ~(PR_MANA);
		prt_sp();
	}

	if (p_ptr->redraw & (PR_GOLD))
	{
		p_ptr->redraw &= ~(PR_GOLD);
		prt_gold();
	}

	if (p_ptr->redraw & (PR_DEPTH))
	{
		p_ptr->redraw &= ~(PR_DEPTH);
		prt_depth();
	}

	if (p_ptr->redraw & (PR_HEALTH))
	{
		p_ptr->redraw &= ~(PR_HEALTH);
		health_redraw();
	}


	if (p_ptr->redraw & (PR_EXTRA))
	{
		p_ptr->redraw &= ~(PR_EXTRA);
		p_ptr->redraw &= ~(PR_CUT | PR_STUN);
		p_ptr->redraw &= ~(PR_HUNGER);
		p_ptr->redraw &= ~(PR_BLIND | PR_CONFUSED);
		p_ptr->redraw &= ~(PR_AFRAID | PR_POISONED);
		p_ptr->redraw &= ~(PR_STATE | PR_SPEED | PR_STUDY);
		prt_frame_extra();
	}

	if (p_ptr->redraw & (PR_CUT))
	{
		p_ptr->redraw &= ~(PR_CUT);
		prt_cut();
	}

	if (p_ptr->redraw & (PR_STUN))
	{
		p_ptr->redraw &= ~(PR_STUN);
		prt_stun();
	}

	if (p_ptr->redraw & (PR_HUNGER))
	{
		p_ptr->redraw &= ~(PR_HUNGER);
		prt_hunger();
	}

	if (p_ptr->redraw & (PR_BLIND|PR_DETECT))
	{
		p_ptr->redraw &= ~(PR_BLIND|PR_DETECT);
		prt_blind();
	}

	if (p_ptr->redraw & (PR_CONFUSED))
	{
		p_ptr->redraw &= ~(PR_CONFUSED);
		prt_confused();
	}

	if (p_ptr->redraw & (PR_AFRAID))
	{
		p_ptr->redraw &= ~(PR_AFRAID);
		prt_afraid();
	}

	if (p_ptr->redraw & (PR_POISONED))
	{
		p_ptr->redraw &= ~(PR_POISONED);
		prt_poisoned();
	}

	if (p_ptr->redraw & (PR_STATE))
	{
		p_ptr->redraw &= ~(PR_STATE);
		prt_state();
	}

	if (p_ptr->redraw & (PR_SPEED))
	{
		p_ptr->redraw &= ~(PR_SPEED);
		prt_speed();
	}

	if (p_ptr->redraw & (PR_STUDY))
	{
		p_ptr->redraw &= ~(PR_STUDY);
		prt_study();
	}

	if (p_ptr->redraw & (PR_TIME))
	{
		p_ptr->redraw &= ~(PR_TIME);
		prt_time();
	}

	if (p_ptr->redraw & (PR_BUTTONS))
	{
		p_ptr->redraw &= ~(PR_BUTTONS);
		(void)button_print(0,0);
	}

	/* Do not update map it, doesn't exist */
	if (!character_dungeon) return;

	if (p_ptr->redraw & (PR_MAP))
	{
		p_ptr->redraw &= ~(PR_MAP);
		prt_map();
	}
}


/*
 * Handle "p_ptr->window"
 */
void window_stuff(void)
{
	int j;

	u32b mask = 0L;

	/* Nothing to do */
	if (!p_ptr->window) return;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		/* Save usable flags */
		if (angband_term[j]) mask |= window_flag[j];
	}

	/* Display inventory */
	if (p_ptr->window & (PW_INVEN))
	{
		p_ptr->window &= ~(PW_INVEN);
		fix_inven();
	}

	/* Display equipment */
	if (p_ptr->window & (PW_EQUIP))
	{
		p_ptr->window &= ~(PW_EQUIP);
		fix_equip();
	}

	/* Apply usable flags */
	p_ptr->window &= mask;

	/* Nothing to do */
	if (!p_ptr->window) return;

	/* Display spell list */
	if (p_ptr->window & (PW_SPELL))
	{
		p_ptr->window &= ~(PW_SPELL);
		fix_spell();
	}

	/* Display player */
	if (p_ptr->window & (PW_PLAYER))
	{
		p_ptr->window &= ~(PW_PLAYER);
		fix_player();
	}

	/* Display overhead view */
	if (p_ptr->window & (PW_MESSAGE))
	{
		p_ptr->window &= ~(PW_MESSAGE);
		fix_message();
	}

	/* Display object recall */
	if (p_ptr->window & (PW_OBJECT))
	{
		p_ptr->window &= ~(PW_OBJECT);
		fix_object();
	}

	/* Display monster recall */
	if (p_ptr->window & (PW_MONSTER))
	{
		p_ptr->window &= ~(PW_MONSTER);
		fix_monster();
	}

	/* Do not update map it, doesn't exist */
	if (!character_dungeon) return;

	/* Display monster list */
	if (p_ptr->window & (PW_VISIBLE))
	{
		p_ptr->window &= ~(PW_VISIBLE);
		fix_visible();
	}
	/* Display overhead view */
	if (p_ptr->window & (PW_OVERHEAD))
	{
		p_ptr->window &= ~(PW_OVERHEAD);
		fix_overhead();
	}

	/* Display overhead view */
	if (p_ptr->window & (PW_DUNGEON))
	{
		p_ptr->window &= ~(PW_DUNGEON);
		fix_dungeon();
	}
}


/*
 * Handle "p_ptr->update" and "p_ptr->redraw" and "p_ptr->window"
 */
void handle_stuff(void)
{
	/* Update stuff */
	if (p_ptr->update) update_stuff();

	/* Redraw stuff */
	if (p_ptr->redraw) redraw_stuff();

	/* Window stuff */
	if (p_ptr->window) window_stuff();
}

/*
 * Handle "p_ptr->change"
 */
void change_stuff(void)
{
	/* Nothing to do */
	if (!p_ptr->change) return;

	/* Redraw screen after a wiz_lite() */
	if (p_ptr->change & (PC_WIZ_LITE))
	{
		p_ptr->change &= ~(PC_WIZ_LITE);

		change_wiz_lite();
	}

	/* Shimmer monsters */
	if (p_ptr->change & (PC_SHIMMER))
	{
		p_ptr->change &= ~(PC_SHIMMER);

		change_shimmer();
	}

	/* Repair monsters */
	if (p_ptr->change & (PC_REPAIR))
	{
		p_ptr->change &= ~(PC_REPAIR);

		change_repair();
	}

	/* Give beastman mutation at birth */
	if (p_ptr->change & (PC_MUTATE))
	{
		p_ptr->change &= ~(PC_MUTATE);

		msgf("You feel different!");
		(void)gain_mutation(0);
	}
}

/*
 * Roll a saving throw for the player
 */
bool player_save(int power)
{
	return saving_throw(p_ptr->skills[SKILL_SAV] - power);
}
