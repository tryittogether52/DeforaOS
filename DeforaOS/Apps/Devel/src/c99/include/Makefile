SUBDIRS	= C99
PREFIX	= /usr/local
DESTDIR	=
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install
INCLUDEDIR= $(PREFIX)/include


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

install:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 0644 -- C99.h $(DESTDIR)$(INCLUDEDIR)/C99.h

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/C99.h

.PHONY: all subdirs clean distclean install uninstall
