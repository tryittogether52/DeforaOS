/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Network Probe */
/* Probe is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * Probe is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Probe; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <System.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "../config.h"

#define DAMON_DEFAULT_REFRESH 10

#ifndef ETCDIR
# define ETCDIR PREFIX "/etc"
#endif


/* types */
typedef struct _Host
{
	AppClient * appclient;
	char * hostname;
	char ** ifaces;
	char ** vols;
} Host;


/* DaMon */
/* types */
typedef struct _DaMon
{
	unsigned int refresh;
	Host * hosts;
	unsigned int hosts_cnt;
	Event * event;
} DaMon;

/* functions */
/* public */
/* damon */
static int _damon_init(DaMon * damon);
static void _damon_destroy(DaMon * damon);

static int _damon(void)
{
	DaMon damon;

	if(_damon_init(&damon) != 0)
		return 1;
	if(event_loop(damon.event) != 0)
		error_print("DaMon");
	_damon_destroy(&damon);
	return 1;
}


/* _damon_init */
static int _init_config(DaMon * damon);
static int _damon_refresh(DaMon * damon);

static int _damon_init(DaMon * damon)
{
	struct timeval tv;

	if(_init_config(damon) != 0)
		return 1;
	if((damon->event = event_new()) == NULL)
		return error_print("DaMon");
	_damon_refresh(damon);
	tv.tv_sec = damon->refresh;
	tv.tv_usec = 0;
	event_register_timeout(damon->event, tv,
			(EventTimeoutFunc)_damon_refresh, damon);
	return 0;
}

/* _init_config */
static int _config_hosts(DaMon * damon, Config * config, char * hosts);

static int _init_config(DaMon * damon)
{
	Config * config;
	char * filename = ETCDIR "/damon.cfg";
	char * p;
	char * q;
	int tmp;

	if((config = config_new()) == NULL)
		return 1;
	damon->refresh = DAMON_DEFAULT_REFRESH;
	damon->hosts = NULL;
	damon->hosts_cnt = 0;
	if(config_load(config, filename) != 0)
	{
		fprintf(stderr, "DaMon: %s: Could not load configuration\n",
				filename);
		config_delete(config);
		return 1;
	}
	if((p = config_get(config, "", "refresh")) != NULL)
	{
		tmp = strtol(p, &q, 10);
		damon->refresh = (*p == '\0' || *q != '\0' || tmp <= 0)
			? DAMON_DEFAULT_REFRESH : tmp;
#ifdef DEBUG
		fprintf(stderr, "refresh set to %d\n", damon->refresh);
#endif
	}
	if((p = config_get(config, "", "hosts")) != NULL)
		_config_hosts(damon, config, p);
	config_delete(config);
	return 0;
}

static int _hosts_host(Config * config, Host * host, char * h,
		unsigned int pos);
static int _config_hosts(DaMon * damon, Config * config, char * hosts)
{
	char * h = hosts;
	unsigned int pos = 0;
	Host * p;

	while(h[0] != '\0')
	{
		if(h[pos] != '\0' && h[pos] != ',')
		{
			pos++;
			continue;
		}
		if(pos == 0)
		{
			h++;
			continue;
		}
		if((p = realloc(damon->hosts, sizeof(Host)
						* (damon->hosts_cnt + 1)))
				== NULL)
			return error_set_print("DaMon", 1, "%s",
					strerror(errno));
		damon->hosts = p;
		if(_hosts_host(config, &damon->hosts[damon->hosts_cnt++], h,
					pos) != 0)
			return 1;
		h+=pos;
		pos = 0;
	}
	return 0;
}

static char ** _host_comma(char * line);
static int _hosts_host(Config * config, Host * host, char * h, unsigned int pos)
{
	char * p;

	host->appclient = NULL;
	host->ifaces = NULL;
	host->vols = NULL;
	if((host->hostname = malloc(pos + 1)) == NULL)
		return error_set_print("DaMon", 1, "%s", strerror(errno));
	strncpy(host->hostname, h, pos);
	host->hostname[pos] = '\0';
#ifdef DEBUG
	fprintf(stderr, "config: Host %s\n", host->hostname);
#endif
	if((p = config_get(config, host->hostname, "interfaces")) != NULL)
		host->ifaces = _host_comma(p);
	if((p = config_get(config, host->hostname, "volumes")) != NULL)
		host->vols = _host_comma(p);
	return 0;
}

static char ** _host_comma(char * line)
{
	char * l = line;
	unsigned int pos = 0;
	char ** values = NULL;
	char ** p;
	unsigned int cnt = 0;

	for(; l[0] != '\0';)
	{
		if(l[pos] != '\0' && l[pos] != ',')
		{
			pos++;
			continue;
		}
		if(pos == 0)
		{
			l++;
			continue;
		}
		if((p = realloc(values, sizeof(char*) * (cnt+2))) == NULL)
			break;
		values = p;
		if((values[cnt] = malloc(pos+1)) != NULL)
		{
			strncpy(values[cnt], l, pos);
			values[cnt][pos] = '\0';
#ifdef DEBUG
			fprintf(stderr, "config: %s\n", values[cnt]);
#endif
		}
		values[++cnt] = NULL;
		if(values[cnt-1] == NULL)
			break;
		l+=pos;
		pos = 0;
	}
	if(l[0] == '\0')
		return values;
	if(values == NULL)
		return NULL;
	for(p = values; *p != NULL; p++)
		free(*p);
	free(values);
	return NULL;
}

static void _damon_destroy(DaMon * damon)
{
	unsigned int i;

	for(i = 0; i < damon->hosts_cnt; i++)
	{
		free(damon->hosts[i].hostname);
		if(damon->hosts[i].appclient != NULL)
			appclient_delete(damon->hosts[i].appclient);
	}
	event_delete(damon->event);
	free(damon->hosts);
}

static AppClient * _refresh_connect(Host * host, Event * event);
static int _refresh_uptime(AppClient * ac, Host * host, char * rrd);
static int _refresh_load(AppClient * ac, Host * host, char * rrd);
static int _refresh_ram(AppClient * ac, Host * host, char * rrd);
static int _refresh_swap(AppClient * ac, Host * host, char * rrd);
static int _refresh_procs(AppClient * ac, Host * host, char * rrd);
static int _refresh_users(AppClient * ac, Host * host, char * rrd);
static int _refresh_ifaces(AppClient * ac, Host * host, char * rrd);
static int _refresh_vols(AppClient * ac, Host * host, char * rrd);
static int _damon_refresh(DaMon * damon)
{
	unsigned int i;
	AppClient * ac = NULL;
	char * rrd = NULL;
	char * p;
	Host * hosts = damon->hosts;

#ifdef DEBUG
	fprintf(stderr, "%s", "_damon_refresh()\n");
#endif
	for(i = 0; i < damon->hosts_cnt; i++)
	{
		if((ac = hosts[i].appclient) == NULL)
			if((ac = _refresh_connect(&hosts[i], damon->event))
					== NULL)
				continue;
		if((p = realloc(rrd, string_length(hosts[i].hostname) + 12))
				== NULL)
			break;
		rrd = p;
		if(_refresh_uptime(ac, &hosts[i], rrd) != 0
				|| _refresh_load(ac, &hosts[i], rrd) != 0
				|| _refresh_ram(ac, &hosts[i], rrd) != 0
				|| _refresh_swap(ac, &hosts[i], rrd) != 0
				|| _refresh_procs(ac, &hosts[i], rrd) != 0
				|| _refresh_users(ac, &hosts[i], rrd) != 0
				|| _refresh_ifaces(ac, &hosts[i], rrd) != 0
				|| _refresh_vols(ac, &hosts[i], rrd) != 0)
		{
			appclient_delete(ac);
			hosts[i].appclient = NULL;
			continue;
		}
		ac = NULL;
	}
	free(rrd);
	if(ac != NULL)
		fprintf(stderr, "%s", "DaMon: refresh: An error occured\n");
	return 0;
}

static AppClient * _refresh_connect(Host * host, Event * event)
{
	if(setenv("APPSERVER_Probe", host->hostname, 1) != 0)
		return NULL;
	if((host->appclient = appclient_new_event("Probe", event))
			== NULL)
		error_print("DaMon");
	return host->appclient;
}

static int _rrd_update(char * file, int args_cnt, ...);
static int _refresh_uptime(AppClient * ac, Host * host, char * rrd)
{
	int32_t ret;

	if(appclient_call(ac, &ret, "uptime") != 0)
		return error_print("DaMon");
	sprintf(rrd, "%s_%s", host->hostname, "uptime.rrd");
	_rrd_update(rrd, 1, ret);
	return 0;
}

static int _refresh_load(AppClient * ac, Host * host, char * rrd)
{
	int32_t ret[3];

	if(appclient_call(ac, &ret[0], "load_1") != 0
			|| appclient_call(ac, &ret[1], "load_5") != 0
			|| appclient_call(ac, &ret[2], "load_15") != 0)
		return error_print("DaMon");
	sprintf(rrd, "%s_%s", host->hostname, "load.rrd");
	_rrd_update(rrd, 3, ret[0], ret[1], ret[2]);
	return 0;
}

static int _refresh_procs(AppClient * ac, Host * host, char * rrd)
{
	int32_t res;

	if(appclient_call(ac, &res, "procs") != 0)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "procs.rrd");
	_rrd_update(rrd, 1, res);
	return 0;
}

static int _refresh_ram(AppClient * ac, Host * host, char * rrd)
{
	int32_t res[4];

	if(appclient_call(ac, &res[0], "ram_total") != 0
			|| appclient_call(ac, &res[1], "ram_free") != 0
			|| appclient_call(ac, &res[2], "ram_shared") != 0
			|| appclient_call(ac, &res[3], "ram_buffer") != 0)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "ram.rrd");
	_rrd_update(rrd, 4, res[0], res[1], res[2], res[3]);
	return 0;
}

static int _refresh_swap(AppClient * ac, Host * host, char * rrd)
{
	int32_t res[2];

	if(appclient_call(ac, &res[0], "swap_total") != 0
			|| appclient_call(ac, &res[1], "swap_free") != 0)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "swap.rrd");
	_rrd_update(rrd, 2, res[0], res[1]);
	return 0;
}

static int _refresh_users(AppClient * ac, Host * host, char * rrd)
{
	int32_t res;

	if(appclient_call(ac, &res, "users") != 0)
		return 1;
	sprintf(rrd, "%s_%s", host->hostname, "users.rrd");
	_rrd_update(rrd, 1, res);
	return 0;
}

static int _ifaces_if(AppClient * ac, Host * host, char * rrd, char * iface);
static int _refresh_ifaces(AppClient * ac, Host * host, char * rrd)
{
	char ** p = host->ifaces;
	int ret = 0;

	if(p == NULL)
		return 0;
	for(; *p != NULL; p++)
		ret |= _ifaces_if(ac, host, rrd, *p);
	return ret;
}

static int _ifaces_if(AppClient * ac, Host * host, char * rrd, char * iface)
{
	int32_t res[2];

	if(appclient_call(ac, &res[0], "ifrxbytes", iface) != 0
			|| appclient_call(ac, &res[1], "iftxbytes", iface) != 0)
		return 1;
	sprintf(rrd, "%s_%s%s", host->hostname, iface, ".rrd");
	_rrd_update(rrd, 2, res[0], res[1]);
	return 0;
}

static int _vols_vol(AppClient * ac, Host * host, char * rrd, char * vol);
static int _refresh_vols(AppClient * ac, Host * host, char * rrd)
{
	char ** p = host->vols;
	int ret = 0;

	if(p == NULL)
		return 0;
	for(; *p != NULL; p++)
		ret |= _vols_vol(ac, host, rrd, *p);
	return ret;
}

static int _vols_vol(AppClient * ac, Host * host, char * rrd, char * vol)
{
	int32_t res[2];

	if(appclient_call(ac, &res[0], "voltotal", vol) != 0
			|| appclient_call(ac, &res[1], "volfree", vol) != 0)
		return 1;
	sprintf(rrd, "%s%s%s", host->hostname, vol, ".rrd"); /* FIXME */
	_rrd_update(rrd, 2, res[0], res[1]);
	return 0;
}

static int _exec(char * argv[]);
static int _rrd_update(char * file, int args_cnt, ...)
{
	char * argv[] = { "rrdtool", "update", file, NULL, NULL };
	struct timeval tv;
	int pos;
	int i;
	va_list args;
	int ret;

	if(gettimeofday(&tv, NULL) != 0)
		return error_set_print("DaMon", 1, "%s%s", "gettimeofday: ",
				strerror(errno));
	if((argv[3] = malloc((args_cnt + 1) * 12)) == NULL)
		return error_set_print("DaMon", 1, "%s", strerror(errno));
	pos = sprintf(argv[3], "%ld", tv.tv_sec);
	va_start(args, args_cnt);
	for(i = 0; i < args_cnt; i++)
		pos += sprintf(&argv[3][pos], ":%u", va_arg(args, unsigned));
	va_end(args);
	ret = _exec(argv);
	free(argv[3]);
	return ret;
}

static int _exec(char * argv[])
{
	pid_t pid;
	int status;
	int ret;

	if((pid = fork()) == -1)
		return error_set_print("DaMon", 1, "%s%s", "fork: ",
				strerror(errno));
	if(pid == 0)
	{
		execvp(argv[0], argv);
		error_set_print("DaMon", 1, "%s%s%s", argv[0], ": ",
				strerror(errno));
		exit(2);
	}
	while(*argv != NULL)
		fprintf(stderr, "%s ", *argv++);
	fprintf(stderr, "\n");
	while((ret = waitpid(pid, &status, 0)) != -1)
		if(WIFEXITED(status))
			break;
	if(ret == -1)
		return error_set_print("DaMon", -1, "%s%s", "waitpid: ",
				strerror(errno));
	return WEXITSTATUS(status);
}


/* main */
int main(void)
{
	return _damon() == 0 ? 0 : 2;
}
