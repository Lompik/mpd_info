AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = mpd_info		
mpd_info_SOURCES = src/mpd_info.c src/parse_config.c

LIBS=\
    @x11_LIBS@\
    @xcb_LIBS@\
    @pango_LIBS@\
    @cairo_LIBS@\
    @libmpdclient_LIBS@
INCLUDE=\
    @x11_CFLAGS@\
    @xcb_CFLAGS@\
    @pango_CFLAGS@\
    @cairo_CFLAGS@\
    @libmpdclient_CFLAGS@

AM_LIBS=$(LIBS)

if DEBUG
AM_CFLAGS = $(EXTRA_CFLAGS) $(INCLUDE) -g3 -O0
AM_CXXFLAGS = -g3 -O0
else
AM_CFLAGS = $(EXTRA_CFLAGS)  $(INCLUDE) -O2
AM_CXXFLAGS = $(INCLUDE) -O2
endif
