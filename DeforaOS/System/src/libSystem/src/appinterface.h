/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef LIBSYSTEM_APPINTERFACE_H
# define LIBSYSTEM_APPINTERFACE_H


/* AppInterface */
/* types */
typedef struct _AppInterface AppInterface;


/* functions */
AppInterface * appinterface_new(char const * app);
AppInterface * appinterface_new_server(char const * app);
void appinterface_delete(AppInterface * appinterface);

/* returns */
int appinterface_port(AppInterface * appinterface);

/* useful */
int appinterface_call(AppInterface * appinterface, char * call, char buf[],
		int buflen, void ** args);
int appinterface_call_receive(AppInterface * appinterface, int * ret,
		char * func, void ** args, char buf[], int buflen);
int appinterface_receive(AppInterface * appinterface, char buf[], int buflen,
		char bufw[], int bufwlen, int * bufwpos, int * ret);

#endif /* !LIBSYSTEM_APPINTERFACE_H */
