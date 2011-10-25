/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <openssl/sha.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Phone.h"


/* SMSCrypt */
/* private */
/* types */
typedef struct _SMSCrypt
{
	/* internal */
	unsigned char buf[20];
	size_t len;
	/* widgets */
	GtkWidget * window;
	GtkListStore * store;
	GtkWidget * view;
} SMSCrypt;


/* prototypes */
static void _smscrypt_clear(PhonePlugin * plugin);
static gboolean _smscrypt_confirm(PhonePlugin * plugin, char const * message);
static int _smscrypt_init(PhonePlugin * plugin);
static void _smscrypt_destroy(PhonePlugin * plugin);
static int _smscrypt_event(PhonePlugin * plugin, PhoneEvent * event);
static int _smscrypt_secret(PhonePlugin * plugin, char const * number);
static void _smscrypt_settings(PhonePlugin * plugin);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"SMS encryption",
	"application-certificate",
	_smscrypt_init,
	_smscrypt_destroy,
	_smscrypt_event,
	_smscrypt_settings,
	NULL
};


/* private */
/* functions */
/* smscrypt_clear */
static void _smscrypt_clear(PhonePlugin * plugin)
{
	SMSCrypt * smscrypt = plugin->priv;

	memset(smscrypt->buf, 0, smscrypt->len);
}


/* smscrypt_confirm */
static gboolean _smscrypt_confirm(PhonePlugin * plugin, char const * message)
{
	GtkWidget * dialog;
	int res;

	fprintf(stderr, "DEBUG: %s()\n", __func__);
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			"Question");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return (res == GTK_RESPONSE_YES) ? TRUE : FALSE;
}


/* smscrypt_init */
static void _init_foreach(char const * variable, char const * value,
		void * priv);

static int _smscrypt_init(PhonePlugin * plugin)
{
	SMSCrypt * smscrypt;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((smscrypt = malloc(sizeof(*smscrypt))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	plugin->priv = smscrypt;
	smscrypt->len = sizeof(smscrypt->buf);
	smscrypt->window = NULL;
	smscrypt->store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	plugin->helper->config_foreach(plugin->helper->phone, "smscrypt",
			_init_foreach, smscrypt);
	return 0;
}

static void _init_foreach(char const * variable, char const * value,
		void * priv)
{
	SMSCrypt * smscrypt = priv;
	GtkTreeIter iter;

	gtk_list_store_append(smscrypt->store, &iter);
	gtk_list_store_set(smscrypt->store, &iter, 0, variable, 1, value, -1);
}


/* smscrypt_destroy */
static void _smscrypt_destroy(PhonePlugin * plugin)
{
	SMSCrypt * smscrypt = plugin->priv;

	if(smscrypt->window != NULL)
		gtk_widget_destroy(smscrypt->window);
	free(smscrypt);
}


/* smscrypt_event */
static int _smscrypt_event_sms_receiving(PhonePlugin * plugin,
		char const * number, PhoneEncoding * encoding, char * buf,
		size_t * len);
static int _smscrypt_event_sms_sending(PhonePlugin * plugin,
		char const * number, PhoneEncoding * encoding, char * buf,
		size_t * len);

static int _smscrypt_event(PhonePlugin * plugin, PhoneEvent * event)
{
	int ret = 0;
	char const * number;
	PhoneEncoding * encoding;
	char ** buf;
	size_t * len;

	switch(event->type)
	{
#if 0 /* FIXME re-implement */
		/* our deal */
		case PHONE_EVENT_TYPE_SMS_RECEIVING:
			number = va_arg(ap, char const *);
			encoding = va_arg(ap, PhoneEncoding *);
			buf = va_arg(ap, char **);
			len = va_arg(ap, size_t *);
			ret = _smscrypt_event_sms_receiving(plugin, number,
					encoding, *buf, len);
			break;
		case PHONE_EVENT_TYPE_SMS_SENDING:
			number = va_arg(ap, char const *);
			encoding = va_arg(ap, PhoneEncoding *);
			buf = va_arg(ap, char **);
			len = va_arg(ap, size_t *);
			ret = _smscrypt_event_sms_sending(plugin, number,
					encoding, *buf, len);
			break;
#endif
		/* ignore the rest */
		default:
			break;
	}
	return ret;
}

static int _smscrypt_event_sms_receiving(PhonePlugin * plugin,
		char const * number, PhoneEncoding * encoding, char * buf,
		size_t * len)
{
	SMSCrypt * smscrypt = plugin->priv;
	char const * error = "There is no known secret for this number."
		" The message could not be decrypted.";
	size_t i;
	size_t j = 0;
	SHA_CTX sha1;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, buf, %lu)\n", __func__, *encoding,
			(unsigned long)*len);
#endif
	if(*encoding != PHONE_ENCODING_DATA)
		return 0; /* not for us */
	if(_smscrypt_secret(plugin, number) != 0)
		return plugin->helper->error(plugin->helper->phone, error, 1);
	for(i = 0; i < *len; i++)
	{
		buf[i] ^= smscrypt->buf[j];
		smscrypt->buf[j++] ^= buf[i];
		if(j != smscrypt->len)
			continue;
		SHA1_Init(&sha1);
		SHA1_Update(&sha1, smscrypt->buf, smscrypt->len);
		SHA1_Final(smscrypt->buf, &sha1);
		j = 0;
	}
	*encoding = PHONE_ENCODING_UTF8;
	_smscrypt_clear(plugin);
	return 0;
}

static int _smscrypt_event_sms_sending(PhonePlugin * plugin,
		char const * number, PhoneEncoding * encoding, char * buf,
		size_t * len)
{
	SMSCrypt * smscrypt = plugin->priv;
	char const * confirm = "There is no secret defined for this number."
		" The message will be sent unencrypted.\nContinue?";
	size_t i;
	size_t j = 0;
	SHA_CTX sha1;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %u, buf, %lu)\n", __func__, number,
			(unsigned)*encoding, (unsigned long)*len);
#endif
	if(*encoding != PHONE_ENCODING_UTF8)
		return 0; /* not for us */
	if(_smscrypt_secret(plugin, number) != 0)
		return (_smscrypt_confirm(plugin, confirm) == TRUE) ? 0 : 1;
	*encoding = PHONE_ENCODING_DATA;
	for(i = 0; i < *len; i++)
	{
		buf[i] ^= smscrypt->buf[j];
		smscrypt->buf[j++] = buf[i];
		if(j != smscrypt->len)
			continue;
		SHA1_Init(&sha1);
		SHA1_Update(&sha1, smscrypt->buf, smscrypt->len);
		SHA1_Final(smscrypt->buf, &sha1);
		j = 0;
	}
	*encoding = PHONE_ENCODING_DATA;
	_smscrypt_clear(plugin);
	return 0;
}


/* smscrypt_secret */
static int _smscrypt_secret(PhonePlugin * plugin, char const * number)
{
	SMSCrypt * smscrypt = plugin->priv;
	char const * secret = NULL;
	SHA_CTX sha1;

	if(number != NULL)
		secret = plugin->helper->config_get(plugin->helper->phone,
				"smscrypt", number);
	if(secret == NULL)
		secret = plugin->helper->config_get(plugin->helper->phone,
				"smscrypt", "secret");
	if(secret == NULL)
		return 1;
	SHA1_Init(&sha1);
	SHA1_Update(&sha1, (unsigned char const *)secret, strlen(secret));
	SHA1_Final(smscrypt->buf, &sha1);
	return 0;
}


/* smscrypt_settings */
static gboolean _on_settings_closex(gpointer data);
static void _on_settings_delete(gpointer data);
static void _on_settings_new(gpointer data);
static void _on_settings_number_edited(GtkCellRenderer * renderer, gchar * arg1,
		gchar * arg2, gpointer data);
static void _on_settings_secret_edited(GtkCellRenderer * renderer, gchar * arg1,
		gchar * arg2, gpointer data);

static void _smscrypt_settings(PhonePlugin * plugin)
{
	SMSCrypt * smscrypt = plugin->priv;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(smscrypt->window != NULL)
	{
		gtk_window_present(GTK_WINDOW(smscrypt->window));
		return;
	}
	smscrypt->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(smscrypt->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	/* XXX find something more appropriate */
	gtk_window_set_icon_name(GTK_WINDOW(smscrypt->window), "smscrypt");
#endif
	gtk_window_set_title(GTK_WINDOW(smscrypt->window), "SMS encryption");
	g_signal_connect_swapped(G_OBJECT(smscrypt->window), "delete-event",
			G_CALLBACK(_on_settings_closex), plugin);
	vbox = gtk_vbox_new(FALSE, 0);
	/* toolbar */
	widget = gtk_toolbar_new();
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_on_settings_new), plugin);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_on_settings_delete), plugin);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* view */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	smscrypt->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				smscrypt->store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(smscrypt->view), TRUE);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "editable", TRUE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
				_on_settings_number_edited), plugin);
	column = gtk_tree_view_column_new_with_attributes("Number",
			renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(smscrypt->view), column);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "editable", TRUE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
				_on_settings_secret_edited), plugin);
	column = gtk_tree_view_column_new_with_attributes("Secret",
			renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(smscrypt->view), column);
	gtk_container_add(GTK_CONTAINER(widget), smscrypt->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(smscrypt->window), vbox);
	gtk_widget_show_all(smscrypt->window);
}

static gboolean _on_settings_closex(gpointer data)
{
	PhonePlugin * plugin = data;
	SMSCrypt * smscrypt = plugin->priv;

	gtk_widget_hide(smscrypt->window);
	return TRUE;
}

static void _on_settings_delete(gpointer data)
{
	PhonePlugin * plugin = data;
	SMSCrypt * smscrypt = plugin->priv;
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	char * number = NULL;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						smscrypt->view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(smscrypt->store), &iter, 0, &number,
			-1);
	if(number == NULL)
		return;
	plugin->helper->config_set(plugin->helper->phone, "smscrypt", number,
			NULL);
	gtk_list_store_remove(smscrypt->store, &iter);
	g_free(number);
}

static void _on_settings_new(gpointer data)
{
	PhonePlugin * plugin = data;
	SMSCrypt * smscrypt = plugin->priv;
	GtkTreeIter iter;

	gtk_list_store_append(smscrypt->store, &iter);
	gtk_list_store_set(smscrypt->store, &iter, 0, "number", -1);
}

static void _on_settings_number_edited(GtkCellRenderer * renderer, gchar * arg1,
		gchar * arg2, gpointer data)
{
	PhonePlugin * plugin = data;
	SMSCrypt * smscrypt = plugin->priv;
	GtkTreeModel * model = GTK_TREE_MODEL(smscrypt->store);
	GtkTreeIter iter;
	char * number = NULL;
	char const * secret;

	if(gtk_tree_model_get_iter_from_string(model, &iter, arg1) == TRUE)
		gtk_tree_model_get(model, &iter, 0, &number, -1);
	if(number == NULL)
		return;
	/* FIXME check that there are no duplicates */
	secret = plugin->helper->config_get(plugin->helper->phone, "smscrypt",
			number);
	if(plugin->helper->config_set(plugin->helper->phone, "smscrypt", arg2,
				secret) == 0
			&& plugin->helper->config_set(plugin->helper->phone,
				"smscrypt", number, NULL) == 0)
		gtk_list_store_set(smscrypt->store, &iter, 0, arg2, -1);
	g_free(number);
}

static void _on_settings_secret_edited(GtkCellRenderer * renderer, gchar * arg1,
		gchar * arg2, gpointer data)
{
	PhonePlugin * plugin = data;
	SMSCrypt * smscrypt = plugin->priv;
	GtkTreeModel * model = GTK_TREE_MODEL(smscrypt->store);
	GtkTreeIter iter;
	char * number = NULL;

	if(gtk_tree_model_get_iter_from_string(model, &iter, arg1) == TRUE)
		gtk_tree_model_get(model, &iter, 0, &number, -1);
	if(number == NULL)
		return;
	if(plugin->helper->config_set(plugin->helper->phone, "smscrypt", number,
				arg2) == 0)
		gtk_list_store_set(smscrypt->store, &iter, 1, arg2, -1);
	g_free(number);
}
