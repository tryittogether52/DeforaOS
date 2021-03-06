/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
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
#ifdef DEBUG
# include <stdio.h>
#endif
#include "session.h"
#include "../data/Init.h"
#include "init.h"
#include "../config.h"

#ifndef PREFIX
# define PREFIX "/usr/local"
#endif


/* Init */
/* private */
/* types */
struct _Init
{
	Event * event;
	Session * session;
	AppServer * appserver;
};


/* public */
/* functions */
/* init_new */
Init * init_new(AppServerOptions options, char const * profile)
{
	Init * init;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, profile);
#endif
	if((init = object_new(sizeof(*init))) == NULL)
		return NULL;
	init->event = event_new();
	init->session = (init->event != NULL)
		? session_new("Init", profile, init->event) : NULL;
	/* FIXME ASO_LOCAL or ASO_REMOTE comes from the configuration file */
	init->appserver = (init->event != NULL)
		? appserver_new_event("Init", options, init->event) : NULL;
	/* FIXME handle signals (Event class?) */
	/* error handling */
	if(init->event == NULL || init->appserver == NULL)
	{
		init_delete(init);
		return NULL;
	}
	return init;
}


/* init_delete */
void init_delete(Init * init)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(init->event != NULL)
		event_delete(init->event);
	object_delete(init);
}


/* AppInterface */
/* init_get_profile */
int Init_get_profile(String ** profile)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p)\n", __func__, (void*)profile);
#endif
	/* FIXME implement */
	return -1;
}


/* init_get_session */
int Init_get_session(String const * session)
{
	int ret = -1;
	String * filename;
	Config * config;
	String const * p;
	String * q;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, session);
#endif
	if(string_compare(session, "Init") == 0)
		return 0;
	/* FIXME look in registered sessions instead */
	if((filename = string_new_append(PREFIX "/etc/AppInterface/", session,
					".interface", NULL)) == NULL)
		return -1;
	if((config = config_new()) != NULL
			&& config_load(config, filename) == 0
			&& (p = config_get(config, NULL, "port")) != NULL)
	{
		ret = strtol(p, &q, 0);
		if(p[0] == '\0' || *q != '\0' || ret <= 0 || ret > 65535)
			ret = -1;
	}
	if(config != NULL)
		config_delete(config);
	string_delete(filename);
	return ret;
}


/* init_login */
uint16_t Init_login(String const * username)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, username);
#endif
	/* FIXME implement */
	return -1;
}


/* init_logout */
uint16_t Init_logout(void)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME implement */
	return -1;
}


/* init_register */
uint16_t Init_register(String const * service, uint16_t port)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %d)\n", __func__, service, port);
#endif
	return session_register(service, port);
}


/* init_set_profile */
int32_t Init_set_profile(String const * profile)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, profile);
#endif
	/* FIXME implement */
	return -1;
}


/* useful */
int init_loop(Init * init)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return event_loop(init->event);
}
