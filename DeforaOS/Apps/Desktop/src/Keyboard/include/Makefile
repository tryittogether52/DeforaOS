PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install
INCLUDEDIR= $(PREFIX)/include


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 -- Keyboard.h $(DESTDIR)$(PREFIX)/include/Desktop/Keyboard.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Keyboard.h

.PHONY: all clean distclean install uninstall
