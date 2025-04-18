/* File: xtra2.c */

/* Purpose: effects of various "objects" */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"
#include "button.h"


/*
 * Advance experience levels and print experience
 */
void check_experience(void)
{
	bool level_reward = FALSE;
	bool multi_rew = FALSE;
	bool level_mutation = FALSE;

	/* Hack -- lower limit */
	if (p_ptr->exp < 0) p_ptr->exp = 0;

	/* Hack -- lower limit */
	if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;

	/* Hack -- upper limit */
	if (p_ptr->exp > PY_MAX_EXP) p_ptr->exp = PY_MAX_EXP;

	/* Hack -- upper limit */
	if (p_ptr->max_exp > PY_MAX_EXP) p_ptr->max_exp = PY_MAX_EXP;

	/* Hack -- maintain "max" experience */
	if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

	/* Redraw experience */
	p_ptr->redraw |= (PR_EXP);

	/* Handle stuff */
	handle_stuff();

	/* Lose levels while possible */
	while ((p_ptr->lev > 1) &&
		   (p_ptr->exp < (player_exp[p_ptr->lev - 2] * p_ptr->expfact / 100L)))
	{
		/* Lose a level */
		p_ptr->lev--;
		lite_spot(p_ptr->px, p_ptr->py);

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE | PR_EXP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Handle stuff */
		handle_stuff();
	}


	/* Gain levels while possible */
	while ((p_ptr->lev < PY_MAX_LEVEL) &&
		   (p_ptr->exp >= (player_exp[p_ptr->lev - 1] * p_ptr->expfact / 100L)))
	{
		/* Gain a level */
		p_ptr->lev++;
		lite_spot(p_ptr->px, p_ptr->py);

		/*
		 * If auto-note taking enabled, write a note to the file.
		 * Only write this note when the level is gained for the first time.
		 */
		if (take_notes && auto_notes && (p_ptr->lev > p_ptr->max_lev))
		{
			/* Write note */
			add_note('L', "Reached level %d", p_ptr->lev);
		}

		/* Save the highest level */
		if (p_ptr->lev > p_ptr->max_lev)
		{
			int vir;
			for (vir = 0; vir < MAX_PLAYER_VIRTUES; vir++)
				p_ptr->virtues[vir] = p_ptr->virtues[vir] + 1;

			if (FLAG(p_ptr, TR_MUTATE))
			{
				if (one_in_(5)) level_mutation = TRUE;
			}

			p_ptr->max_lev = p_ptr->lev;

			if ((FLAG(p_ptr, TR_PATRON)) ||
				(one_in_(7) && (FLAG(p_ptr, TR_STRANGE_LUCK))))
			{
				level_reward = TRUE;
			}
		}

		/* Sound */
		sound(SOUND_LEVEL);

		/* Message */
		msgf(MSGT_LEVEL, "Welcome to level %d.", p_ptr->lev);
		msg_effect(MSG_LEVEL, p_ptr->lev);

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE | PR_EXP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER | PW_SPELL);

		/* Handle stuff */
		handle_stuff();

		if (level_reward)
		{
			if (!multi_rew)
			{
				gain_level_reward(0);
				multi_rew = TRUE;
				level_reward = FALSE;
			}
		}

		if (level_mutation)
		{
			msgf("You feel different...");
			(void)gain_mutation(0);
			level_mutation = FALSE;
		}
	}
}


/*
 * Hack -- Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 *
 * XXX XXX XXX Note the use of actual "monster names"
 */
static int get_coin_type(const monster_race *r_ptr)
{
	cptr name = mon_race_name(r_ptr);

	/* Analyze "coin" monsters */
	if (r_ptr->d_char == '$')
	{
		/* Look for textual clues */
		if (strstr(name, " copper ")) return (2);
		if (strstr(name, " silver ")) return (5);
		if (strstr(name, " gold ")) return (10);
		if (strstr(name, " mithril ")) return (16);
		if (strstr(name, " adamantite ")) return (17);

		/* Look for textual clues */
		if (strstr(name, "Copper ")) return (2);
		if (strstr(name, "Silver ")) return (5);
		if (strstr(name, "Gold ")) return (10);
		if (strstr(name, "Mithril ")) return (16);
		if (strstr(name, "Adamantite ")) return (17);
	}

	/* Assume nothing */
	return (0);
}


/*
 * Handle the "death" of a monster.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that monsters can now carry objects, and when a monster dies,
 * it drops all of its objects, which may disappear in crowded rooms.
 *
 * Hack - monsters only explode if explode is TRUE.
 */
bool monster_death(int m_idx, bool explode)
{
	int i, j, y, x;

	int dump_item = 0;
	int dump_gold = 0;

	int number = 0;

	monster_type *m_ptr = &m_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool visible = (m_ptr->ml || (FLAG(r_ptr, RF_UNIQUE)));

	bool good = FLAG(r_ptr, RF_DROP_GOOD);
	bool great = FLAG(r_ptr, RF_DROP_GREAT);

	bool do_gold = (!(FLAG(r_ptr, RF_ONLY_ITEM)));
	bool do_item = (!(FLAG(r_ptr, RF_ONLY_GOLD)));
	bool cloned = FALSE;
	bool dropped_corpse = FALSE;
	int force_coin = get_coin_type(r_ptr);

	object_type *q_ptr;
	field_type *f_ptr;

	int level;

	/* Notice changes in view */
	if (FLAG(r_ptr, RF_LITE_1) || FLAG(r_ptr, RF_LITE_2))
	{
		/* Update some things */
		p_ptr->update |= (PU_MON_LITE);
	}

	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;

	/* Prepare for object memory */
	if (m_ptr->r_idx >= HERO_MIN)
	{
		current_object_source.type = OM_HERO;
		current_object_source.place_num = p_ptr->place_num;
		current_object_source.depth = p_ptr->depth;
		current_object_source.data = (u32b)m_ptr->r_idx-HERO_MIN;
	}
	else
	{
		current_object_source.type = OM_MONST;
		current_object_source.place_num = p_ptr->place_num;
		current_object_source.depth = p_ptr->depth;
		current_object_source.data = (u32b)m_ptr->r_idx;
	}



	if (m_ptr->smart & SM_CLONED)
		cloned = TRUE;

	/* Let monsters explode! */
	for (i = 0; i < 4; i++)
	{
		if ((r_ptr->blow[i].method == RBM_EXPLODE) && explode)
		{
			u16b flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
			int typ = GF_MISSILE;
			int d_dice = r_ptr->blow[i].d_dice;
			int d_side = r_ptr->blow[i].d_side;
			int damage = damroll(d_dice, d_side);

			/* ToDo: Apply the correct effects */
			switch (r_ptr->blow[i].effect)
			{
				case RBE_HURT:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_POISON:
				{
					typ = GF_POIS;
					break;
				}
				case RBE_UN_BONUS:
				{
					typ = GF_DISENCHANT;
					break;
				}
				case RBE_UN_POWER:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_EAT_GOLD:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_EAT_ITEM:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_EAT_FOOD:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_EAT_LITE:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_ACID:
				{
					typ = GF_ACID;
					break;
				}
				case RBE_ELEC:
				{
					typ = GF_ELEC;
					break;
				}
				case RBE_FIRE:
				{
					typ = GF_FIRE;
					break;
				}
				case RBE_COLD:
				{
					typ = GF_COLD;
					break;
				}
				case RBE_BLIND:
				{
					/* Hack - for Yellow light */
					typ = GF_LITE;
					break;
				}
				case RBE_CONFUSE:
				{
					typ = GF_CONFUSION;
					break;
				}
				case RBE_TERRIFY:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_PARALYZE:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_LOSE_STR:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_LOSE_DEX:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_LOSE_CON:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_LOSE_INT:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_LOSE_WIS:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_LOSE_CHR:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_LOSE_ALL:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_SHATTER:
				{
					typ = GF_ROCKET;
					break;
				}
				case RBE_EXP_10:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_EXP_20:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_EXP_40:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_EXP_80:
				{
					typ = GF_MISSILE;
					break;
				}
				case RBE_DISEASE:
				{
					typ = GF_POIS;
					break;
				}
				case RBE_TIME:
				{
					typ = GF_TIME;
					break;
				}
				case RBE_EXP_VAMP:
				{
					typ = GF_MISSILE;
					break;
				}
			}

			(void)project(m_idx, 3, x, y, damage, typ, flg);
			break;
		}
	}

	/* Complete quests */
	if (FLAG(r_ptr, RF_UNIQUE))
	{
		trigger_quest_complete(QX_KILL_UNIQUE, (vptr)m_ptr);
	}
	else
	{
		trigger_quest_complete(QX_KILL_MONST, (vptr)m_ptr);
	}

	/* Hack XXX XXX - trigger on killing winner */
	if ((m_ptr->r_idx == QW_OBERON) || (m_ptr->r_idx == QW_SERPENT))
	{
		trigger_quest_complete(QX_KILL_WINNER, (vptr)m_ptr);
	}

	if (((m_ptr->r_idx == QW_SAURON) || (m_ptr->r_idx == QW_MORGOTH)) && !amber_monsters)
	{
		trigger_quest_complete(QX_KILL_WINNER, (vptr)m_ptr);
	}


	/* Hack: Do not drop a corpse in a random quest.  */
	if ((one_in_(FLAG(r_ptr, RF_UNIQUE) ? 1 : 2) &&
		 ((FLAG(r_ptr, RF_DROP_CORPSE)) ||
		  (FLAG(r_ptr, RF_DROP_SKELETON))))
		&& !(FLAG(r_ptr, RF_QUESTOR)))
	{
		/* Assume skeleton */
		bool corpse = FALSE;

		/*
		 * We cannot drop a skeleton? Note, if we are in this check,
		 * we *know* we can drop at least a corpse or a skeleton
		 */
		if (!(FLAG(r_ptr, RF_DROP_SKELETON)))
			corpse = TRUE;

		/* Else, a corpse is more likely unless we did a "lot" of damage */
		else if (FLAG(r_ptr, RF_DROP_CORPSE))
		{
			/* Lots of damage in one blow */
			if ((0 - ((m_ptr->maxhp) / 4)) > m_ptr->hp)
			{
				if (one_in_(3)) corpse = TRUE;
			}
			else
			{
				if (!one_in_(3)) corpse = TRUE;
			}
		}

		/* Hack - corpses only appear on certain floors */
		if (cave_clean_grid(area(x, y)))
		{
			if (corpse)
			{
				/* Make a corpse */
				f_ptr = place_field(x, y, FT_CORPSE);

				if (f_ptr)
				{
					/* Initialise it */
					(void)field_script_single(f_ptr, FIELD_ACT_INIT,
						"i:", LUA_VAR_NAMED(m_ptr->r_idx, "r_idx"));
				}
			}
			else
			{
				/* Make a skeleton */
				f_ptr = place_field(x, y, FT_SKELETON);

				if (f_ptr)
				{
					/* Initialise it */
					(void)field_script_single(f_ptr, FIELD_ACT_INIT,
						"i:", LUA_VAR_NAMED(m_ptr->r_idx, "r_idx"));
				}
			}

			/* We dropped a corpse */
			dropped_corpse = TRUE;
		}
	}

	/* Drop objects being carried */
	drop_object_list(&m_ptr->hold_o_idx, m_ptr->fx, m_ptr->fy);


	/*
	 * Mega^3-hack: killing a 'Warrior of the Dawn' is likely to
	 * spawn another in the fallen one's place!
	 */
	if (mon_name_cont(r_ptr, "the Dawn"))
	{
		if (!one_in_(20))
		{
			int wy = y, wx = x;
			int attempts = 100;

			do
			{
				scatter(&wx, &wy, x, y, 20);
			}
			while (!(in_bounds2(wx, wy) && cave_floor_grid(area(wx, wy)))
				   && --attempts);

			if (attempts > 0)
			{
				if (summon_specific(0, wx, wy, 100, SUMMON_DAWN,
									FALSE, is_friendly(m_ptr), FALSE))
				{
					if (player_can_see_bold(wx, wy))
						msgf("A new warrior steps forth!");
				}
			}
		}
	}

	/* Pink horrors are replaced with 2 Blue horrors */
	else if (mon_name_cont(r_ptr, "ink horror"))
	{
		bool notice = FALSE;

		for (i = 0; i < 2; i++)
		{
			int wy = y, wx = x;

			if (summon_specific(0, wx, wy, 100, SUMMON_BLUE_HORROR,
								FALSE, is_friendly(m_ptr), FALSE))
			{
				if (player_can_see_bold(wx, wy))
					notice = TRUE;
			}
		}

		if (notice)
			msgf("The Pink horror divides!");
	}

	/* One more ultra-hack: An Unmaker goes out with a big bang! */
	else if (mon_name_cont(r_ptr, "Unmaker") && explode)
	{
		u16b flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
		(void)project(m_idx, 6, x, y, 100, GF_CHAOS, flg);
	}

	/* Bloodletters of Khorne may drop a blade of chaos */
	else if (mon_name_cont(r_ptr, "Bloodletter") &&
			 (randint1(100) < 15))
	{
		/* Prepare to make a Blade of Chaos */
		q_ptr = object_prep(lookup_kind(TV_SWORD, SV_BLADE_OF_CHAOS));

		apply_magic(q_ptr, base_level(), 0, 0);

		/* Drop it in the dungeon */
		drop_near(q_ptr, -1, x, y);
	}

	/* Mega^2-hack -- Get a t-shirt from our first Greater Hell-beast kill */
	else if (!r_ptr->r_pkills
			 && mon_name_cont(r_ptr, "Greater hell-beast"))
	{
		/* Prepare to make the T-shirt */
		q_ptr = object_prep(lookup_kind(TV_SOFT_ARMOR, SV_T_SHIRT));

		/* Mega-Hack -- Name the shirt */
		q_ptr->xtra_name =
			quark_add
			("'I killed the GHB and this badass T-shirt magically dropped from nowhere!!");

		q_ptr->flags[2] |= (TR2_IGNORE_ACID | TR2_IGNORE_ELEC |
						  TR2_IGNORE_FIRE | TR2_IGNORE_COLD);

		/* Drop it in the dungeon */
		drop_near(q_ptr, -1, x, y);
	}

	/* Mega-Hack -- drop "winner" treasures */
	else if (FLAG(r_ptr, RF_DROP_CHOSEN))
	{
		if (mon_name_cont(r_ptr, "Serpent of Chaos") ||
		    (mon_name_cont(r_ptr, "Morgoth") && !amber_monsters))
		{
			/* Make Grond */
			place_named_art(ART_GROND, x, y);

			/* Make Crown of Morgoth */
			place_named_art(ART_MORGOTH, x, y);
		}
		else if (mon_name_cont(r_ptr, "Stormbringer"))
		{
			/* Create the artifact */
			place_named_art(ART_STORMBRINGER, x, y);

			/* The artifact has been created */
			a_info[ART_STORMBRINGER].cur_num = 1;
		}
		else
		{
			byte a_idx = 0;
			int chance = 0;

			if (mon_name_cont(r_ptr, "Oberon,"))
			{
				if (one_in_(3))
				{
					a_idx = ART_THRAIN;
					chance = 33;
				}
				else
				{
					a_idx = ART_GONDOR;
					chance = 50;
				}
			}
			else if (mon_name_cont(r_ptr, "Barimen"))
			{
				a_idx = ART_THRAIN;
				chance = 20;
			}
			else if (mon_name_cont(r_ptr, "Sauron,"))
			{
				a_idx = ART_POWER;
				chance = 25;
			}
			else if (mon_name_cont(r_ptr, "Brand, "))
			{
				if (one_in_(3))
				{
					a_idx = ART_ANGUIREL;
					chance = 33;
				}
				else
				{
					a_idx = ART_BRAND;
					chance = 25;
				}
			}
			else if (mon_name_cont(r_ptr, "Corwin,"))
			{
				if (one_in_(3))
				{
					a_idx = ART_CORWIN;
					chance = 33;
				}
				else
				{
					a_idx = ART_GRAYSWANDIR;
					chance = 33;
				}
			}
			else if (mon_name_cont(r_ptr, "Saruman of"))
			{
				a_idx = ART_ELENDIL;
				chance = 20;
			}
			else if (mon_name_cont(r_ptr, "Fiona the"))
			{
				a_idx = ART_BELANGIL;
				chance = 50;
			}
			else if (mon_name_cont(r_ptr, "Julian, "))
			{
				a_idx = ART_CELEBORN;
				chance = 45;
			}
			else if (mon_name_cont(r_ptr, "Klings"))
			{
				a_idx = ART_OROME;
				chance = 40;
			}
			else if (mon_name_cont(r_ptr, "Groo"))
			{
				a_idx = ART_GROO;
				chance = 75;
			}
			else if (mon_name_cont(r_ptr, "Hagen,"))
			{
				a_idx = ART_NIMLOTH;
				chance = 66;
			}
			else if (mon_name_cont(r_ptr, "Caine,"))
			{
				a_idx = ART_ANGRIST;
				chance = 50;
			}

			if ((a_idx > 0) && ((randint1(99) < chance) || (p_ptr->state.wizard)))
			{
				if (a_info[a_idx].cur_num == 0 && !(FLAG(&a_info[a_idx], TR_QUESTITEM)))
				{
					/* Create the artifact */
					place_named_art(a_idx, x, y);

					/* The artifact has been created */
					a_info[a_idx].cur_num = 1;
				}
			}
		}
	}

	/* Determine how much we can drop */
	if ((FLAG(r_ptr, RF_DROP_60)) && (randint0(100) < 60)) number++;
	if ((FLAG(r_ptr, RF_DROP_90)) && (randint0(100) < 90)) number++;
	if (FLAG(r_ptr, RF_DROP_1D2)) number += damroll(1, 2);
	if (FLAG(r_ptr, RF_DROP_2D2)) number += damroll(2, 2);
	if (FLAG(r_ptr, RF_DROP_3D2)) number += damroll(3, 2);
	if (FLAG(r_ptr, RF_DROP_4D2)) number += damroll(4, 2);

	if (cloned) number = 0;		/* Clones drop no stuff */

	/* Average dungeon and monster levels */
	if (p_ptr->depth)
		level = (p_ptr->depth + r_ptr->level) / 2;
	else
		level = r_ptr->level;

	/* Drop some objects */
	for (j = 0; j < number; j++)
	{
		/* Make Gold */
		if (do_gold && (!do_item || one_in_(2)))
		{
			/* Make some gold */
			q_ptr = make_gold(level, force_coin);

			/* XXX XXX XXX */
			dump_gold++;
		}

		/* Make Object */
		else
		{
            u16b delta_level = (good ? 15 : 0) + (great ? 15 : 0);

            for (i = 0; i < 1000; i++)
            {
                /* Make an object */
                q_ptr = make_object(level, delta_level, &r_ptr->obj_drop);

                if (!q_ptr) continue;

                /* "Good" and "great" drops must not be worthless */
                if (good || great)
                {
                    if (cursed_p(q_ptr)) continue;
                    if (object_value_real(q_ptr) <= 0) continue;
                }

                break;
            }

			/* Paranoia */
			if (!q_ptr) continue;

			/* XXX XXX XXX */
			dump_item++;
		}

		/* Drop it in the dungeon */
		drop_near(q_ptr, -1, x, y);
	}

	/* Take note of any dropped treasure */
	if (visible && (dump_item || dump_gold))
	{
		/* Take notes on treasure */
		lore_treasure(m_idx, dump_item, dump_gold);
	}

	/* Return TRUE if we dropped a corpse for the player to see */
	return (dropped_corpse);
}

/*
 * Modify the physical damage done to the monster.
 * (for example when it's invulnerable or shielded)
 *
 * ToDo: Accept a damage-type to calculate the modified damage from
 * things like fire, frost, lightning, poison, ... attacks.
 *
 * "type" is not yet used and should be 0.
 */
int mon_damage_mod(const monster_type *m_ptr, int dam, int type)
{
	/* Hack - ignore type for now */
	(void)type;

	if (m_ptr->invulner && !one_in_(PENETRATE_INVULNERABILITY))
		return (0);
	else
		return (dam);
}

/*
 * This function calculates the experience gained for killing a monster.
 *
 * Chaged -- monster experience grows quadratically, now the divider does too.
 * This should, I hope, lead to better balance and a bigger struggle gaining
 * levels in the midgame.
 */
void exp_for_kill(const monster_race *r_ptr, s32b *new_exp, s32b *new_exp_frac)
{
	s32b div, exp;
	int plev = p_ptr->lev;

	if (r_ptr->mexp)
	{
		div = 1+MAX((plev-1), (plev*plev)/13);

		exp = r_ptr->mexp;

		/* calculate the integer exp part */
		*new_exp = ((long)exp / div);

		/* Handle fractional experience */
		*new_exp_frac = ((long)(exp % div) * 0x10000L / div);
	}
	else
	{
		*new_exp = 0;
		*new_exp_frac = 0;
	}
}

/*
 * Decreases monsters hit points, handling monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Only "physical attacks" can induce the "You have slain" message.
 * Missile and Spell attacks will induce the "dies" message, or
 * various "specialized" messages.  Note that "You have destroyed"
 * and "is destroyed" are synonyms for "You have slain" and "dies".
 *
 * Hack -- unseen monsters yield "You have killed it." message.
 *
 * Added fear (DGK) and check whether to print fear messages -CWS
 *
 * Made name, sex, and capitalization generic -BEN-
 *
 * As always, the "ghost" processing is a total hack.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 */
bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	s32b new_exp, new_exp_frac;

	/* Innocent until proven otherwise */
	bool innocent = TRUE, thief = FALSE;
	bool corpse = FALSE;
	bool visible = FALSE;
	int i;

	/* Redraw (later) if needed */
	if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

	/* Wake it up */
	m_ptr->csleep = 0;

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now */
	if (m_ptr->hp < 0)
	{
		char m_name[80];

		/* Extract monster name */
		monster_desc(m_name, m_ptr, 0, 80);

		if (FLAG(r_ptr, RF_CAN_SPEAK))
		{
			char line_got[1024];

			if (speak_unique)
			{
				/* Dump a message */
				if (!get_rnd_line("mondeath.txt", m_ptr->r_idx, line_got))
					msgf("%^s says: %s", m_name, line_got);
			}
		}

		if (r_ptr->level > p_ptr->depth)
		{
			if (randint1(10) <= (p_ptr->depth - r_ptr->level))
				chg_virtue(V_VALOUR, 1);
		}
		if (r_ptr->level >= 2 * (p_ptr->lev))
			chg_virtue(V_VALOUR, 1);

		if ((FLAG(r_ptr, RF_UNIQUE)) && ((FLAG(r_ptr, RF_EVIL)) ||
											 (FLAG(r_ptr, RF_GOOD))))

			chg_virtue(V_HARMONY, 2);

		if ((FLAG(r_ptr, RF_UNIQUE)) && (FLAG(r_ptr, RF_GOOD)))
		{
			chg_virtue(V_UNLIFE, 2);
			chg_virtue(V_VITALITY, -2);
		}

		if ((FLAG(r_ptr, RF_UNIQUE)) && one_in_(3))
			chg_virtue(V_INDIVIDUALISM, -1);

		if (mon_name_cont(r_ptr, "beggar") ||
			mon_name_cont(r_ptr, "leper"))
		{
			chg_virtue(V_COMPASSION, -1);
		}

		if ((FLAG(r_ptr, RF_GOOD)) &&
			((r_ptr->level) / 10 + (3 * p_ptr->depth) >= randint1(100)))

			chg_virtue(V_UNLIFE, 1);

		/* "Good" angels */
		if ((r_ptr->d_char == 'A') && !(FLAG(r_ptr, RF_EVIL)))
		{
			if (FLAG(r_ptr, RF_UNIQUE))
				chg_virtue(V_FAITH, -2);
			else if ((r_ptr->level) / 10 + (3 * p_ptr->depth) >= randint1(100))
				chg_virtue(V_FAITH, -1);
		}

		/* "Evil" angel or a demon (what's the theological difference,
		   anyway...) */
		else if ((r_ptr->d_char == 'A') || (FLAG(r_ptr, RF_DEMON)))
		{
			if (FLAG(r_ptr, RF_UNIQUE))
				chg_virtue(V_FAITH, 2);
			else if ((r_ptr->level) / 10 + (3 * p_ptr->depth) >= randint1(100))
				chg_virtue(V_FAITH, 1);
		}

		if ((FLAG(r_ptr, RF_UNDEAD)) && (FLAG(r_ptr, RF_UNIQUE)))
			chg_virtue(V_VITALITY, 2);

		if (r_ptr->r_deaths)
		{
			if (FLAG(r_ptr, RF_UNIQUE))
			{
				chg_virtue(V_HONOUR, 10);
			}
			else if ((r_ptr->level) / 10 + (2 * p_ptr->depth) >= randint1(100))
			{
				chg_virtue(V_HONOUR, 1);
			}
		}

		for (i = 0; i < 4; i++)
		{
			if (r_ptr->blow[i].d_dice != 0) innocent = FALSE;	/* Murderer! */

			if ((r_ptr->blow[i].effect == RBE_EAT_ITEM) ||
				(r_ptr->blow[i].effect == RBE_EAT_GOLD))
				thief = TRUE;	/* Thief! */
		}

		/* The new law says it is illegal to live in the dungeon */
		if (r_ptr->level != 0) innocent = FALSE;

		if (thief)
		{
			if (FLAG(r_ptr, RF_UNIQUE))
				chg_virtue(V_JUSTICE, 3);
			else if (1 + (r_ptr->level / 10 + (2 * p_ptr->depth)) >=
					 randint1(100))
				chg_virtue(V_JUSTICE, 1);
		}
		else if (innocent)
		{
			chg_virtue(V_JUSTICE, -1);
		}

		if ((FLAG(r_ptr, RF_ANIMAL)) && !(FLAG(r_ptr, RF_EVIL)))
		{
			if (one_in_(3)) chg_virtue(V_NATURE, -1);
		}

		/* Make a sound */
		sound(SOUND_KILL);

		/* Death by Missile/Spell attack */
		if (note)
		{
			msgf(MSGT_KILL, "%^s%s", m_name, note);
			msg_effect(MSG_KILL, m_ptr->r_idx);
		}

		/* Death by physical attack -- invisible monster */
		else if (!m_ptr->ml)
		{
			msgf(MSGT_KILL, "You have killed %s.", m_name);
			msg_effect(MSG_KILL, m_ptr->r_idx);
		}

		/* Death by Physical attack -- non-living monster */
		else if (!monster_living(r_ptr))
		{
			msgf(MSGT_KILL, "You have destroyed %s.", m_name);
			msg_effect(MSG_KILL, m_ptr->r_idx);
		}

		/* Death by Physical attack -- living monster */
		else
		{
			msgf(MSGT_KILL, "You have slain %s.", m_name);
			msg_effect(MSG_KILL, m_ptr->r_idx);
		}

		/* Get how much the kill was worth */
		exp_for_kill(r_ptr, &new_exp, &new_exp_frac);

		/* Clones or summoned monsters can't be killed for XP. */
		if (m_ptr->smart & SM_CLONED)
		{
			new_exp = 0;
			new_exp_frac = 0;
		}

		/* Save visibility at death */
		visible = m_ptr->ml;

		/* Generate treasure */
		corpse = monster_death(m_idx, TRUE);

		/* Handle fractional experience */
		new_exp_frac += p_ptr->exp_frac;

		/* Keep track of experience */
		if (new_exp_frac >= 0x10000L)
		{
			new_exp++;
			p_ptr->exp_frac = (u16b)(new_exp_frac - 0x10000L);
		}
		else
		{
			p_ptr->exp_frac = (u16b)new_exp_frac;
		}

		/* Gain experience */
		gain_exp(new_exp);

		/* When the player kills a Unique, it stays dead */
		if (FLAG(r_ptr, RF_UNIQUE)) r_ptr->max_num = 0;

		/*
		 * If the player kills a Unique,
		 * and the notes options are on, write a note
		 */
		if ((FLAG(r_ptr, RF_UNIQUE)) && take_notes && auto_notes)
		{
			/* Get true name even if blinded/hallucinating and write note */
			add_note('U', "Killed %s", mon_race_name(r_ptr));
		}

		/* When the player kills a Nazgul, it stays dead */
		if (FLAG(r_ptr, RF_UNIQUE_7)) r_ptr->max_num--;

		/* Recall even invisible uniques or winners */
		if (visible || (FLAG(r_ptr, RF_UNIQUE)) || corpse)
		{
			/* Count kills this life */
			if (r_ptr->r_pkills < MAX_SHORT) r_ptr->r_pkills++;

			/* Count kills in all lives */
			if (r_ptr->r_tkills < MAX_SHORT) r_ptr->r_tkills++;

			/* Hack -- Auto-recall */
			monster_race_track(m_ptr->r_idx);
		}

		/* Don't kill Amberites */
		if ((FLAG(r_ptr, RF_AMBERITE)) && one_in_(2))
		{
			int curses = rand_range(2, 4);
			bool stop_ty = FALSE;
			int count = 0;

			msgf("%^s puts a terrible blood curse on you!", m_name);
			curse_equipment(100, 50);

			do
			{
				stop_ty = activate_ty_curse(stop_ty, &count);
			}
			while (--curses);
		}

		/* Delete the monster */
		delete_monster_idx(m_idx);

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}

	/* Mega-Hack -- Pain cancels fear */
	if (m_ptr->monfear && (dam > 0))
	{
		int tmp = randint1(dam/2);

		/* Cure a little fear */
		if (tmp < m_ptr->monfear)
		{
			/* Reduce fear */
			m_ptr->monfear -= tmp;
		}

		/* Cure all the fear */
		else
		{
			/* Cure fear */
			m_ptr->monfear = 0;

			/* No more fear */
			(*fear) = FALSE;
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!m_ptr->monfear && !FLAG(r_ptr, RF_NO_FEAR) && (dam > 0))
	{
		int percentage;

		/* Percentage of fully healthy */
		percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

		/*
		 * Run (sometimes) if at 10% or less of max hit points,
		 * or (usually) when hit for half its current hit points
		 */
		if ((randint1(7) >= percentage) ||
			((dam >= m_ptr->hp) && (randint0(100) < 80)))
		{
			/* Hack -- note fear */
			(*fear) = TRUE;

			/* XXX XXX XXX Hack -- Add some timed fear */
			m_ptr->monfear = (randint1(7) +
							  (((dam >= m_ptr->hp) && (percentage > 5)) ?
							   20 : ((11 - percentage) * 5)));
		}
	}

	/* Not dead yet */
	return (FALSE);
}


/*
 * Get size of map on screen
 */
void get_map_size(int *x, int *y)
{
	int wid, hgt;

	/* Get size */
	Term_get_size(&wid, &hgt);

  /* Offset (for status area) */
	hgt -= ROW_MAP + 1;
	wid -= COL_MAP + 1;

	/* Hack - bigmap has half resolution */
	if (tile_width_mult > 1) 	wid = wid / tile_width_mult;
	if (tile_height_mult > 1) 	hgt = hgt / tile_height_mult;

	/* Return values */
	*x = wid;
	*y = hgt;
}


/*
 * Get panel max bounds
 * where (x, y) is a provisional top left corner.
 */
static bool panel_bounds(int x, int y, int wid, int hgt)
{
	int xmax, ymax;

	/* Hack - vanilla town is special */
	if (vanilla_town && (!p_ptr->depth)
			&& !((tile_width_mult == 1) && (tile_height_mult == 1))) {
		/* Same bounds all the time */
		x = max_wild * WILD_BLOCK_SIZE / 2 - wid / 2 - 15;
		y = max_wild * WILD_BLOCK_SIZE / 2 - hgt / 2 - 5;
	} else {
		/* Bounds */
		if (y > p_ptr->max_hgt - hgt) y = p_ptr->max_hgt - hgt;
		if (y < p_ptr->min_hgt) y = p_ptr->min_hgt;
		if (x > p_ptr->max_wid - wid) x = p_ptr->max_wid - wid;
		if (x < p_ptr->min_wid) x = p_ptr->min_wid;
	}

	xmax = x + wid;
	ymax = y + hgt;

	/* Handle "changes" */
	if ((x != p_ptr->panel_x1) || (y != p_ptr->panel_y1) ||
		(xmax != p_ptr->panel_x2) || (ymax != p_ptr->panel_y2)) {
		/* Save the new panel info */
		p_ptr->panel_x1 = x;
		p_ptr->panel_y1 = y;
		p_ptr->panel_x2 = xmax;
		p_ptr->panel_y2 = ymax;

		/* Update stuff */
		p_ptr->update |= (PU_MONSTERS);

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Handle stuff */
		handle_stuff();

		return (TRUE);
	}

	return (FALSE);
}

/*
 * Handle a request to change the current panel
 *
 * Return TRUE if the panel was changed.
 *
 * Also used in do_cmd_locate
 */
bool change_panel(int dx, int dy)
{
	int x, y;
	int wid, hgt;

	get_map_size(&wid, &hgt);

	/* Apply the motion */
	x = p_ptr->panel_x1 + dx * (wid / 2);
	y = p_ptr->panel_y1 + dy * (hgt / 2);

	/* Get new bounds */
	return (panel_bounds(x, y, wid, hgt));
}


/*
 * Recenter the map around the player as required
 */
void verify_panel(void)
{
	int wid, hgt;

	int x, y;
	int nx, ny;

	get_map_size(&wid, &hgt);

	if (center_player && !(avoid_center && p_ptr->state.running))
	{
		/* Center it */
		x = p_ptr->px - wid / 2;
		y = p_ptr->py - hgt / 2;
	}
	else
	{
		/* Get new 'best value' */
		x = p_ptr->panel_x1;
		y = p_ptr->panel_y1;
		nx = p_ptr->px - wid / 2;
		ny = p_ptr->py - hgt / 2;

		/* How far to move panel? */
		if (abs(nx - x) > wid / 4) x = nx;
		if (abs(ny - y) > hgt / 4) y = ny;
	}

	/* Get new bounds */
	if (panel_bounds(x, y, wid, hgt))
	{
		/* Hack -- optional disturb on "panel change" */
		if (disturb_panel && !center_player) disturb(FALSE);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

		/* Handle stuff */
		handle_stuff();
	}
}

/*
 * Center the dungeon display around the given square
 */
bool panel_center(int x, int y)
{
	int wid, hgt;

	/* Get size */
	get_map_size(&wid, &hgt);

	x -= wid / 2;
	y -= hgt / 2;

	/* How far to move panel? */
	if (abs(p_ptr->panel_x1 - x) < wid / 4) x = p_ptr->panel_x1;
	if (abs(p_ptr->panel_y1 - y) < hgt / 4) y = p_ptr->panel_y1;

	/* Get new bounds */
	if (panel_bounds(x, y, wid, hgt))
	{
		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

		/* Handle stuff */
		handle_stuff();

		return (TRUE);
	}

	return (FALSE);
}

/*
 * Force the dungeon display to center around the player
 */
bool do_cmd_center_map(void)
{
	int wid, hgt;
	int x, y;

	get_map_size(&wid, &hgt);

	x = p_ptr->px - wid / 2;
	y = p_ptr->py - hgt / 2;
	/* Get new bounds */
	if (panel_bounds(x, y, wid, hgt)) {
		/* Hack -- optional disturb on "panel change" */
		if (disturb_panel && !center_player) disturb(FALSE);

		/* Window stuff */
		p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);

		/* Handle stuff */
		handle_stuff();
		return (TRUE);
	}

	return (FALSE);
}


static int map_wid_old = 66;


void map_panel_size(void)
{
	int wid, hgt;

	/* Only if the map exists */
	if (!character_dungeon) return;

	/* Hack - wait until are done with menus before updating */
	if (character_icky)
	{
		p_ptr->update |= PU_MAP;
		return;
	}

	/* Get size */
	get_map_size(&wid, &hgt);

	/* Kill previous size of line */

	/* String of terrain characters along one row of the map */
	if (mp_ta) KILL(mp_ta);
	if (mp_tc) KILL(mp_tc);

	/* String of characters along one row of the map */
	if (mp_a) KILL(mp_a);
	if (mp_c) KILL(mp_c);

	/* Save size */
	map_wid_old = wid;

	/* Make the new lines */

	/* String of terrain characters along one row of the map */
	C_MAKE(mp_ta, wid, byte);
	C_MAKE(mp_tc, wid, char);

	/* String of characters along one row of the map */
	C_MAKE(mp_a, wid, byte);
	C_MAKE(mp_c, wid, char);

    /* Verify the panel */
	verify_panel();
}


/*
 * Monster health description
 */
cptr look_mon_desc(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	bool living;
	int perc;


	/* Determine if the monster is "living" */
	living = monster_living(r_ptr);


	/* Healthy monsters */
	if (m_ptr->hp >= m_ptr->maxhp)
	{
		/* No damage */
		return (living ? "unhurt" : "undamaged");
	}


	/* Calculate a health "percentage" */
	perc = 100L * m_ptr->hp / m_ptr->maxhp;

	if (perc >= 60)
	{
		return (living ? "somewhat wounded" : "somewhat damaged");
	}

	if (perc >= 25)
	{
		return (living ? "wounded" : "damaged");
	}

	if (perc >= 10)
	{
		return (living ? "badly wounded" : "badly damaged");
	}

	return (living ? "almost dead" : "almost destroyed");
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort_aux(vptr u, vptr v, int p, int q)
{
	int z, a, b;

	/* Done sort */
	if (p >= q) return;

	/* Pivot */
	z = p;

	/* Begin */
	a = p;
	b = q;

	/* Partition */
	while (TRUE)
	{
		/* Slide i2 */
		while (!(*ang_sort_comp) (u, v, b, z)) b--;

		/* Slide i1 */
		while (!(*ang_sort_comp) (u, v, z, a)) a++;

		/* Done partition */
		if (a >= b) break;

		/* Swap */
		(*ang_sort_swap) (u, v, a, b);

		/* Advance */
		a++, b--;
	}

	/* Recurse left side */
	ang_sort_aux(u, v, p, b);

	/* Recurse right side */
	ang_sort_aux(u, v, b + 1, q);
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort(vptr u, vptr v, int n)
{
	/* Sort the array */
	ang_sort_aux(u, v, 0, n - 1);
}


/*** Targeting Code ***/

/*
 * Track a new monster
 */
void health_track(int m_idx)
{
	/* Track a new guy */
	p_ptr->health_who = m_idx;

	/* Redraw (later) */
	p_ptr->redraw |= (PR_HEALTH);
}


/*
 * Hack -- track the given monster race
 */
void monster_race_track(int r_idx)
{
	/* Save this monster ID */
	p_ptr->monster_race_idx = r_idx;

	/* Window stuff */
	p_ptr->window |= (PW_MONSTER);
}



/*
 * Hack -- track the given object kind
 */
void object_kind_track(int k_idx)
{
	/* Save this monster ID */
	p_ptr->object_kind_idx = k_idx;

	/* Window stuff */
	p_ptr->window |= (PW_OBJECT);
}


/*
 * Determine is a monster makes a reasonable target
 *
 * The concept of "targeting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and if
 * the player can hit it with a projection, and the player is not
 * hallucinating.  This allows use of "use closest target" macros.
 */
bool target_able(int m_idx)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	monster_type *m_ptr = &m_list[m_idx];

	/* Monster must be alive */
	if (!m_ptr->r_idx) return (FALSE);

	/* Monster must be visible */
	if (!m_ptr->ml) return (FALSE);

	/* Monster must not be a mimic */
	if (m_ptr->smart & SM_MIMIC) return (FALSE);

	/* Monster must be projectable */
	if (!projectable(px, py, m_ptr->fx, m_ptr->fy)) return (FALSE);

	/* Hack -- no targeting hallucinations */
	if (query_timed(TIMED_IMAGE)) return (FALSE);

	/* Assume okay */
	return (TRUE);
}


/*
 * Hack - function to get object name of mimic
 */
bool mimic_desc(char *m_name, const monster_race *r_ptr)
{
	/* Hack - look at default character */
	switch (r_ptr->d_char)
	{
		case '$':
		{
			/* XXX XXX XXX Mega-Hack */
			strcpy(m_name, mon_race_name(r_ptr) + sizeof("Creeping ") - 1);
			return (TRUE);
		}

		case '|':
		{
			/* Hack */
			strcpy(m_name, mon_race_name(r_ptr));
			return (TRUE);
		}

		case '?':
		{
			if (mon_name_cont(r_ptr, "Tome "))
			{
				strcpy(m_name, "tome");
			}
			else
			{
				strcpy(m_name, "scroll");
			}

			return (TRUE);
		}

		case '!':
		{
			strcpy(m_name, "potion");
			return (TRUE);
		}

		case '=':
		{
			strcpy(m_name, "ring");
			return (TRUE);
		}

		case '+':
		{
			strcpy(m_name, "door");
			return (TRUE);
		}

		case '&':
		{
			strcpy(m_name, "chest");
			return (TRUE);
		}

		case '(':
		{
			strcpy(m_name, "cloak");
			return (TRUE);
		}

		case '>':
		{
			strcpy(m_name, "down staircase");
			return (TRUE);
		}

		case '.':
		{
			/* Hack - do not notice lurkers etc. */
			return (FALSE);
		}

		case '#':
		{
			strcpy(m_name, "granite wall");
			return (TRUE);
		}

		default:
		{
			return (TRUE);
		}
	}
}


int get_chaos_patron(void)
{
	return ((p_ptr->rp.age + p_ptr->rp.sc) % MAX_PATRON);
}


void gain_level_reward(int chosen_reward)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	object_type *q_ptr;

	char wrath_reason[32] = "";
	int nasty_chance = 6;
	int tval, sval;
	int type, effect;
	int i;
	int patron = p_ptr->chaos_patron;

	int count = 0;

	if (p_ptr->lev == 13) nasty_chance = 2;
	else if (!(p_ptr->lev % 13)) nasty_chance = 3;
	else if (!(p_ptr->lev % 14)) nasty_chance = 12;

	/* Strange luck can give chaos rewards from a random patron */
	if (!(FLAG(p_ptr, TR_PATRON)))
	{
		nasty_chance *= 2;
		patron = randint0(MAX_PATRON);
	}

	if (one_in_(nasty_chance))
		type = randint1(20);	/* Allow the 'nasty' effects */
	else
		type = rand_range(5, 20);	/* Or disallow them */

	if (type < 1) type = 1;
	if (type > 20) type = 20;
	type--;

	current_object_source.type = OM_PATRON;
	current_object_source.place_num = p_ptr->place_num;
	current_object_source.depth = p_ptr->depth;
	current_object_source.data = 0;

	strnfmt(wrath_reason, 32, "the Wrath of %s", chaos_patrons[patron]);

	effect = chaos_rewards[patron][type];

	if (one_in_(6) && !chosen_reward)
	{
		msgf("%^s rewards you with a mutation!",
				   chaos_patrons[patron]);
		(void)gain_mutation(0);
		return;
	}

	switch (chosen_reward ? chosen_reward : effect)
	{
		case REW_POLY_SLF:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Thou needst a new form, mortal!'");
			do_poly_self();
			break;
		}
		case REW_GAIN_EXP:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Well done, mortal! Lead on!'");
			if (p_ptr->exp < PY_MAX_EXP)
			{
				s32b ee = (p_ptr->exp / 2) + 10;
				if (ee > 100000L) ee = 100000L;
				msgf("You feel more experienced.");
				gain_exp(ee);
			}
			break;
		}
		case REW_LOSE_EXP:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Thou didst not deserve that, slave.'");
			lose_exp(p_ptr->exp / 6);
			break;
		}
		case REW_GOOD_OBJ:
		{
			msgf("The voice of %s whispers:",
					   chaos_patrons[patron]);
			msgf("'Use my gift wisely.'");
			acquirement(px, py, 1, FALSE, FALSE);
			break;
		}
		case REW_GREA_OBJ:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Use my gift wisely.'");
			acquirement(px, py, 1, TRUE, FALSE);
			break;
		}
		case REW_CHAOS_WP:
		{
			msgf("The voice of %s comes from the void:",
					   chaos_patrons[patron]);
			msgf("'Thy deed hath earned thee a worthy blade.'");

			tval = TV_SWORD;
			switch (randint1(p_ptr->lev))
			{
				case 0:  case 1:
					sval = SV_DAGGER;
					break;
				case 2:  case 3:
					sval = SV_MAIN_GAUCHE;
					break;
				case 4:
					sval = SV_TANTO;
					break;
				case 5:  case 6:
					sval = SV_RAPIER;
					break;
				case 7:  case 8:
					sval = SV_SMALL_SWORD;
					break;
				case 9:  case 10:
					sval = SV_BASILLARD;
					break;
				case 11:  case 12:  case 13:
					sval = SV_SHORT_SWORD;
					break;
				case 14:  case 15:
					sval = SV_SABRE;
					break;
				case 16:  case 17:
					sval = SV_CUTLASS;
					break;
				case 18:
					sval = SV_WAKIZASHI;
					break;
				case 19:
					sval = SV_KHOPESH;
					break;
				case 20:
					sval = SV_TULWAR;
					break;
				case 21:
					sval = SV_BROAD_SWORD;
					break;
				case 22:  case 23:
					sval = SV_LONG_SWORD;
					break;
				case 24:  case 25:
					sval = SV_SCIMITAR;
					break;
				case 26:
					sval = SV_NINJATO;
					break;
				case 27:
					sval = SV_KATANA;
					break;
				case 28:  case 29:
					sval = SV_BASTARD_SWORD;
					break;
				case 30:
					sval = SV_GREAT_SCIMITAR;
					break;
				case 31:
					sval = SV_CLAYMORE;
					break;
				case 32:
					sval = SV_ESPADON;
					break;
				case 33:
					sval = SV_TWO_HANDED_SWORD;
					break;
				case 34:
					sval = SV_FLAMBERGE;
					break;
				case 35:
					sval = SV_NO_DACHI;
					break;
				case 36:
					sval = SV_EXECUTIONERS_SWORD;
					break;
				case 37:
					sval = SV_ZWEIHANDER;
					break;
				case 38:
					sval = SV_DIAMOND_EDGE;
					break;
				default:
					sval = SV_BLADE_OF_CHAOS;
			}

			q_ptr = object_prep(lookup_kind(tval, sval));

			q_ptr->to_h = 3 + randint1(p_ptr->depth) % 10;
			q_ptr->to_d = 3 + randint1(p_ptr->depth) % 10;

			add_ego_power(EGO_XTRA_ANY_RESIST, q_ptr);

			add_ego_flags(q_ptr, EGO_CHAOTIC);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, px, py);
			break;
		}
		case REW_GOOD_OBS:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Thy deed hath earned thee a worthy reward.'");
			acquirement(px, py, rand_range(2, 3), FALSE, FALSE);
			break;
		}
		case REW_GREA_OBS:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Behold, mortal, how generously I reward thy loyalty.'");
			acquirement(px, py, rand_range(2, 3), TRUE, FALSE);
			break;
		}
		case REW_TY_CURSE:
		{
			msgf("The voice of %s thunders:",
					   chaos_patrons[patron]);
			msgf("'Thou art growing arrogant, mortal.'");
			(void)activate_ty_curse(FALSE, &count);
			break;
		}
		case REW_SUMMON_M:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'My pets, destroy the arrogant mortal!'");
			for (i = 0; i < rand_range(2, 6); i++)
			{
				(void)summon_specific(0, px, py, p_ptr->depth, 0, TRUE, FALSE,
									  FALSE);
			}
			break;
		}
		case REW_H_SUMMON:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Thou needst worthier opponents!'");
			(void)activate_hi_summon();
			break;
		}
		case REW_DO_HAVOC:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Death and destruction! This pleaseth me!'");
			call_chaos(0);
			break;
		}
		case REW_GAIN_ABL:
		{
			msgf("The voice of %s rings out:",
					   chaos_patrons[patron]);
			msgf("'Stay, mortal, and let me mold thee.'");
			if (one_in_(3) && !(chaos_stats[patron] < 0))
				(void)do_inc_stat(chaos_stats[patron]);
			else
				(void)do_inc_stat(randint0(A_MAX));
			break;
		}
		case REW_LOSE_ABL:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'I grow tired of thee, mortal.'");
			if (one_in_(3) && !(chaos_stats[patron] < 0))
				(void)do_dec_stat(chaos_stats[patron]);
			else
				(void)do_dec_stat(randint0(A_MAX));
			break;
		}
		case REW_RUIN_ABL:
		{
			msgf("The voice of %s thunders:",
					   chaos_patrons[patron]);
			msgf("'Thou needst a lesson in humility, mortal!'");
			msgf("You feel less powerful!");
			for (i = 0; i < A_MAX; i++)
			{
				(void)dec_stat(i, rand_range(10, 25), TRUE);
			}
			break;
		}
		case REW_POLY_WND:
		{
			msgf("You feel the power of %s touch you.",
					   chaos_patrons[patron]);
			do_poly_wounds();
			break;
		}
		case REW_AUGM_ABL:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Receive this modest gift from me!'");
			for (i = 0; i < A_MAX; i++)
			{
				(void)do_inc_stat(i);
			}
			break;
		}
		case REW_HURT_LOT:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Suffer, pathetic fool!'");
			(void)fire_ball(GF_DISINTEGRATE, 0, p_ptr->lev * 4, 4);
			take_hit(p_ptr->lev * 4, wrath_reason);
			break;
		}
		case REW_HEAL_FUL:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Rise, my servant!'");
			(void)restore_level();
			(void)clear_poisoned();
			(void)clear_blind();
			(void)clear_confused();
			(void)clear_image();
			(void)clear_stun();
			(void)clear_cut();
			for (i = 0; i < A_MAX; i++)
			{
				(void)do_res_stat(i);
			}

			/* Recalculate max. hitpoints */
			update_stuff();

			(void)hp_player(5000);
			break;
		}
		case REW_CURSE_WP:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Thou reliest too much on thy weapon.'");
			(void)curse_weapon();
			break;
		}
		case REW_CURSE_AR:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Thou reliest too much on thine equipment.'");
			(void)curse_armor();
			break;
		}
		case REW_PISS_OFF:
		{
			msgf("The voice of %s whispers:",
					   chaos_patrons[patron]);
			msgf("'Now thou shalt pay for annoying me.'");
			switch (randint1(4))
			{
				case 1:
				{
					(void)activate_ty_curse(FALSE, &count);
					break;
				}
				case 2:
					(void)activate_hi_summon();
					break;
				case 3:
					if (one_in_(2)) (void)curse_weapon();
					else
						(void)curse_armor();
					break;
				default:
					for (i = 0; i < A_MAX; i++)
					{
						(void)dec_stat(i, rand_range(10, 25), TRUE);
					}
			}
			break;
		}
		case REW_WRATH:
		{
			msgf("The voice of %s thunders:",
					   chaos_patrons[patron]);
			msgf("'Die, mortal!'");

			take_hit(p_ptr->lev * 4, wrath_reason);

			for (i = 0; i < A_MAX; i++)
			{
				(void)dec_stat(i, rand_range(10, 25), FALSE);
			}

			(void)activate_hi_summon();
			(void)activate_ty_curse(FALSE, &count);

			if (one_in_(2)) (void)curse_weapon();
			if (one_in_(2)) (void)curse_armor();

			break;
		}
		case REW_DESTRUCT:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Death and destruction! This pleaseth me!'");
			(void)destroy_area(px, py, 25);
			break;
		}
		case REW_GENOCIDE:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Let me relieve thee of thine oppressors!'");
			(void)genocide(FALSE);
			break;
		}
		case REW_MASS_GEN:
		{
			msgf("The voice of %s booms out:",
					   chaos_patrons[patron]);
			msgf("'Let me relieve thee of thine oppressors!'");
			(void)mass_genocide(FALSE);
			break;
		}
		case REW_DISPEL_C:
		{
			msgf("You can feel the power of %s assault your enemies!",
					   chaos_patrons[patron]);
			(void)dispel_monsters(p_ptr->lev * 4);
			break;
		}
		case REW_IGNORE:
		{
			msgf("%s ignores you.", chaos_patrons[patron]);
			break;
		}
		case REW_SER_DEMO:
		{
			msgf("%s rewards you with a demonic servant!",
					   chaos_patrons[patron]);
			if (!summon_specific
				(-1, px, py, p_ptr->depth, SUMMON_DEMON, FALSE, TRUE, TRUE))
				msgf("Nobody ever turns up...");
			break;
		}
		case REW_SER_MONS:
		{
			msgf("%s rewards you with a servant!",
					   chaos_patrons[patron]);
			if (!summon_specific
				(-1, px, py, p_ptr->depth, SUMMON_NO_UNIQUES, FALSE, TRUE,
				 TRUE))
				msgf("Nobody ever turns up...");
			break;
		}
		case REW_SER_UNDE:
		{
			msgf("%s rewards you with an undead servant!",
					   chaos_patrons[patron]);
			if (!summon_specific
				(-1, px, py, p_ptr->depth, SUMMON_UNDEAD, FALSE, TRUE, TRUE))
				msgf("Nobody ever turns up...");
			break;
		}
		default:
		{
			msgf("The voice of %s stammers:",
					   chaos_patrons[patron]);
			msgf("'Uh... uh... the answer's %d/%d, what's the question?'",
					   type, effect);
		}
	}
}


/*
 * Find the maximum a stat can be raised to
 */
int stat_cap(int stat)
{
	int bonus = rp_ptr->r_adj[stat] + cp_ptr->c_adj[stat];

	if (bonus > 12)
		return 400;
	else
		return 280 + 10 * bonus;
}


int adjust_stat(int stat, int value, int amount)
{
	int cap = stat_cap(stat);

	/* Apply bonus/penalty */
	value += amount * 10;

	/* Cap value */
	if (value > cap) value = cap;
	if (value < 30)  value = 30;

	/* Return the result */
	return (value);
}
