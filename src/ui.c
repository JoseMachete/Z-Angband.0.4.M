/* File: ui.c */

/* Purpose: Angband user interface -SF- */

#include "angband.h"
#include "button.h"


/*
 * Center a format string in the buffer.
 *
 * The first parameter on the stack must be the width
 *  to center in.
 *
 * The second must be the string to center with.
 * This is treated as a format string - so may contain
 * other commands etc...
 */
void center_string(char *buf, uint max, cptr fmt, va_list *vp)
{
	int i, j;

	cptr str;

	char tmp[1024];
	int size;

	/* Unused parameter */
	(void)fmt;

	/* Get the size of the string to center in */
	size = va_arg(*vp, int);

	/* Get the string to center with. */
	str = va_arg(*vp, cptr);

	/* Expand the string */
	vstrnfmt(tmp, 1024, str, vp);

	/* Total length */
	i = strlen(tmp);

	/* Necessary border */
	j = (size - i) / 2;

	/* Mega-Hack center the (format) string in the buffer */
	strnfmt(buf, max, "%*s%s%*s", j, "", tmp, size - i - j, "");
}


/*
 * Function used to print a flag in coloured binary.
 */
void binary_fmt(char *buf, uint max, cptr fmt, va_list *vp)
{
	uint i;
	u32b mask = 1;

	int len = 0;

	u32b arg;

	/* Unused parameter */
	(void)fmt;

	/* Pre-terminate buffer */
	buf[0] = '\0';

	/* Get the argument */
	arg = va_arg(*vp, u32b);

	/* Scan the flags */
	for (i = 1; ((i <= 32) && (i < max)); i++)
	{
		/* Dump set bits */
		if (arg & mask)
		{
			strnfcat(buf, max, &len, CLR_BLUE "*");
		}

		/* Dump unset bits */
		else
		{
			strnfcat(buf, max, &len, CLR_WHITE "-");
		}

		mask *= 2;
	}
}


/*
 * Generic "get choice from menu" function
 */
int get_player_choice(cptr *choices, int num, int col, int wid,
                             cptr helpfile, void (*hook) (cptr))
{
	int top = 0, cur = 0;
	/* int bot = 13; */
	int i, dir;
	char c;
	char buf[80];
	bool done = FALSE;
	int hgt;


	/* Autoselect if able */
	if (num == 1) done = TRUE;

	/* Clear */
	for (i = TABLE_ROW; i < Term->hgt; i++)
	{
		/* Clear */
		Term_erase(col, i, Term->wid - wid);
	}

	/* Choose */
	while (TRUE)
	{
		/*
		 * Note to Melkor: What happens when the screen is resized?
		 * There is no 'redraw' hook at this point...
		 * (That is why the original code restricted itself to what
		 * would fit in the smallest possible screen.) -SF-
		 */
		hgt = Term->hgt - TABLE_ROW - 1;

		/* Redraw the list */
		button_backup_all(FALSE);
		for (i = 0; ((i + top < num) && (i <= hgt)); i++)
		{
			if (i + top < 26)
			{
				strnfmt(buf, 80, "%c) %s", I2A(i + top), choices[i + top]);
			}
			else
			{
				/* ToDo: Fix the ASCII dependency */
				strnfmt(buf, 80, "%c) %s", 'A' + (i + top - 26), choices[i + top]);
			}

			/* Clear */
			Term_erase(col, i + TABLE_ROW, wid);

			/* Display */
			if (i == (cur - top))
			{
				/* Highlight the current selection */
				put_fstr(col, i + TABLE_ROW, CLR_L_BLUE "$U%s$V", buf);
			}
			else
			{
				put_fstr(col, i + TABLE_ROW, "$U%s$V", buf);
			}
		}

		if (done) {
			button_restore();
			return (cur);
		}

		/* Display auxiliary information if any is available. */
		if (hook) hook(choices[cur]);

		/* Move the cursor */
		Term_gotoxy(col, TABLE_ROW + cur - top);

		c = inkey();

		button_restore();

		if (c == KTRL('X'))
		{
			quit(NULL);
		}
		if (c == ESCAPE)
		{
			/* Mega Hack - go back. */
			return (INVALID_CHOICE);
		}
		if (c == '*')
		{
			/* Select at random */
			cur = randint0(num);

			/* Move it onto the screen */
			if ((cur < top) || (cur > top + hgt))
			{
				top = cur;
			}

			/* Done */
			done = TRUE;
		}
		else if (c == '?')
		{
			(void)show_file(helpfile, NULL, 0, 0);
		}
		else if (c == '=')
		{
			do_cmd_options(OPT_FLAG_BIRTH | OPT_FLAG_SERVER | OPT_FLAG_PLAYER);
		}
		else if ((c == '\n') || (c == '\r'))
		{
			/* Done */
			return (cur);
		}
		else if (isdigit(c))
		{
			/* Get a direction from the key */
			dir = get_keymap_dir(c);

			/* Going up? */
			if (dir == 8)
			{
				if (cur != 0)
				{
					/* Move selection */
					cur--;
				}

				if ((top > 0) && ((cur - top) < 4))
				{
					/* Scroll up */
					top--;
				}
			}

			/* Going down? */
			if (dir == 2)
			{
				if (cur != (num - 1))
				{
					/* Move selection */
					cur++;
				}

				if ((top + hgt < (num - 1)) && ((top + hgt - cur) < 4))
				{
					/* Scroll down */
					top++;
				}
			}
		}
		else if (isalpha(c))
		{
			int choice;

			if (islower(c))
			{
				choice = A2I(c);
			}
			else
			{
				choice = c - 'A' + 26;
			}

			/* Validate input */
			if ((choice > -1) && (choice < num))
			{
				if (choice == cur) {
					/* Done */
					done = TRUE;
				}

				cur = choice;

				/* Move it into the center of the screen */
				top = cur - (hgt>>1);
				if (top + hgt > num-1)
					top = num - hgt - 1;
				if (top < 0)
					top = 0;
			}
			else
			{
				/* Invalid input */
				bell("Illegal birth choice!");
			}
		}
	}

	return (INVALID_CHOICE);
}


/*
 * Sorting hook -- comp function -- strings (see below)
 *
 * We use "u" to point to an array of strings.
 */
bool ang_sort_comp_hook_string(const vptr u, const vptr v, int a, int b)
{
	cptr *x = (cptr *)(u);

	/* Hack - ignore v */
	(void)v;

	return (strcmp(x[a], x[b]) <= 0);
}


/*
 * Sorting hook -- swap function -- array of strings (see below)
 *
 * We use "u" to point to an array of strings.
 */
void ang_sort_swap_hook_string(const vptr u, const vptr v, int a, int b)
{
	cptr *x = (cptr *)(u);

	cptr temp;

	/* Hack - ignore v */
	(void)v;

	/* Swap */
	temp = x[a];
	x[a] = x[b];
	x[b] = temp;
}


/*
 * Present a sorted list to the player, and get a selection
 */
int get_player_sort_choice(cptr *choices, int num, int col, int wid,
                                  cptr helpfile, void (*hook) (cptr))
{
	int i;
	int choice;
	cptr *strings;

	C_MAKE(strings, num, cptr);

	/* Initialise the sorted string array */
	for (i = 0; i < num; i++)
	{
		strings[i] = choices[i];
	}

	/* Sort the strings */
	ang_sort_comp = ang_sort_comp_hook_string;
	ang_sort_swap = ang_sort_swap_hook_string;

	/* Sort the (unique) slopes */
	ang_sort((void *)strings, NULL, num);

	/* Get the choice */
	choice = get_player_choice(strings, num, col, wid, helpfile, hook);

	/* Invert the choice */
	for (i = 0; i < num; i++)
	{
		/* Does the string match the one we selected? */
		if (choices[i] == strings[choice])
		{
			/* Save the choice + exit */
			choice = i;
			break;
		}
	}

	/* Free the strings */
	FREE((void *)strings);

	/* Return the value from the list */
	return (choice);
}


/* Show the option */
static bool show_action_option(int x, int y, menu_action *option, char c, bool scroll, bool select)
{
	if (option->flags & MN_ACTIVE)
	{
		/* Is this option selected */
		if (select)
		{
			/* Highlight this option? */
			if (scroll)
			{
				prtf(x, y, " $U%c) " CLR_L_BLUE "%s$V", c, option->text);
			}
			else
			{
				prtf(x, y, "*$U%c) %s$V", c, option->text);
			}
		}
		else
		{
			prtf(x, y, " $U%c) %s$V", c, option->text);
		}

		return (TRUE);
	}

	/* Not a valid option */
	prtf(x, y, "    %s", option->text);

	return (FALSE);
}

/*
 * Array for converting numbers to a logical list symbol
 */
static const char listsym[] =
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'\0'
};



/*
 * Display a menu of choices on the screen
 *
 * We return the number of active options.
 */
static int show_action_menu(int num, menu_action *options, int select, bool scroll,
					 int (*disp)(int), cptr prompt)
{
	int cnt = 0;
	int i;
	bool select_me;

	int x, y;

	int offset = 0;

	/*
	 * Display 'special' information
	 */
	if (disp) offset = disp(num);

	/* Border on top of menu */
	clear_row(1);

	/* Will they fit in one column? */
	if (num < 19)
	{
		for (i = 0; i < num; i++)
		{
			select_me = i == select;

			if (show_action_option(0, i + 2 + offset, &options[i], I2A(cnt), scroll, select_me))
			{
				cnt++;
			}
		}

		/* Border below menu */
		clear_row(num + 2 + offset);
	}

	/* Two columns (use numbers as well) */
	else if (num < 37)
	{
		for (i = 0; i < num; i++)
		{
			select_me = i == select;

			x = (i / 18) * 40;
			y = (i % 18) + 2;

			if (show_action_option(x, y + offset, &options[i], listsym[cnt], scroll, select_me))
			{
				cnt++;
			}
		}

		/* Border below menu */
		clear_row(20 + offset);
	}

	/* Three columns - need to use upper case letters */
	else
	{
		for (i = 0; i < num; i++)
		{
			select_me = i == select;

			x = (i / 20) * 30;
			y = (i % 20) + 2;

			if (show_action_option(x, y + offset, &options[i], listsym[cnt], scroll, select_me))
			{
				cnt++;
			}
		}

		/* Border below menu */
		clear_row(22 + offset);
	}

	/*
	 * Display the prompt.
	 * (Do this last, so we get the cursor in the right spot)
	 */
	if (!cnt)
	{
		prtf(0, 0, "%s (No commands available, $UESC=exit$Y%c$V)", prompt, ESCAPE);
	}
	else if (cnt == 1)
	{
		/* Display the prompt */
		prtf(0, 0, "%s (Command (a), $UESC=exit$Y%c$V)", prompt ? prompt : "Select a command: ", ESCAPE);
	}
	else if (cnt < 19)
	{
		/* Display the prompt */
		prtf(0, 0, "%s (Command (a-%c), $UESC=exit$Y%c$V)",
			 prompt ? prompt : "Select a command: " ,I2A(cnt - 1), ESCAPE);
	}
	else
	{
		/* Display the prompt */
		prtf(0, 0, "%s (Command (0-%c), $UESC=exit$Y%c$V)",
			 prompt ? prompt : "Select a command: " ,listsym[cnt - 1], ESCAPE);
	}


	return (cnt);
}

/*
 * Get the choice corresponding to the character
 * chosen, and the number of possible choices.
 *
 * Return whether we want this choice verified or not.
 */
static int get_choice(char *c, int num, bool *ask)
{
	int asked;

	int i;

	*c = inkey();

	/* Handle "cancel" */
	if (*c == ESCAPE)
    {
        	return (-2);
    }

	if (num < 19)
	{
		if (isalpha(*c))
		{
			/* Note verify */
			asked = (isupper(*c));

			/* Lowercase */
			if (asked) *c = tolower(*c);

			*ask = (asked != FALSE);

			/* Extract request */
			return(A2I(*c));
		}

		/* Invalid choice */
		*ask = FALSE;
		return (-1);
	}

	/* Else - look for a match */
	for (i = 0; i < num; i++)
	{
		if (listsym[i] == *c)
		{
			/* Hack - we cannot ask if there are too many options */
			*ask = FALSE;
			return (i);
		}
	}

	/* No match? */
	*ask = FALSE;

	return (-1);
}


/*
 * Display a menu, and get a choice.
 * Return false if escape is pressed.
 *
 * 'options' is an array that contains the strings to print.
 *       plus all the functions to call together with the
 *       flags for each option.
 * 'select' shows the default/current option.
 *       If negative, it is ignored.
 * 'scroll' controls whether or not to allow scrolling option
 *       selection.
 * 'disp' contains an optional function to print some extra
 *       information when constucting the menu.
 * 'prompt' is an optional prompt.
 */
bool display_action_menu(menu_action *options, int select, bool scroll, int (*disp)(int),
					cptr prompt)
{
	int i = -1, j, cnt;
	bool ask = FALSE;
	char choice = 0;
	int num = 0;
	int save_choice;

	/* Calculate the number of strings we have */
	while (options[num].text) num++;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Save the screen */
	screen_save();

	/* store are previous buttons */
	button_backup_all(TRUE);

	/* Show the list */
	cnt = show_action_menu(num, options, select, scroll, disp, prompt);

	/* Paranoia */
	if (!cnt)
	{
		while (inkey() != ESCAPE)
		{
			/* Do nothing */
		}

		/* Restore the screen */
		screen_load();
		/* restore any previous mouse buttons */
		button_restore();
		return (FALSE);
	}

	/* Get a command from the user */
	while (TRUE)
	{
		/* Try to get previously saved value */
		if (!repeat_pull(&i)) {
			/* Try to match with available options */
			i = get_choice(&choice, num, &ask);
   	}


		/* Handle "cancel" */
		if (i == -2) {
			/* Restore the screen */
			screen_load();
			/* restore any previous mouse buttons */
			button_restore();
			return (FALSE);
		}

		/* No match? */
		if (i == -1) {
			/* Pick default option */
			if ((choice == '\r') || (choice == ' '))
			{
				i = 0;

				/* Scan options */
        		if (num > 1)
				{
					/* Count active options up to our selection */
					for (j = 0; j < select; j++)
					{
						if (options[j].flags & MN_ACTIVE)
						{
							i++;
						}
					}
				}
			}

			/* Scroll selected option up or down */
			else if ((choice == '8') && scroll)
			{
				do
				{
					/* Find previous option */
					select--;

					/* Scroll over */
					if (select < 0) select = num - 1;
				}
				while(!(options[select].flags & MN_SELECT));

				/* kill the current menu buttons */
				button_kill_all();

				/* Show the list */
				show_action_menu(num, options, select, scroll, disp, prompt);

				/* Next time */
				continue;
			}

			/* Scroll selected option up or down */
			else if ((choice == '2') && scroll)
			{
				do
				{
					/* Find next option */
					select++;

					/* Scroll over */
					if (select >= num) select = 0;
				}
				while(!(options[select].flags & MN_SELECT));

				/* kill the current menu buttons */
				button_kill_all();

				/* Show the list */
				show_action_menu(num, options, select, scroll, disp, prompt);

				/* Next time */
				continue;
			}

			/* Context-sensitive help */
			else if (choice == '?')
			{
				/* Do we have a help entry? */
				if ((select >= 0) && options[select].help)
				{
					/* Show the information */
					show_file(options[select].help, NULL, 0, 0);

					/* kill the current menu buttons */
					button_kill_all();

					/* Show the list */
					show_action_menu(num, options, select, scroll, disp, prompt);

					/* Next time */
					continue;
				}
				else
				{
					bell("No context sensitive help available!");
					continue;
				}
			}
		}

		/* Totally Illegal */
		if ((i < 0) || (i >= cnt))
		{
			bell("Illegal choice!");
			continue;
		}

		save_choice = i;

		/* Find the action to call */
		for (j = 0; j < num; j++)
		{
			if (options[j].flags & MN_ACTIVE)
			{
				if (!i)
				{
					/* Verify it */
					if (ask)
					{
						/* Belay that order */
						if (!get_check("Use %s? ", options[j].text))
						{
							/* kill the current menu buttons */
							button_kill_all();

							/* Show the list */
							show_action_menu(num, options, select, scroll, disp, prompt);
							break;
						}
					}

					/* Hack - restore the screen */
					if (options[j].flags & MN_CLEAR)
					{
						/* restore any previous mouse buttons */
						button_restore();

						screen_load();
					}

					if (options[j].action(j))
					{
						/* Hack - restore the screen */
						if (!(options[j].flags & MN_CLEAR))
						{
							/* restore any previous mouse buttons */
							button_restore();

							/* Restore the screen */
							screen_load();
						}

						/* Save for later */
						repeat_push(save_choice);

						/* Success */
						return (TRUE);
					}

					/* Hack - save the screen */
					if (options[j].flags & MN_CLEAR)
					{
						screen_save();

						/* store are previous buttons */
						button_backup_all(TRUE);
					}

					/*
					 * Select this option for next time
					 * if had a previous selection.
					 */
					if ((select >= 0) && (options[j].flags & MN_SELECT))
					{
						select = j;
					}

					/* Hack - flush messages */
					message_flush();

					/* Show the list */
					show_action_menu(num, options, select, scroll, disp, prompt);

					/* Get a new command */
					break;
				}

				/* Decrement count until reach selected option */
				i--;
			}
		}
	}

	/* restore any previous mouse buttons */
	button_restore();
	/* Paranoia for dumb compilers */
	quit("Unreachable code in display_action_menu");
	return (FALSE);
}


/*
 * Flush the screen, make a noise
 */
void bell(cptr reason)
{
	/* Mega-Hack -- Flush the output */
	Term_fresh();

	/* Hack -- memorize the reason if possible */
	if (character_generated && reason)
	{
		message_add(reason, MSG_BELL);

		/* Window stuff */
		p_ptr->window |= (PW_MESSAGE);

		/* Force window redraw */
		window_stuff();
	}

	/* Make a bell noise (if allowed) */
	if (ring_bell) Term_xtra(TERM_XTRA_NOISE, 0);

	/* Flush the input (later!) */
	flush();
}


/*
 * Hack -- Make a (relevant?) sound
 */
void sound(int val)
{
	/* No sound */
	if (!use_sound) return;

	/* Make a sound (if allowed) */
	Term_xtra(TERM_XTRA_SOUND, val);
}

/*
 * Convert a "color letter" into an "actual" color
 * The colors are: dwsorgbuDWvyRGBU, as shown below
 */
int color_char_to_attr(char c)
{
	switch (c)
	{
		case 'd': return (TERM_DARK);
		case 'w': return (TERM_WHITE);
		case 's': return (TERM_SLATE);
		case 'o': return (TERM_ORANGE);
		case 'r': return (TERM_RED);
		case 'g': return (TERM_GREEN);
		case 'b': return (TERM_BLUE);
		case 'u': return (TERM_UMBER);

		case 'D': return (TERM_L_DARK);
		case 'W': return (TERM_L_WHITE);
		case 'v': return (TERM_VIOLET);
		case 'y': return (TERM_YELLOW);
		case 'R': return (TERM_L_RED);
		case 'G': return (TERM_L_GREEN);
		case 'B': return (TERM_L_BLUE);
		case 'U': return (TERM_L_UMBER);
	}

	return (-1);
}

char attr_to_color_char(byte c)
{
	switch (c)
	{
		case TERM_DARK: return('d');
		case TERM_WHITE: return('w');
		case TERM_SLATE: return('s');
		case TERM_ORANGE: return('o');
		case TERM_RED: return('r');
		case TERM_GREEN: return('g');
		case TERM_BLUE: return('b');
		case TERM_UMBER: return('u');

		case TERM_L_DARK: return('D');
		case TERM_L_WHITE: return('W');
		case TERM_VIOLET: return('v');
		case TERM_YELLOW: return('y');
		case TERM_L_RED: return('R');
		case TERM_L_GREEN: return('G');
		case TERM_L_BLUE: return('B');
		case TERM_L_UMBER: return('U');
	}

	return ('&');
}


/*
 * Save the screen, and increase the "icky" depth.
 *
 * This function must match exactly one call to "screen_load()".
 */
void screen_save(void)
{
	/* Hack -- Flush messages */
	message_flush();

	/* Save the screen */
	Term_save();

	/* Increase "icky" depth */
	character_icky++;
}


/*
 * Load the screen, and decrease the "icky" depth.
 *
 * This function must match exactly one call to "screen_save()".
 */
void screen_load(void)
{
	/* Hack -- Flush messages */
	message_flush();

	/* Load the screen */
	Term_load();

	/* Decrease "icky" depth */
	character_icky--;

	/* Mega hack to clear the screen on the last screen load
	 * from Angband */
	/* TODO: should really be a scan to check for changed cells and if
	 * the change is the skip attr/char pair, then make sure that the parent 
	 * cell is in the refresh queue as well (with different old data).
	 */
	if ((character_icky == 0)
		&& ((tile_width_mult > 1) || (tile_height_mult > 1)))
	{
		Term->total_erase = (TRUE);
	}
}

/*
 * Find offset of str2 in str1, including the
 * effects of formatting characters.
 *
 * This is not equal to str2 - str1 in general.
 */
int fmt_offset(cptr str1, cptr str2)
{
	cptr c = str1;
	int i = 0;

	while (*c && (c != str2))
	{
		/* Does this character match the escape code? */
		if (*c == '$')
		{
			/* Scan the next character */
			c++;

			/* Is it a colour specifier? */
			if (((*c >= 'A') && (*c <= 'R')) ||
				((*c >= 'a') && (*c <= 'r')))
			{
				c++;

				continue;
			}
		}

		/* Next position */
		i++;
		c++;
	}

	return (i);
}

/*
 * Remove the formatting escape sequences from a buffer.
 */
void fmt_clean(char *buf)
{
	char *p = buf, *c = buf;

	while (*c)
	{
		/* Does this character match the escape code? */
		if (*c == '$')
		{
			/* Scan the next character */
			c++;

			/* Is it an escape sequence? */
			if ((*c >= 'A') && (*c <= 'R'))
			{
				/* Ignore it */
				c++;

				continue;
			}
			if ((*c >= 'U') && (*c <= 'X'))
			{
				/* Ignore it */
				c++;


				continue;
			}
			if (*c == 'Y')
			{
				/* Ignore it */
				c+=2;


				continue;
			}

			/*
			 * Hack XXX XXX - otherwise, ignore the dollar sign
			 * and copy the string value.
			 *
			 * This makes "$$" turn into just "$".
			 */
			*p++ = *c;

			/* Stop if reach null */
			if (*c == 0) break;
		}
		else
		{
			/* Copy the value */
			*p++ = *c++;
		}
	}

	/* Terminate buffer */
	*p = '\0';
}

/*
 * count the number of displayed characters in a string with control characters
 */
int len_cstr(cptr str)
{
	cptr c = str;
	int len = 0;

	while (*c) {
		if (*c == '$') {
			/* the dollar sign is not displayed */
			c++;
			if ((*c >= 'A') && (*c <= 'R')) {
				/* a color - not displayed */
				c++;
			} else
			if ((*c >= 'U') && (*c <= 'X')) {
				/* a button control - not displayed */
				c++;
			} else
			if (*c == 'Y') {
				/* a key press for a button - not displayed, nor is next char */
				c+=2;
			} else
			{
				/* this character, but not the dollar sign, are displayed */
				len++;
				/* Next position */
				c++;
			}
		} else {
			if (*c != '\n') {
				len++;
			}
			/* Next position */
			c++;
		}
	}
	return len;
}
/*
 * Put a string with control characters at a given location
 */
void put_cstr(int col, int row, cptr str, int clear)
{
	cptr c = str;

	/* Default to white */
	byte a = TERM_WHITE;
	byte da = a;
	int x = col;
	int mx = col + 255;

	/* Clear line, position cursor */
	if (clear > 0) Term_erase(col, row, clear);
	else if (clear < 0) clear = len_cstr(str);

	if (clear) mx = col+clear;

	while (*c)
	{
		/* Does this character match the escape code? */
		if (*c == '$')
		{
			/* Scan the next character */
			c++;

			/* Is it a colour specifier? */
			if ((*c >= 'A') && (*c <= 'P'))
			{
				/*
				 * Save the new colour
				 *
				 * Hack - this depends on ASCII symbols
				 */
				a = *c - 'A';
				c++;

				/* Hack -- fake monochrome */
				if (!use_color) a = TERM_WHITE;

				continue;
			}

			/* Default colour change? */
			else if (*c == 'Q')
			{
				/* Save current colour as 'default' */
				da = a;
				c++;

				continue;
			}

			/* Go back to default colour */
			else if (*c == 'R')
			{
				a = da;
				c++;

				continue;
			}

			/* Start a button area */
			else if (*c == 'U')
			{
				(void)button_add_start(row, x,*(c+1));
				c++;

				continue;
			}

			/* End a button area */
			else if (*c == 'V')
			{
				(void)button_add_end(NULL ,0, row, x);
				c++;

				continue;
			}

			/* Set the keypress of the current button area */
			else if (*c == 'Y')
			{
				(void)button_last_key(*(c+1));
				c+=2;

				continue;
			}

			/* add a one word button */
			else if (*c == 'W')
			{
				int len = 0;
				const char *pc = c+1;
				while (*pc && !isspace(*(pc++))) {
					len++;
				}
				(void)button_add_2d(row,x,row,x+len,NULL,*(c+1));
				c++;

				continue;
			}

			/* add a one character button */
			else if (*c == 'X')
			{
				(void)button_add_2d(row,x,row,x,NULL,*(c+1));
				c++;

				continue;
			}

			/*
			 * Hack XXX XXX - otherwise, ignore the dollar sign
			 *
			 * This makes "$$" turn into just "$".
			 */

			/* Stop if reach null */
			else if (*c == 0) break;
		}

		if (*c == '\n')
		{
			/* Reset to the 'start' of the next row. */
			row++;
			x = col;
			c++;

			/* Clear line, position cursor */
			if (clear > 0) Term_erase(col, row, clear);

			continue;
		}

		if (x <= mx) {
		/* Display the character */
		Term_putch(x, row, a, *c);
		}

		/* Next position */
		x++;
		c++;
	}
}

/*
 * Put a string with formatting information at a given location
 */
void put_fstr(int col, int row, cptr str, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, str);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, str, &vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	put_cstr(col, row, buf, 0);
}


/*
 * Put a string with formatting information at a given location.
 * Clear the line before we start printing.
 */
void prtf(int col, int row, cptr str, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, str);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, str, &vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Display */
	put_cstr(col, row, buf, 255);
}


/* Set a limit on the display of roff text (below)
 * mainly to show object descriptions with context menu */
static int roff_width = 0;
void roff_set_width(int width)
{
	if (width) {
		roff_width = width;
	} else {
		roff_width = 0;
	}
}


/*
 * Print some (colored) text to the screen at the current cursor position,
 * automatically "wrapping" existing text (at spaces) when necessary to
 * avoid placing any text into the last column, and clearing every line
 * before placing any text in that line.  Also, allow "newline" to force
 * a "wrap" to the next line.  Advance the cursor as needed so sequential
 * calls to this function will work correctly.
 *
 * Once this function has been called, the cursor should not be moved
 * until all the related "roff()" calls to the window are complete.
 *
 * This function will correctly handle any width up to the maximum legal
 * value of 256, though it works best for a standard 80 character width.
 */
void roff(cptr str, ...)
{
	int x, y;

	int w, h;

	int clear;

	cptr s;

	byte a = TERM_WHITE;
	byte da = a;

	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, str);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, str, &vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Obtain the size */
	(void)Term_get_size(&w, &h);

	/* Obtain the cursor */
	(void)Term_locate(&x, &y);

	/* limit the width, if requested */
	if (roff_width) {
		clear = roff_width;
		if (roff_width < w)
			w = roff_width;
	} else {
		clear = 255;
	}

	/* Process the string */
	for (s = buf; *s; s++)
	{
		char ch;

		/* Force wrap */
		if (*s == '\n')
		{
			/* Wrap */
			x = 0;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, clear);

			continue;
		}

		/* Does this character match the escape code? */
		if (*s == '$')
		{
			/* Scan the next character */
			s++;

			/* Is it a colour specifier? */
			if ((*s >= 'A') && (*s <= 'P'))
			{
				/*
				 * Save the new colour
				 *
				 * Hack - this depends on ASCII symbols
				 */
				a = *s - 'A';

				/* Hack -- fake monochrome */
				if (!use_color) a = TERM_WHITE;

				continue;
			}

			/* Default colour change? */
			else if (*s == 'Q')
			{
				/* Save current colour as 'default' */
				da = a;

				continue;
			}

			/* Go back to default colour */
			else if (*s == 'R')
			{
				a = da;

				continue;
			}

			/* currently roff is only used in monster and object descriptions
			 * so we do not need button support here, so just skip characters */
			else if (*s == 'U')
			{
				continue;
			}
			else if (*s == 'V')
			{
				continue;
			}
			else if (*s == 'Y')
			{
				s++;
				continue;
			}
			else if (*s == 'W')
			{
				continue;
			}
			else if (*s == 'X')
			{
				continue;
			}

			/*
			 * Hack XXX XXX - otherwise, ignore the dollar sign
			 *
			 * This makes "$$" turn into just "$".
			 */

			/* Stop if now reach null */
			else if (*s == 0) break;
		}

		/* Clean up the char */
		ch = (isprint(*s) ? *s : ' ');

		/* Wrap words as needed */
		if ((x >= w - 1) && (ch != ' '))
		{
			int i, n = 0;

			byte av[256];
			char cv[256];

			/* Wrap word */
			if (x < w)
			{
				/* Scan existing text */
				for (i = w - 2; i >= 0; i--)
				{
					/* Grab existing attr/char */
					(void)Term_what(i, y, &av[i], &cv[i]);

					/* Break on space */
					if (cv[i] == ' ') break;

					/* Track current word */
					n = i;
				}
			}

			/* Special case */
			if (n == 0) n = w;

			/* Clear line */
			Term_erase(n, y, clear);

			/* Wrap */
			x = 0;
			y++;

			/* Clear line, move cursor */
			Term_erase(x, y, clear);

			/* Wrap the word (if any) */
			for (i = n; i < w - 1; i++)
			{
				/* Dump */
				Term_addch(av[i], cv[i]);

				/* Advance (no wrap) */
				if (++x > w) x = w;
			}
		}

		/* Dump */
		Term_addch(a, ch);

		/* Advance */
		if (++x > w) x = w;
	}
}

/*
 * Like the above roff(), print lines.
 * However, print them to a file like fprintf().
 *
 * froff() is smarter than fprintf() though.
 * It will also understand the '%v' format control sequence.
 */
void froff(ang_file *fff, cptr str, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, str);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, str, &vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* Output it to the file */
	file_putf(fff, "%s", buf);
}

/*
 * Print a string, wrapped, to a given file.
 *
 * margin: amount to indent each line
 * rowmax: max string characters to put on each line.
 */
void wrap_froff(ang_file *fff, char *buf, int margin, int rowmax)
{
	int len = strlen(buf);
	int i = 0;
	int j, brk;

	/* Paranoia */
	if (margin < 0 || rowmax <= 0 || len == 0)
		return;

	while (i < len)
	{
		/* Print a line */
		brk = 0;

		/* Find where to divide the line. */
		for (j = i; j < i+rowmax; j++)
		{
			/* A \n character forces a line break. */
			if (buf[j] == '\n')
			{
				brk = j;
				break;
			}
			else if (isspace((int)buf[j]))
				brk = j;
			else if (buf[j] == 0)
			{
				brk = j;
				break;
			}
		}

		/* Print a newline, except the first time. */
		if (i > 0) froff(fff, "\n");

		/* Print margin */
		for (j = 0; j < margin; j++)
		{
			froff(fff, " ");
		}

		/* Special case: no line break found. */
		if (brk == 0)
		{
			/* Bad behavior, but better than none */
			froff(fff, "%s", &buf[i]);
			return;
		}

		/* Eat up any extra whitespace */
		while(isspace((int)buf[brk+1])) brk++;

		/* Make a termination point at the break */
		buf[brk] = 0;

		/* Print the line up to the break */
		froff(fff, "%s", &buf[i]);

		/* Continue processing the string from the character
		   after the break */
		i = brk+1;
	}
}


/*
 * Clear part of the screen
 */
void clear_from(int row)
{
	int y;

	/* Erase requested rows */
	for (y = row; y < Term->hgt; y++)
	{
		/* Erase part of the screen */
		Term_erase(0, y, 255);
	}
}

/*
 * Clear top line of screen
 */
void clear_msg(void)
{
	Term_erase(0, 0, 255);
}

/*
 * Clear a line of the screen
 */
void clear_row(int row)
{
	Term_erase(0, row, 255);
}

/*
 * Clear a region of the screen, starting from (x,y1)
 * going to (255,y2)
 */
void clear_region(int x, int y1, int y2)
{
	int y;
	for (y = y1; (y < Term->hgt) && (y <= y2); y++)
	{
		/* Erase part of the screen */
		Term_erase(x, y, 255);
	}
}


/*
 * Get some input at the cursor location.
 * Assume the buffer is initialized to a default string.
 * Note that this string is often "empty" (see below).
 * The default buffer is displayed in yellow until cleared.
 * Pressing RETURN right away accepts the default entry.
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * ESCAPE clears the buffer and the window and returns FALSE.
 * RETURN accepts the current buffer contents and returns TRUE.
 *
 * Note that 'len' refers to the size of the buffer.  The maximum length
 * of the input is 'len-1'.
 */
bool askfor_aux(char *buf, int len)
{
	int y, x;

	int i = 0;

	int k = 0;

	bool done = FALSE;

	/* see if we have a ui override */
	if (Term->askfor_hook) return (*(Term->askfor_hook))(buf, len);

	/* Locate the cursor */
	(void)Term_locate(&x, &y);


	/* Paranoia -- check len */
	if (len < 1) len = 1;

	/* Paranoia -- check column */
	if ((x < 0) || (x >= 80)) x = 0;

	/* Restrict the length */
	if (x + len > 80) len = 80 - x;


	/* Paranoia -- Clip the default entry */
	buf[len - 1] = '\0';


	/* Display the default answer */
	Term_erase(x, y, len);

	put_fstr(x, y, CLR_YELLOW "%s", buf);

	/* Process input */
	while (!done)
	{
		/* Get a key */
		i = inkey();

		/* Analyze the key */
		switch (i)
		{
			case ESCAPE:
				k = 0;
				done = TRUE;
				break;

			case '\n':
			case '\r':
				k = strlen(buf);
				done = TRUE;
				break;

			case 0x7F:
			case '\010':
				if (k > 0) k--;
				break;

			default:
				if ((k < len - 1) && (isprint(i)))
				{
					buf[k++] = i;
				}
				else
				{
					bell("Illegal edit key!");
				}
				break;
		}

		/* Terminate */
		buf[k] = '\0';

		/* Update the entry */
		Term_erase(x, y, len);
		put_fstr(x, y, "%s", buf);
	}

	/* Aborted */
	if (i == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Get a string from the user
 *
 * The "prompt" should take the form "Prompt: "
 *
 * Note that the initial contents of the string is used as
 * the default response, so be sure to "clear" it if needed.
 *
 * We clear the input, and return FALSE, on "ESCAPE".
 */
bool get_string(char *buf, int len, cptr str, ...)
{
	bool res;

	va_list vp;

	char prompt[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, str);

	/* Format the args, save the length */
	(void)vstrnfmt(prompt, 1024, str, &vp);

	/* End the Varargs Stuff */
	va_end(vp);

	/* see if we have a ui override */
	if (Term->get_string_hook) return (*(Term->get_string_hook))(buf, len, prompt);

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display prompt */
	prtf(0, 0, prompt);

	/* Ask the user for a string */
	res = askfor_aux(buf, len);

	/* Clear prompt */
	clear_msg();

	/* Result */
	return (res);
}


/*
 * Verify something with the user
 *
 * The "prompt" should take the form "Query? "
 *
 * Note that "[y/n]" is appended to the prompt.
 */
bool get_check_base(bool def, bool esc, cptr prompt)
{
	int i;

	/* see if we have a ui override */
	if (Term->get_check_hook) return (*(Term->get_check_hook))(def,esc,prompt);

	/* Do not skip */
	p_ptr->state.skip_more = FALSE;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Prompt for it */
	prtf(0, 0, "%.70s$U[y$Yy$V$U/n]$Yn$V ", prompt);

	/* Get an acceptable answer */
	while (TRUE)
	{
		i = inkey();
		if (quick_messages) break;
		/* Unreachable code now */
		/* if (i == ESCAPE) break;
		if (strchr("YyNn\n\r", i)) break;
		bell("Illegal response to a 'yes/no' question!"); */
	}

	/* Erase the prompt */
	clear_msg();

	/* Success? */
	switch (i)
	{
		case 'y': case 'Y':
			return (TRUE);

		case ESCAPE:
			return (esc);

		case '\n': case '\r':
			return (def);

		default:
			return (FALSE);
	}
}

bool get_check_ext(bool def, bool esc, cptr prompt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, prompt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, prompt, &vp);

	/* End the Varargs Stuff */
	va_end(vp);

	return get_check_base(def, esc, buf);
}

bool get_check(cptr prompt, ...)
{
	va_list vp;

	char buf[1024];

	/* Begin the Varargs Stuff */
	va_start(vp, prompt);

	/* Format the args, save the length */
	(void)vstrnfmt(buf, 1024, prompt, &vp);

	/* End the Varargs Stuff */
	va_end(vp);

	return get_check_base(FALSE, FALSE, buf);
}

/*
 * Prompts for a keypress
 *
 * The "prompt" should take the form "Command: "
 *
 * Returns TRUE unless the character is "Escape"
 */
bool get_com(cptr prompt, char *command)
{
	/* see if we have a ui override */
	if (Term->get_com_hook) return (*(Term->get_com_hook))(prompt, command);

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display a prompt */
	prtf(0, 0, prompt);

	/* Get a key */
	*command = inkey();

	/* Clear the prompt */
	clear_msg();

	/* Handle "cancel" */
	if (*command == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}
bool get_com_m(cptr prompt, char *command)
{
	/* see if we have a ui override */
	if (Term->get_com_m_hook) return (*(Term->get_com_m_hook))(prompt, command);

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display a prompt */
	prtf(0, 0, prompt);

	/* Get a key */
	*command = inkey_m();

	/* Clear the prompt */
	clear_msg();

	/* Handle "cancel" */
	if (*command == ESCAPE) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * Request a "quantity" from the user
 *
 * Hack -- allow "command_arg" to specify a quantity
 */
s32b get_quantity_big(cptr prompt, s32b initial, s32b max)
{
	long amt;

	char tmp[80];

	int y, x, i;
	s32b scale;
	keycode_t k = 0;

	bool done = FALSE;
	bool first = TRUE;

	/* Use "command_arg" */
	if (p_ptr->cmd.arg)
	{
		/* Extract a number */
		amt = p_ptr->cmd.arg;

		/* Clear "cmd.arg" */
		p_ptr->cmd.arg = 0;

		/* Enforce the maximum */
		if (amt > max) amt = max;

		/* Use it */
		return (amt);
	}

	/* Get the item index */
	if ((max != 1) && repeat_pull((int *)&amt))
	{
		/* Enforce the maximum */
		if (amt > max) amt = max;

		/* Enforce the minimum */
		if (amt < 0) amt = 0;

		/* Use it */
		return (amt);
	}

	/* Build a prompt if needed */
	if (!prompt)
	{
		/* Build a prompt */
		strnfmt(tmp, 80, "Quantity (1-%d): ", max);

		/* Use that prompt */
		prompt = tmp;
	}


	/* see if we have a ui override */
	if (Term->get_quantity_big_hook) return (*(Term->get_quantity_big_hook))(prompt, initial, max);

	/* Default to one */
	amt = initial;

	/* Paranoia XXX XXX XXX */
	message_flush();

	/* Display prompt */
	prtf(0, 0, prompt);

	scale = max / 100;
	if (scale < 1) scale = 1;

	/* Locate the cursor */
	(void)Term_locate(&x, &y);

	/* Paranoia -- check column, allow 10 spaces for the number */
	if ((x < 0) || (x >= 70)) x = 0;

	/* Display the default answer */
	Term_erase(x, y, 10);
	put_fstr(x, y, CLR_YELLOW "%d", amt);

	/* Process input */
	while (!done) {
		/* Get a key */
		k = inkey_m();

		if (k & 0x80) {
			/* translate a mouse press */
			char mb, mods;
			int mx, my;
			Term_getmousepress(&mb, &mx, &my);
			mods = mb & 0x78;
			mb = mb & 0x07;
			k = 0;
			if (mb == 1) {
				/* accept the number */
				done = TRUE;
			} else
			if (mb == 2) {
				/* cancel the transaction */
				amt = 0;
				done = TRUE;
			} else
			if (mb == 4) {
				amt += scale;
			} else
			if (mb == 5) {
				amt -= scale;
			}
			if (amt > max) amt = max;
			if (amt < 0) amt = 0;
			first = FALSE;
		} else
		if (isdigit(k)) {
			if (first) {
				amt = (k-'0');
				first = FALSE;
			} else {
				amt = amt*10 + (k-'0');
			}
			if (amt > max) amt = max;
		} else
		/* Analyze the key */
		switch (k)
		{
			case ESCAPE:
				amt = 0;
				done = TRUE;
				break;

			case '\n':
			case '\r':
				done = TRUE;
				break;

			case 0x7F:
			case '\010':
				amt = amt / 10;
				if (amt < 0) amt = 0;
				break;

			default:
				/* A letter means "all" */
				i = get_keymap_dir(k);
				if (i) {
					if (i == 8) {
						amt += scale;
					} else
					if (i == 2) {
						amt -= scale;
					} else
					if (i == 4) {
						amt -= scale*10;
					} else
					if (i == 6) {
						amt += scale*10;
					} else
					if ((i == 7) || (i == 9)) {
						amt = max;
					} else
					if ((i == 1) || (i == 3)) {
						amt = 1;
					}
					first = FALSE;
					if (amt > max) amt = max;
					if (amt < 0) amt = 0;
				}
				if (isprint(k)) {
					amt = max;
					first = FALSE;
				} else {
					bell("Illegal edit key!");
				}
				break;
		}

		/* Update the entry */
		Term_erase(x, y, 10);
		put_fstr(x, y, "%d", amt);
	}

	/* Clear prompt */
	clear_msg();

	if (amt == 0) return (0);

	/* Enforce the maximum */
	if (amt > max) amt = max;

	/* Enforce the minimum */
	if (amt < 0) amt = 0;

	if (amt) repeat_push(amt);

	/* Return the result */
	return ((s32b)amt);
}

s16b get_quantity(cptr prompt, s16b initial, s16b max)
{
	/* see if we have a ui override */
	if (Term->get_quantity_hook) return (*(Term->get_quantity_hook))(prompt, initial, max);

	return ((s16b)(get_quantity_big)(prompt, initial, max));
}

/*
 * Request a "number" from the user
 */
u32b get_number(cptr prompt, u32b initial)
{
	u32b amt;

	char tmp[80];

	char buf[80];

	/* Build a prompt if needed */
	if (!prompt)
	{
		/* Build a prompt */
		strnfmt(tmp, 80, "Enter a number: ");

		/* Use that prompt */
		prompt = tmp;
	}

	/* see if we have a ui override */
	if (Term->get_number_hook) return (*(Term->get_number_hook))(prompt, initial);


	/* Default to zero */
	amt = initial;

	/* Build the default */
	strnfmt(buf, 80, "%d", amt);

	/* Ask for a quantity */
	if (!get_string(buf, 20, prompt)) return (0);

	/* Extract a number */
	amt = (u32b) atol(buf);

	if (amt) repeat_push(amt);

	/* Return the result */
	return (amt);
}

/*
 * Pause for user response XXX XXX XXX
 */
void pause_line(int row)
{
	prtf(0, row, "");
	put_fstr(23, row, "[Press any key to continue]");
	inkey();
	prtf(0, row, "");
}


/*
 * GH
 * Called from cmd4.c and a few other places. Just extracts
 * a direction from the keymap for ch (the last direction,
 * in fact) byte or char here? I'm thinking that keymaps should
 * generally only apply to single keys, which makes it no more
 * than 128, so a char should suffice... but keymap_act is 256...
 */
int get_keymap_dir(char ch)
{
	cptr act, s;
	int d = 0;

	if (isdigit(ch))
	{
		d = D2I(ch);
	}
	else
	{
		if (rogue_like_commands)
		{
			act = keymap_act[KEYMAP_MODE_ROGUE][(byte)ch];
		}
		else
		{
			act = keymap_act[KEYMAP_MODE_ORIG][(byte)ch];
		}

		if (act)
		{
			/* Convert to a direction */
			for (s = act; *s; ++s)
			{
				/* Use any digits in keymap */
				if (isdigit(*s)) d = D2I(*s);
			}
		}
	}
	return d;
}

