SUBDIRS	= amd64 i386
TARGETS	= libm.a libm.so.0.0 libm.so.0 libm.so
PREFIX	= /usr/local
DESTDIR	=
LIBDIR	= $(PREFIX)/lib
AS	= $(CC)
ASFLAGS	= -W -Wall -g -pedantic -ffreestanding -c
CC	?= cc
CPPFLAGSF= -nostdinc -isystem ../../include -isystem ../../include/math -DPIC
CPPFLAGS?=
CFLAGSF	= -W -fPIC
CFLAGS	= -Wall -g -O2 -pedantic -ffreestanding
AR	?= ar
RANLIB	?= ranlib
CCSHARED= $(CC) -nostdlib -shared
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

libm_OBJS = arch.o math.o
libm_ASFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(ASFLAGS)
libm_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
libm_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

libm.a: $(libm_OBJS)
	$(AR) -rc libm.a $(libm_OBJS)
	$(RANLIB) libm.a

libm.so.0.0 libm.so.0 libm.so: $(libm_OBJS)
	$(CCSHARED) -o libm.so.0.0 -Wl,-soname,libm.so.0 $(libm_OBJS) $(libm_LDFLAGS)
	$(LN) -s -- libm.so.0.0 libm.so.0
	$(LN) -s -- libm.so.0.0 libm.so

arch.o: arch.S amd64/math.S i386/math.S
	$(AS) $(libm_ASFLAGS) -o arch.o arch.S

math.o: math.c ../../include/math/math.h
	$(CC) $(libm_CFLAGS) -c math.c

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done
	$(RM) -- $(libm_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done
	$(RM) -- $(libm_OBJS)
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 0644 -- libm.a $(DESTDIR)$(LIBDIR)/libm.a
	$(INSTALL) -m 0755 -- libm.so.0.0 $(DESTDIR)$(LIBDIR)/libm.so.0.0
	$(LN) -s -- libm.so.0.0 $(DESTDIR)$(LIBDIR)/libm.so.0
	$(LN) -s -- libm.so.0.0 $(DESTDIR)$(LIBDIR)/libm.so

uninstall:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(LIBDIR)/libm.a
	$(RM) -- $(DESTDIR)$(LIBDIR)/libm.so.0.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libm.so.0
	$(RM) -- $(DESTDIR)$(LIBDIR)/libm.so

.PHONY: all subdirs clean distclean install uninstall
