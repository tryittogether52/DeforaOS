/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/* FIXME:
 * - check if it resets the modem on "OK" (and if so, avoid it) */



#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Phone.h"


/* GPRS */
/* private */
/* types */
typedef struct _PhonePlugin
{
	PhonePluginHelper * helper;
	guint source;
	gboolean roaming;
	gboolean connected;

	gboolean active;
	GtkWidget * window;
	GtkWidget * notebook;
	GtkWidget * attach;
	GtkWidget * apn;
	GtkWidget * username;
	GtkWidget * password;
	GtkWidget * connect;
	GtkWidget * st_image;
	GtkWidget * st_label;
	GtkWidget * st_in;
	GtkWidget * st_out;
#if GTK_CHECK_VERSION(2, 10, 0)
	GtkWidget * systray;
	GtkStatusIcon * icon;
#endif
} GPRS;


/* prototypes */
/* plugins */
static GPRS * _gprs_init(PhonePluginHelper * helper);
static void _gprs_destroy(GPRS * gprs);
static int _gprs_event(GPRS * gprs, PhoneEvent * event);
static void _gprs_settings(GPRS * gprs);

static void _gprs_set_connected(GPRS * gprs, gboolean connected,
		char const * message, size_t in, size_t out);

static int _gprs_access_point(GPRS * gprs);
static int _gprs_connect(GPRS * gprs);
static int _gprs_disconnect(GPRS * gprs);

/* callbacks */
static void _gprs_on_activate(gpointer data);
static void _gprs_on_popup_menu(GtkStatusIcon * icon, guint button,
		guint time, gpointer data);
static gboolean _gprs_on_timeout(gpointer data);


/* public */
/* variables */
PhonePluginDefinition plugin =
{
	"GPRS",
	"stock_internet",
	NULL,
	_gprs_init,
	_gprs_destroy,
	_gprs_event,
	_gprs_settings
};


/* private */
/* functions */
/* gprs_init */
static GPRS * _gprs_init(PhonePluginHelper * helper)
{
	GPRS * gprs;
#if GTK_CHECK_VERSION(2, 10, 0)
	char const * p;
	gboolean active;
#endif

	if((gprs = object_new(sizeof(*gprs))) == NULL)
		return NULL;
	gprs->helper = helper;
	gprs->source = 0;
	gprs->roaming = FALSE;
	gprs->connected = FALSE;
	gprs->active = FALSE;
	gprs->window = NULL;
#if GTK_CHECK_VERSION(2, 10, 0)
	gprs->icon = gtk_status_icon_new_from_icon_name("stock_internet");
# if GTK_CHECK_VERSION(2, 16, 0)
	gtk_status_icon_set_tooltip_text(gprs->icon, "Not connected");
# endif
# if GTK_CHECK_VERSION(2, 18, 0)
	gtk_status_icon_set_title(gprs->icon, "GPRS");
#  if GTK_CHECK_VERSION(2, 20, 0)
	gtk_status_icon_set_name(gprs->icon, "phone-gprs");
#  endif
# endif
	g_signal_connect_swapped(gprs->icon, "activate", G_CALLBACK(
				_gprs_on_activate), gprs);
	g_signal_connect(gprs->icon, "popup-menu", G_CALLBACK(
				_gprs_on_popup_menu), gprs);
	active = ((p = helper->config_get(helper->phone, "gprs", "systray"))
			!= NULL && strtoul(p, NULL, 10) != 0) ? TRUE : FALSE;
	gtk_status_icon_set_visible(gprs->icon, active);
#endif
	return gprs;
}


/* gprs_destroy */
static void _gprs_destroy(GPRS * gprs)
{
#if GTK_CHECK_VERSION(2, 10, 0)
	g_object_unref(gprs->icon);
#endif
	if(gprs->source != 0)
		g_source_remove(gprs->source);
	if(gprs->window != NULL)
		gtk_widget_destroy(gprs->window);
	object_delete(gprs);
}


/* gprs_event */
static int _gprs_event_modem(GPRS * gprs, ModemEvent * event);

static int _gprs_event(GPRS * gprs, PhoneEvent * event)
{
	switch(event->type)
	{
		case PHONE_EVENT_TYPE_MODEM_EVENT:
			return _gprs_event_modem(gprs,
					event->modem_event.event);
		case PHONE_EVENT_TYPE_OFFLINE:
		case PHONE_EVENT_TYPE_UNAVAILABLE:
			gprs->roaming = FALSE;
			return 0;
		default: /* not relevant */
			return 0;
	}
}

static int _gprs_event_modem(GPRS * gprs, ModemEvent * event)
{
	gboolean connected;

	switch(event->type)
	{
		case MODEM_EVENT_TYPE_CONNECTION:
			connected = event->connection.connected;
			_gprs_set_connected(gprs, connected, connected
					? "Connected" : "Not connected",
					event->connection.in,
					event->connection.out);
			break;
		case MODEM_EVENT_TYPE_REGISTRATION:
			gprs->roaming = event->registration.roaming;
			if(gprs->active != FALSE)
				break;
			if(event->registration.status
					!= MODEM_REGISTRATION_STATUS_REGISTERED)
				break;
			gprs->active = TRUE;
			/* FIXME optionally force GPRS registration */
			break;
		default:
			break;
	}
	return 0;
}


/* gprs_settings */
static GtkWidget * _settings_preferences(GPRS * gprs);
static GtkWidget * _settings_status(GPRS * gprs);
/* callbacks */
static void _settings_on_apply(gpointer data);
static void _settings_on_cancel(gpointer data);
static gboolean _settings_on_closex(gpointer data);
static void _settings_on_connect(gpointer data);
static void _settings_on_ok(gpointer data);

static void _gprs_settings(GPRS * gprs)
{
	GtkWidget * vbox;
	GtkWidget * bbox;
	GtkWidget * widget;

	if(gprs->window != NULL)
	{
		gtk_notebook_set_current_page(GTK_NOTEBOOK(gprs->notebook), 0);
		gtk_window_present(GTK_WINDOW(gprs->window));
		return;
	}
	gprs->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(gprs->window), 4);
	gtk_window_set_default_size(GTK_WINDOW(gprs->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(gprs->window), "stock_internet");
#endif
	gtk_window_set_title(GTK_WINDOW(gprs->window), "GPRS");
	g_signal_connect_swapped(gprs->window, "delete-event", G_CALLBACK(
				_settings_on_closex), gprs);
	vbox = gtk_vbox_new(FALSE, 4);
	gprs->notebook = gtk_notebook_new();
	/* preferences */
	widget = _settings_preferences(gprs);
	gtk_notebook_append_page(GTK_NOTEBOOK(gprs->notebook), widget,
			gtk_label_new("Preferences"));
	/* status */
	widget = _settings_status(gprs);
	gtk_notebook_append_page(GTK_NOTEBOOK(gprs->notebook), widget,
			gtk_label_new("Status"));
	gtk_box_pack_start(GTK_BOX(vbox), gprs->notebook, TRUE, TRUE, 0);
	/* button box */
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_settings_on_cancel), gprs);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(_settings_on_ok),
			gprs);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(gprs->window), vbox);
	_settings_on_cancel(gprs);
	_gprs_on_timeout(gprs);
	gtk_widget_show_all(gprs->window);
}

static GtkWidget * _settings_preferences(GPRS * gprs)
{
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = gtk_vbox_new(FALSE, 4);
	/* attachment */
	gprs->attach = gtk_check_button_new_with_label(
			"Force GPRS registration");
	gtk_box_pack_start(GTK_BOX(vbox), gprs->attach, FALSE, TRUE, 0);
	/* access point */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Access point:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gprs->apn = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), gprs->apn, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* username */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Username:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gprs->username = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), gprs->username, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* password */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Password:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gprs->password = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(gprs->password), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->password, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* systray */
	gprs->systray = gtk_check_button_new_with_label("Show in system tray");
	gtk_box_pack_start(GTK_BOX(vbox), gprs->systray, FALSE, TRUE, 0);
	return vbox;
}

static GtkWidget * _settings_status(GPRS * gprs)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkWidget * bbox;

	vbox = gtk_vbox_new(FALSE, 4);
	/* details */
	widget = gtk_frame_new("Details");
	bbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(bbox), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	gprs->st_image = gtk_image_new_from_icon_name(GTK_STOCK_DISCONNECT,
			GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->st_image, FALSE, TRUE, 0);
	gprs->st_label = gtk_label_new("Not connected");
	gtk_misc_set_alignment(GTK_MISC(gprs->st_label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->st_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(bbox), hbox, FALSE, TRUE, 0);
	gprs->st_in = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(gprs->st_in), 0.0, 0.5);
	gtk_widget_set_no_show_all(gprs->st_in, TRUE);
	gtk_box_pack_start(GTK_BOX(bbox), gprs->st_in, FALSE, TRUE, 0);
	gprs->st_out = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(gprs->st_out), 0.0, 0.5);
	gtk_widget_set_no_show_all(gprs->st_out, TRUE);
	gtk_box_pack_start(GTK_BOX(bbox), gprs->st_out, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(widget), bbox);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* connect */
	gprs->connect = gtk_button_new_from_stock(GTK_STOCK_CONNECT);
	g_signal_connect_swapped(gprs->connect, "clicked", G_CALLBACK(
				_settings_on_connect), gprs);
	gtk_box_pack_start(GTK_BOX(vbox), gprs->connect, FALSE, TRUE, 0);
	return vbox;
}

static void _settings_on_apply(gpointer data)
{
	GPRS * gprs = data;
	PhonePluginHelper * helper = gprs->helper;
	gboolean active;
	char const * p;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gprs->attach));
	helper->config_set(helper->phone, "gprs", "attach", active ? "1" : "0");
	p = gtk_entry_get_text(GTK_ENTRY(gprs->apn));
	helper->config_set(helper->phone, "gprs", "apn", p);
	p = gtk_entry_get_text(GTK_ENTRY(gprs->username));
	helper->config_set(helper->phone, "gprs", "username", p);
	p = gtk_entry_get_text(GTK_ENTRY(gprs->password));
	helper->config_set(helper->phone, "gprs", "password", p);
#if GTK_CHECK_VERSION(2, 10, 0)
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gprs->systray));
	helper->config_set(helper->phone, "gprs", "systray", active
			? "1" : "0");
	gtk_status_icon_set_visible(gprs->icon, active);
#endif
	_gprs_access_point(gprs);
	gprs->active = FALSE;
}

static void _settings_on_cancel(gpointer data)
{
	GPRS * gprs = data;
	PhonePluginHelper * helper = gprs->helper;
	char const * p;
	gboolean active;

	gtk_widget_hide(gprs->window);
	active = ((p = helper->config_get(helper->phone, "gprs", "attach"))
			!= NULL && strtoul(p, NULL, 10) != 0) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gprs->attach), active);
	if((p = helper->config_get(helper->phone, "gprs", "apn")) == NULL)
		p = "";
	gtk_entry_set_text(GTK_ENTRY(gprs->apn), p);
	if((p = helper->config_get(helper->phone, "gprs", "username")) == NULL)
		p = "";
	gtk_entry_set_text(GTK_ENTRY(gprs->username), p);
	if((p = helper->config_get(helper->phone, "gprs", "password")) == NULL)
		p = "";
	gtk_entry_set_text(GTK_ENTRY(gprs->password), p);
#if GTK_CHECK_VERSION(2, 10, 0)
	active = ((p = helper->config_get(helper->phone, "gprs", "systray"))
			!= NULL && strtoul(p, NULL, 10) != 0) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gprs->systray), active);
#endif
}

static gboolean _settings_on_closex(gpointer data)
{
	GPRS * gprs = data;

	_settings_on_cancel(gprs);
	return TRUE;
}

static void _settings_on_connect(gpointer data)
{
	GPRS * gprs = data;

	_settings_on_apply(gprs);
	if(gprs->connected)
		_gprs_disconnect(gprs);
	else
		_gprs_connect(gprs);
}

static void _settings_on_ok(gpointer data)
{
	GPRS * gprs = data;

	gtk_widget_hide(gprs->window);
	_settings_on_apply(gprs);
}


/* accessors */
/* gprs_set_connected */
static void _gprs_set_connected(GPRS * gprs, gboolean connected,
		char const * message, size_t in, size_t out)
{
	char buf[64];

	gprs->connected = connected;
	if(gprs->window == NULL)
		return;
	gtk_image_set_from_icon_name(GTK_IMAGE(gprs->st_image), connected
			? GTK_STOCK_CONNECT : GTK_STOCK_DISCONNECT,
			GTK_ICON_SIZE_BUTTON);
	gtk_label_set_text(GTK_LABEL(gprs->st_label), message);
	gtk_button_set_label(GTK_BUTTON(gprs->connect), connected
			? GTK_STOCK_DISCONNECT : GTK_STOCK_CONNECT);
	if(connected)
	{
		snprintf(buf, sizeof(buf), "Received: %lu kB",
				(unsigned long)in / 1024);
		gtk_label_set_text(GTK_LABEL(gprs->st_in), buf);
		snprintf(buf, sizeof(buf), "Sent: %lu kB",
				(unsigned long)out / 1024);
		gtk_label_set_text(GTK_LABEL(gprs->st_out), buf);
		gtk_widget_show(gprs->st_in);
		gtk_widget_show(gprs->st_out);
#if GTK_CHECK_VERSION(2, 16, 0)
		snprintf(buf, sizeof(buf), "%s\nReceived: %lu kB\nSent: %lu kB",
				message, (unsigned long)in / 1024,
				(unsigned long)out / 1024);
		gtk_status_icon_set_tooltip_text(gprs->icon, buf);
#endif
		if(gprs->source == 0)
			gprs->source = g_timeout_add(1000, _gprs_on_timeout,
					gprs);
	}
	else
	{
		if(gprs->source != 0)
			g_source_remove(gprs->source);
		gprs->source = 0;
		gtk_widget_hide(gprs->st_in);
		gtk_widget_hide(gprs->st_out);
#if GTK_CHECK_VERSION(2, 16, 0)
		gtk_status_icon_set_tooltip_text(gprs->icon, message);
#endif
	}
}


/* useful */
/* gprs_access_point */
static int _gprs_access_point(GPRS * gprs)
{
	int ret = 0;
	PhonePluginHelper * helper = gprs->helper;
	char const * p;
	ModemRequest request;

	if((p = helper->config_get(helper->phone, "gprs", "apn")) == NULL)
		return 0;
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_AUTHENTICATE;
	/* set the access point */
	request.authenticate.name = "APN";
	request.authenticate.username = "IP";
	request.authenticate.password = p;
	ret |= helper->request(helper->phone, &request);
	/* set the credentials */
	request.authenticate.name = "GPRS";
	p = helper->config_get(helper->phone, "gprs", "username");
	request.authenticate.username = p;
	p = helper->config_get(helper->phone, "gprs", "password");
	request.authenticate.password = p;
	ret |= helper->request(helper->phone, &request);
	return ret;
}


/* gprs_connect */
static int _gprs_connect(GPRS * gprs)
{
	GtkDialogFlags flags = GTK_DIALOG_MODAL
		| GTK_DIALOG_DESTROY_WITH_PARENT;
	char const message[] = "You are currently roaming, and additional"
		" charges are therefore likely to apply.\n"
		"Do you really want to connect?";
	GtkWidget * widget;
	int res;
	ModemRequest request;

	if(_gprs_access_point(gprs) != 0)
		return -1;
	if(gprs->roaming)
	{
		widget = gtk_message_dialog_new(GTK_WINDOW(gprs->window), flags,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
#if GTK_CHECK_VERSION(2, 6, 0)
				"Warning");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
					widget),
#endif
				message);
		gtk_window_set_title(GTK_WINDOW(widget), "Warning");
		res = gtk_dialog_run(GTK_DIALOG(widget));
		gtk_widget_destroy(widget);
		if(res != GTK_RESPONSE_YES)
			return 0;
	}
	_gprs_set_connected(gprs, TRUE, "Connecting...", 0, 0);
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_CALL;
	request.call.call_type = MODEM_CALL_TYPE_DATA;
	request.call.number = "*99***1#"; /* XXX specific to GSM/GPRS */
	return gprs->helper->request(gprs->helper->phone, &request);
}


/* gprs_disconnect */
static int _gprs_disconnect(GPRS * gprs)
{
	ModemRequest request;

	if(_gprs_access_point(gprs) != 0)
		return -1;
	_gprs_set_connected(gprs, TRUE, "Disconnecting...", 0, 0);
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_CALL_HANGUP;
	return gprs->helper->request(gprs->helper->phone, &request);
}


/* callbacks */
/* gprs_on_activate */
static void _gprs_on_activate(gpointer data)
{
	GPRS * gprs = data;

	_gprs_settings(gprs);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(gprs->notebook), 1);
	gtk_window_present(GTK_WINDOW(gprs->window));
}


/* gprs_on_popup_menu */
static void _gprs_on_popup_menu(GtkStatusIcon * icon, guint button,
		guint time, gpointer data)
{
	GPRS * gprs = data;
	GtkWidget * menu;
	GtkWidget * menuitem;
	GtkWidget * image;

	menu = gtk_menu_new();
	/* status */
	menuitem = gtk_image_menu_item_new_with_mnemonic("_Status");
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(
				_gprs_on_activate), gprs);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	/* connection */
	menuitem = gtk_image_menu_item_new_with_mnemonic(gprs->connected
			? "_Disconnect" : "_Connect");
	image = gtk_image_new_from_stock(gprs->connected ? GTK_STOCK_DISCONNECT
			: GTK_STOCK_CONNECT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(
				gprs->connected ? _gprs_disconnect
				: _gprs_connect), gprs);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	/* preferences */
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES,
			NULL);
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(
				_gprs_settings), gprs);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, time);
}


/* gprs_on_timeout */
static gboolean _gprs_on_timeout(gpointer data)
{
	GPRS * gprs = data;

	gprs->helper->trigger(gprs->helper->phone, MODEM_EVENT_TYPE_CONNECTION);
	return TRUE;
}
