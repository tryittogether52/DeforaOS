/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Keyboard */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";
/* TODO:
 * - display "likely" keys (after modifiers) as well
 * - see if XKB could be used to define the keyboard */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION(3, 0, 0)
# include <gtk/gtkx.h>
#endif
#include <Desktop.h>
#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include "callbacks.h"
#include "layout.h"
#include "keyboard.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Keyboard */
/* private */
/* types */
struct _Keyboard
{
	/* preferences */
	KeyboardMode mode;

	KeyboardLayout ** layouts;
	size_t layouts_cnt;

	PangoFontDescription * font;
	GtkWidget * window;
#if GTK_CHECK_VERSION(2, 10, 0)
	GtkStatusIcon * icon;
#endif
	GtkWidget * ab_window;
	GdkRectangle geometry;
	int width;
	int height;
	int x;
	int y;
};

typedef struct _KeyboardKeyDefinition
{
	unsigned int row;
	unsigned int width;
	unsigned int modifier;
	unsigned int keysym;
	char const * label;
} KeyboardKeyDefinition;

typedef enum _KeyboardLayoutSection
{
	KLS_LETTERS = 0,
	KLS_KEYPAD,
	KLS_SPECIAL
} KeyboardLayoutSection;
#define KLS_LAST KLS_SPECIAL
#define KLS_COUNT (KLS_LAST + 1)

typedef struct _KeyboardLayoutDefinition
{
	char const * label;
	KeyboardKeyDefinition const * keys;
} KeyboardLayoutDefinition;


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static const DesktopMenu _keyboard_menu_file[] =
{
	{ N_("_Quit"), G_CALLBACK(on_file_quit), GTK_STOCK_QUIT,
		GDK_CONTROL_MASK, GDK_KEY_Q },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _keyboard_menu_view[] =
{
	{ N_("_Hide"), G_CALLBACK(on_view_hide), NULL, GDK_CONTROL_MASK,
		GDK_KEY_H },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _keyboard_menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
#else
	{ N_("_About"), G_CALLBACK(on_help_about), NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenubar _keyboard_menubar[] =
{
	{ N_("_File"), _keyboard_menu_file },
	{ N_("_View"), _keyboard_menu_view },
	{ N_("_Help"), _keyboard_menu_help },
	{ NULL, NULL }
};


/* variables */
static KeyboardKeyDefinition const _keyboard_layout_letters[] =
{
	{ 0, 2, 0, XK_q, "q" },
	{ 0, 0, XK_Shift_L, XK_Q, "Q" },
	{ 0, 2, 0, XK_w, "w" },
	{ 0, 0, XK_Shift_L, XK_W, "W" },
	{ 0, 2, 0, XK_e, "e" },
	{ 0, 0, XK_Shift_L, XK_E, "E" },
	{ 0, 2, 0, XK_r, "r" },
	{ 0, 0, XK_Shift_L, XK_R, "R" },
	{ 0, 2, 0, XK_t, "t" },
	{ 0, 0, XK_Shift_L, XK_T, "T" },
	{ 0, 2, 0, XK_y, "y" },
	{ 0, 0, XK_Shift_L, XK_Y, "Y" },
	{ 0, 2, 0, XK_u, "u" },
	{ 0, 0, XK_Shift_L, XK_U, "U" },
	{ 0, 2, 0, XK_i, "i" },
	{ 0, 0, XK_Shift_L, XK_I, "I" },
	{ 0, 2, 0, XK_o, "o" },
	{ 0, 0, XK_Shift_L, XK_O, "O" },
	{ 0, 2, 0, XK_p, "p" },
	{ 0, 0, XK_Shift_L, XK_P, "P" },
	{ 1, 1, 0, 0, NULL },
	{ 1, 2, 0, XK_a, "a" },
	{ 1, 0, XK_Shift_L, XK_A, "A" },
	{ 1, 2, 0, XK_s, "s" },
	{ 1, 0, XK_Shift_L, XK_S, "S" },
	{ 1, 2, 0, XK_d, "d" },
	{ 1, 0, XK_Shift_L, XK_D, "D" },
	{ 1, 2, 0, XK_f, "f" },
	{ 1, 0, XK_Shift_L, XK_F, "F" },
	{ 1, 2, 0, XK_g, "g" },
	{ 1, 0, XK_Shift_L, XK_G, "G" },
	{ 1, 2, 0, XK_h, "h" },
	{ 1, 0, XK_Shift_L, XK_H, "H" },
	{ 1, 2, 0, XK_j, "j" },
	{ 1, 0, XK_Shift_L, XK_J, "J" },
	{ 1, 2, 0, XK_k, "k" },
	{ 1, 0, XK_Shift_L, XK_K, "K" },
	{ 1, 2, 0, XK_l, "l" },
	{ 1, 0, XK_Shift_L, XK_L, "L" },
	{ 2, 2, 0, XK_Shift_L, "\xe2\x87\xa7" },
	{ 2, 2, 0, XK_z, "z" },
	{ 2, 0, XK_Shift_L, XK_Z, "Z" },
	{ 2, 2, 0, XK_x, "x" },
	{ 2, 0, XK_Shift_L, XK_X, "X" },
	{ 2, 2, 0, XK_c, "c" },
	{ 2, 0, XK_Shift_L, XK_C, "C" },
	{ 2, 2, 0, XK_v, "v" },
	{ 2, 0, XK_Shift_L, XK_V, "V" },
	{ 2, 2, 0, XK_b, "b" },
	{ 2, 0, XK_Shift_L, XK_B, "B" },
	{ 2, 2, 0, XK_n, "n" },
	{ 2, 0, XK_Shift_L, XK_N, "N" },
	{ 2, 2, 0, XK_m, "m" },
	{ 2, 0, XK_Shift_L, XK_M, "M" },
	{ 2, 2, 0, XK_comma, "," },
	{ 2, 0, XK_Shift_L, XK_comma, "<" },
	{ 2, 2, 0, XK_period, "." },
	{ 2, 0, XK_Shift_L, XK_period, ">" },
	{ 3, 3, 0, 0, NULL },
	{ 3, 3, 0, XK_Control_L, "Ctrl" },
	{ 3, 3, 0, XK_Alt_L, "Alt" },
	{ 3, 5, 0, XK_space, " " },
	{ 3, 0, XK_Shift_L, XK_space, " " },
	{ 3, 3, 0, XK_Return, "\xe2\x86\xb2" },
	{ 3, 3, 0, XK_BackSpace, "\xe2\x8c\xab" },
	{ 0, 0, 0, 0, NULL }
};

static KeyboardKeyDefinition const _keyboard_layout_keypad[] =
{
	{ 0, 3, 0, XK_Num_Lock, "Num" },
	{ 0, 1, 0, 0, NULL },
	{ 0, 4, 0, XK_KP_Home, "\xe2\x86\x96" },
	{ 0, 0, XK_Num_Lock, XK_7, "7" },
	{ 0, 4, 0, XK_KP_Up, "\xe2\x86\x91" },
	{ 0, 0, XK_Num_Lock, XK_8, "8" },
	{ 0, 4, 0, XK_KP_Page_Up, "\xe2\x87\x9e" },
	{ 0, 0, XK_Num_Lock, XK_9, "9" },
	{ 0, 1, 0, 0, NULL },
	{ 0, 3, 0, XK_KP_Subtract, "-" },
	{ 1, 3, 0, XK_KP_Divide, "/" },
	{ 1, 1, 0, 0, NULL },
	{ 1, 4, 0, XK_KP_Left, "\xe2\x86\x90" },
	{ 1, 0, XK_Num_Lock, XK_4, "4" },
	{ 1, 4, 0, XK_5, "5" },
	{ 1, 0, XK_Num_Lock, XK_5, "5" },
	{ 1, 4, 0, XK_KP_Right, "\xe2\x86\x92" },
	{ 1, 0, XK_Num_Lock, XK_6, "6" },
	{ 1, 1, 0, 0, NULL },
	{ 1, 3, 0, XK_KP_Add, "+" },
	{ 2, 3, 0, XK_KP_Multiply, "*" },
	{ 2, 1, 0, 0, NULL },
	{ 2, 4, 0, XK_KP_End, "\xe2\x86\x99" },
	{ 2, 0, XK_Num_Lock, XK_1, "1" },
	{ 2, 4, 0, XK_KP_Down, "\xe2\x86\x93" },
	{ 2, 0, XK_Num_Lock, XK_2, "2" },
	{ 2, 4, 0, XK_KP_Page_Down, "\xe2\x87\x9f" },
	{ 2, 0, XK_Num_Lock, XK_3, "3" },
	{ 2, 1, 0, 0, NULL },
	{ 2, 3, 0, XK_KP_Enter, "\xe2\x86\xb2" },
	{ 3, 3, 0, 0, NULL },
	{ 3, 1, 0, 0, NULL },
	{ 3, 8, 0, XK_KP_Insert, "Ins" },
	{ 3, 0, XK_Num_Lock, XK_0, "0" },
	{ 3, 4, 0, XK_KP_Delete, "Del" },
	{ 3, 0, XK_Num_Lock, XK_KP_Decimal, "." },
	{ 3, 1, 0, 0, NULL },
	{ 3, 3, 0, XK_BackSpace, "\xe2\x8c\xab" },
	{ 0, 0, 0, 0, NULL }
};

static KeyboardKeyDefinition const _keyboard_layout_special[] =
{
	{ 0, 3, 0, XK_Escape, "Esc" },
	{ 0, 2, 0, XK_F1, "F1" },
	{ 0, 2, 0, XK_F2, "F2" },
	{ 0, 2, 0, XK_F3, "F3" },
	{ 0, 2, 0, XK_F4, "F4" },
	{ 0, 1, 0, 0, NULL },
	{ 0, 2, 0, XK_F5, "F5" },
	{ 0, 0, XK_Shift_L, XK_F9, "F9" },
	{ 0, 2, 0, XK_F6, "F6" },
	{ 0, 0, XK_Shift_L, XK_F10, "F10" },
	{ 0, 2, 0, XK_F7, "F7" },
	{ 0, 0, XK_Shift_L, XK_F11, "F11" },
	{ 0, 2, 0, XK_F8, "F8" },
	{ 0, 0, XK_Shift_L, XK_F12, "F12" },
	{ 1, 2, 0, XK_1, "1" },
	{ 1, 0, XK_Shift_L, XK_1, "!" },
	{ 1, 2, 0, XK_2, "2" },
	{ 1, 0, XK_Shift_L, XK_2, "@" },
	{ 1, 2, 0, XK_3, "3" },
	{ 1, 0, XK_Shift_L, XK_3, "#" },
	{ 1, 2, 0, XK_4, "4" },
	{ 1, 0, XK_Shift_L, XK_4, "$" },
	{ 1, 2, 0, XK_5, "5" },
	{ 1, 0, XK_Shift_L, XK_5, "%" },
	{ 1, 2, 0, XK_6, "6" },
	{ 1, 0, XK_Shift_L, XK_6, "^" },
	{ 1, 2, 0, XK_7, "7" },
	{ 1, 0, XK_Shift_L, XK_7, "&" },
	{ 1, 2, 0, XK_8, "8" },
	{ 1, 0, XK_Shift_L, XK_8, "*" },
	{ 1, 2, 0, XK_9, "9" },
	{ 1, 0, XK_Shift_L, XK_9, "(" },
	{ 1, 2, 0, XK_0, "0" },
	{ 1, 0, XK_Shift_L, XK_0, ")" },
	{ 2, 3, 0, XK_Tab, "\xe2\x86\xb9" },
	{ 2, 2, 0, XK_grave, "`" },
	{ 2, 0, XK_Shift_L, XK_grave, "~" },
	{ 2, 2, 0, XK_minus, "-" },
	{ 2, 0, XK_Shift_L, XK_minus, "_" },
	{ 2, 2, 0, XK_equal, "=" },
	{ 2, 0, XK_Shift_L, XK_equal, "+" },
	{ 2, 2, 0, XK_backslash, "\\" },
	{ 2, 0, XK_Shift_L, XK_backslash, "|" },
	{ 2, 2, 0, XK_bracketleft, "[" },
	{ 2, 0, XK_Shift_L, XK_bracketleft, "{" },
	{ 2, 2, 0, XK_bracketright, "]" },
	{ 2, 0, XK_Shift_L, XK_bracketright, "}" },
	{ 2, 2, 0, XK_semicolon, ";" },
	{ 2, 0, XK_Shift_L, XK_semicolon, ":" },
	{ 2, 2, 0, XK_apostrophe, "'" },
	{ 2, 0, XK_Shift_L, XK_apostrophe, "\"" },
	{ 3, 3, 0, 0, NULL },
	{ 3, 2, 0, XK_Shift_L, "\xe2\x87\xa7" },
	{ 3, 3, 0, XK_space, " " },
	{ 3, 0, XK_Shift_L, XK_space, " " },
	{ 3, 2, 0, XK_comma, "," },
	{ 3, 0, XK_Shift_L, XK_comma, "<" },
	{ 3, 2, 0, XK_period, "." },
	{ 2, 0, XK_Shift_L, XK_period, ">" },
	{ 3, 2, 0, XK_slash, "/" },
	{ 3, 0, XK_Shift_L, XK_slash, "?" },
	{ 3, 3, 0, XK_Return, "\xe2\x86\xb2" },
	{ 3, 3, 0, XK_BackSpace, "\xe2\x8c\xab" },
	{ 0, 0, 0, 0, NULL }
};

static KeyboardLayoutDefinition _keyboard_layout[KLS_COUNT] =
{
	{ "Abc", _keyboard_layout_letters	},
	{ "123", _keyboard_layout_keypad	},
	{ ",./", _keyboard_layout_special	}
};


/* prototypes */
static GtkWidget * _keyboard_add_layout(Keyboard * keyboard,
		KeyboardLayoutDefinition * definitions,
		size_t definitions_cnt, KeyboardLayoutSection section);


/* public */
/* functions */
/* keyboard_new */
static void _new_mode(Keyboard * keyboard, KeyboardMode mode);
static void _new_mode_docked(Keyboard * keyboard);
static void _new_mode_embedded(Keyboard * keyboard);
static void _new_mode_popup(Keyboard * keyboard);
static void _new_mode_windowed(Keyboard * keyboard);

Keyboard * keyboard_new(KeyboardPrefs * prefs)
{
	Keyboard * keyboard;
	GtkAccelGroup * group;
	GdkScreen * screen;
	GtkWidget * vbox;
	GtkWidget * widget;
	PangoFontDescription * bold;
	GdkColor gray = { 0x90909090, 0x9090, 0x9090, 0x9090 };
	unsigned long id;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((keyboard = malloc(sizeof(*keyboard))) == NULL)
		return NULL;
	keyboard->mode = prefs->mode;
	keyboard->layouts = NULL;
	keyboard->layouts_cnt = 0;
	screen = gdk_screen_get_default();
	if(prefs != NULL && prefs->monitor > 0
			&& prefs->monitor < gdk_screen_get_n_monitors(screen))
		gdk_screen_get_monitor_geometry(screen, prefs->monitor,
				&keyboard->geometry);
	else
		gdk_screen_get_monitor_geometry(screen, 0, &keyboard->geometry);
	/* windows */
	_new_mode(keyboard, prefs->mode);
	gtk_widget_modify_bg(keyboard->window, GTK_STATE_NORMAL, &gray);
	keyboard->icon = NULL;
	keyboard->ab_window = NULL;
	/* fonts */
	if(prefs->font != NULL)
		keyboard->font = pango_font_description_from_string(
				prefs->font);
	else
	{
		keyboard->font = pango_font_description_new();
		pango_font_description_set_weight(keyboard->font,
				PANGO_WEIGHT_BOLD);
	}
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_add(GTK_CONTAINER(keyboard->window), vbox);
	/* menubar */
	if(prefs->mode == KEYBOARD_MODE_WINDOWED)
	{
		group = gtk_accel_group_new();
		gtk_window_add_accel_group(GTK_WINDOW(keyboard->window), group);
		widget = desktop_menubar_create(_keyboard_menubar, keyboard,
				group);
		gtk_widget_show_all(widget);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, FALSE, 0);
		/* XXX border hack */
		widget = gtk_vbox_new(FALSE, 4);
		gtk_container_set_border_width(GTK_CONTAINER(widget), 4);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		gtk_widget_show(vbox);
		vbox = widget;
	}
	/* layouts */
	if((widget = _keyboard_add_layout(keyboard, _keyboard_layout,
					KLS_COUNT, KLS_LETTERS)) != NULL)
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	if((widget = _keyboard_add_layout(keyboard, _keyboard_layout,
					KLS_COUNT, KLS_KEYPAD)) != NULL)
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	if((widget = _keyboard_add_layout(keyboard, _keyboard_layout,
					KLS_COUNT, KLS_SPECIAL)) != NULL)
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_widget_show(vbox);
	if(prefs->mode != KEYBOARD_MODE_EMBEDDED)
	{
#if GTK_CHECK_VERSION(2, 10, 0)
		/* create the systray icon */
		keyboard->icon = gtk_status_icon_new_from_icon_name(
				"input-keyboard");
# if GTK_CHECK_VERSION(2, 16, 0)
		gtk_status_icon_set_tooltip_text(keyboard->icon,
				_("Virtual keyboard"));
# endif
		g_signal_connect_swapped(keyboard->icon, "activate", G_CALLBACK(
					on_systray_activate), keyboard);
		g_signal_connect(keyboard->icon, "popup-menu", G_CALLBACK(
					on_systray_popup_menu), keyboard);
#endif
		/* show the window */
		if(prefs->wait == 0)
			gtk_widget_show(keyboard->window);
	}
	else
	{
		/* print the window ID and force a flush */
		id = gtk_plug_get_id(GTK_PLUG(keyboard->window));
		printf("%lu\n", id);
		fclose(stdout);
	}
	keyboard_set_layout(keyboard, KLS_LETTERS);
	pango_font_description_free(bold);
	/* messages */
	desktop_message_register(KEYBOARD_CLIENT_MESSAGE, on_keyboard_message,
			keyboard);
	return keyboard;
}

static void _new_mode(Keyboard * keyboard, KeyboardMode mode)
{
	switch(mode)
	{
		case KEYBOARD_MODE_DOCKED:
			_new_mode_docked(keyboard);
			break;
		case KEYBOARD_MODE_EMBEDDED:
			_new_mode_embedded(keyboard);
			break;
		case KEYBOARD_MODE_POPUP:
			_new_mode_popup(keyboard);
			break;
		case KEYBOARD_MODE_WINDOWED:
			_new_mode_windowed(keyboard);
			break;
	}
}

static void _new_mode_docked(Keyboard * keyboard)
{
	keyboard->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(keyboard->window), 4);
	gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window), FALSE);
#endif
	gtk_window_set_type_hint(GTK_WINDOW(keyboard->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_stick(GTK_WINDOW(keyboard->window));
	keyboard->width = keyboard->geometry.width;
	keyboard->height = (keyboard->geometry.width / 11) * 3;
	keyboard->x = keyboard->geometry.x;
	keyboard->y = keyboard->geometry.y + keyboard->geometry.height
		- keyboard->height;
	gtk_widget_set_size_request(keyboard->window, keyboard->width,
			keyboard->height);
	gtk_window_move(GTK_WINDOW(keyboard->window), keyboard->x, keyboard->y);
	g_signal_connect_swapped(G_OBJECT(keyboard->window), "delete-event",
			G_CALLBACK(on_keyboard_delete_event), keyboard);
}

static void _new_mode_embedded(Keyboard * keyboard)
{
	keyboard->window = gtk_plug_new(0);
	keyboard->width = 0;
	keyboard->height = 0;
	keyboard->x = 0;
	keyboard->y = 0;
	g_signal_connect_swapped(G_OBJECT(keyboard->window), "embedded",
			G_CALLBACK(on_keyboard_embedded), keyboard);
}

static void _new_mode_popup(Keyboard * keyboard)
{
	keyboard->window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_container_set_border_width(GTK_CONTAINER(keyboard->window), 4);
	gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window), FALSE);
	gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window), FALSE);
	keyboard->width = keyboard->geometry.width;
	keyboard->height = (keyboard->geometry.width / 11) * 3;
	keyboard->x = keyboard->geometry.x;
	keyboard->y = keyboard->geometry.y + keyboard->geometry.height
		- keyboard->height;
	gtk_window_move(GTK_WINDOW(keyboard->window), keyboard->x, keyboard->y);
	gtk_widget_set_size_request(keyboard->window, keyboard->width,
			keyboard->height);
	g_signal_connect_swapped(G_OBJECT(keyboard->window), "delete-event",
			G_CALLBACK(on_keyboard_delete_event), keyboard);
}

static void _new_mode_windowed(Keyboard * keyboard)
{
	keyboard->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	keyboard->width = 0;
	keyboard->height = 0;
	keyboard->x = 0;
	keyboard->y = 0;
	gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window), FALSE);
	gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(keyboard->window),
			"input-keyboard");
#endif
	gtk_window_set_title(GTK_WINDOW(keyboard->window), _("Keyboard"));
	g_signal_connect_swapped(G_OBJECT(keyboard->window), "delete-event",
			G_CALLBACK(on_keyboard_delete_event), keyboard);
}


/* keyboard_delete */
void keyboard_delete(Keyboard * keyboard)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_widget_destroy(keyboard->window);
	pango_font_description_free(keyboard->font);
	free(keyboard);
}


/* accessors */
/* keyboard_is_visible */
gboolean keyboard_is_visible(Keyboard * keyboard)
{
	return gtk_widget_get_visible(keyboard->window);
}


/* keyboard_set_layout */
void keyboard_set_layout(Keyboard * keyboard, unsigned int which)
{
	size_t i;
	GtkWidget * widget;

	for(i = 0; i < keyboard->layouts_cnt; i++)
		if((widget = keyboard_layout_get_widget(keyboard->layouts[i]))
				== NULL)
			continue;
		else if(i == which)
			gtk_widget_show_all(widget);
		else
			gtk_widget_hide(widget);
}


/* keyboard_set_modifier */
void keyboard_set_modifier(Keyboard * keyboard, unsigned int modifier)
{
	size_t i;

	for(i = 0; i < keyboard->layouts_cnt; i++)
		keyboard_layout_apply_modifier(keyboard->layouts[i], modifier);
}


/* keyboard_set_page */
void keyboard_set_page(Keyboard * keyboard, KeyboardPage page)
{
	/* FIXME really implement */
	switch(page)
	{
		case KEYBOARD_PAGE_DEFAULT:
			keyboard_set_layout(keyboard, 0);
			break;
		case KEYBOARD_PAGE_KEYPAD:
			keyboard_set_layout(keyboard, 1);
			break;
	}
}


/* useful */
/* keyboard_show */
void keyboard_show(Keyboard * keyboard, gboolean show)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, show ? "TRUE" : "FALSE");
#endif
	if(show == TRUE)
	{
		gtk_window_get_size(GTK_WINDOW(keyboard->window),
				&keyboard->width, &keyboard->height);
		gtk_widget_show(keyboard->window);
		gtk_window_get_position(GTK_WINDOW(keyboard->window),
				&keyboard->x, &keyboard->y);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() width=%d, height=%d\n", __func__,
				keyboard->width, keyboard->height);
		fprintf(stderr, "DEBUG: %s() x=%d, y=%d\n", __func__,
				keyboard->x, keyboard->y);
#endif
	}
	else if(keyboard->mode != KEYBOARD_MODE_EMBEDDED)
		gtk_widget_hide(keyboard->window);
}


/* keyboard_show_about */
/* callbacks */
static gboolean _about_on_closex(gpointer data);

void keyboard_show_about(Keyboard * keyboard)
{
	if(keyboard->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(keyboard->ab_window));
		return;
	}
	keyboard->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(keyboard->ab_window),
			GTK_WINDOW(keyboard->window));
	desktop_about_dialog_set_authors(keyboard->ab_window, _authors);
	desktop_about_dialog_set_comments(keyboard->ab_window,
			_("Virtual keyboard for the DeforaOS desktop"));
	desktop_about_dialog_set_copyright(keyboard->ab_window, _copyright);
	desktop_about_dialog_set_logo_icon_name(keyboard->ab_window,
			"input-keyboard");
	desktop_about_dialog_set_license(keyboard->ab_window, _license);
	desktop_about_dialog_set_name(keyboard->ab_window, PACKAGE);
	desktop_about_dialog_set_version(keyboard->ab_window, VERSION);
	desktop_about_dialog_set_website(keyboard->ab_window,
			"http://www.defora.org/");
	g_signal_connect_swapped(keyboard->ab_window, "delete-event",
			G_CALLBACK(_about_on_closex), keyboard);
	gtk_widget_show(keyboard->ab_window);
}

/* callbacks */
static gboolean _about_on_closex(gpointer data)
{
	Keyboard * keyboard = data;

	gtk_widget_hide(keyboard->ab_window);
	return TRUE;
}


/* private */
/* keyboard_add_layout */
static void _layout_clicked(GtkWidget * widget, gpointer data);

static GtkWidget * _keyboard_add_layout(Keyboard * keyboard,
		KeyboardLayoutDefinition * definitions,
		size_t definitions_cnt, KeyboardLayoutSection section)
{
	KeyboardLayout ** p;
	KeyboardLayout * layout;
	KeyboardKeyDefinition const * keys;
	size_t i;
	KeyboardKey * key;
	GtkWidget * label;
	GtkWidget * widget;
	unsigned long l;
	GdkColor black = { 0x00000000, 0x0000, 0x0000, 0x0000 };
	GdkColor white = { 0xffffffff, 0xffff, 0xffff, 0xffff };
	GdkColor gray = { 0xd0d0d0d0, 0xd0d0, 0xd0d0, 0xd0d0 };

	if((p = realloc(keyboard->layouts, sizeof(*p) * (keyboard->layouts_cnt
						+ 1))) == NULL)
		return NULL;
	keyboard->layouts = p;
	if((layout = keyboard_layout_new()) == NULL)
		return NULL;
	keyboard->layouts[keyboard->layouts_cnt++] = layout;
	keys = definitions[section].keys;
	for(i = 0; keys[i].width != 0; i++)
	{
		key = keyboard_layout_add(layout, keys[i].row, keys[i].width,
				keys[i].keysym, keys[i].label);
		if(key == NULL)
			continue;
		keyboard_key_set_background(key, &gray);
		keyboard_key_set_foreground(key, &black);
		keyboard_key_set_font(key, keyboard->font);
		for(; keys[i + 1].width == 0 && keys[i + 1].modifier != 0; i++)
		{
			keyboard_key_set_background(key, &white);
			keyboard_key_set_modifier(key, keys[i + 1].modifier,
					keys[i + 1].keysym, keys[i + 1].label);
		}
	}
	l = (section + 1) % definitions_cnt;
	label = gtk_label_new(definitions[l].label);
	gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
	gtk_widget_modify_font(label, keyboard->font);
	widget = gtk_button_new();
	gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &white);
	gtk_container_add(GTK_CONTAINER(widget), label);
	g_object_set_data(G_OBJECT(widget), "layout", (void *)l);
	g_signal_connect(widget, "clicked", G_CALLBACK(_layout_clicked),
			keyboard);
	keyboard_layout_add_widget(layout, 3, 0, 3, widget);
	return keyboard_layout_get_widget(layout);
}

static void _layout_clicked(GtkWidget * widget, gpointer data)
{
	Keyboard * keyboard = data;
	unsigned long d;
	KeyboardLayoutSection section;

	d = (unsigned long)g_object_get_data(G_OBJECT(widget), "layout");
	section = d;
	switch(section)
	{
		case KLS_LETTERS:
		case KLS_KEYPAD:
		case KLS_SPECIAL:
			keyboard_set_layout(keyboard, section);
			break;
	}
}
