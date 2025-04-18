/* File: main-win.c */

/*
 * Copyright (c) 1997 Ben Harrison, Skirmantas Kligys, Robert Ruehlmann,
 * and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with Windows computers.
 *
 * To use this file, use an appropriate "Makefile" or "Project File",
 * make sure that "WINDOWS" and/or "WIN32" are defined somewhere, and
 * make sure to obtain various extra files as described below.
 *
 * The Windows version has been tested to compile with Visual C++ 5.0
 * and 6.0, Cygwin 1.0, Borland C++ 5.5 command line tools, and lcc-win32.
 *
 *
 * See also "main-dos.c" and "main-ibm.c".
 *
 *
 * The "lib/pref/pref-win.prf" file contains keymaps, macro definitions,
 * and/or color redefinitions.
 *
 * The "lib/pref/font-win.prf" contains attr/char mappings for use with the
 * normal "*.fon" font files in the "lib/xtra/font/" directory.
 *
 * The "lib/pref/graf-win.prf" contains attr/char mappings for use with the
 * special "*.bmp" bitmap files in the "lib/xtra/graf/" directory, which
 * are activated by a menu item.
 *
 *
 * Compiling this file, and using the resulting executable, requires
 * several extra files not distributed with the standard Angband code.
 * If "USE_GRAPHICS" is defined, then "readdib.h" and "readdib.c" must
 * be placed into "src/", and the "8x8.bmp" bitmap file must be placed
 * into "lib/xtra/graf".  In any case, some "*.fon" files (including
 * "8X13.FON" if nothing else) must be placed into "lib/xtra/font/".
 * If "USE_SOUND" is defined, then some special library (for example,
 * "winmm.lib") may need to be linked in, and desired "*.WAV" sound
 * files must be placed into "lib/xtra/sound/".  All of these extra
 * files can be found in the "ext-win" archive.
 *
 *
 * The "Term_xtra_win_clear()" function should probably do a low-level
 * clear of the current window, and redraw the borders and other things,
 * if only for efficiency.  XXX XXX XXX
 *
 * A simpler method is needed for selecting the "tile size" for windows.
 * XXX XXX XXX
 *
 * Special "Windows Help Files" can be placed into "lib/xtra/help/" for
 * use with the "winhelp.exe" program.  These files *may* be available
 * at the ftp site somewhere, but I have not seen them.  XXX XXX XXX
 *
 * ToDo: The screensaver mode should implement ScreenSaverConfigureDialog,
 * DefScreenSaverProc, and ScreenSaverProc.
 *
 * Initial framework (and most code) by Ben Harrison (benh@phial.com).
 *
 * Original code by Skirmantas Kligys (kligys@scf.usc.edu).
 *
 * Additional code by Ross E Becker (beckerr@cis.ohio-state.edu),
 * and Chris R. Martin (crm7479@tam2000.tamu.edu).
 *
 * Additional code by Robert Ruehlmann <rr9@angband.org>.
 */

#include "angband.h"

#include "maid-grf.h"

#include "settings.h"
#include "grafmode.h"
#include "win-menu.h"
#include "button.h"

#ifdef WINDOWS


/*
 * Use HTML-Help.
 */
/* #define HTML_HELP */

#ifdef HTML_HELP
# define HELP_GENERAL "angband.chm"
# define HELP_SPOILERS "angband.chm"
#else /* HTML_HELP */
# define HELP_GENERAL "angband.hlp"
# define HELP_SPOILERS "spoilers.hlp"
#endif /* HTML_HELP */


/*
 * Extract the "WIN32" flag from the compiler
 */
#if defined(__WIN32__) || defined(__WINNT__) || defined(__NT__)
# ifndef WIN32
#  define WIN32
# endif
#endif


#ifdef ALLOW_BORG

/*
 * Hack -- allow use of "screen saver" mode
 */
/* #define USE_SAVER */

#endif /* ALLOW_BORG */

/*
 * This may need to be removed for some compilers XXX XXX XXX
 */
#define STRICT

/*
 * Exclude parts of WINDOWS.H that are not needed
 */
#define NOCOMM            /* Comm driver APIs and definitions */
#define NOLOGERROR        /* LogError() and related definitions */
#define NOPROFILER        /* Profiler APIs */
#define NOLFILEIO         /* _l* file I/O routines */
#define NOOPENFILE        /* OpenFile and related definitions */
#define NORESOURCE        /* Resource management */
#define NOATOM            /* Atom management */
#define NOLANGUAGE        /* Character test routines */
#define NOLSTRING         /* lstr* string management routines */
#define NODBCS            /* Double-byte character set routines */
#define NOKEYBOARDINFO    /* Keyboard driver routines */
#define NOCOLOR           /* COLOR_* color values */
#define NODRAWTEXT        /* DrawText() and related definitions */
#define NOSCALABLEFONT    /* Truetype scalable font support */
#define NOMETAFILE        /* Metafile support */
#define NOSYSTEMPARAMSINFO /* SystemParametersInfo() and SPI_* definitions */
#define NODEFERWINDOWPOS  /* DeferWindowPos and related definitions */
#define NOKEYSTATES       /* MK_* message key state flags */
#define NOWH              /* SetWindowsHook and related WH_* definitions */
#define NOCLIPBOARD       /* Clipboard APIs and definitions */
#define NOICONS           /* IDI_* icon IDs */
#define NOMDI             /* MDI support */
#define NOHELP            /* Help support */

/* Not defined since it breaks Borland C++ 5.5 */
/* #define NOCTLMGR */    /* Control management and controls */

/*
 * Exclude parts of WINDOWS.H that are not needed (Win32)
 */
#define WIN32_LEAN_AND_MEAN
#define NONLS             /* All NLS defines and routines */
#define NOSERVICE         /* All Service Controller routines, SERVICE_ equates, etc. */
#define NOKANJI           /* Kanji support stuff. */
#define NOMCX             /* Modem Configuration Extensions */


/* Mega-hack, these include files require double and float to work */
#undef float
#undef double

/*
 * Include the "windows" support file
 */
#include <windows.h>
#include <windowsx.h>


#ifndef WM_MOUSEWHEEL
// copied from winuser.h
#define WM_MOUSEWHEEL                   0x020A
#endif

#ifdef USE_SOUND

/*
 * Exclude parts of MMSYSTEM.H that are not needed
 */
#define MMNODRV          /* Installable driver support */
#define MMNOWAVE         /* Waveform support */
#define MMNOMIDI         /* MIDI support */
#define MMNOAUX          /* Auxiliary audio support */
#define MMNOTIMER        /* Timer support */
#define MMNOJOY          /* Joystick support */
#define MMNOMCI          /* MCI support */
#define MMNOMMIO         /* Multimedia file I/O support */
#define MMNOMMSYSTEM     /* General MMSYSTEM functions */

#include <mmsystem.h>

#endif /* USE_SOUND */

#include <commdlg.h>

/*
 * HTML-Help requires htmlhelp.h and htmlhelp.lib from Microsoft's
 * HTML Workshop < msdn.microsoft.com/workshop/author/htmlhelp/ >.
 */
#ifdef HTML_HELP
#include <htmlhelp.h>
#endif /* HTML_HELP */

/*
 * Include the support for loading bitmaps
 */
#ifdef USE_GRAPHICS
# include "readdib.h"
#endif /* USE_GRAPHICS */

/*
 * Hack -- Fake declarations from "dos.h" XXX XXX XXX
 */
#ifdef WIN32
#define INVALID_FILE_NAME (DWORD)0xFFFFFFFF
#else /* WIN32 */
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
unsigned _cdecl _dos_getfileattr(const char *, unsigned *);
#endif /* WIN32 */

/* Mega-hack redefine them again */
#undef float
#define float floating_point_is_not_allowed
#undef double
#define double floating_point_is_not_allowed

/*
 * Silliness in WIN32 drawing routine
 */
#ifdef WIN32
# define MoveTo(H,X,Y) MoveToEx(H, X, Y, NULL)
#endif /* WIN32 */

/*
 * Silliness for Windows 95
 */
#ifndef WS_EX_TOOLWINDOW
# define WS_EX_TOOLWINDOW 0
#endif /* WS_EX_TOOLWINDOW */

/*
 * Foreground color bits (hard-coded by DOS)
 */
#define VID_BLACK	0x00
#define VID_BLUE	0x01
#define VID_GREEN	0x02
#define VID_CYAN	0x03
#define VID_RED		0x04
#define VID_MAGENTA	0x05
#define VID_YELLOW	0x06
#define VID_WHITE	0x07

/*
 * Bright text (hard-coded by DOS)
 */
#define VID_BRIGHT	0x08

/*
 * Background color bits (hard-coded by DOS)
 */
#define VUD_BLACK	0x00
#define VUD_BLUE	0x10
#define VUD_GREEN	0x20
#define VUD_CYAN	0x30
#define VUD_RED		0x40
#define VUD_MAGENTA	0x50
#define VUD_YELLOW	0x60
#define VUD_WHITE	0x70

/*
 * Blinking text (hard-coded by DOS)
 */
#define VUD_BRIGHT	0x80


/*
 * Extra "term" data
 *
 * Note the use of "font_want" for the names of the font file requested by
 * the user, and the use of "font_file" for the currently active font file.
 *
 * The "font_file" is uppercased, and takes the form "8X13.FON", while
 * "font_want" can be in almost any form as long as it could be construed
 * as attempting to represent the name of a font.
 */
#include "win-term.h"


/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];

/*
 * Hack -- global "window creation" pointer
 */
static term_data *my_td;

/*
 * Default window layout function
 */
int default_layout_win(term_data *data, int maxterms);

/*
 * game in progress
 */
bool game_in_progress = FALSE;

/*
 * note when "open"/"new" become valid
 */
bool initialized = FALSE;

/*
 * screen paletted, i.e. 256 colors
 */
bool paletted = FALSE;

/*
 * 16 colors screen, don't use RGB()
 */
bool colors16 = FALSE;

static bool low_priority = FALSE;

/*
 * Saved instance handle
 */
static HINSTANCE hInstance;

/*
 * Yellow brush for the cursor
 */
static HBRUSH hbrYellow;

/*
 * An icon
 */
static HICON hIcon;

/*
 * A palette
 */
static HPALETTE hPal;


#ifdef USE_SAVER

/*
 * The screen saver window
 */
static HWND hwndSaver;

static bool screensaver = FALSE;
static bool screensaver_active = FALSE;

static HANDLE screensaverSemaphore;

static char saverfilename[1024];

static HMENU main_menu;

#define MOUSE_SENS 10

#endif /* USE_SAVER */


#ifdef USE_GRAPHICS

/*
 * Flag set once "graphics" has been initialized
 */
static bool can_use_graphics = FALSE;

/*
 * The global bitmap
 */
static DIBINIT infGraph;

/*
 * The global bitmap mask
 */
static DIBINIT infMask;

static int overdraw = 0;
static int overdrawmax = -1;

static int alphablend = 0;
static BLENDFUNCTION blendfn;

#endif /* USE_GRAPHICS */


#ifdef USE_SOUND

/*
 * Flag set once "sound" has been initialized
 */
static bool can_use_sound = FALSE;

#define SAMPLE_MAX 8

/*
 * An array of sound file names
 */
static cptr sound_file[SOUND_MAX][SAMPLE_MAX];

#endif /* USE_SOUND */


/*
 * Full path to ANGBAND.INI
 */
static cptr ini_file = NULL;

/*
 * Name of application
 */
static cptr AppName = VERSION_NAME;

/*
 * Name of sub-window type
 */
static cptr AngList = "AngList";

/*
 * Directory names
 */
//static cptr ANGBAND_DIR_XTRA_FONT;
//static cptr ANGBAND_DIR_XTRA_GRAF;
//static cptr ANGBAND_DIR_XTRA_SOUND;
static cptr ANGBAND_DIR_XTRA_HELP;
#if USE_MUSIC
static cptr ANGBAND_DIR_XTRA_MUSIC;
#endif /* USE_MUSIC */

/*
 * The "complex" color values
 */
static COLORREF win_clr[256];


/*
 * The "simple" color values
 *
 * See "main-ibm.c" for original table information
 *
 * The entries below are taken from the "color bits" defined above.
 *
 * Note that many of the choices below suck, but so do crappy monitors.
 */
static byte win_pal[256] =
{
	VID_BLACK,					/* Dark */
	VID_WHITE,					/* White */
	VID_CYAN,					/* Slate XXX */
	VID_RED | VID_BRIGHT,		/* Orange XXX */
	VID_RED,					/* Red */
	VID_GREEN,					/* Green */
	VID_BLUE,					/* Blue */
	VID_YELLOW,					/* Umber XXX */
	VID_BLACK | VID_BRIGHT,		/* Light Dark */
	VID_CYAN | VID_BRIGHT,		/* Light Slate XXX */
	VID_MAGENTA,				/* Violet XXX */
	VID_YELLOW | VID_BRIGHT,	/* Yellow */
	VID_MAGENTA | VID_BRIGHT,	/* Light Red XXX */
	VID_GREEN | VID_BRIGHT,		/* Light Green */
	VID_BLUE | VID_BRIGHT,		/* Light Blue */
	VID_YELLOW					/* Light Umber XXX */
};


#ifdef SUPPORT_GAMMA
static int gamma_correction;
#endif /* SUPPORT_GAMMA */


/*
 * Hack -- define which keys are "special"
 */
static bool special_key[256];

/*
 * Hack -- initialization list for "special_key"
 *
 * We ignore the modifier keys (shift, control, alt, num lock, scroll lock),
 * and the normal keys (escape, tab, return, letters, numbers, etc), but we
 * catch the keypad keys (with and without numlock set, including keypad 5),
 * the function keys (including the "menu" key which maps to F10), and the
 * "pause" key (between scroll lock and numlock).  We also catch a few odd
 * keys which I do not recognize, but which are listed among keys which we
 * do catch, so they should be harmless to catch.
 */
static const byte special_key_list[] =
{
	VK_CLEAR,		/* 0x0C (KP<5>) */

	VK_PAUSE,		/* 0x13 (pause) */

	VK_PRIOR,		/* 0x21 (KP<9>) */
	VK_NEXT,		/* 0x22 (KP<3>) */
	VK_END,			/* 0x23 (KP<1>) */
	VK_HOME,		/* 0x24 (KP<7>) */
	VK_LEFT,		/* 0x25 (KP<4>) */
	VK_UP,			/* 0x26 (KP<8>) */
	VK_RIGHT,		/* 0x27 (KP<6>) */
	VK_DOWN,		/* 0x28 (KP<2>) */
	VK_SELECT,		/* 0x29 (?) */
	VK_PRINT,		/* 0x2A (?) */
	VK_EXECUTE,		/* 0x2B (?) */
	VK_SNAPSHOT,	/* 0x2C (?) */
	VK_INSERT,		/* 0x2D (KP<0>) */
	VK_DELETE,		/* 0x2E (KP<.>) */
	VK_HELP,		/* 0x2F (?) */

#if 0
	VK_NUMPAD0,		/* 0x60 (KP<0>) */
	VK_NUMPAD1,		/* 0x61 (KP<1>) */
	VK_NUMPAD2,		/* 0x62 (KP<2>) */
	VK_NUMPAD3,		/* 0x63 (KP<3>) */
	VK_NUMPAD4,		/* 0x64 (KP<4>) */
	VK_NUMPAD5,		/* 0x65 (KP<5>) */
	VK_NUMPAD6,		/* 0x66 (KP<6>) */
	VK_NUMPAD7,		/* 0x67 (KP<7>) */
	VK_NUMPAD8,		/* 0x68 (KP<8>) */
	VK_NUMPAD9,		/* 0x69 (KP<9>) */
	VK_MULTIPLY,	/* 0x6A (KP<*>) */
	VK_ADD,			/* 0x6B (KP<+>) */
	VK_SEPARATOR,	/* 0x6C (?????) */
	VK_SUBTRACT,	/* 0x6D (KP<->) */
	VK_DECIMAL,		/* 0x6E (KP<.>) */
	VK_DIVIDE,		/* 0x6F (KP</>) */
#endif /* 0 */

	VK_F1,			/* 0x70 */
	VK_F2,			/* 0x71 */
	VK_F3,			/* 0x72 */
	VK_F4,			/* 0x73 */
	VK_F5,			/* 0x74 */
	VK_F6,			/* 0x75 */
	VK_F7,			/* 0x76 */
	VK_F8,			/* 0x77 */
	VK_F9,			/* 0x78 */
	VK_F10,			/* 0x79 */
	VK_F11,			/* 0x7A */
	VK_F12,			/* 0x7B */
	VK_F13,			/* 0x7C */
	VK_F14,			/* 0x7D */
	VK_F15,			/* 0x7E */
	VK_F16,			/* 0x7F */
	VK_F17,			/* 0x80 */
	VK_F18,			/* 0x81 */
	VK_F19,			/* 0x82 */
	VK_F20,			/* 0x83 */
	VK_F21,			/* 0x84 */
	VK_F22,			/* 0x85 */
	VK_F23,			/* 0x86 */
	VK_F24,			/* 0x87 */

	0
};

#if 0
/*
 * Hack -- given a pathname, point at the filename
 */
static cptr extract_file_name(cptr s)
{
	cptr p;

	/* Start at the end */
	p = s + strlen(s) - 1;

	/* Back up to divider */
	while ((p >= s) && (*p != ':') && (*p != '\\')) p--;

	/* Return file name */
	return (p+1);
}
#endif /* 0 */


static void show_win_error(void)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	              NULL, GetLastError(),
	              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	              (LPTSTR) &lpMsgBuf, 0, NULL);

	MessageBox(NULL, lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION);

	LocalFree(lpMsgBuf);
}


/*
 * Hack -- given a simple filename, extract the "font size" info
 *
 * Return a pointer to a static buffer holding the capitalized base name.
 */
static char *analyze_font(char *path, int *wp, int *hp)
{
	int wid, hgt;

	char *s, *p;

	/* Start at the end */
	p = path + strlen(path) - 1;

	/* Back up to divider */
	while ((p >= path) && (*p != ':') && (*p != '\\')) --p;

	/* Advance to file name */
	++p;

	/* Capitalize */
	for (s = p; *s; ++s)
	{
		/* Capitalize (be paranoid) */
		if (islower((unsigned char)*s)) *s = toupper((unsigned char)*s);
	}

	/* Find first 'X' */
	s = strchr(p, 'X');

	/* Extract font width */
	wid = atoi(p);

	/* Extract height */
	hgt = s ? atoi(s+1) : 0;

	/* Save results */
	(*wp) = wid;
	(*hp) = hgt;

	/* Result */
	return (p);
}


/*
 * Check for existance of a file
 */
static bool check_file(cptr s)
{
	char path[1024];

#ifdef WIN32

	DWORD attrib;

#else /* WIN32 */

	unsigned int attrib;

#endif /* WIN32 */

	/* Copy it */
	strnfmt(path, 1024, "%s", s);

#ifdef WIN32

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return (FALSE);

	/* Prohibit directory */
	if (attrib & FILE_ATTRIBUTE_DIRECTORY) return (FALSE);

#else /* WIN32 */

	/* Examine and verify */
	if (_dos_getfileattr(path, &attrib)) return (FALSE);

	/* Prohibit something */
	if (attrib & FA_LABEL) return (FALSE);

	/* Prohibit directory */
	if (attrib & FA_DIREC) return (FALSE);

#endif /* WIN32 */

	/* Success */
	return (TRUE);
}


/*
 * Check for existance of a directory
 */
static bool check_dir(cptr s)
{
	int i;

	char path[1024];

#ifdef WIN32

	DWORD attrib;

#else /* WIN32 */

	unsigned int attrib;

#endif /* WIN32 */

	/* Copy it */
	strcpy(path, s);

	/* Check length */
	i = strlen(path);

	/* Remove trailing backslash */
	if (i && (path[i-1] == '\\')) path[--i] = '\0';

#ifdef WIN32

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return (FALSE);

	/* Require directory */
	if (!(attrib & FILE_ATTRIBUTE_DIRECTORY)) return (FALSE);

#else /* WIN32 */

	/* Examine and verify */
	if (_dos_getfileattr(path, &attrib)) return (FALSE);

	/* Prohibit something */
	if (attrib & FA_LABEL) return (FALSE);

	/* Require directory */
	if (!(attrib & FA_DIREC)) return (FALSE);

#endif /* WIN32 */

	/* Success */
	return (TRUE);
}


/*
 * Validate a file
 */
static void validate_file(cptr s)
{
	/* Verify or fail */
	if (!check_file(s))
	{
		quit_fmt("Cannot find required file:\n%s", s);
	}
}


/*
 * Validate a directory
 */
static void validate_dir(cptr s)
{
	/* Verify or fail */
	if (!check_dir(s))
	{
		quit_fmt("Cannot find required directory:\n%s", s);
	}
}


/*
 * Get the "size" for a window
 */
static void term_getsize(term_data *td)
{
	RECT rc;

	int wid, hgt;

	/* Paranoia */
	if (td->cols < 1) td->cols = 1;
	if (td->rows < 1) td->rows = 1;

	/* Window sizes */
	wid = td->cols * td->tile_wid + td->size_ow1 + td->size_ow2;
	hgt = td->rows * td->tile_hgt + td->size_oh1 + td->size_oh2;

	/* Fake window size */
	rc.left = 0;
	rc.right = rc.left + wid;
	rc.top = 0;
	rc.bottom = rc.top + hgt;

	/* XXX XXX XXX */
	/* rc.right += 1; */
	/* rc.bottom += 1; */

	/* Adjust */
	AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

	/* Total size */
	td->size_wid = rc.right - rc.left;
	td->size_hgt = rc.bottom - rc.top;

	/* See CreateWindowEx */
	if (!td->w) return;

	/* Extract actual location */
	GetWindowRect(td->w, &rc);

	/* Save the location */
	td->pos_x = rc.left;
	td->pos_y = rc.top;
}


/*
 * Write the "prefs" for a single term
 */
static void save_prefs_aux(term_data *td, ini_settings *ini, cptr sec_name)
{
	char buf[256];

	RECT rc;

	WINDOWPLACEMENT lpwndpl;

	/* Paranoia */
	if (!td->w) return;

	/* Visible */
	strncpy(buf, td->visible ? "1" : "0", 256);
	ini_setting_set_string(ini, sec_name, "Visible", buf, 256);

	/* Font */
	strncpy(buf, td->font_file ? td->font_file : "8X13.FON", 256);
	ini_setting_set_string(ini, sec_name, "Font", buf, 256);

	/* Tile size (x) */
	strnfmt(buf, 256, "%d", td->tile_wid);
	ini_setting_set_string(ini, sec_name, "TileWid", buf, 256);

	/* Tile size (y) */
	strnfmt(buf, 256, "%d", td->tile_hgt);
	ini_setting_set_string(ini, sec_name, "TileHgt", buf, 256);

	/* Window size (x) */
	strnfmt(buf, 256, "%d", td->cols);
	ini_setting_set_string(ini, sec_name, "NumCols", buf, 256);

	/* Window size (y) */
	strnfmt(buf, 256, "%d", td->rows);
	ini_setting_set_string(ini, sec_name, "NumRows", buf, 256);

	/* Get window placement and dimensions */
	lpwndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(td->w, &lpwndpl);

	/* Acquire position in *normal* mode (not minimized) */
	rc = lpwndpl.rcNormalPosition;

	/* Get information about the placement of the window */
	if (lpwndpl.flags & SW_SHOWMAXIMIZED)
		td->maximized = TRUE;
	else
		td->maximized = FALSE;

	/* Window position (x) */
	strnfmt(buf, 256, "%d", rc.left);
	ini_setting_set_string(ini, sec_name, "PositionX", buf, 256);

	/* Window position (y) */
	strnfmt(buf, 256, "%d", rc.top);
	ini_setting_set_string(ini, sec_name, "PositionY", buf, 256);

	/* Maximized */
	strcpy(buf, td->maximized ? "1" : "0");
	ini_setting_set_string(ini, sec_name, "Maximized", buf, 256);
}


/*
 * Write the "prefs"
 *
 * We assume that the windows have all been initialized
 */
static void save_prefs(void)/*cptr ini_file)*/
{
	int i;
	ini_settings *ini = NULL;
	char buf[128];

	if (ini_settings_new(&ini) < 0)
		return;

	/* Save the "arg_graphics" flag */
	strnfmt(buf, 128, "%d", arg_graphics);
	ini_setting_set_string(ini, "Angband", "Graphics", buf, 128);

	/* Save the tile width */
	strnfmt(buf, 128, "%d", tile_width_mult);
	ini_setting_set_string(ini, "Angband", "TileWidth", buf, 128);

	/* Save the tile height */
	strnfmt(buf, 128, "%d", tile_height_mult);
	ini_setting_set_string(ini, "Angband", "TileHeight", buf, 128);

	/* Save the "use_bigtile" flag */
	strnfmt(buf, 128, "%d", use_bigtile ? 1 : 0);
	ini_setting_set_string(ini, "Angband", "Bigtile", buf, 128);

	/* Save the "arg_sound" flag */
	strnfmt(buf, 128, "%d", arg_sound ? 1 : 0);
	ini_setting_set_string(ini, "Angband", "Sound", buf, 128);

	/* some optional flags */
	strnfmt(buf, 128, "%d", arg_fiddle ? 1 : 0);
	ini_setting_set_string_def(ini, "Angband", "Fiddle", buf, 128, "0");

	strnfmt(buf, 128, "%d", arg_wizard ? 1 : 0);
	ini_setting_set_string_def(ini, "Angband", "Wizard", buf, 128, "0");

	strnfmt(buf, 128, "%d", arg_force_roguelike ? 1 : 0);
	ini_setting_set_string_def(ini, "Angband", "force_roguelike", buf, 128, "0");

	strnfmt(buf, 128, "%d", arg_force_original ? 1 : 0);
	ini_setting_set_string_def(ini, "Angband", "force_original", buf, 128, "0");

#ifdef SUPPORT_GAMMA
	if (gamma_correction > 0) {
		strnfmt(buf, 128, "%d", gamma_correction);
		ini_setting_set_string(ini, "Angband", "Gamma", buf, 128);
	}
#endif /* SUPPORT_GAMMA */

	/* Save window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		strnfmt(buf, 128, "Term-%d", i);

		save_prefs_aux(td, ini, buf);
	}

	/* actually write the file */
	ini_settings_save(ini_file, ini);

	/* cleanup the ini memory */
	ini_settings_close(&ini);

	/* Success */
	return;
}


/*
 * Load the "prefs" for a single term
 */
static void load_prefs_aux(term_data *td, ini_settings *ini, cptr sec_name)
{
	char tmp[256];

	int wid, hgt;

	/* Visible */
	td->visible = (ini_setting_get_uint32(ini, sec_name, "Visible", td->visible) != 0);

	/* Visible */
	td->maximized = (ini_setting_get_uint32(ini, sec_name, "Maximized", td->maximized) != 0);

	/* Desired font, with default */
	ini_setting_get_string(ini, sec_name, "Font", tmp, 255, "8x13.FON");

	/* Analyze font, save desired font name */
	if (td->font_want) string_free(td->font_want);
	td->font_want = string_make(analyze_font(tmp, &wid, &hgt));

	/* Tile size */
	td->tile_wid = (uint)ini_setting_get_uint32(ini, sec_name, "TileWid", wid);
	td->tile_hgt = (uint)ini_setting_get_uint32(ini, sec_name, "TileHgt", hgt);

	/* Window size */
	td->cols = (byte)ini_setting_get_uint32(ini, sec_name, "NumCols", td->cols);
	td->rows = (byte)ini_setting_get_uint32(ini, sec_name, "NumRows", td->rows);

	/* Window position */
	td->pos_x = (uint)ini_setting_get_uint32(ini, sec_name, "PositionX", td->pos_x);
	td->pos_y = (uint)ini_setting_get_uint32(ini, sec_name, "PositionY", td->pos_y);
}


/*
 * Load the "prefs"
 */
static bool load_prefs(void)/*cptr ini_file)*/
{
	int i;

	char buf[256];
	bool first_start;
	ini_settings *ini = NULL;

	i = ini_settings_load(ini_file, &ini);
	first_start = FALSE;
	if (i<0) {
		if (i == -4) {
			first_start = TRUE;
		} else {
			return FALSE;
		}
	}

	/* Extract the "arg_graphics" flag */
	arg_graphics = (byte)ini_setting_get_uint32(ini, "Angband", "Graphics", GRAPHICS_NONE);
	
	/* Extract the tile width */
	tile_width_mult = (byte)ini_setting_get_uint32(ini, "Angband", "TileWidth", 1);

	/* Extract the tile height */
	tile_height_mult = (byte)ini_setting_get_uint32(ini, "Angband", "TileHeight", 1);

	/* Extract the "use_bigtile" flag */
	use_bigtile = (bool)ini_setting_get_uint32(ini, "Angband", "Bigtile", 0);

	/* Extract the "arg_sound" flag */
	arg_sound = (ini_setting_get_uint32(ini, "Angband", "Sound", 0) != 0);

	/* Extract the "arg_fiddle" flag */
	arg_fiddle = (ini_setting_get_uint32(ini, "Angband", "Fiddle", 0) != 0);

	/* Extract the "arg_wizard" flag */
	arg_wizard = (ini_setting_get_uint32(ini, "Angband", "Wizard", 0) != 0);

	/* Extract the "arg_roguelike" flag */
	arg_force_roguelike = (ini_setting_get_uint32(ini, "Angband", "force_roguelike", 0) != 0);

	/* Extract the "arg_original" flag */
	arg_force_original = (ini_setting_get_uint32(ini, "Angband", "force_original", 0) != 0);

#ifdef SUPPORT_GAMMA

	/* Extract the gamma correction */
	gamma_correction = ini_setting_get_uint32(ini, "Angband", "Gamma", 0);

#endif /* SUPPORT_GAMMA */

	/* Load window prefs */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data *td = &data[i];

		strnfmt(buf, 256, "Term-%d", i);

		load_prefs_aux(td, ini, buf);
	}

	/* cleanup the ini memory */
	ini_settings_close(&ini);

	if (first_start) {
		default_layout_win(data, MAX_TERM_DATA);
	}

	/* Paranoia */
	if (data[0].cols < 80) data[0].cols = 80;
	if (data[0].rows < 24) data[0].rows = 24;
	data[0].visible = 1;

	/* Success */
	if (first_start) {
		return FALSE;
	} else {
		return TRUE;
	}
}


#ifdef USE_SOUND

/*
 * XXX XXX XXX - Taken from files.c.
 *
 * Extract "tokens" from a buffer
 *
 * This function uses "whitespace" as delimiters, and treats any amount of
 * whitespace as a single delimiter.  We will never return any empty tokens.
 * When given an empty buffer, or a buffer containing only "whitespace", we
 * will return no tokens.  We will never extract more than "num" tokens.
 *
 * By running a token through the "text_to_ascii()" function, you can allow
 * that token to include (encoded) whitespace, using "\s" to encode spaces.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 */
static s16b tokenize_whitespace(char *buf, s16b num, char **tokens)
{
	int k = 0;

	char *s = buf;


	/* Process */
	while (k < num)
	{
		char *t;

		/* Skip leading whitespace */
		for ( ; *s && isspace((unsigned char)*s); ++s) /* loop */;

		/* All done */
		if (!*s) break;

		/* Find next whitespace, if any */
		for (t = s; *t && !isspace((unsigned char)*t); ++t) /* loop */;

		/* Nuke and advance (if necessary) */
		if (*t) *t++ = '\0';

		/* Save the token */
		tokens[k++] = s;

		/* Advance */
		s = t;
	}

	/* Count */
	return (k);
}


static void load_sound_prefs(void)
{
	int i, j, num;
	char tmp[1024];
	char ini_path[1024];
	char wav_path[1024];
	char *zz[SAMPLE_MAX];

	/* Access the sound.cfg */
	path_make(ini_path, ANGBAND_DIR_XTRA_SOUND, "sound.cfg");

	for (i = 0; i < SOUND_MAX; i++)
	{
		/* Ignore empty sound strings */
		if (!angband_sound_name[i][0]) continue;
	
		GetPrivateProfileString("Sound", angband_sound_name[i], "", tmp, sizeof(tmp), ini_path);

		num = tokenize_whitespace(tmp, SAMPLE_MAX, zz);

		for (j = 0; j < num; j++)
		{
			/* Access the sound */
			path_make(wav_path, ANGBAND_DIR_XTRA_SOUND, zz[j]);

			/* Save the sound filename, if it exists */
			if (check_file(wav_path))
				sound_file[i][j] = string_make(zz[j]);
		}
	}
}

#endif /* USE_SOUND */


/*
 * Create the new global palette based on the bitmap palette
 * (if any), and the standard 16 entry palette derived from
 * "win_clr[]" which is used for the basic 16 Angband colors.
 *
 * This function is never called before all windows are ready.
 *
 * This function returns FALSE if the new palette could not be
 * prepared, which should normally be a fatal error.  XXX XXX
 *
 * Note that only some machines actually use a "palette".
 */
static int new_palette(void)
{
#ifdef USE_GRAPHICS
	HPALETTE hBmPal;
#endif /* USE_GRAPHICS */
	HPALETTE hNewPal;
	HDC hdc;
	int i, nEntries;
	int pLogPalSize;
	LPLOGPALETTE pLogPal;
	LPPALETTEENTRY lppe;

	term_data *td;


	/* This makes no sense */
	if (!paletted) return (TRUE);

	/* No bitmap */
	lppe = NULL;
	nEntries = 0;

#ifdef USE_GRAPHICS

	/* Check the bitmap palette */
	hBmPal = infGraph.hPalette;

	/* Use the bitmap */
	if (hBmPal)
	{
		lppe = ralloc(256 * sizeof(PALETTEENTRY));
		nEntries = GetPaletteEntries(hBmPal, 0, 255, lppe);
		if ((nEntries == 0) || (nEntries > 220))
		{
			/* Warn the user */
			plog("Please switch to high- or true-color mode.");

			/* Cleanup */
			free(lppe);

			/* Fail */
			return (FALSE);
		}
	}

#endif /* USE_GRAPHICS */

	/* Size of palette */
	pLogPalSize = sizeof(LOGPALETTE) + (nEntries + 16) * sizeof(PALETTEENTRY);

	/* Allocate palette */
	pLogPal = (LPLOGPALETTE)ralloc(pLogPalSize);

	/* Version */
	pLogPal->palVersion = 0x300;

	/* Make room for bitmap and normal data */
	pLogPal->palNumEntries = nEntries + 16;

	/* Save the bitmap data */
	for (i = 0; i < nEntries; i++)
	{
		pLogPal->palPalEntry[i] = lppe[i];
	}

	/* Save the normal data */
	for (i = 0; i < 16; i++)
	{
		LPPALETTEENTRY p;

		/* Access the entry */
		p = &(pLogPal->palPalEntry[i+nEntries]);

		/* Save the colors */
		p->peRed = GetRValue(win_clr[i]);
		p->peGreen = GetGValue(win_clr[i]);
		p->peBlue = GetBValue(win_clr[i]);

#ifdef SUPPORT_GAMMA

		if (gamma_correction > 0)
		{
			p->peRed = gamma_table[p->peRed];
			p->peGreen = gamma_table[p->peGreen];
			p->peBlue = gamma_table[p->peBlue];
		}

#endif /* SUPPORT_GAMMA */

		/* Save the flags */
		p->peFlags = PC_NOCOLLAPSE;
	}

	/* Free something */
	if (lppe) free(lppe);

	/* Create a new palette, or fail */
	hNewPal = CreatePalette(pLogPal);
	if (!hNewPal) quit("Cannot create palette!");

	/* Free the palette */
	free(pLogPal);

	/* Main window */
	td = &data[0];

	/* Realize the palette */
	hdc = GetDC(td->w);
	SelectPalette(hdc, hNewPal, 0);
	i = RealizePalette(hdc);
	ReleaseDC(td->w, hdc);
	if (i == 0) quit("Cannot realize palette!");

	/* Sub-windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];

		hdc = GetDC(td->w);
		SelectPalette(hdc, hNewPal, 0);
		ReleaseDC(td->w, hdc);
	}

	/* Delete old palette */
	if (hPal) DeleteObject(hPal);

	/* Save new palette */
	hPal = hNewPal;

	/* Success */
	return (TRUE);
}


#ifdef USE_GRAPHICS
extern BOOL ReadDIB_DX9(HWND hWnd, LPSTR lpFileName, DIBINIT *pInfo);
extern BOOL ReadDIB2_DX9(HWND hWnd, LPSTR lpFileName, DIBINIT *pInfo, DIBINIT *pMask);
/*
 * Initialize graphics
 */
static bool init_graphics(void)
{
	/* Initialize once */
	/* if (can_use_graphics != arg_graphics) */
	{
		char buf[1024];
		int wid, hgt;
		cptr name;
		cptr mask = NULL;

		current_graphics_mode = get_graphics_mode(arg_graphics);
		if (current_graphics_mode) {
			wid = current_graphics_mode->cell_width;
			hgt = current_graphics_mode->cell_height;
			name = current_graphics_mode->file;
			use_transparency = TRUE;
		} else {
			wid = 8;
			hgt = 8;
			name = "8x8.png";
			use_transparency = TRUE;
		}

		/* Try loading from the user directory first */
		path_build(buf, 1024, ANGBAND_DIR_USER, name);

		if (!file_exists(buf)) {
			/* Access the bitmap file */
			path_build(buf, 1024, ANGBAND_DIR_XTRA_GRAF, name);
		}

		/* Load the bitmap or quit */
		//if (!ReadDIB(data[0].w, buf, &infGraph))
		//if (!ReadDIB_DX9(data[0].w, buf, &infGraph))
		if (mask) {
			if (!ReadDIB_DX9(data[0].w, buf, &infGraph)) {
				plog_fmt("Cannot read bitmap file '%s'", name);
				return (FALSE);
			}
			
			/* Access the mask file */
			path_build(buf, sizeof(buf), ANGBAND_DIR_XTRA_GRAF, mask);

			/* Load the bitmap or quit */
			if (!ReadDIB_DX9(data[0].w, buf, &infMask))
			{
				plog_fmt("Cannot read bitmap file '%s'", buf);
				return (FALSE);
			}
		} else {
			if (!ReadDIB2_DX9(data[0].w, buf, &infGraph, &infMask)) {
				plog_fmt("Cannot read bitmap file '%s'", name);
				return (FALSE);
			}
		}

		/* Save the new sizes */
		infGraph.CellWidth = wid;
		infGraph.CellHeight = hgt;

		/* Activate a palette */
		if (!new_palette())
		{
			/* Free bitmap XXX XXX XXX */

			/* Oops */
			plog("Cannot activate palette!");
			return (FALSE);
		}

		/* Graphics available */
		can_use_graphics = arg_graphics;
	}

	/* Result */
	return (can_use_graphics);
}
#endif /* USE_GRAPHICS */


#ifdef USE_SOUND
/*
 * Initialize sound
 */
static bool init_sound(void)
{
	/* Initialize once */
	if (!can_use_sound)
	{
		/* Load the prefs */
		load_sound_prefs();

		/* Sound available */
		can_use_sound = TRUE;
	}

	/* Result */
	return (can_use_sound);
}
#endif /* USE_SOUND */


/*
 * Resize a window
 */
static void term_window_resize(const term_data *td)
{
	/* Require window */
	if (!td->w) return;

	/* Resize the window */
	SetWindowPos(td->w, 0, 0, 0,
	             td->size_wid, td->size_hgt,
	             SWP_NOMOVE | SWP_NOZORDER);

	/* Redraw later */
	InvalidateRect(td->w, NULL, TRUE);
}


/*
 * Force the use of a new "font file" for a term_data
 *
 * This function may be called before the "window" is ready
 *
 * This function returns zero only if everything succeeds.
 *
 * Note that the "font name" must be capitalized!!!
 */
static errr term_force_font(term_data *td, cptr path)
{
	int i;

	int wid, hgt;

	char *base;

	char buf[1024];


	/* Forget the old font (if needed) */
	if (td->font_id) DeleteObject(td->font_id);

	/* Forget old font */
	if (td->font_file)
	{
		bool used = FALSE;

		/* Scan windows */
		for (i = 0; i < MAX_TERM_DATA; i++)
		{
			/* Don't check when closing the application */
			if (!path) break;

			/* Check "screen" */
			if ((td != &data[i]) &&
			    (data[i].font_file) &&
			    (streq(data[i].font_file, td->font_file)))
			{
				used = TRUE;
			}
		}

		/* Remove unused font resources */
		if (!used) RemoveFontResource(td->font_file);

		/* Free the old name */
		string_free(td->font_file);

		/* Forget it */
		td->font_file = NULL;
	}


	/* No path given */
	if (!path) return (1);


	/* Local copy */
	strnfmt(buf, sizeof(buf), "%s", path);

	/* Analyze font path */
	base = analyze_font(buf, &wid, &hgt);

	/* Verify suffix */
	if (!suffix(base, ".FON")) return (1);

	/* Verify file */
	if (!check_file(buf)) return (1);

	/* Load the new font */
	if (!AddFontResource(buf)) return (1);

	/* Save new font name */
	td->font_file = string_make(base);

	/* Remove the "suffix" */
	base[strlen(base)-4] = '\0';

	/* Create the font (using the 'base' of the font file name!) */
	td->font_id = CreateFont(hgt, wid, 0, 0, FW_DONTCARE, 0, 0, 0,
	                         ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	                         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                         FIXED_PITCH | FF_DONTCARE, base);

	/* Hack -- Unknown size */
	if (!wid || !hgt)
	{
		HDC hdcDesktop;
		HFONT hfOld;
		TEXTMETRIC tm;

		/* all this trouble to get the cell size */
		hdcDesktop = GetDC(HWND_DESKTOP);
		hfOld = SelectObject(hdcDesktop, td->font_id);
		GetTextMetrics(hdcDesktop, &tm);
		SelectObject(hdcDesktop, hfOld);
		ReleaseDC(HWND_DESKTOP, hdcDesktop);

		/* Font size info */
		wid = tm.tmAveCharWidth;
		hgt = tm.tmHeight;
	}

	/* Save the size info */
	td->font_wid = wid;
	td->font_hgt = hgt;

	/* Success */
	return (0);
}



/*
 * Allow the user to change the font for this window.
 */
static void term_change_font(term_data *td)
{
	OPENFILENAME ofn;

	char tmp[1024] = "";

	/* Extract a default if possible */
	if (td->font_file) strcpy(tmp, td->font_file);

	/* Ask for a choice */
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = data[0].w;
	ofn.lpstrFilter = "Angband Font Files (*.fon)\0*.fon\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = tmp;
	ofn.nMaxFile = 128;
	ofn.lpstrInitialDir = ANGBAND_DIR_XTRA_FONT;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "fon";

	/* Force choice if legal */
	if (GetOpenFileName(&ofn))
	{
		/* Force the font */
		if (term_force_font(td, tmp))
		{
			/* Access the standard font file */
			path_make(tmp, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

			/* Force the use of that font */
			(void)term_force_font(td, tmp);
		}

		/* Reset the tile info */
		td->tile_wid = td->font_wid;
		td->tile_hgt = td->font_hgt;

		/* Analyze the font */
		term_getsize(td);

		/* Resize the window */
		term_window_resize(td);
	}
}


static void windows_map_aux(void);


/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
	if (td->map_active)
	{
		/* Redraw the map */
		windows_map_aux();
	}
	else
	{
		/* Activate the term */
		Term_activate(&td->t);

		/* Redraw the contents */
		Term_redraw();

		/* Restore the term */
		Term_activate(term_screen);
	}
}


/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw_section(term_data *td, int x1, int y1, int x2, int y2)
{
	/* Activate the term */
	Term_activate(&td->t);

	/* Redraw the area */
	Term_redraw_section(x1, y1, x2, y2);

	/* Restore the term */
	Term_activate(term_screen);
}



/*** Function hooks needed by "Term" ***/


#if 0

/*
 * Initialize a new Term
 */
static void Term_init_win(term *t)
{
	/* XXX Unused */
}


/*
 * Nuke an old Term
 */
static void Term_nuke_win(term *t)
{
	/* XXX Unused */
}

#endif /* 0 */


/*
 * Interact with the User
 */
static errr Term_user_win(int n)
{
	/* Unused parameter */
	(void)n;

	/* Success */
	return (0);
}


/*
 * React to global changes
 */
static errr Term_xtra_win_react(void)
{
	int i;


	/* Simple color */
	if (colors16)
	{
		/* Save the default colors */
		for (i = 0; i < 256; i++)
		{
			/* Simply accept the desired colors */
			win_pal[i] = angband_color_table[i][0];
		}
	}

	/* Complex color */
	else
	{
		COLORREF code;

		byte rv, gv, bv;

		bool change = FALSE;

		/* Save the default colors */
		for (i = 0; i < 256; i++)
		{
			/* Extract desired values */
			rv = angband_color_table[i][1];
			gv = angband_color_table[i][2];
			bv = angband_color_table[i][3];

#ifdef SUPPORT_GAMMA

			if (gamma_correction > 0)
			{
				rv = gamma_table[rv];
				gv = gamma_table[gv];
				bv = gamma_table[bv];
			}

#endif /* SUPPORT_GAMMA */

			/* Extract a full color code */
			code = PALETTERGB(rv, gv, bv);

			/* Activate changes */
			if (win_clr[i] != code)
			{
				/* Note the change */
				change = TRUE;

				/* Apply the desired color */
				win_clr[i] = code;
			}
		}

		/* Activate the palette if needed */
		if (change) (void)new_palette();
	}


#ifdef USE_SOUND

	/* Handle "arg_sound" */
	if (use_sound != arg_sound)
	{
		/* Initialize (if needed) */
		if (arg_sound && !init_sound())
		{
			/* Warning */
			plog("Cannot initialize sound!");

			/* Cannot enable */
			arg_sound = FALSE;
		}

		/* Change setting */
		use_sound = arg_sound;
	}

#endif /* USE_SOUND */


#ifdef USE_GRAPHICS

	/* Handle "arg_graphics" */
	if (use_graphics != arg_graphics)
	{
		/* Switch off transparency */
		use_transparency = FALSE;
		
		/* Free the bitmap stuff */
		FreeDIB(&infGraph);
		FreeDIB(&infMask);

    /* Initialize (if needed) */
		if (arg_graphics && !init_graphics())
		{
			/* Warning */
			plog("Cannot initialize graphics!");

			/* Cannot enable */
			arg_graphics = GRAPHICS_NONE;
		}
    
    /* Change setting */
		use_graphics = arg_graphics;

		/* Reset visuals */
    #ifdef ANGBAND_2_8_1
		  reset_visuals();
    #else /* ANGBAND_2_8_1 */
		  reset_visuals(TRUE);
    #endif /* ANGBAND_2_8_1 */

#if (0)
    /* Initialize (if needed) */
    /* moved this down here because prefs are set in reset_visuals and I want
     * info loaded there in init_graphics - Brett */
		if (use_graphics && !init_graphics())
		{
			/* Warning */
			plog("Cannot initialize graphics!");

			/* Cannot enable */
			use_graphics = GRAPHICS_NONE;
		  /* Reset visuals */
      #ifdef ANGBAND_2_8_1
		    reset_visuals();
      #else /* ANGBAND_2_8_1 */
		    reset_visuals(TRUE);
      #endif /* ANGBAND_2_8_1 */
		}
#endif
	}

#endif /* USE_GRAPHICS */


	/* Clean up windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term *old = Term;

		term_data *td = &data[i];

		/* Update resized windows */
		if ((td->cols != td->t.wid) || (td->rows != td->t.hgt))
		{
			/* Activate */
			Term_activate(&td->t);

			/* Hack -- Resize the term */
			Term_resize(td->cols, td->rows);

			/* Redraw the contents */
			Term_redraw();

			/* Restore */
			Term_activate(old);
		}
	}


	/* Success */
	return (0);
}


/*
 * Process at least one event
 */
static errr Term_xtra_win_event(int v)
{
	MSG msg;

	/* Wait for an event */
	if (v)
	{
		/* Block */
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* Check for an event */
	else
	{
		/* Check */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* Success */
	return 0;
}


/*
 * Process all pending events
 */
static errr Term_xtra_win_flush(void)
{
	MSG msg;

	/* Process all pending events */
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* Success */
	return (0);
}


/*
 * Hack -- clear the screen
 *
 * Make this more efficient XXX XXX XXX
 */
static errr Term_xtra_win_clear(void)
{
	term_data *td = (term_data*)(Term->data);

	HDC hdc;
	RECT rc;

	/* Rectangle to erase */
	rc.left = td->size_ow1;
	rc.right = rc.left + td->cols * td->tile_wid;
	rc.top = td->size_oh1;
	rc.bottom = rc.top + td->rows * td->tile_hgt;

	/* Erase it */
	hdc = GetDC(td->w);
	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, td->font_id);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/*
 * Hack -- make a noise
 */
static errr Term_xtra_win_noise(void)
{
	MessageBeep(MB_ICONASTERISK);
	return (0);
}


/*
 * Hack -- make a sound
 */
static errr Term_xtra_win_sound(int v)
{
#ifdef USE_SOUND
	int i;
	char buf[1024];
#endif /* USE_SOUND */

	/* Sound disabled */
	if (!use_sound) return (1);

	/* Illegal sound */
	if ((v < 0) || (v >= SOUND_MAX)) return (1);

#ifdef USE_SOUND

	/* Count the samples */
	for (i = 0; i < SAMPLE_MAX; i++)
	{
		if (!sound_file[v][i])
			break;
	}

	/* No sample */
	if (i == 0) return (1);

	/* Build the path */
	path_make(buf, ANGBAND_DIR_XTRA_SOUND, sound_file[v][Rand_simple(i)]);

#ifdef WIN32

	/* Play the sound, catch errors */
        /* HACK: disabling sounds
        return (PlaySound(buf, 0, SND_FILENAME | SND_ASYNC)); */
	/* Oops */
	return (1);

#else /* WIN32 */

	/* Play the sound, catch errors */
	return (sndPlaySound(buf, SND_ASYNC));

#endif /* WIN32 */

#else /* USE_SOUND */

	/* Oops */
	return (1);

#endif /* USE_SOUND */
}


/*
 * Delay for "x" milliseconds
 */
static int Term_xtra_win_delay(int v)
{

#ifdef WIN32

	/* Sleep */
	if (v > 0) Sleep(v);

#else /* WIN32 */

	DWORD t;
	MSG msg;

	/* Final count */
	t = GetTickCount() + v;

	/* Wait for it */
	while (GetTickCount() < t)
	{
		/* Handle messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#endif /* WIN32 */

	/* Success */
	return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_win(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a bell sound */
		case TERM_XTRA_NOISE:
		{
			return (Term_xtra_win_noise());
		}

		/* Make a special sound */
		case TERM_XTRA_SOUND:
		{
			return (Term_xtra_win_sound(v));
		}

		/* Process random events */
		case TERM_XTRA_BORED:
		{
			return (Term_xtra_win_event(0));
		}

		/* Process an event */
		case TERM_XTRA_EVENT:
		{
			return (Term_xtra_win_event(v));
		}

		/* Flush all events */
		case TERM_XTRA_FLUSH:
		{
			return (Term_xtra_win_flush());
		}

		/* React to global changes */
		case TERM_XTRA_REACT:
		{
			return (Term_xtra_win_react());
		}

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
		{
			return (Term_xtra_win_delay(v));
		}
	}

	/* Oops */
	return 1;
}

/*
 * Find the square a particular pixel is part of.
 */
static void pixel_to_square(int *x, int *y, int ox, int oy)
{
	term_data *td = (term_data*)(Term->data);
	
	if (td->map_active) {
		(*x) = (ox - td->size_ow1) / td->map_tile_wid;
		(*y) = (oy - td->size_oh1) / td->map_tile_hgt;
	} else {
		(*x) = (ox - td->size_ow1) / td->tile_wid;
		(*y) = (oy - td->size_oh1) / td->tile_hgt;
	/*
		if ((tile_height_mult > 1)
				&& ((*y) > Term->scr->big_y1)
				&& ((*y) <= Term->scr->big_y2)
				&& ((*x) >= Term->scr->big_x1))
		{
			(*y) = (((*y) - Term->scr->big_y1) / tile_height_mult) + Term->scr->big_y1;
		}
		if ((tile_width_mult > 1)
				&& ((*y) >= Term->scr->big_y1)
				&& ((*y) <= Term->scr->big_y2)
				&& ((*x) > Term->scr->big_x1))
		{
			(*x) = (((*x) - Term->scr->big_x1) / tile_width_mult) + Term->scr->big_x1;
		}*/
	}
}

/*
 * Find the pixel at the top-left corner of a square.
 */
static void square_to_pixel(int *x, int *y, int ox, int oy)
{
	term_data *td = (term_data*)(Term->data);
	
	if (td->map_active) {
		(*x) = ox * td->map_tile_wid + td->size_ow1;
		(*y) = oy * td->map_tile_hgt + td->size_oh1;
	} else
	if (is_bigtiled(ox, oy)) {
		(*x) = ox * td->tile_wid * tile_width_mult + td->size_ow1;
		(*y) = oy * td->tile_hgt * tile_height_mult + td->size_oh1;
	} else 
	{
		(*x) = ox * td->tile_wid + td->size_ow1;
		(*y) = oy * td->tile_hgt + td->size_oh1;
	}
	/*{
		if ((tile_width_mult > 1)
				&& (oy >= Term->scr->big_y1)
				&& (oy <= Term->scr->big_y2)
				&& (ox > Term->scr->big_x1))
		{
			(*x) = Term->scr->big_x1 * td->tile_wid + td->size_ow1;
			(*x) += (ox-Term->scr->big_x1) * td->tile_wid * tile_width_mult;
		} else {
			(*x) = ox * td->tile_wid + td->size_ow1;
		}
		if ((tile_height_mult > 1)
				&& (oy > Term->scr->big_y1)
				&& (oy <= Term->scr->big_y2)
				&& (ox >= Term->scr->big_x1))
		{
	 		(*y) = Term->scr->big_y1 * td->tile_hgt + td->size_oh1;
			(*y) += (oy-Term->scr->big_y1) * td->tile_hgt * tile_height_mult;
			// (*y) = (Term->scr->big_y1 + (oy-Term->scr->big_y1) * tile_height_mult);
			// (*y) = (*y) * td->tile_hgt + td->size_oh1;
		} else {
			(*y) = oy * td->tile_hgt + td->size_oh1;
		}
	}*/
}

/*
 * Low level graphics (Assumes valid input).
 *
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
static errr Term_curs_win(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	RECT rc;
	HDC hdc;

	int tile_wid, tile_hgt;
	
	if (td->map_active) {
		tile_wid = td->map_tile_wid;
		tile_hgt = td->map_tile_hgt;
	} else {
		tile_wid = td->tile_wid;
		tile_hgt = td->tile_hgt;
	}

	/* Frame the grid */
	rc.left = x * tile_wid + td->size_ow1;
	rc.top = y * tile_hgt + td->size_oh1;
	if (is_bigtiled(x, y)) {
		rc.right = rc.left + tile_wid * tile_width_mult;
		rc.bottom = rc.top + tile_hgt * tile_height_mult;
	} else {
		rc.right = rc.left + tile_wid;
		rc.bottom = rc.top + tile_hgt;
	}

	/* Cursor is done as a yellow "box" */
	hdc = GetDC(td->w);
	FrameRect(hdc, &rc, hbrYellow);
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}

/*
 *
 * Erase a "block" of "n" characters starting at (x,y).
 */
static errr Term_wipe_win(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	HDC hdc;
	RECT rc;
	
	/* Find the dimensions of the rectangle to erase in client coords */
	rc.left = x * td->tile_wid + td->size_ow1;
	rc.top = y * td->tile_hgt + td->size_oh1;
	rc.right = (x+n) * td->tile_wid + td->size_ow1;
	rc.bottom = (y+1) * td->tile_hgt + td->size_oh1;

	if ((rc.right > rc.left) && (rc.bottom > rc.top)) {
	hdc = GetDC(td->w);
	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, td->font_id);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	ReleaseDC(td->w, hdc);
	}
	/* Success */
	return 0;
}


/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * All "graphic" data is handled by "Term_pict_win()", below.
 *
 * One would think there is a more efficient method for telling a window
 * what color it should be using to draw with, but perhaps simply changing
 * it every time is not too inefficient.  XXX XXX XXX
 */
static errr Term_text_win(int cx, int cy, int n, byte a, cptr s)
{
	term_data *td = (term_data*)(Term->data);
	RECT rc;
	HDC hdc;

	int i;
	
	int x, y;

	/* Acquire DC */
	hdc = GetDC(td->w);

	/* Background color */
	SetBkColor(hdc, RGB(0, 0, 0));

	/* Foreground color */
	if (colors16) {
		SetTextColor(hdc, PALETTEINDEX(win_pal[a]));
	} else
	if (paletted) {
		SetTextColor(hdc, win_clr[a&0x0F]);
	} else
	{
		SetTextColor(hdc, win_clr[a]);
	}

	/* Use the font */
	SelectObject(hdc, td->font_id);

	/* Erase complete rectangle */
	Term_wipe_win(cx, cy, n);
	
	/* Get top left corner */
	//square_to_pixel(&x, &y, cx, cy);
	x = cx * td->tile_wid + td->size_ow1;
	y = cy * td->tile_hgt + td->size_oh1;

	rc.top = y + ((td->tile_hgt - td->font_hgt) / 2);
	rc.bottom = rc.top + td->font_hgt;

	/* Dump each character */
	for (i = 0; i < n; i++) {
		if (is_bigtiled(cx + i, cy)) {
			rc.left = x + ((td->tile_wid - td->font_wid) / 2);
			rc.right = rc.left + td->font_wid;
			
			/* Dump the text */
			ExtTextOut(hdc, rc.left, rc.top, 0, &rc,
						s+i, 1, NULL);

			/* Advance */
			x += td->tile_wid * tile_width_mult;
		} else {
			rc.left = x + ((td->tile_wid - td->font_wid) / 2);
			rc.right = rc.left + td->font_wid;
			
			/* Dump the text */
			ExtTextOut(hdc, rc.left, rc.top, 0, &rc,
						s+i, 1, NULL);

			/* Advance */
			x += td->tile_wid;
		}
	}

	/* Release DC */
	ReleaseDC(td->w, hdc);

	/* Success */
	return 0;
}


/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw an array of "special" attr/char pairs at the given location.
 *
 * We use the "Term_pict_win()" function for "graphic" data, which are
 * encoded by setting the "high-bits" of both the "attr" and the "char"
 * data.  We use the "attr" to represent the "row" of the main bitmap,
 * and the "char" to represent the "col" of the main bitmap.  The use
 * of this function is induced by the "higher_pict" flag.
 *
 * If "graphics" is not available, we simply "wipe" the given grids.
 */
static errr Term_pict_win(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
{
	term_data *td = (term_data*)(Term->data);

#ifdef USE_GRAPHICS

	int i;
	int x1, y1, w1, h1;
	int x2, y2, w2, h2, tw2, th2;
	int x3, y3;

	HDC hdcMask;
	HDC hdc;
	HDC hdcSrc;
	HBITMAP hbmSrcOld;

	/* Paranoia */
	if (!use_graphics) {
		/* Erase the grids */
		return (Term_wipe_win(x, y, n));
	}

	/* Size of bitmap cell */
	w1 = infGraph.CellWidth;
	h1 = infGraph.CellHeight;

	/* Size of window cell */
	if (td->map_active) {
		w2 = td->map_tile_wid;
		h2 = td->map_tile_hgt;
		tw2 = w2;
		th2 = h2;
	} else {
		w2 = td->tile_wid;
		h2 = td->tile_hgt;

		/* big tile mode */
		if (is_bigtiled(x,y)) {
			tw2 = tile_width_mult  * w2;
			th2 = tile_height_mult * h2;
		} else {
 			tw2 = w2;
			th2 = h2;
		}
	}

	/* Starting point */
	//square_to_pixel(&x2, &y2, x, y);
	/* Location of window cell */
	x2 = x * w2 + td->size_ow1;
	y2 = y * h2 + td->size_oh1;

	/* Info */
	hdc = GetDC(td->w);

	/* More info */
	hdcSrc = CreateCompatibleDC(hdc);
	hbmSrcOld = SelectObject(hdcSrc, infGraph.hBitmap);

	if (infMask.hBitmap) {
		hdcMask = CreateCompatibleDC(hdc);
		SelectObject(hdcMask, infMask.hBitmap);
	} else {
		hdcMask = NULL;
	}

	/* Draw attr/char pairs */
	for (i = 0; i < n; i++, x2 += tw2) {
		byte a = ap[i];
		char c = cp[i];

		/* Extract picture */
		int row = (a & 0x7F);
		int col = (c & 0x7F);

		/* Location of bitmap cell */
		x1 = col * w1;
		y1 = row * h1;

		if (hdcMask) {
			x3 = (tcp[i] & 0x7F) * w1;
			y3 = (tap[i] & 0x7F) * h1;

			/* Perfect size */
			if ((w1 == tw2) && (h1 == th2)) {
 				/* Copy the terrain picture from the bitmap to the window */
				BitBlt(hdc, x2, y2, tw2, th2, hdcSrc, x3, y3, SRCCOPY);
 
 				/* Mask out the tile */
				BitBlt(hdc, x2, y2, tw2, th2, hdcMask, x1, y1, SRCAND);
 
 				/* Draw the tile */
				BitBlt(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, SRCPAINT);
			} else

			/* Need to stretch */
			{
				/* Set the correct mode for stretching the tiles */
				SetStretchBltMode(hdc, COLORONCOLOR);

				/* Copy the terrain picture from the bitmap to the window */
				StretchBlt(hdc, x2, y2, tw2, th2, hdcSrc, x3, y3, w1, h1, SRCCOPY);

				/* Only draw if terrain and overlay are different */
				if ((x1 != x3) || (y1 != y3)) {
					/* Mask out the tile */
					StretchBlt(hdc, x2, y2, tw2, th2, hdcMask, x1, y1, w1, h1, SRCAND);

					/* Draw the tile */
					StretchBlt(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, w1, h1, SRCPAINT);
				}
			}
		} else {
			/* Perfect size */
			if ((w1 == tw2) && (h1 == th2)) {
				/* Copy the picture from the bitmap to the window */
				BitBlt(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, SRCCOPY);
			} else

			/* Need to stretch */
			{
				/* Set the correct mode for stretching the tiles */
				SetStretchBltMode(hdc, COLORONCOLOR);

				/* Copy the picture from the bitmap to the window */
				StretchBlt(hdc, x2, y2, tw2, th2, hdcSrc, x1, y1, w1, h1, SRCCOPY);
			}
		}
	}

	/* Release */
	SelectObject(hdcSrc, hbmSrcOld);
	DeleteDC(hdcSrc);

	if (hdcMask)
	{
		/* Release */
		SelectObject(hdcMask, hbmSrcOld);
		DeleteDC(hdcMask);
	}

	/* Release */
	ReleaseDC(td->w, hdc);

#else /* USE_GRAPHICS */

	/* Just erase this grid */
	return (Term_wipe_win(x, y, n));

#endif /* USE_GRAPHICS */

	/* Success */
	return 0;
}


static int player_x;
static int player_y;

/*
 * Notice that the player has moved
 */
static void win_player_move(int x, int y, vptr dummy)
{
	/* Hack - ignore parameter */
	(void) dummy;

	/* Save for later */
	player_x = x;
	player_y = y;
}


/*
 * Save the information so we can access it later
 */
static void win_map_info(map_block *mb_ptr, const term_map *map, vptr dummy)
{
	/* Hack -- ignore parameter */
	(void) dummy;

	/* Store feature code for later */
	if (map->terrain) 
	{
		mb_ptr->a = f_info[map->terrain].x_attr;
		mb_ptr->c = f_info[map->terrain].x_char;
	}
	else
	{
		mb_ptr->a = 0;
		mb_ptr->c = 0;
	}
}

static void windows_map_aux(void)
{
	term_data *td = &data[0];
	map_block *mb_ptr;
	byte a;
	char c;
	int x, y;
	
	int min_x, min_y, max_x, max_y;

	/* Get size */
	td->map_tile_wid = (td->tile_wid * td->cols) / MAX_WID;
	td->map_tile_hgt = (td->tile_hgt * td->rows) / MAX_HGT;
	
	/* Get bounds */
	min_x = player_x - MAX_WID / 2;
	max_x = min_x + MAX_WID;
	
	min_y = player_y - MAX_HGT / 2;
	max_y = player_y + MAX_HGT;

	/* Draw the map */
	for (x = min_x; x < max_x; x++)
	{
		for (y = min_y; y < max_y; y++)
		{
			if (map_in_bounds(x, y))
			{
				mb_ptr = map_loc(x, y);
				
				/* Attr / char */
				a = mb_ptr->a;
				c = mb_ptr->c;
	
				/* Ignore non-graphics */
				if ((a & 0x80) && (c & 0x80))
				{
					Term_pict_win(x - min_x, y - min_y, 1, &a, &c, &a, &c);
				}
			}
		}
	}

	/* Hilite the player */
	Term_curs_win(player_x - min_x, player_y - min_y);
}


/*
 * MEGA_HACK - Display a graphical map of the dungeon.
 */
static void windows_map(void)
{
	term_data *td = &data[0];
	char ch;

	/* Only in graphics mode since the fonts can't be scaled */
	if (!use_graphics) return;

	/* Prevent various menu-actions from working */
	initialized = FALSE;

	/* Clear screen */
	Term_xtra_win_clear();

	td->map_active = TRUE;

	/* Draw the map */
	windows_map_aux();

	/* Wait for a keypress, flush key buffer */
	Term_inkey(&ch, TRUE, TRUE);
	Term_flush();

	/* Switch off the map display */
	td->map_active = FALSE;

	/* Restore screen */
	Term_xtra_win_clear();
	Term_redraw();

	/* We are ready again */
	initialized = TRUE;
}


/*** Other routines ***/


/*
 * Create and initialize a "term_data" given a title
 */
static void term_data_link(term_data *td)
{
	term *t = &td->t;

	/* Initialize the term */
	term_init(t, td->cols, td->rows, td->keys);

	/* Use a "software" cursor */
	t->soft_cursor = TRUE;

	/* Use "Term_pict" for "graphic" data */
	t->higher_pict = TRUE;

	/* Erase with "black space" */
	t->attr_blank = TERM_DARK;
	t->char_blank = ' ';

#if 0
	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_win;
	t->nuke_hook = Term_nuke_win;
#endif /* 0 */

	/* Prepare the template hooks */
	t->user_hook = Term_user_win;
	t->xtra_hook = Term_xtra_win;
	t->curs_hook = Term_curs_win;
	t->wipe_hook = Term_wipe_win;
	t->text_hook = Term_text_win;
	t->pict_hook = Term_pict_win;

	/* Remember where we came from */
	t->data = (vptr)(td);
}


/*
 * Create the windows
 *
 * First, instantiate the "default" values, then read the "ini_file"
 * to over-ride selected values, then create the windows, and fonts.
 *
 * Must use SW_SHOW not SW_SHOWNA, since on 256 color display
 * must make active to realize the palette.  XXX XXX XXX
 */
static void init_windows(void)
{
	int i;

	term_data *td;

	char buf[1024];

	MENUITEMINFO mii;
	HMENU hm;
	graphics_mode *mode;

	/* Main window */
	td = &data[0];
	WIPE(td, term_data);
	td->s = angband_term_name[0];
	td->keys = 1024;
	td->rows = 24;
	td->cols = 80;
	td->visible = TRUE;
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh1 = 2;
	td->size_oh2 = 2;
	td->pos_x = 30;
	td->pos_y = 20;

	/* Sub windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];
		WIPE(td, term_data);
		td->s = angband_term_name[i];
		td->keys = 16;
		td->rows = 24;
		td->cols = 80;
		td->visible = FALSE;
		td->size_ow1 = 1;
		td->size_ow2 = 1;
		td->size_oh1 = 1;
		td->size_oh2 = 1;
		td->pos_x = (7 - i) * 30;
		td->pos_y = (7 - i) * 20;
	}


	/* Load prefs */
	load_prefs();


	/* Main window (need these before term_getsize gets called) */
	td = &data[0];
	td->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU |
	               WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION |
	               WS_VISIBLE);
	if (td->maximized) td->dwStyle |= WS_MAXIMIZE;
	td->dwExStyle = 0;
	td->visible = TRUE;

	/* Sub windows (need these before term_getsize gets called) */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];
		td->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU | WS_CAPTION);
		td->dwExStyle = (WS_EX_TOOLWINDOW);
	}


	/* All windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];

		/* Access the standard font file */
		path_make(buf, ANGBAND_DIR_XTRA_FONT, td->font_want);

		/* Activate the chosen font */
		if (term_force_font(td, buf))
		{
			/* Access the standard font file */
			path_make(buf, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

			/* Force the use of that font */
			(void)term_force_font(td, buf);

			/* Oops */
			td->tile_wid = 8;
			td->tile_hgt = 13;
		}

		/* Analyze the font */
		term_getsize(td);

		/* Resize the window */
		term_window_resize(td);
	}


	/* Sub windows (reverse order) */
	for (i = MAX_TERM_DATA - 1; i >= 1; --i)
	{
		td = &data[i];

		my_td = td;
		td->w = CreateWindowEx(td->dwExStyle, AngList,
		                       td->s, td->dwStyle,
		                       td->pos_x, td->pos_y,
		                       td->size_wid, td->size_hgt,
		                       HWND_DESKTOP, NULL, hInstance, NULL);
		my_td = NULL;
		if (!td->w) quit("Failed to create sub-window");

		if (td->visible)
		{
			td->size_hack = TRUE;
			ShowWindow(td->w, SW_SHOW);
			td->size_hack = FALSE;
		}

		term_data_link(td);
		angband_term[i] = &td->t;

		if (td->visible)
		{
			/* Activate the window */
			SetActiveWindow(td->w);

			/* Bring window to top */
			SetWindowPos(td->w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
	}


	/* Main window */
	td = &data[0];

	/* Main window */
	my_td = td;
	td->w = CreateWindowEx(td->dwExStyle, AppName,
	                       td->s, td->dwStyle,
	                       td->pos_x, td->pos_y,
	                       td->size_wid, td->size_hgt,
	                       HWND_DESKTOP, NULL, hInstance, NULL);
	my_td = NULL;
	if (!td->w) quit_fmt("Failed to create %s window", VERSION_NAME);

	term_data_link(td);
	term_screen = &td->t;

	/* Activate the main window */
	SetActiveWindow(td->w);

	/* Bring main window back to top */
	SetWindowPos(td->w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

#ifdef SUPPORT_GAMMA

	if (gamma_correction > 0)
		build_gamma_table(gamma_correction);

#endif /* SUPPORT_GAMMA */

	/* New palette XXX XXX XXX */
	(void)new_palette();


	/* Create a "brush" for drawing the "cursor" */
	hbrYellow = CreateSolidBrush(win_clr[TERM_YELLOW]);

	/* Populate the graphic options sub menu with the graphics modes */
	hm = GetMenu(data[0].w);
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_ID | MIIM_TYPE;
	mii.fType = MFT_STRING;
	mode = graphics_modes;
	while (mode) {
		if (mode->grafID != GRAPHICS_NONE) {
			mii.wID = mode->grafID + IDM_OPTIONS_GRAPHICS_NONE;
			mii.dwTypeData = mode->menuname;
			mii.cch = strlen(mode->menuname);
			InsertMenuItem(hm,IDM_OPTIONS_BIGTILE, FALSE, &mii);
		}
		mode = mode->pNext;
	}
	//mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_SEPARATOR;
	mii.wID = 399;
	mii.dwTypeData = 0;
	mii.cch = 0;
	InsertMenuItem(hm,IDM_OPTIONS_BIGTILE, FALSE, &mii);

	/* setup the alpha blending function */
	blendfn.BlendOp = AC_SRC_OVER;
	blendfn.BlendFlags = 0;
	blendfn.AlphaFormat = AC_SRC_NO_PREMULT_ALPHA;//AC_SRC_ALPHA;
	blendfn.SourceConstantAlpha = 255;

	/* Process pending messages */
	(void)Term_xtra_win_flush();
}


#ifdef USE_SAVER

/*
 * Stop the screensaver
 */
static void stop_screensaver(void)
{
	if (screensaver)
		SendMessage(data[0].w, WM_CLOSE, 0, 0);
	else
		SendMessage(data[0].w, WM_COMMAND, IDM_OPTIONS_SAVER, 0);
}

#endif /* USE_SAVER */


/*
 * Prepare the menus
 */
static void setup_menus(void)
{
	int i;
	u32b u;

	HMENU hm = GetMenu(data[0].w);
	graphics_mode *mode;

#ifdef USE_SAVER
	main_menu = hm;
#endif /* USE_SAVER */

	/* Menu "File", Disable all */
	EnableMenuItem(hm, IDM_FILE_NEW,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_OPEN,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_SAVE,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_EXIT,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_SCORE,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);


	/* No character available */
	if (!character_generated)
	{
		/* Menu "File", Item "New" */
		EnableMenuItem(hm, IDM_FILE_NEW, MF_BYCOMMAND | MF_ENABLED);

		/* Menu "File", Item "Open" */
		EnableMenuItem(hm, IDM_FILE_OPEN, MF_BYCOMMAND | MF_ENABLED);
	}

	/* A character available */
	if (game_in_progress && character_generated && p_ptr->cmd.inkey_flag)
	{
		/* Menu "File", Item "Save" */
		EnableMenuItem(hm, IDM_FILE_SAVE, MF_BYCOMMAND | MF_ENABLED);
	}

	if (!game_in_progress || !character_generated ||
	    (p_ptr->cmd.inkey_flag))
	{
		/* Menu "File", Item "Exit" */
		EnableMenuItem(hm, IDM_FILE_EXIT, MF_BYCOMMAND | MF_ENABLED);
	}

	if (initialized)
	{
		/* Menu "File", Item "Show Scores" */
		EnableMenuItem(hm, IDM_FILE_SCORE, MF_BYCOMMAND | MF_ENABLED);
	}


	/* Menu "Window::Visibility" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		CheckMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		              (data[i].visible ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		               MF_BYCOMMAND | MF_ENABLED);
	}

	/* Menu "Window::Font" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_FONT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_FONT_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);
		}
	}

	/* Menu "Window::Increase Tile Width" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_I_WID_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_I_WID_0 + i,
	    		           MF_BYCOMMAND | MF_ENABLED);

		}
	}

	/* Menu "Window::Decrease Tile Width" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_D_WID_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_D_WID_0 + i,
	    		           MF_BYCOMMAND | MF_ENABLED);

		}
	}

	/* Menu "Window::Increase Tile Height" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_I_HGT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_I_HGT_0 + i,
	    		           MF_BYCOMMAND | MF_ENABLED);

		}
	}

	/* Menu "Window::Decrease Tile Height" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_D_HGT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_D_HGT_0 + i,
	    		           MF_BYCOMMAND | MF_ENABLED);

		}
	}

	EnableMenuItem(hm, IDM_WINDOW_OPT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_WINDOW_RESET, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	/* Menu "Options", disable all */
	mode = graphics_modes;
	while (mode) {
		EnableMenuItem(hm, mode->grafID + IDM_OPTIONS_GRAPHICS_NONE,
					   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		mode = mode->pNext;
	} 
	//EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_NICE,
	EnableMenuItem(hm, IDM_OPTIONS_BIGTILE,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	for (i = IDM_OPTIONS_TILE_MIN; i <= IDM_OPTIONS_TILE_MAX; i++) {
		EnableMenuItem(hm, i, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	for (i = IDM_OPTIONS_FONT_MIN; i <= IDM_OPTIONS_FONT_MAX; i++) {
		EnableMenuItem(hm, i, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	EnableMenuItem(hm, IDM_OPTIONS_SOUND,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_OPTIONS_SAVER,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_OPTIONS_LOW_PRIORITY,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	/* Menu "Options", Item "Map" */
	if (p_ptr->cmd.inkey_flag && initialized && (use_graphics != GRAPHICS_NONE))
		EnableMenuItem(GetMenu(data[0].w), IDM_OPTIONS_MAP, MF_BYCOMMAND | MF_ENABLED);
	else
		EnableMenuItem(GetMenu(data[0].w), IDM_OPTIONS_MAP,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);


	/* Menu "Options", update all */
	mode = graphics_modes;
	while (mode) {
		CheckMenuItem(hm, mode->grafID + IDM_OPTIONS_GRAPHICS_NONE,
	                (arg_graphics == mode->grafID ? MF_CHECKED : MF_UNCHECKED));
		mode = mode->pNext;
	} 

	CheckMenuItem(hm, IDM_OPTIONS_BIGTILE,
	              (((tile_width_mult > 1) || (tile_height_mult > 1)) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hm, IDM_OPTIONS_SOUND,
	              (arg_sound ? MF_CHECKED : MF_UNCHECKED));

	/* see which tile sizes should be checked */
	if ((tile_width_mult == 1) && (tile_height_mult == 1))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_1x1, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_1x1, MF_UNCHECKED);

	if ((tile_width_mult == 2) && (tile_height_mult == 1))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_2x1, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_2x1, MF_UNCHECKED);

	if ((tile_width_mult == 2) && (tile_height_mult == 2))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_2x2, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_2x2, MF_UNCHECKED);

	if ((tile_width_mult == 3) && (tile_height_mult == 1))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_3x1, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_3x1, MF_UNCHECKED);

	if ((tile_width_mult == 3) && (tile_height_mult == 3))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_3x3, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_3x3, MF_UNCHECKED);

	if ((tile_width_mult == 4) && (tile_height_mult == 2))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_4x2, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_4x2, MF_UNCHECKED);

	if ((tile_width_mult == 4) && (tile_height_mult == 4))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_4x4, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_4x4, MF_UNCHECKED);

	if ((tile_width_mult == 6) && (tile_height_mult == 3))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_6x3, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_6x3, MF_UNCHECKED);

	if ((tile_width_mult == 6) && (tile_height_mult == 6))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_6x6, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_6x6, MF_UNCHECKED);

	if ((tile_width_mult == 8) && (tile_height_mult == 4))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_8x4, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_8x4, MF_UNCHECKED);

	if ((tile_width_mult == 8) && (tile_height_mult == 8))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_8x8, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_8x8, MF_UNCHECKED);

	if ((tile_width_mult == 16) && (tile_height_mult == 8))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_16x8, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_16x8, MF_UNCHECKED);

	if ((tile_width_mult == 16) && (tile_height_mult == 16))
		CheckMenuItem(hm, IDM_OPTIONS_TILE_16x16, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_OPTIONS_TILE_16x16, MF_UNCHECKED);

	/* see which font sizes should be checked */
	u = data[0].tile_hgt;
	if ((data[0].tile_wid == data[0].font_wid) && (u == data[0].font_hgt))
		CheckMenuItem(hm, IDM_TILE_FONT, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_FONT, MF_UNCHECKED);

	if ((data[0].tile_wid == 8) && (u == 16))
		CheckMenuItem(hm, IDM_TILE_08X16, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_08X16, MF_UNCHECKED);

	if ((data[0].tile_wid == 8) && (u == 13))
		CheckMenuItem(hm, IDM_TILE_08X13, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_08X13, MF_UNCHECKED);

	if ((data[0].tile_wid == 8) && (u == 8))
		CheckMenuItem(hm, IDM_TILE_08X08, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_08X08, MF_UNCHECKED);

	if ((data[0].tile_wid == 16) && (u == 16))
		CheckMenuItem(hm, IDM_TILE_16X16, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_16X16, MF_UNCHECKED);

	if ((data[0].tile_wid == 32) && (u == 32))
		CheckMenuItem(hm, IDM_TILE_32X32, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_32X32, MF_UNCHECKED);

	if ((data[0].tile_wid == 10) && (u == 20))
		CheckMenuItem(hm, IDM_TILE_10X20, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_10X20, MF_UNCHECKED);

	if ((data[0].tile_wid == 16) && (u == 32))
		CheckMenuItem(hm, IDM_TILE_16X32, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_16X32, MF_UNCHECKED);

	if ((data[0].tile_wid == 10) && (u == 17))
		CheckMenuItem(hm, IDM_TILE_10X17, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_10X17, MF_UNCHECKED);

	if ((data[0].tile_wid == 12) && (u == 13))
		CheckMenuItem(hm, IDM_TILE_12X13, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_12X13, MF_UNCHECKED);

	if ((data[0].tile_wid == 12) && (u == 20))
		CheckMenuItem(hm, IDM_TILE_12X20, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_12X20, MF_UNCHECKED);

	if ((data[0].tile_wid == 16) && (u == 25))
		CheckMenuItem(hm, IDM_TILE_16X25, MF_CHECKED);
	else
		CheckMenuItem(hm, IDM_TILE_16X25, MF_UNCHECKED);

#ifdef USE_SAVER
	CheckMenuItem(hm, IDM_OPTIONS_SAVER,
	              (hwndSaver ? MF_CHECKED : MF_UNCHECKED));
#endif /* USE_SAVER */

	CheckMenuItem(hm, IDM_OPTIONS_LOW_PRIORITY,
	              (low_priority ? MF_CHECKED : MF_UNCHECKED));

#ifdef USE_GRAPHICS
	if (initialized && p_ptr->cmd.inkey_flag)
	{
		mode = graphics_modes;
		while (mode) {
			if ((mode->grafID == 0) || (mode->file && mode->file[0])) {
				EnableMenuItem(hm, mode->grafID + IDM_OPTIONS_GRAPHICS_NONE, MF_ENABLED);
			}
			mode = mode->pNext;
		} 
		/*EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_NONE, MF_ENABLED);
		EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_OLD, MF_ENABLED);
		EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_ADAM, MF_ENABLED);
		EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_DAVID, MF_ENABLED);
		EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_DAVID_6, MF_ENABLED);
		EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_DAVID_7, MF_ENABLED);
		EnableMenuItem(hm, IDM_OPTIONS_GRAPHICS_DAVID_8, MF_ENABLED);*/
		EnableMenuItem(hm, IDM_OPTIONS_BIGTILE, MF_ENABLED);
	}
#endif /* USE_GRAPHICS */

	if (initialized && p_ptr->cmd.inkey_flag)
	{
		EnableMenuItem(hm, IDM_WINDOW_OPT, MF_ENABLED);
		EnableMenuItem(hm, IDM_WINDOW_RESET, MF_ENABLED);

		for (i = IDM_OPTIONS_TILE_MIN; i <= IDM_OPTIONS_TILE_MAX; i++) {
			EnableMenuItem(hm, i, MF_ENABLED);
		}
		for (i = IDM_OPTIONS_FONT_MIN; i <= IDM_OPTIONS_FONT_MAX; i++) {
			EnableMenuItem(hm, i, MF_ENABLED);
		}
	}

#ifdef USE_SOUND
	if (initialized && p_ptr->cmd.inkey_flag)
	{
		/* Menu "Options", Item "Sound" */
		EnableMenuItem(hm, IDM_OPTIONS_SOUND, MF_ENABLED);
	}
#endif /* USE_SOUND */

#ifdef USE_SAVER
	/* Menu "Options", Item "ScreenSaver" */
	EnableMenuItem(hm, IDM_OPTIONS_SAVER,
	               MF_BYCOMMAND | MF_ENABLED);
#endif /* USE_SAVER */

	EnableMenuItem(hm, IDM_OPTIONS_LOW_PRIORITY,
	               MF_BYCOMMAND | MF_ENABLED);
}


/*
 * Check for double clicked (or dragged) savefile
 *
 * Apparently, Windows copies the entire filename into the first
 * piece of the "command line string".  Perhaps we should extract
 * the "basename" of that filename and append it to the "save" dir.
 */
static void check_for_save_file(LPSTR cmd_line)
{
	char *s, *p;

	/* First arg */
	s = cmd_line;

	/* No args */
	if (!s || !*s) return;

	/* Next arg */
	p = strchr(s, ' ');

	/* Tokenize */
	if (p) *p = '\0';

	/* Extract filename */
	*savefile = '\0';
	strncat(savefile, s, sizeof(savefile) - 1);

	/* Validate the file */
	validate_file(savefile);

	/* Game in progress */
	game_in_progress = TRUE;

	Term_fresh();
}


#ifdef USE_SAVER

#ifdef ALLOW_BORG

/*
 * Hook into the inkey() function so that flushing keypresses
 * doesn't affect us.
 *
 * ToDo: Try to implement recording and playing back of games
 * by saving/reading the keypresses to/from a file. Note that
 * interrupting certain actions (resting, running, and other
 * repeated actions) would mess that up, so this would have to
 * be switched off when recording.
 */

extern char (*inkey_hack)(int flush_first);

static char screensaver_inkey_hack_buffer[1024];

static char screensaver_inkey_hack(int flush_first)
{
	static int screensaver_inkey_hack_index = 0;

	if (screensaver_inkey_hack_index < sizeof(screensaver_inkey_hack_buffer))
		return (screensaver_inkey_hack_buffer[screensaver_inkey_hack_index++]);
	else
		return ESCAPE;
}

#endif /* ALLOW_BORG */


/*
 * Start the screensaver
 */
static void start_screensaver(void)
{
	bool file_exists;

#ifdef ALLOW_BORG
	int i, j;
#endif /* ALLOW_BORG */

	if (savefile[0] == 0) {
		/* Set the name for process_player_name() */
		strnfmt(op_ptr->full_name, sizeof(op_ptr->full_name), "%s", saverfilename);

		/* Set 'savefile' to a valid name */
		process_player_name(TRUE);
	}

	/* Does the savefile already exist? */
	file_exists = check_file(savefile);

	/* Don't try to load a non-existant savefile */
	if (!file_exists) savefile[0] = '\0';

	/* Game in progress */
	game_in_progress = TRUE;

	Term_fresh();

	/* Screensaver mode on */
	SendMessage(data[0].w, WM_COMMAND, IDM_OPTIONS_SAVER, 0);

	/* Low priority */
	SendMessage(data[0].w, WM_COMMAND, IDM_OPTIONS_LOW_PRIORITY, 0);

#ifdef ALLOW_BORG

	/*
	 * MegaHack - Try to start the Borg.
	 *
	 * The simulated keypresses will be processed when play_game()
	 * is called.
	 */

	inkey_hack = screensaver_inkey_hack;
	j = 0;

	/*
	 * If no savefile is present or then go through the steps necessary
	 * to create a random character.  If a savefile already is present
	 * then the simulated keypresses will either clean away any [-more-]
	 * prompts (if the character is alive), or create a new random
	 * character.
	 *
	 * Luckily it's possible to send the same keypresses no matter if
	 * the character is alive, dead, or not even yet created.
	 */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Gender */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Race */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Class */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Modify options */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Reroll */

	if (!file_exists)
	{
		/* Savefile name */
		int n = strlen(saverfilename);
		for (i = 0; i < n; i++)
			screensaver_inkey_hack_buffer[j++] = saverfilename[i];
	}

	screensaver_inkey_hack_buffer[j++] = '\r'; /* Return */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Character info */

	/*
	 * Make sure the "verify_special" options is off, so that we can
	 * get into Borg mode without confirmation.
	 */

	screensaver_inkey_hack_buffer[j++] = '='; /* Enter options screen */
	screensaver_inkey_hack_buffer[j++] = '2'; /* Disturbance options */

	/* Cursor down to "verify_special" */
	for (i = 0; i < 10; i++)
		screensaver_inkey_hack_buffer[j++] = '2';

	screensaver_inkey_hack_buffer[j++] = 'n'; /* Switch off "verify_special" */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Leave disturbance options */

	/*
	 * Make sure the "cheat_live" option is set, so that the Borg can
	 * automatically restart.
	 */

	screensaver_inkey_hack_buffer[j++] = '6'; /* Cheat options */

	/* Cursor down to "cheat live" */
	for (i = 0; i < OPT_cheat_live - OPT_CHEAT; i++)
		screensaver_inkey_hack_buffer[j++] = '2';

	screensaver_inkey_hack_buffer[j++] = 'y'; /* Switch on "cheat_live" */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Leave cheat options */
	screensaver_inkey_hack_buffer[j++] = ESCAPE; /* Leave options */

	/*
	 * Now start the Borg!
	 */

	screensaver_inkey_hack_buffer[j++] = KTRL('Z'); /* Enter borgmode */
	screensaver_inkey_hack_buffer[j++] = 'z'; /* Run Borg */
#endif /* ALLOW_BORG */
}

#endif /* USE_SAVER */


/*
 * Display a help file
 */
static void display_help(cptr filename)
{
	char tmp[1024];

	path_make(tmp, ANGBAND_DIR_XTRA_HELP, filename);

	if (check_file(tmp)) {
#ifdef HTML_HELP

		HtmlHelp(data[0].w, tmp, HH_DISPLAY_TOPIC, 0);

#else /* HTML_HELP */

		char buf[1024];

		strnfmt(buf, 1024, "winhelp.exe %s", tmp);
		WinExec(buf, SW_NORMAL);

#endif /* HTML_HELP */

	} else {
		/*plog_fmt("Cannot find help file: %s", tmp);
		plog("Use the online help files instead.");*/
		Term_keypress('?');
	}
}


/*
 * Process a menu command
 */
static void process_menus(WORD wCmd)
{
	int i;

	term_data *td;

	OPENFILENAME ofn;

	/* Analyze */
	switch (wCmd)
	{
		/* New game */
		case IDM_FILE_NEW:
		{
			if (!initialized) {
				plog("You cannot do that yet...");
			} else
			if (game_in_progress) {
				plog("You can't start a new game while you're still playing!");
			} else
			{
				game_in_progress = TRUE;
			}
			break;
		}

		/* Open game */
		case IDM_FILE_OPEN:
		{
			if (!initialized) {
				plog("You cannot do that yet...");
			} else
			if (game_in_progress) {
				plog("You can't open a new game while you're still playing!");
			} else
			{
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = data[0].w;
				ofn.lpstrFilter = "Save Files (*.)\0*\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = savefile;
				ofn.nMaxFile = 1024;
				ofn.lpstrInitialDir = ANGBAND_DIR_SAVE;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

				if (GetOpenFileName(&ofn)) {
					/* Load 'savefile' */
					validate_file(savefile);
					game_in_progress = TRUE;
				}
			}
			break;
		}

		/* Save game */
		case IDM_FILE_SAVE:
		{
			if (game_in_progress && character_generated &&
					p_ptr->cmd.inkey_flag)
			{
				/* Hack -- Forget messages */
				msg_flag = FALSE;

				/* Save the game */
#ifdef ZANGBAND
				do_cmd_save_game(FALSE);
#else /* ZANGBAND */
				do_cmd_save_game();
#endif /* ZANGBAND */
			} else {
				/* Paranoia */
				plog("You may not do that right now.");
			}
			break;
		}

		/* Show scores */
		case IDM_FILE_SCORE:
		{
			if (!initialized) {
				plog("You may not do that right now.");
				break;
			}
			
			/* Show score */
			ingame_score(&initialized, game_in_progress);

			break;
		}

		/* Exit */
		case IDM_FILE_EXIT:
		{
			if (game_in_progress && character_generated) {
				/* Paranoia */
				if (!p_ptr->cmd.inkey_flag) {
					plog("You may not do that right now.");
					break;
				}

				/* Hack -- Forget messages */
				msg_flag = FALSE;

				/* Save the game */
#ifdef ZANGBAND
				do_cmd_save_game(FALSE);
#else /* ZANGBAND */
				do_cmd_save_game();
#endif /* ZANGBAND */
			}
			quit(NULL);
			break;
		}

		case IDM_WINDOW_VIS_0:
		{
			plog("You are not allowed to do that!");

			break;
		}

		/* Window visibility */
		case IDM_WINDOW_VIS_1:
		case IDM_WINDOW_VIS_2:
		case IDM_WINDOW_VIS_3:
		case IDM_WINDOW_VIS_4:
		case IDM_WINDOW_VIS_5:
		case IDM_WINDOW_VIS_6:
		case IDM_WINDOW_VIS_7:
		{
			i = wCmd - IDM_WINDOW_VIS_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			if (!td->visible) {
				td->visible = TRUE;
				ShowWindow(td->w, SW_SHOW);
				term_data_redraw(td);
			} else {
				td->visible = FALSE;
				ShowWindow(td->w, SW_HIDE);
			}

			break;
		}

		/* Window fonts */
		case IDM_WINDOW_FONT_0:
		case IDM_WINDOW_FONT_1:
		case IDM_WINDOW_FONT_2:
		case IDM_WINDOW_FONT_3:
		case IDM_WINDOW_FONT_4:
		case IDM_WINDOW_FONT_5:
		case IDM_WINDOW_FONT_6:
		case IDM_WINDOW_FONT_7:
		{
			i = wCmd - IDM_WINDOW_FONT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			term_change_font(td);

			break;
		}

		/* Increase Tile Width */
		case IDM_WINDOW_I_WID_0:
		case IDM_WINDOW_I_WID_1:
		case IDM_WINDOW_I_WID_2:
		case IDM_WINDOW_I_WID_3:
		case IDM_WINDOW_I_WID_4:
		case IDM_WINDOW_I_WID_5:
		case IDM_WINDOW_I_WID_6:
		case IDM_WINDOW_I_WID_7:
		{
			i = wCmd - IDM_WINDOW_I_WID_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_wid += 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Decrease Tile Height */
		case IDM_WINDOW_D_WID_0:
		case IDM_WINDOW_D_WID_1:
		case IDM_WINDOW_D_WID_2:
		case IDM_WINDOW_D_WID_3:
		case IDM_WINDOW_D_WID_4:
		case IDM_WINDOW_D_WID_5:
		case IDM_WINDOW_D_WID_6:
		case IDM_WINDOW_D_WID_7:
		{
			i = wCmd - IDM_WINDOW_D_WID_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_wid -= 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Increase Tile Height */
		case IDM_WINDOW_I_HGT_0:
		case IDM_WINDOW_I_HGT_1:
		case IDM_WINDOW_I_HGT_2:
		case IDM_WINDOW_I_HGT_3:
		case IDM_WINDOW_I_HGT_4:
		case IDM_WINDOW_I_HGT_5:
		case IDM_WINDOW_I_HGT_6:
		case IDM_WINDOW_I_HGT_7:
		{
			i = wCmd - IDM_WINDOW_I_HGT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_hgt += 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Decrease Tile Height */
		case IDM_WINDOW_D_HGT_0:
		case IDM_WINDOW_D_HGT_1:
		case IDM_WINDOW_D_HGT_2:
		case IDM_WINDOW_D_HGT_3:
		case IDM_WINDOW_D_HGT_4:
		case IDM_WINDOW_D_HGT_5:
		case IDM_WINDOW_D_HGT_6:
		case IDM_WINDOW_D_HGT_7:
		{
			i = wCmd - IDM_WINDOW_D_HGT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_hgt -= 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}
		case IDM_WINDOW_OPT: {
			/* Paranoia */
			if (!p_ptr->cmd.inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}
			Term_keypress('=');
			Term_keypress('m');

			break;
		}
		case IDM_WINDOW_RESET: {
			/* Paranoia */
			if (!p_ptr->cmd.inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}
			if (MessageBox(NULL,
				  "This will reset the size and layout of the angband windows\n based on your screen size. Do you want to continue?",
				  "z+Angband", MB_YESNO|MB_ICONWARNING) == IDYES)
			{
				term *old = Term;
				int i;
				RECT rc;

				(void)default_layout_win(data,MAX_TERM_DATA);
        
				for (i=0; i < MAX_TERM_DATA; i++) {
					/* Activate */
					Term_activate(&(data[i].t));
	        
					/* Resize the term */
					Term_resize(data[i].cols, data[i].rows);
				}
				/* Restore */
				Term_activate(old);

				/* Do something to sub-windows */
				for (i = MAX_TERM_DATA - 1; i >= 0; i--) {
					if (!(data[i].w)) continue;
					/* Client window size */
					rc.left = 0;
					rc.top = 0;
					rc.right = rc.left + data[i].cols * data[i].tile_wid + data[i].size_ow1 + data[i].size_ow2;
					rc.bottom = rc.top + data[i].rows * data[i].tile_hgt + data[i].size_oh1 + data[i].size_oh2;

					/* Get total window size (without menu for sub-windows) */
					AdjustWindowRectEx(&rc, data[i].dwStyle, TRUE, data[i].dwExStyle);

					/* Total size */
					data[i].size_wid = rc.right - rc.left;
					data[i].size_hgt = rc.bottom - rc.top;

					if (i==0) {
						SetWindowPos(data[i].w, 0, data[i].pos_x, data[i].pos_y,
							data[i].size_wid, data[i].size_hgt, 0);
					} else {
						SetWindowPos(data[i].w, data[0].w, data[i].pos_x, data[i].pos_y,
							data[i].size_wid, data[i].size_hgt, 0);
					}
					if (data[i].visible) {
						char buf[1024];

						/* Access the standard font file */
						path_make(buf, ANGBAND_DIR_XTRA_FONT, data[i].font_want);

						/* Activate the chosen font */
						if (term_force_font(&data[i], buf)) {
							/* Access the standard font file */
							path_make(buf, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

							/* Force the use of that font */
							(void)term_force_font(&data[i], buf);

							/* Oops */
							data[i].tile_wid = 8;
							data[i].tile_hgt = 13;
						}
						/* Reset the tile info */
						if (!data[i].tile_wid || !data[i].tile_hgt) {
							data[i].tile_wid = data[i].font_wid;
							data[i].tile_hgt = data[i].font_hgt;
						}

						ShowWindow(data[i].w, SW_SHOW);
					} else {
						ShowWindow(data[i].w, SW_HIDE);
					}
 
					/* Redraw later */
					InvalidateRect(data[i].w, NULL, TRUE);
				}

				/* Focus on main window */
				SetFocus(data[0].w);
			}

			break;
		}


		case IDM_TILE_08X08:
		{
			td = &data[0];
			td->tile_wid = 8;
			td->tile_hgt = 8;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		case IDM_TILE_08X16:
		{
			td = &data[0];
			td->tile_wid = 8;
			td->tile_hgt = 16;

			term_getsize(td);

			term_window_resize(td);

			break;
		}
		case IDM_TILE_16X16:
		{
			td = &data[0];
			td->tile_wid = 16;
			td->tile_hgt = 16;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		case IDM_TILE_32X32:
		{
			td = &data[0];
			td->tile_wid = 32;
			td->tile_hgt = 32;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		case IDM_TILE_12X13:
		{
			td = &data[0];
			td->tile_wid = 12;
			td->tile_hgt = 13;

			term_getsize(td);

			term_window_resize(td);

			break;
		}
		case IDM_TILE_FONT:
		{
			td = &data[0];
			td->tile_wid = td->font_wid;
			td->tile_hgt = td->font_hgt;

			term_getsize(td);

			term_window_resize(td);

			break;
		}
		case IDM_TILE_16X32:
		{
			td = &data[0];
			td->tile_wid = 16;
			td->tile_hgt = 32;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		case IDM_OPTIONS_GRAPHICS_NICE: {
			/* Paranoia */
			if (!p_ptr->cmd.inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}

			/* Toggle "arg_graphics_nice" */
			use_bigtile = !use_bigtile;

			/* Hack - redraw everything + recalc bigtile regions */
			angband_term[0]->resize_hook();
			
			break;
		}

		case IDM_OPTIONS_TILE_1x1:
		case IDM_OPTIONS_TILE_2x1:
		case IDM_OPTIONS_TILE_2x2:
		case IDM_OPTIONS_TILE_3x1:
		case IDM_OPTIONS_TILE_3x2:
		case IDM_OPTIONS_TILE_3x3:
		case IDM_OPTIONS_TILE_4x2:
		case IDM_OPTIONS_TILE_4x3:
		case IDM_OPTIONS_TILE_4x4:
		case IDM_OPTIONS_TILE_6x3:
		case IDM_OPTIONS_TILE_6x6:
		case IDM_OPTIONS_TILE_8x4:
		case IDM_OPTIONS_TILE_8x8:
		case IDM_OPTIONS_TILE_16x8:
		case IDM_OPTIONS_TILE_16x16:
		{
			/* Paranoia */
			if (!p_ptr->cmd.inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}
			switch (wCmd)
			{
			case IDM_OPTIONS_TILE_1x1:
			{
				tile_width_mult = 1;
				tile_height_mult = 1;
				break;
			}
			case IDM_OPTIONS_TILE_2x1:
			{
				tile_width_mult = 2;
				tile_height_mult = 1;
				break;
			}
			case IDM_OPTIONS_TILE_2x2:
			{
				tile_width_mult = 2;
				tile_height_mult = 2;
				break;
			}
			case IDM_OPTIONS_TILE_3x1:
			{
				tile_width_mult = 3;
				tile_height_mult = 1;
				break;
			}
			case IDM_OPTIONS_TILE_3x2:
			{
				tile_width_mult = 3;
				tile_height_mult = 2;
				break;
			}
			case IDM_OPTIONS_TILE_3x3:
			{
				tile_width_mult = 3;
				tile_height_mult = 3;
				break;
			}
			case IDM_OPTIONS_TILE_4x2:
			{
				tile_width_mult = 4;
				tile_height_mult = 2;
				break;
			}
			case IDM_OPTIONS_TILE_4x3:
			{
				tile_width_mult = 4;
				tile_height_mult = 3;
				break;
			}
			case IDM_OPTIONS_TILE_4x4:
			{
				tile_width_mult = 4;
				tile_height_mult = 4;
				break;
			}
			case IDM_OPTIONS_TILE_6x3:
			{
				tile_width_mult = 6;
				tile_height_mult = 3;
				break;
			}
			case IDM_OPTIONS_TILE_6x6:
			{
				tile_width_mult = 6;
				tile_height_mult = 6;
				break;
			}
			case IDM_OPTIONS_TILE_8x4:
			{
				tile_width_mult = 8;
				tile_height_mult = 4;
				break;
			}
			case IDM_OPTIONS_TILE_8x8:
			{
				tile_width_mult = 8;
				tile_height_mult = 8;
				break;
			}
			case IDM_OPTIONS_TILE_16x8:
			{
				tile_width_mult = 16;
				tile_height_mult = 8;
				break;
			}
			case IDM_OPTIONS_TILE_16x16:
			{
				tile_width_mult = 16;
				tile_height_mult = 16;
				break;
			}
			}

			/* Clear screen */
			Term_xtra_win_clear();

			/* Hack - redraw everything + recalc bigtile regions */
			angband_term[0]->resize_hook();

			break;
		}

		case IDM_OPTIONS_SOUND:
		{
			/* Paranoia */
			if (!p_ptr->cmd.inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}

			/* Toggle "arg_sound" */
			arg_sound = !arg_sound;

			/* React to changes */
			Term_xtra_win_react();

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));

			break;
		}

#ifdef USE_SAVER

		case IDM_OPTIONS_SAVER:
		{
			if (hwndSaver) {
				DestroyWindow(hwndSaver);
				hwndSaver = NULL;
				screensaver_active = FALSE;

				/* Switch main menu back on */
				SetMenu(data[0].w, main_menu);

				for (i = MAX_TERM_DATA - 1; i >= 0; --i) {
					td = &data[i];

					if (td->visible) {
						/* Turn the Windows back to normal */
						SetWindowLong(td->w, GWL_STYLE, td->dwStyle);

						/* Push the window to the top */
						SetWindowPos(td->w, HWND_NOTOPMOST, 0, 0, 0, 0,
							   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
					}
				}

				ShowCursor(TRUE);
			} else {
				/* Create a screen saver window */
				hwndSaver = CreateWindowEx(WS_EX_TOPMOST, "WindowsScreenSaverClass",
				                           "Angband Screensaver",
				                           WS_POPUP | WS_MAXIMIZE | WS_VISIBLE,
				                           0, 0, GetSystemMetrics(SM_CXSCREEN),
				                           GetSystemMetrics(SM_CYSCREEN),
				                           NULL, NULL, hInstance, NULL);

				if (hwndSaver) {
					for (i = MAX_TERM_DATA - 1; i >= 0; --i) {
						td = &data[i];

						if (td->visible) {
							/* Switch off border and titlebar */
							SetWindowLong(td->w, GWL_STYLE, WS_VISIBLE);

							/* Switch off menu */
							SetMenu(td->w, NULL);

							/* Push the window to the top */
							SetWindowPos(td->w, HWND_TOPMOST, 0, 0, 0, 0,
								   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
						}
					}

					ShowCursor(FALSE);

					screensaver_active = TRUE;
				} else {
					plog("Failed to create saver window");
				}
			}

			break;
		}

#endif /* USE_SAVER */

		case IDM_OPTIONS_LOW_PRIORITY:
		{
			/* Lower or reset the priority of the current process */
			if (low_priority)
				SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
			else
				SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

			/* Toggle priority */
			low_priority = !low_priority;

			break;
		}

		case IDM_OPTIONS_MAP:
		{
			/* Paranoia */
			if (!p_ptr->cmd.inkey_flag || !initialized) {
				plog("You may not do that right now.");
				break;
			}

			windows_map();
			break;
		}

		case IDM_HELP_GENERAL:
		{
			display_help(HELP_GENERAL);
			break;
		}

		case IDM_HELP_SPOILERS:
		{
			display_help(HELP_SPOILERS);
			break;
		}
		default: {
			if ((wCmd >= IDM_OPTIONS_GRAPHICS_NONE) && (wCmd <= IDM_OPTIONS_GRAPHICS_NONE+graphics_mode_high_id)) {
				graphics_mode *mode;
				int selected_mode = 0;
				int desired_mode = wCmd - IDM_OPTIONS_GRAPHICS_NONE;

				/* Paranoia */
				if (!p_ptr->cmd.inkey_flag || !initialized) {
					plog("You may not do that right now.");
					break;
				}

				mode = graphics_modes;
				while (mode) {
					if (mode->grafID == desired_mode) {
						selected_mode = desired_mode;
						break;
					}
					mode = mode->pNext;
				}

				/* Toggle "arg_graphics" */
				if (arg_graphics != selected_mode) {
					arg_graphics = selected_mode;

					/* React to changes */
					Term_xtra_win_react();

					/* Hack -- Force redraw */
					Term_key_push(KTRL('R'));
				}
			}
			break;
		}
	}
}


/*
 * Redraw a section of a window
 */
static void handle_wm_paint(HWND hWnd)
{
	int x1, y1, x2, y2;
	PAINTSTRUCT ps;
	term_data *td;

	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLong(hWnd, 0);

	BeginPaint(hWnd, &ps);

	if (td->map_active) {
		/* Redraw the map */
		/* ToDo: Only redraw the necessary parts */
		windows_map_aux();
	} else {
		/* Get the area that should be updated (rounding up/down) */
		/* ToDo: Take the window borders into account */
		x1 = (ps.rcPaint.left / td->tile_wid) - 1;
		x2 = (ps.rcPaint.right / td->tile_wid) + 1;
		y1 = (ps.rcPaint.top / td->tile_hgt) - 1;
		y2 = (ps.rcPaint.bottom / td->tile_hgt) + 1;

		/* Redraw */
		if (td) term_data_redraw_section(td, x1, y1, x2, y2);
	}

	EndPaint(hWnd, &ps);
}


static LRESULT FAR PASCAL AngbandWndProc(HWND hWnd, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	term_data *td;
	int i;

#ifdef USE_SAVER
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;
#endif /* USE_SAVER */

	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLong(hWnd, 0);

	/* Handle message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLong(hWnd, 0, (LONG)(my_td));
			break;
		}

		/* XXX XXX XXX */
		case WM_CREATE:
		{
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO FAR *lpmmi;
			RECT rc;

			lpmmi = (MINMAXINFO FAR *)lParam;

			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* Minimum window size is 80x24 */
			rc.left = rc.top = 0;
			rc.right = rc.left + 80 * td->tile_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 24 * td->tile_hgt + td->size_oh1 + td->size_oh2 + 1;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			return 0;
		}

		case WM_PAINT:
		{
			handle_wm_paint(hWnd);

			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			bool mc = FALSE;
			bool ms = FALSE;
			bool ma = FALSE;

#ifdef USE_SAVER
			if (screensaver_active) {
				stop_screensaver();
				screensaver_active = FALSE;
				return 0;
			}
#endif /* USE_SAVER */

			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) mc = TRUE;
			if (GetKeyState(VK_SHIFT)   & 0x8000) ms = TRUE;
			if (GetKeyState(VK_MENU)    & 0x8000) ma = TRUE;

			/* Handle "special" keys */
			if (special_key[(byte)(wParam)]) {
				/* Begin the macro trigger */
				Term_keypress(31);

				/* Send the modifiers */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (ma) Term_keypress('A');

				/* Extract "scan code" */
				i = LOBYTE(HIWORD(lParam));

				/* Introduce the scan code */
				Term_keypress('x');

				/* Encode the hexidecimal scan code */
				Term_keypress(hexsym[i/16]);
				Term_keypress(hexsym[i%16]);

				/* End the macro trigger */
				Term_keypress(13);

				return 0;
			}

			break;
		}

		case WM_CHAR:
		{
			Term_keypress(wParam);
			return 0;
		}


		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		{
#ifdef USE_SAVER
			if (screensaver_active) {
				stop_screensaver();
				screensaver_active = FALSE;
				return 0;
			} else {
#endif /* USE_SAVER */
				char button, mods;
				int xPos, yPos;

				/* Get the text grid */
				pixel_to_square(&xPos, &yPos,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));

				if (uMsg == WM_LBUTTONDOWN)
					button = 1;
				else if (uMsg == WM_RBUTTONDOWN)
					button = 2;
				else
					button = 3;

				/* Extract the modifiers */
				mods = 0;
				if (GetKeyState(VK_CONTROL) & 0x8000) mods |= 2;
				if (GetKeyState(VK_SHIFT)   & 0x8000) mods |= 1;
				if (GetKeyState(VK_MENU)    & 0x8000) mods |= 4;

				Term_mousepress(button, mods, xPos, yPos);
#ifdef USE_SAVER
			}
#endif /* USE_SAVER */

			break;
		}

  case WM_MOUSEWHEEL:
    {
#ifdef USE_SAVER
			if (screensaver_active) {
				stop_screensaver();
				screensaver_active = FALSE;
				return 0;
			} else {
#endif /* USE_SAVER */
				char button, mods;
				int xPos, yPos;
				short n = (short) HIWORD(wParam);

				/* Get the text grid */
				pixel_to_square(&xPos, &yPos,GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));

				if (n > 0) {
					button = 4;
				} else if (n < 0) {
					button = 5;
				}

				/* Extract the modifiers */
				mods = 0;
				if (GetKeyState(VK_CONTROL) & 0x8000) mods |= 2;
				if (GetKeyState(VK_SHIFT)   & 0x8000) mods |= 1;
				if (GetKeyState(VK_MENU)    & 0x8000) mods |= 4;

				Term_mousepress(button, mods, xPos, yPos);
#ifdef USE_SAVER
			}
#endif /* USE_SAVER */
    } break;

#ifdef USE_SAVER
		case WM_MOUSEMOVE:
		{
			if (!screensaver_active) break;

			if (iMouse) {
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS)) {
					stop_screensaver();
				}
			}

			/* Save last location */
			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);

			return 0;
		}
#endif /* USE_SAVER */

		case WM_INITMENU:
		{
			setup_menus();
			return 0;
		}

		case WM_CLOSE:
		{
			if (game_in_progress && character_generated) {
				if (!p_ptr->cmd.inkey_flag) {
					plog("You may not do that right now.");
					return 0;
				}

				/* Hack -- Forget messages */
				msg_flag = FALSE;

				/* Save the game */
#ifdef ZANGBAND
				do_cmd_save_game(FALSE);
#else /* ZANGBAND */
				do_cmd_save_game();
#endif /* ZANGBAND */
			}
			quit(NULL);
			return 0;
		}

		case WM_QUIT:
		{
			quit(NULL);
			return 0;
		}

		case WM_COMMAND:
		{
			process_menus(LOWORD(wParam));
			return 0;
		}

		case WM_SIZE:
		{
			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* it was sent from inside CreateWindowEx */
			if (!td->w) return 1;

			/* was sent from WM_SIZE */
			if (td->size_hack) return 1;

			switch (wParam)
			{
				case SIZE_MINIMIZED:
				{
					/* Hide sub-windows */
					for (i = 1; i < MAX_TERM_DATA; i++) {
						if (data[i].visible) ShowWindow(data[i].w, SW_HIDE);
					}
					return 0;
				}

				case SIZE_MAXIMIZED:
				{
					/* fall through XXX XXX XXX */
				}

				case SIZE_RESTORED:
				{
					int cols = (LOWORD(lParam) - td->size_ow1) / td->tile_wid;
					int rows = (HIWORD(lParam) - td->size_oh1) / td->tile_hgt;

					/* New size */
					if ((td->cols != cols) || (td->rows != rows)) {
						/* Save the new size */
						td->cols = cols;
						td->rows = rows;

						/* Activate */
						Term_activate(&td->t);

						/* Resize the term */
						Term_resize(td->cols, td->rows);

						/* Redraw later */
						InvalidateRect(td->w, NULL, TRUE);
					}

					td->size_hack = TRUE;

					/* Show sub-windows */
					for (i = 1; i < MAX_TERM_DATA; i++) {
						if (data[i].visible) ShowWindow(data[i].w, SW_SHOW);
					}

					td->size_hack = FALSE;

					return 0;
				}
			}
			break;
		}

		case WM_PALETTECHANGED:
		{
			/* Ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return 0;

			/* Fall through... */
		}

		case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return 0;

			hdc = GetDC(hWnd);

			SelectPalette(hdc, hPal, FALSE);

			i = RealizePalette(hdc);

			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, TRUE);

			ReleaseDC(hWnd, hdc);

			return 0;
		}

		case WM_ACTIVATE:
		{
			if (wParam && !HIWORD(lParam)) {
				/* Do something to sub-windows */
				for (i = 1; i < MAX_TERM_DATA; i++) {
					SetWindowPos(data[i].w, hWnd, 0, 0, 0, 0,
					             SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
				}

				/* Focus on main window */
				SetFocus(hWnd);

				return 0;
			}

			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static LRESULT FAR PASCAL AngbandListProc(HWND hWnd, UINT uMsg,
                                           WPARAM wParam, LPARAM lParam)
{
	term_data *td;
	HDC hdc;
	int i;

#ifdef USE_SAVER
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;
#endif /* USE_SAVER */


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLong(hWnd, 0);

	/* Process message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			SetWindowLong(hWnd, 0, (LONG)(my_td));
			break;
		}

		/* XXX XXX XXX */
		case WM_CREATE:
		{
			return 0;
		}

		case WM_GETMINMAXINFO:
		{
#if 0
			MINMAXINFO FAR *lpmmi;
			RECT rc;

			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			lpmmi = (MINMAXINFO FAR *)lParam;

			/* Minimum size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 8 * td->tile_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 2 * td->tile_hgt + td->size_oh1 + td->size_oh2;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save the minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			/* Maximum window size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 80 * td->tile_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 24 * td->tile_hgt + td->size_oh1 + td->size_oh2;

			/* Paranoia */
			rc.right += (td->tile_wid - 1);
			rc.bottom += (td->tile_hgt - 1);

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, TRUE, td->dwExStyle);

			/* Save maximum size */
			lpmmi->ptMaxSize.x = rc.right - rc.left;
			lpmmi->ptMaxSize.y = rc.bottom - rc.top;

			/* Save the maximum size */
			lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
			lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;
#endif /* 0 */
			return 0;
		}

		case WM_SIZE:
		{
			int cols;
			int rows;

			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* it was sent from inside CreateWindowEx */
			if (!td->w) return 1;

			/* was sent from inside WM_SIZE */
			if (td->size_hack) return 1;

			td->size_hack = TRUE;

			cols = (LOWORD(lParam) - td->size_ow1) / td->tile_wid;
			rows = (HIWORD(lParam) - td->size_oh1) / td->tile_hgt;

			/* New size */
			if ((td->cols != cols) || (td->rows != rows)) {
				/* Save old term */
				term *old_term = Term;

				/* Save the new size */
				td->cols = cols;
				td->rows = rows;

				/* Activate */
				Term_activate(&td->t);

				/* Resize the term */
				Term_resize(td->cols, td->rows);

				/* Activate */
				Term_activate(old_term);

				/* Redraw later */
				InvalidateRect(td->w, NULL, TRUE);

				/* HACK - Redraw all windows */
				p_ptr->window = 0xFFFFFFFF;
				window_stuff();
			}

			td->size_hack = FALSE;

			return 0;
		}

		case WM_PAINT:
		{
			handle_wm_paint(hWnd);

			return 0;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			bool mc = FALSE;
			bool ms = FALSE;
			bool ma = FALSE;

#ifdef USE_SAVER
			if (screensaver_active) {
				stop_screensaver();
				return 0;
			}
#endif /* USE_SAVER */

			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) mc = TRUE;
			if (GetKeyState(VK_SHIFT)   & 0x8000) ms = TRUE;
			if (GetKeyState(VK_MENU)    & 0x8000) ma = TRUE;

			/* Handle "special" keys */
			if (special_key[(byte)(wParam)]) {
				/* Begin the macro trigger */
				Term_keypress(31);

				/* Send the modifiers */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (ma) Term_keypress('A');

				/* Extract "scan code" */
				i = LOBYTE(HIWORD(lParam));

				/* Introduce the scan code */
				Term_keypress('x');

				/* Encode the hexidecimal scan code */
				Term_keypress(hexsym[i/16]);
				Term_keypress(hexsym[i%16]);

				/* End the macro trigger */
				Term_keypress(13);

				return 0;
			}

			break;
		}

		case WM_CHAR:
		{
			Term_keypress(wParam);
			return 0;
		}

#ifdef USE_SAVER

		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONDOWN:
		{
			if (screensaver_active) {
				stop_screensaver();
				return 0;
			}

			break;
		}

		case WM_MOUSEMOVE:
		{
			if (!screensaver_active) break;

			if (iMouse) {
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS)) {
					stop_screensaver();
				}
			}

			/* Save last location */
			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);

			return 0;
		}
#endif /* USE_SAVER */

		case WM_PALETTECHANGED:
		{
			/* ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return FALSE;
			/* otherwise, fall through!!! */
		}

		case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return 0;
			hdc = GetDC(hWnd);
			SelectPalette(hdc, hPal, FALSE);
			i = RealizePalette(hdc);
			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, TRUE);
			ReleaseDC(hWnd, hdc);
			return 0;
		}

		case WM_NCLBUTTONDOWN:
		{

#ifdef HTCLOSE
			if (wParam == HTCLOSE) wParam = HTSYSMENU;
#endif /* HTCLOSE */

			if (wParam == HTSYSMENU) {
				if (td->visible) {
					td->visible = FALSE;
					ShowWindow(td->w, SW_HIDE);
				}

				return 0;
			}

			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef USE_SAVER

LRESULT FAR PASCAL AngbandSaverProc(HWND hWnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;


	/* Process */
	switch (uMsg)
	{
		/* XXX XXX XXX */
		case WM_NCCREATE:
		{
			break;
		}

		case WM_SETCURSOR:
		{
			SetCursor(NULL);
			return 0;
		}

#if 0
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE) break;

			/* else fall through */
		}
#endif /* 0 */

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_KEYDOWN:
		{
			stop_screensaver();
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			if (iMouse) {
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS)) {
					stop_screensaver();
				}
			}

			/* Save last location */
			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);

			return 0;
		}

		case WM_CLOSE:
		{
			DestroyWindow(hwndSaver);
			if (screensaver)
				SendMessage(data[0].w, WM_CLOSE, 0, 0);
			hwndSaver = NULL;
			return 0;
		}
	}

	/* Oops */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif /* USE_SAVER */





/*** Temporary Hooks ***/


/*
 * Display warning message (see "z-util.c")
 */
static void hack_plog(cptr str)
{
	/* Give a warning */
	if (str) {
		MessageBox(NULL, str, "Warning",
		           MB_ICONEXCLAMATION | MB_OK);
	}
}


/*
 * Display error message and quit (see "z-util.c")
 */
static void hack_quit(cptr str)
{
	/* Give a warning */
	if (str) {
		MessageBox(NULL, str, "Error",
		           MB_ICONEXCLAMATION | MB_OK | MB_ICONSTOP);
	}

	/* Unregister the classes */
	UnregisterClass(AppName, hInstance);

	/* Destroy the icon */
	if (hIcon) DestroyIcon(hIcon);

#ifdef USE_SAVER
	if (screensaverSemaphore) CloseHandle(screensaverSemaphore);
#endif /* USE_SAVER */

	/* Exit */
	exit(0);
}



/*** Various hooks ***/


/*
 * Display warning message (see "z-util.c")
 */
static void hook_plog(cptr str)
{
#ifdef USE_SAVER
	if (screensaver_active) return;
#endif /* USE_SAVER */

	/* Warning */
	if (str) {
		MessageBox(data[0].w, str, "Warning",
		           MB_ICONEXCLAMATION | MB_OK);
	}
}


/*
 * Display error message and quit (see "z-util.c")
 */
static void hook_quit(cptr str)
{
	int i;

#ifdef USE_SOUND
	int j;
#endif /* USE_SOUND */



#ifdef USE_SAVER
	if (!screensaver_active)
#endif /* USE_SAVER */
	{
		/* Give a warning */
		if (str) {
			MessageBox(data[0].w, str, "Error",
			           MB_ICONEXCLAMATION | MB_OK | MB_ICONSTOP);
		}

		/* Save the preferences */
		save_prefs();
	}

	/*** Could use 'Term_nuke_win()' XXX XXX XXX */

	/* Destroy all windows */
	for (i = MAX_TERM_DATA - 1; i >= 0; --i) {
		term_force_font(&data[i], NULL);
		if (data[i].font_want) string_free(data[i].font_want);
		if (data[i].w) DestroyWindow(data[i].w);
		data[i].w = 0;

		term_nuke(&data[i].t);
	}

#ifdef USE_GRAPHICS
	/* Free the bitmap stuff */
	FreeDIB(&infGraph);
	FreeDIB(&infMask);
#endif /* USE_GRAPHICS */

	close_graphics_modes();

#ifdef USE_SOUND
	/* Free the sound names */
	for (i = 0; i < SOUND_MAX; i++) {
		for (j = 0; j < SAMPLE_MAX; j++) {
			if (!sound_file[i][j]) break;

			string_free(sound_file[i][j]);
		}
	}
#endif /* USE_SOUND */

	/*** Free some other stuff ***/

	DeleteObject(hbrYellow);

	if (hPal) DeleteObject(hPal);

	UnregisterClass(AppName, hInstance);

	if (hIcon) DestroyIcon(hIcon);

	/* Free strings */
	string_free(ini_file);
	string_free(argv0);
	//string_free(ANGBAND_DIR_XTRA_FONT);
	//string_free(ANGBAND_DIR_XTRA_GRAF);
	//string_free(ANGBAND_DIR_XTRA_SOUND);
	string_free(ANGBAND_DIR_XTRA_HELP);

#ifdef USE_MUSIC
	string_free(ANGBAND_DIR_XTRA_MUSIC);
#endif /* USE_MUSIC */

#ifdef HAS_CLEANUP
	cleanup_angband();
#endif /* HAS_CLEANUP */

	exit(0);
}



/*** Initialize ***/


/*
 * Init some stuff
 */
static void init_stuff(void)
{
	int i;

	char path[1024];
#ifdef USE_SAVER
	char tmp[1024];
#endif /* USE_SAVER */

	/* Get program name with full path */
	if (GetModuleFileName(hInstance, path, sizeof(path)) == 0)
		show_win_error();

	/* Paranoia */
	path[sizeof(path) - 1] = '\0';

	/* Save the "program name" */
	argv0 = string_make(path);

	/* Get the name of the "*.ini" file */
	strcpy(path + strlen(path) - 4, ".INI");

#ifdef USE_SAVER

	/* Try to get the path to the Angband folder */
	if (screensaver) {
		/* Extract the filename of the savefile for the screensaver */
		GetPrivateProfileString("Angband", "SaverFile", "", saverfilename, sizeof(saverfilename), path);

		GetPrivateProfileString("Angband", "AngbandPath", "", tmp, sizeof(tmp), path);

		strnmt(path, sizeof(path), "%szangband.ini", tmp);
	}

#endif /* USE_SAVER */

	/* Save the the name of the ini-file */
	ini_file = string_make(path);

	/* Analyze the path */
	i = strlen(path);

	/* Get the path */
	for (; i > 0; i--) {
		if (path[i] == '\\') {
			/* End of path */
			break;
		}
	}

	/* Add "lib" to the path */
	strcpy(path + i + 1, "lib\\");

	/* Validate the path */
	validate_dir(path);

	/* Init the file paths */
#ifdef PRIVATE_USER_PATH
	init_file_paths(path,path,PRIVATE_USER_PATH,NULL);
#else /* PRIVATE_USER_PATH */
	init_file_paths(path,path,path,path);
#endif /* PRIVATE_USER_PATH */

	/* make sure that the user directories exist */
	create_user_dirs();

	/* Hack -- Validate the paths */
	validate_dir(ANGBAND_DIR_APEX);
	validate_dir(ANGBAND_DIR_BONE);
	validate_dir(ANGBAND_DIR_DATA);
	validate_dir(ANGBAND_DIR_EDIT);

	validate_dir(ANGBAND_DIR_SCRIPT);

	validate_dir(ANGBAND_DIR_FILE);
	validate_dir(ANGBAND_DIR_HELP);
	validate_dir(ANGBAND_DIR_INFO);
	validate_dir(ANGBAND_DIR_PREF);
	validate_dir(ANGBAND_DIR_SAVE);
	validate_dir(ANGBAND_DIR_USER);
	validate_dir(ANGBAND_DIR_XTRA);

	/* Build the filename */
	path_make(path, ANGBAND_DIR_FILE, "news.txt");

	/* Hack -- Validate the "news.txt" file */
	validate_file(path);


	/* Build the "font" path */
	//path_make(path, ANGBAND_DIR_XTRA, "font");

	/* Allocate the path */
	//ANGBAND_DIR_XTRA_FONT = string_make(path);

	/* Validate the "font" directory */
	validate_dir(ANGBAND_DIR_XTRA_FONT);

	/* Build the filename */
	path_make(path, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

	/* Hack -- Validate the basic font */
	validate_file(path);


#ifdef USE_GRAPHICS

	/* Build the "graf" path */
	//path_make(path, ANGBAND_DIR_XTRA, "graf");

	/* Allocate the path */
	//ANGBAND_DIR_XTRA_GRAF = string_make(path);

	/* Validate the "graf" directory */
	validate_dir(ANGBAND_DIR_XTRA_GRAF);

#endif /* USE_GRAPHICS */


#ifdef USE_SOUND

	/* Build the "sound" path */
	//path_make(path, ANGBAND_DIR_XTRA, "sound");

	/* Allocate the path */
	//ANGBAND_DIR_XTRA_SOUND = string_make(path);

	/* Validate the "sound" directory */
	validate_dir(ANGBAND_DIR_XTRA_SOUND);

#endif /* USE_SOUND */

#ifdef USE_MUSIC

	/* Build the "music" path */
	path_make(path, ANGBAND_DIR_XTRA, "music");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_MUSIC = string_make(path);

	/* Validate the "music" directory */
	validate_dir(ANGBAND_DIR_XTRA_MUSIC);

#endif /* USE_MUSIC */

	/* Build the "help" path */
	path_make(path, ANGBAND_DIR_XTRA, "help");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_HELP = string_make(path);

#if 0
	/* Validate the "help" directory */
	validate_dir(ANGBAND_DIR_XTRA_HELP);
#endif /* 0 */
}


/*
 * Test to see if we need to work-around bugs in
 * the windows ascii-drawing routines.
 */
bool broken_ascii(void)
{
	OSVERSIONINFO Dozeversion;
 	Dozeversion.dwOSVersionInfoSize = sizeof(Dozeversion);

	if (GetVersionEx(&Dozeversion)) {
		/* Win XP is b0rken */
		if ((Dozeversion.dwPlatformId >= VER_PLATFORM_WIN32_NT) &&
			(Dozeversion.dwMajorVersion >= 5))
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


extern void display_introduction();
extern void tomb_menu(bool dump);

int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                       LPSTR lpCmdLine, int nCmdShow)
{
	int i;

	WNDCLASS wc;
	HDC hdc;
	MSG msg;

	/* Unused parameter */
	(void)nCmdShow;

#ifdef USE_SAVER
	if (lpCmdLine && ((*lpCmdLine == '-') || (*lpCmdLine == '/'))) {
		lpCmdLine++;

		switch (*lpCmdLine)
		{
			case 's':
			case 'S':
			{
				screensaver = TRUE;

				/* Only run one screensaver at the time */
				screensaverSemaphore = CreateSemaphore(NULL, 0, 1, "AngbandSaverSemaphore");

				if (!screensaverSemaphore) exit(0);

				if (GetLastError() == ERROR_ALREADY_EXISTS) {
					CloseHandle(screensaverSemaphore);
					exit(0);
				}

				break;
			}

			case 'P':
			case 'p':
			case 'C':
			case 'c':
			case 'A':
			case 'a':
			{
				/*
				 * ToDo: implement preview, configuration, and changing
				 * the password (as well as checking it).
				 */
				exit(0);
			}
		}
	}

#endif /* USE_SAVER */

	/* Initialize */
	if (hPrevInst == NULL) {
		wc.style         = CS_CLASSDC;
		wc.lpfnWndProc   = AngbandWndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 4; /* one long pointer to term_data */
		wc.hInstance     = hInst;
		wc.hIcon         = hIcon = LoadIcon(hInst, "ANGBAND");
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName  = "ANGBAND";
		wc.lpszClassName = AppName;

		if (!RegisterClass(&wc)) exit(1);

		wc.lpfnWndProc   = AngbandListProc;
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = AngList;

		if (!RegisterClass(&wc)) exit(2);

#ifdef USE_SAVER

		wc.style          = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS | CS_DBLCLKS;
		wc.lpfnWndProc    = AngbandSaverProc;
		wc.hCursor        = NULL;
		wc.lpszMenuName   = NULL;
		wc.lpszClassName  = "WindowsScreenSaverClass";

		if (!RegisterClass(&wc)) exit(3);

#endif /* USE_SAVER */

	}

	/* Save globally */
	hInstance = hInst;

	/* Temporary hooks */
	plog_aux = hack_plog;
	quit_aux = hack_quit;
	core_aux = hack_quit;

	/* Prepare the filepaths */
	init_stuff();

	/* Initialize the keypress analyzer */
	for (i = 0; special_key_list[i]; i++) {
		special_key[special_key_list[i]] = TRUE;
	}

	/* Determine if display is 16/256/true color */
	hdc = GetDC(NULL);
	colors16 = (GetDeviceCaps(hdc, BITSPIXEL) == 4);
	paletted = ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ? TRUE : FALSE);
	ReleaseDC(NULL, hdc);

	/* Initialize the colors */
	for (i = 0; i < 256; i++) {
		byte rv, gv, bv;

		/* Extract desired values */
		rv = angband_color_table[i][1];
		gv = angband_color_table[i][2];
		bv = angband_color_table[i][3];

		/* Extract the "complex" code */
		win_clr[i] = PALETTERGB(rv, gv, bv);

		/* Save the "simple" code */
		angband_color_table[i][0] = win_pal[i];
	}

	/* load the possible graphics modes */
	if (!init_graphics_modes("graphics.txt")) {
		plog_fmt("Graphics list load failed");
	}

	/* Prepare the windows */
	init_windows();

	/* Activate hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;
	core_aux = hook_quit;

	/* Set the system suffix */
	ANGBAND_SYS = "win";
	
	if (broken_ascii()) {
		ANGBAND_SYS = "w2k";
	}

	/* Initialize */
	init_angband();

	/* We are now initialized */
	initialized = TRUE;

	/* Did the user double click on a save file? */
	check_for_save_file(lpCmdLine);
	
	/* Save the hooks into the overhead map */
	set_callback((callback_type) win_map_info, CALL_MAP_INFO, NULL);

	/* Save player movement hook */
	set_callback((callback_type) win_player_move, CALL_PLAYER_MOVE, NULL);

	/* If we are using port specific hooks, set them here */
	/* This section to play repeated games without exiting first was
	 * modified from Sil */
	while (1) {
		bool new_game = FALSE;
    
#ifdef USE_SAVER
		if (screensaver) {
			/* Start the screensaver */
			start_screensaver();
		}
#endif /* USE_SAVER */

		/* Let the player choose a savefile or start a new game */
		if (!game_in_progress) {
			//int choice = 0;
			//int highlight = 1;
			char key;

			button_kill_all();
			//if (p_ptr->state.is_dead) highlight = 4;
			/* Prompt the user */
			if (savefile[0] != 0) {
				prtf(10, 22, "[Choose $U'($NN$R)ew'$Yn$V, $U'($NO$R)pen'$Yo$V, or $U'e($NX$R)it'$Yx$V from the 'File' menu]");
				prtf(10, 23, "  [Or choose to $U'load ($NL$R)ast'$Yl$V or $U'return to ($NG$R)raveyard'$Yg$V]");
			} else {
				prtf(10, 23, "[Choose $U'($NN$R)ew'$Yn$V, $U'($NO$R)pen'$Yo$V, or $U'e($NX$R)it'$Yx$V from the 'File' menu]");
			}
			Term_fresh();


			/* Process Events until "new" or "open" is selected */
			while (!game_in_progress) {
				//OPENFILENAME ofn;

				/* process any windows messages */
				while (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				/* see if there is a key press on the queue */
				Term_inkey(&key, FALSE,TRUE);
				if ((key == 'n') || (key == 'N')) {
					/* New game */
					if (savefile[0] != 0) {
						quit("Quiting because repeated play is not working yet.");
					}
					savefile[0] = 0;
					process_menus(IDM_FILE_NEW);
				} else
				if ((key == 'o') || (key == 'O')) {
					/* Load a save game */
					if (savefile[0] != 0) {
						quit("Quiting because repeated play is not working yet.");
					}
					process_menus(IDM_FILE_OPEN);
				} else
				if ((key == 'l') || (key == 'L')) {
					if (savefile[0] != 0) {
						quit("Quiting because repeated play is not working yet.");
					}
					if (savefile[0] != 0) {
						/* Load the last game */
						game_in_progress = TRUE;
						new_game = FALSE;
					}
				} else
				if ((key == 'g') || (key == 'G')) {
					if (savefile[0] != 0) {
						/* show the graveyard */
						tomb_menu(FALSE);

						button_kill_all();

						/* Show the initial screen again */
						display_introduction();

						/* Prompt the user */
						prtf(10, 22, "[Choose $U'($NN$R)ew'$Yn$V, $U'($NO$R)pen'$Yo$V, or $U'e($NX$R)it'$Yx$V from the 'File' menu]");
						prtf(10, 23, "  [Or choose to $U'load ($NL$R)ast$Yl$V' or $U'return to ($NG$R)raveyard'$Yg$V]");

						/* Flush it */
						Term_fresh();
					}
				} else
				/*if ((key == 't') || (key == 'T')) {
					/* Start the tutorial */ /*
					game_in_progress = TRUE;
					new_game = FALSE;
				} else*/
				if ((key == 's') || (key == 'S')) {		
					/* Show the top scores */
					/* HACK - make sure that there is a resize hook, to prevent
					 * a crash after displaying the scores */
					if (!angband_term[0]->resize_hook) {
						angband_term[0]->resize_hook = resize_map;
					}
					/* show scores */
					top_twenty();

					/* Show the initial screen again */
					display_introduction();

					/* Prompt the user */
					if (savefile[0] != 0) {
						prtf(10, 22, "[Choose $U'($NN$R)ew'$Yn$V, $U'($NO$R)pen'$Yo$V, or $U'e($NX$R)it'$Yx$V from the 'File' menu]");
						prtf(10, 23, "  [Or choose to $U'load ($NL$R)ast'$Yl$V or $U'return to ($NG$R)raveyard'$Yg$V]");
					} else {
						prtf(10, 23, "[Choose $U'($NN$R)ew'$Yn$V, $U'($NO$R)pen'$Yo$V, or $U'e($NX$R)it'$Yx$V from the 'File' menu]");
					}

					/* Flush it */
					Term_fresh();
				} else
				if ((key == 'x') || (key == 'X')) {
					/* Quit */
					quit(NULL);
				}
			}
		}

		button_kill_all();

		/* Handle pending events (most notably update) and flush input */
		Term_flush();

		if (savefile[0] != 0) {
			new_game = FALSE;
		} else {
			new_game = TRUE;
		}
		/*
		 * Play a game -- "new_game" is set by "new", "open" or the open document
		 * event handler as appropriate
		 */
		play_game(new_game);

		// game no longer in progress
		game_in_progress = FALSE;

		/* Free the messages */
		messages_free();

		/* Free the "quarks" */
		//quarks_free();

		//cleanup_angband();

		/* Hack - redraw everything + recalc bigtile regions */
		angband_term[0]->resize_hook();
    
		// reset some globals that start at 0
		character_loaded = FALSE;

		// rerun the first initialization routine
		//init_stuff();
		
		// do some more between-games initialization
		//re_init_some_things();
		/* Initialize the "quark" package */
		//(void)quarks_init();

		/* Initialize the "message" package */
		(void)messages_init();

		/* Show the initial screen again */
		display_introduction();

		/* Flush it */
		Term_fresh();
		//init_angband();

		/* reinitialize arrays that use quarks */
		//note("[Initializing arrays... (wilderness)]");
		//if (init_w_info()) quit("Cannot initialize wilderness");
	}

	/* Paranoia */
	quit(NULL);

	/* Paranoia */
	return (0);
}

#endif /* WINDOWS */
