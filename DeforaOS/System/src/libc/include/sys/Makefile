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
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- ioctl.h $(DESTDIR)$(INCLUDEDIR)/sys/ioctl.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- ipc.h $(DESTDIR)$(INCLUDEDIR)/sys/ipc.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- mman.h $(DESTDIR)$(INCLUDEDIR)/sys/mman.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- mount.h $(DESTDIR)$(INCLUDEDIR)/sys/mount.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- param.h $(DESTDIR)$(INCLUDEDIR)/sys/param.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- reboot.h $(DESTDIR)$(INCLUDEDIR)/sys/reboot.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- resource.h $(DESTDIR)$(INCLUDEDIR)/sys/resource.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- select.h $(DESTDIR)$(INCLUDEDIR)/sys/select.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- shm.h $(DESTDIR)$(INCLUDEDIR)/sys/shm.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- stat.h $(DESTDIR)$(INCLUDEDIR)/sys/stat.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- statvfs.h $(DESTDIR)$(INCLUDEDIR)/sys/statvfs.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- time.h $(DESTDIR)$(INCLUDEDIR)/sys/time.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- times.h $(DESTDIR)$(INCLUDEDIR)/sys/times.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- types.h $(DESTDIR)$(INCLUDEDIR)/sys/types.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- uio.h $(DESTDIR)$(INCLUDEDIR)/sys/uio.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- utsname.h $(DESTDIR)$(INCLUDEDIR)/sys/utsname.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- vt.h $(DESTDIR)$(INCLUDEDIR)/sys/vt.h
	$(MKDIR) $(DESTDIR)$(INCLUDEDIR)/sys
	$(INSTALL) -m 0644 -- wait.h $(DESTDIR)$(INCLUDEDIR)/sys/wait.h

uninstall:
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/ioctl.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/ipc.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/mman.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/mount.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/param.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/reboot.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/resource.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/select.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/shm.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/stat.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/statvfs.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/time.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/times.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/types.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/uio.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/utsname.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/vt.h
	$(RM) -- $(DESTDIR)$(INCLUDEDIR)/sys/wait.h

.PHONY: all clean distclean install uninstall
