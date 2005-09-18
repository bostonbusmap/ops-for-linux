# ops Makefile
# Albert Cahalan, 2005
#
# Recursive make is considered harmful:
# http://google.com/search?q=%22recursive+make+considered+harmful%22
#
# For now this Makefile uses explicit dependencies. The project
# hasn't grown big enough to need something complicated, and the
# dependency tracking files are an ugly annoyance.
#
# This file includes */module.mk files which add on to variables:
# FOO += bar/baz
# There are none at this time.
#
# Set (or uncomment) SKIP if you wish to avoid something.

VERSION      := 13
SUBVERSION   := 0
MINORVERSION := 0
TARVERSION   := $(VERSION).$(SUBVERSION).$(MINORVERSION)

############ vars

# so you can disable it or choose something else
install  := install -D --owner 0 --group 0

usr/bin                  := $(DESTDIR)/usr/bin/
bin                      := $(DESTDIR)/bin/
sbin                     := $(DESTDIR)/sbin/
man1                     := $(DESTDIR)/usr/share/man/man1/
man5                     := $(DESTDIR)/usr/share/man/man5/
man8                     := $(DESTDIR)/usr/share/man/man8/

#SKIP     := $(bin)foo $(man1)bar.1

BINFILES := $(usr/bin)ops

MANFILES := $(man1)ops.1

NAMES    := get_clock mass_storage download_flash usp capture_video \
            powerdown_camcorder download_memory toggle_camera_lcd_screen \
            delete_file upload_file download_file close_camcorder \
            download_all_movies download_last_movie format_camcorder \
            messagebox open_camcorder ops-linux readwrite unlock_camcorder \
            send_monitor_command update_directory_listing

PNG      := opspic1.png opspic2.png opspic3.png opspic4.png

OBJ   := $(addsuffix .o,$(NAMES))
SRC   := $(addsuffix .c,$(NAMES))

TARFILES := AUTHORS README COPYING ChangeLog Makefile Makefile.win32 \
            ops.lsm ops.spec ops-linux.h \
            $(notdir $(MANFILES)) dummy.c $(PNG) $(SRC)

####### build flags

# Preprocessor flags.
PKG_CPPFLAGS := $(shell pkg-config --cflags gtk+-2.0)
CPPFLAGS     :=
ALL_CPPFLAGS := $(PKG_CPPFLAGS) $(CPPFLAGS)

# Left out -Wconversion due to noise in glibc headers.
# Left out -Wunreachable-code and -Wdisabled-optimization
# because gcc spews many useless warnings with them.
#
# Since none of the PKG_CFLAGS things are truly required
# to compile ops, they might best be moved to CFLAGS.
# On the other hand, they aren't normal -O -g things either.
#
# Note that -O2 includes -fomit-frame-pointer only if the arch
# doesn't lose some debugging ability.
#
PKG_CFLAGS   := -fno-common -ffast-math \
  -W -Wall -Wshadow -Wcast-align -Wredundant-decls \
  -Wbad-function-cast -Wcast-qual -Wwrite-strings -Waggregate-return \
  -Wstrict-prototypes -Wmissing-prototypes
# Note that some stuff below is conditional on CFLAGS containing
# an option that starts with "-g". (-g, -g2, -g3, -ggdb, etc.)
CFLAGS       := -O2 -s
ALL_CFLAGS   := $(PKG_CFLAGS) $(CFLAGS)

PKG_LDFLAGS  := -Wl,-warn-common -lusb $(shell pkg-config --libs gtk+-2.0) -lgthread-2.0
LDFLAGS      :=
ALL_LDFLAGS  := $(PKG_LDFLAGS) $(LDFLAGS)

############ Add some extra flags if gcc allows

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),tar)  

# Unlike the kernel one, this check_gcc goes all the way to
# producing an executable. There might be something that works
# until you go trying to link it.
check_gcc = $(shell if $(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS) dummy.c $(ALL_LDFLAGS) $(1) -o /dev/null $(CURSES) > /dev/null 2>&1; then echo "$(1)"; else echo "$(2)"; fi ;)

#ALL_CFLAGS += $(call check_gcc,-Wdeclaration-after-statement,)
#ALL_CFLAGS += $(call check_gcc,-Wpadded,)
ALL_CFLAGS += $(call check_gcc,-Wstrict-aliasing,)

# if not debugging, enable things that could confuse gdb
ifeq (,$(findstring -g,$(filter -g%,$(CFLAGS))))
ALL_CFLAGS += $(call check_gcc,-fweb,)
ALL_CFLAGS += $(call check_gcc,-frename-registers,)
ALL_CFLAGS += $(call check_gcc,-fomit-frame-pointer,)
endif

# in case -O3 is enabled, avoid bloat
ALL_CFLAGS += $(call check_gcc,-fno-inline-functions,)

endif
endif

############ misc.

.SUFFIXES:
.SUFFIXES: .a .o .c .s .h

.PHONY: all clean do_all install tar

ALL := $(notdir $(BINFILES))

CLEAN := $(notdir $(BINFILES))

DIRS :=

INSTALL := $(BINFILES) $(MANFILES)

# want this rule first, use := on ALL, and ALL not filled in yet
all: do_all

-include */module.mk

do_all:    $(ALL)

junk := DEADJOE *~ *.o core gmon.out

# Remove $(junk) from all $(DIRS)
CLEAN += $(junk) $(foreach dir,$(DIRS),$(addprefix $(dir), $(junk)))

##########
# not maintained because it isn't really needed:
#
#ifneq ($(MAKECMDGOALS),clean)
#-include $(addsuffix .d,$(NAMES))
#endif
#
#%.d: %.c
#	depend.sh $(ALL_CPPFLAGS) $(ALL_CFLAGS) $< > $@
############

# don't want to type "make ops-$(TARVERSION).tar.gz"
tar: $(TARFILES)
	mkdir ops-$(TARVERSION)
	(tar cf - $(TARFILES)) | (cd ops-$(TARVERSION) && tar xf -)
	tar cf ops-$(TARVERSION).tar ops-$(TARVERSION)
	gzip -9 ops-$(TARVERSION).tar

clean:
	rm -f $(CLEAN)

###### install

$(BINFILES) : all
	$(install) --mode a=rx $(notdir $@) $@

$(MANFILES) : all
	$(install) --mode a=r $(notdir $@) $@

install: $(filter-out $(SKIP) $(addprefix $(DESTDIR),$(SKIP)),$(INSTALL))

###### build

$(OBJ): %.o: %.c ops-linux.h
	$(CC) -c $(ALL_CPPFLAGS) $(ALL_CFLAGS) $< -o $@

# Two ways to do this, something obsolete might care
ops: $(OBJ)
	$(CC) $(ALL_CFLAGS) $^ $(ALL_LDFLAGS) -o $@
#	$(CC) $(ALL_CFLAGS) $(ALL_LDFLAGS) -o $@ $^

