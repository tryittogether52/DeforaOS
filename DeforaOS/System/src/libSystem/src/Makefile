TARGETS	= libSystem.a libSystem.so.0.0 libSystem.so.0 libSystem.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
CC	?= cc
CPPFLAGSF= -I ../include
CPPFLAGS=
CFLAGSF	= -W -fPIC
CFLAGS	= -g -Wall -O2 -pedantic
AR	?= ar
RANLIB	?= ranlib
CCSHARED?= $(CC) -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

libSystem_OBJS = array.o buffer.o config.o error.o event.o hash.o object.o parser.o plugin.o string.o token.o
libSystem_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libSystem_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)  

libSystem.a: $(libSystem_OBJS)
	$(AR) -rc libSystem.a $(libSystem_OBJS)
	$(RANLIB) libSystem.a

libSystem.so.0.0 libSystem.so.0 libSystem.so: $(libSystem_OBJS)
	$(CCSHARED) -o libSystem.so.0.0 -Wl,-soname,libSystem.so.0 $(libSystem_OBJS) $(libSystem_LDFLAGS)
	$(LN) -s -- libSystem.so.0.0 libSystem.so.0
	$(LN) -s -- libSystem.so.0.0 libSystem.so

array.o: array.c
	$(CC) $(libSystem_CFLAGS) -c array.c

buffer.o: buffer.c
	$(CC) $(libSystem_CFLAGS) -c buffer.c

config.o: config.c
	$(CC) $(libSystem_CFLAGS) -c config.c

error.o: error.c
	$(CC) $(libSystem_CFLAGS) -c error.c

event.o: event.c
	$(CC) $(libSystem_CFLAGS) -c event.c

hash.o: hash.c
	$(CC) $(libSystem_CFLAGS) -c hash.c

object.o: object.c
	$(CC) $(libSystem_CFLAGS) -c object.c

parser.o: parser.c token.h
	$(CC) $(libSystem_CFLAGS) -c parser.c

plugin.o: plugin.c
	$(CC) $(libSystem_CFLAGS) -c plugin.c

string.o: string.c
	$(CC) $(libSystem_CFLAGS) -c string.c

token.o: token.c token.h
	$(CC) $(libSystem_CFLAGS) -c token.c

clean:
	$(RM) -- $(libSystem_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 -- libSystem.a $(DESTDIR)$(LIBDIR)/libSystem.a
	$(INSTALL) -m 0755 -- libSystem.so.0.0 $(DESTDIR)$(LIBDIR)/libSystem.so.0.0
	$(LN) -s -- libSystem.so.0.0 $(DESTDIR)$(LIBDIR)/libSystem.so.0
	$(LN) -s -- libSystem.so.0.0 $(DESTDIR)$(LIBDIR)/libSystem.so

uninstall:
	$(RM) -- $(DESTDIR)$(LIBDIR)/libSystem.a
	$(RM) -- $(DESTDIR)$(LIBDIR)/libSystem.so.0.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libSystem.so.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libSystem.so

.PHONY: all clean distclean install uninstall
