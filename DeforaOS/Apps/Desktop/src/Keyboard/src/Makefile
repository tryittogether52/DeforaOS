TARGETS	= keyboard keyboardctl
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF?=
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -pedantic
LDFLAGSF= `pkg-config --libs libDesktop`
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

keyboard_OBJS = callbacks.o common.o key.o keyboard.o layout.o main.o
keyboard_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
keyboard_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs x11` -lXtst

keyboard: $(keyboard_OBJS)
	$(CC) -o keyboard $(keyboard_OBJS) $(keyboard_LDFLAGS)

keyboardctl_OBJS = keyboardctl.o
keyboardctl_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libDesktop`
keyboardctl_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libDesktop`

keyboardctl: $(keyboardctl_OBJS)
	$(CC) -o keyboardctl $(keyboardctl_OBJS) $(keyboardctl_LDFLAGS)

callbacks.o: callbacks.c callbacks.h
	$(CC) $(keyboard_CFLAGS) -c callbacks.c

common.o: common.c
	$(CC) $(keyboard_CFLAGS) -c common.c

key.o: key.c key.h
	$(CC) $(keyboard_CFLAGS) -c key.c

keyboard.o: keyboard.c callbacks.h keyboard.h ../config.h
	$(CC) $(keyboard_CFLAGS) -c keyboard.c

layout.o: layout.c layout.h
	$(CC) $(keyboard_CFLAGS) -c layout.c

main.o: main.c keyboard.h
	$(CC) $(keyboard_CFLAGS) -c main.c

keyboardctl.o: keyboardctl.c
	$(CC) $(keyboardctl_CFLAGS) -c keyboardctl.c

clean:
	$(RM) -- $(keyboard_OBJS) $(keyboardctl_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- keyboard $(DESTDIR)$(BINDIR)/keyboard
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- keyboardctl $(DESTDIR)$(BINDIR)/keyboardctl

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/keyboard
	$(RM) -- $(DESTDIR)$(BINDIR)/keyboardctl

.PHONY: all clean distclean install uninstall
