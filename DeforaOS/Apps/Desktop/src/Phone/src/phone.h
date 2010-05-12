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



#ifndef PHONE_SRC_PHONE_H
# define PHONE_SRC_PHONE_H

# include <glib.h>


/* Phone */
/* types */
typedef struct _Phone Phone;

typedef enum _PhoneCall
{
	PHONE_CALL_ESTABLISHED = 0,
	PHONE_CALL_INCOMING,
	PHONE_CALL_OUTGOING,
	PHONE_CALL_TERMINATED
} PhoneCall;

typedef enum _PhoneCode
{
	PHONE_CODE_SIM_PIN = 0
} PhoneCode;

typedef enum _PhoneMessage
{
	PHONE_MESSAGE_SHOW = 0
} PhoneMessage;

typedef enum _PhoneMessageShow
{
	PHONE_MESSAGE_SHOW_CONTACTS = 0,
	PHONE_MESSAGE_SHOW_DIALER,
	PHONE_MESSAGE_SHOW_MESSAGES
} PhoneMessageShow;


/* constants */
# define PHONE_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_PHONE_CLIENT"
# define PHONE_EMBED_MESSAGE	"DEFORAOS_DESKTOP_PHONE_EMBED"


/* functions */
Phone * phone_new(char const * device, unsigned int baudrate, int retry,
		int hwflow);
void phone_delete(Phone * phone);


/* useful */
int phone_error(Phone * phone, char const * message, int ret);

/* interface */
void phone_show_call(Phone * phone, gboolean show, ...);
void phone_show_code(Phone * phone, gboolean show, ...);
void phone_show_contacts(Phone * phone, gboolean show);
void phone_show_dialer(Phone * phone, gboolean show);
void phone_show_messages(Phone * phone, gboolean show);
void phone_show_read(Phone * phone, gboolean show, ...);
void phone_show_write(Phone * phone, gboolean show);

/* calls */
void phone_call_answer(Phone * phone);
void phone_call_hangup(Phone * phone);
void phone_call_mute(Phone * phone, gboolean mute);
void phone_call_reject(Phone * phone);

/* code */
int phone_code_append(Phone * phone, char character);
void phone_code_clear(Phone * phone);
void phone_code_enter(Phone * phone);

/* contacts */
void phone_contacts_add(Phone * phone, unsigned int index, char const * name,
		char const * number);
void phone_contacts_call_selected(Phone * phone);
void phone_contacts_write_selected(Phone * phone);

/* dialer */
int phone_dialer_append(Phone * phone, char character);
void phone_dialer_call(Phone * phone, char const * number);
void phone_dialer_hangup(Phone * phone);

/* messages */
void phone_messages_add(Phone * phone, unsigned int index, char const * number,
		time_t date, char const * content);
void phone_messages_read_selected(Phone * phone);
void phone_messages_write(Phone * phone, char const * number,
		char const * text);

/* plugins */
int phone_load(Phone * phone, char const * plugin);

/* write */
void phone_write_count_buffer(Phone * phone);
void phone_write_send(Phone * phone);

#endif /* !PHONE_SRC_PHONE_H */
