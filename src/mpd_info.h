#ifndef MPD_INFO_H
#define MPD_INFO_H

#include "debug.h"

/**************************************************************************/
/* Structs                                                                */
/**************************************************************************/
struct drawarea  { int width, height,padding;} ;

/**************************************************************************/
/* Functions                                                              */
/**************************************************************************/

PangoLayout *pango_create_layout(cairo_t *cr, char *text);

static void do_drawing(cairo_t *cr,PangoLayout *layout, struct drawarea res);
void draw_roundingbox(cairo_t *cr,  struct drawarea res);

static xcb_visualtype_t *get_root_visual_type(xcb_screen_t *s);
void create_xcb_mask(xcb_connection_t *xc, xcb_window_t win, xcb_screen_t *screen,struct drawarea drawa, PangoLayout *layout);

int get_mpd_status();

#endif
