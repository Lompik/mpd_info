

#define _GNU_SOURCE  /* asprintf */
#include <stdio.h>      /* printf() */
#include <unistd.h>      /* pause() */
#include <stdlib.h>      /* malloc() */
#include <string.h> /*strcpy*/


#include <xcb/shape.h>
#include <xcb/xcb.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>

#include <pango/pangocairo.h>

#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/player.h>
#include <mpd/send.h>
#include <mpd/connection.h>
#include <signal.h>

#include <wordexp.h>

#include "mpd_info.h"
#include "parse_config.h"
#include "error.h"

struct {
  char *artist;
  char *album;
  char *title;
  char *time_elapsed;
  char *time_total;
  char *date;
  char *progressbar;
  char *text;
  enum mpd_state state;
} mpd_info = {NULL, NULL, NULL, NULL, NULL, NULL, NULL,NULL,0};


#define M_PI 3.14

#define dbg_printf  debug

static volatile int keepRunning = 1;

void intHandler(int dummy)
{
  dbg_printf("\n-----  Received interrupt: %d -------\n", dummy);
  keepRunning = 0;
}


#define Fc(tag) if(mpd_info.tag) free(mpd_info.tag);

int
main ()
{
  xcb_connection_t *c;
  xcb_screen_t     *screen;
  xcb_window_t      win;

  uint32_t selmask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
  uint32_t selval[] = { 1, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE };

  uint32_t vals[] = { 0, 0 ,0,0};
  uint16_t mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
  struct drawarea self={645,350,20};
  wordexp_t abs_userconf_path;
  int textwidth=0,textheight=0;
  
  userconf = defaultconf;
  wordexp("~/.config/mpd_info/mpd_info.conf", &abs_userconf_path, 0);
  get_user_config(&userconf, abs_userconf_path.we_wordv[0]);
  wordfree(&abs_userconf_path);

  self.padding = userconf.padding;
  
  int i=0;
  signal(SIGINT, intHandler) ;
  signal(SIGTERM, intHandler);

  /* Open the connection to the X server */
  c = xcb_connect (NULL, NULL);

  /* Get the first screen */
  screen = xcb_setup_roots_iterator (xcb_get_setup (c)).data;

  /* Ask for our window's Id */
  win = xcb_generate_id(c);
  int first_run = 0;
  
  while(keepRunning && (get_mpd_status () ==1))
  {
      
    cairo_surface_t *cs;
    cairo_t *cr;

    cs = cairo_xcb_surface_create(c, win, get_root_visual_type(screen), self.width, self.height);
    cr = cairo_create(cs);
    PangoLayout *layout=pango_create_layout(cr,mpd_info.text);
    pango_layout_get_pixel_size(layout, &textwidth, &textheight);

    vals[0] = userconf.x_pos;
    vals[1] = userconf.y_pos;
    self.width  = vals[2] = (textwidth + self.padding/2 > 640)? 640+self.padding/2:textwidth + self.padding/2;
    self.height = vals[3] = textheight + self.padding/2;

    if(first_run++==0)
    {  
      xcb_create_window (c,                             /* Connection          */
                         XCB_COPY_FROM_PARENT,          /* depth (same as root)*/
                         win,                           /* window Id           */
                         screen->root,                  /* parent window       */
                         userconf.x_pos, userconf.y_pos,                          /* x, y                */
                         self.width, self.height,                      /* width, height       */
                         0,                            /* border_width        */
                         XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                         screen->root_visual,           /* visual              */
                         selmask, selval);                      /* masks, not used yet */

      /* Map the window on the screen */
      xcb_map_window (c, win);      
    }
    else xcb_configure_window(c, win, mask, vals); // TODO going to wider rectangle uses previous shape. Maybe remove and redraw in this case 
    xcb_flush (c);
    do_drawing(cr,layout, self);
    log_info("self->Width: %d; heigth: %d", self.width, self.height);

     
    cairo_destroy(cr);
    cairo_surface_destroy(cs);

    create_xcb_mask(c, win, screen,self, layout);

    g_object_unref(layout);

    //free(cs);
    Fc(artist);
    Fc(title);
    Fc(date);
    Fc(album );
    Fc(progressbar);
    Fc(time_elapsed);
    Fc(time_total);
    Fc(text);

    sleep (1);    /* hold client until Ctrl-C */
        
    if(i++>3 && userconf.debug_level==1) keepRunning=0;
  }
  xcb_unmap_window(c, win);
  xcb_destroy_window(c, win); xcb_flush(c);

  return 0;
}

#define FONT "Droid Sans Mono 20"

PangoLayout *pango_create_layout(cairo_t *cr, char *text)
{
  PangoLayout *layout;
  PangoFontDescription *fontdesc;
  layout = pango_cairo_create_layout (cr);

  fontdesc = pango_font_description_from_string (userconf.font);
  pango_layout_set_font_description (layout, fontdesc);
  pango_font_description_free (fontdesc);

  //  pango_layout_set_markup (layout, "<span font='awesome'>&#xf04c;test</span>", -1);
  pango_layout_set_markup (layout, text, -1);

  return( layout);
}

void create_xcb_mask(xcb_connection_t *xc, xcb_window_t win, xcb_screen_t *screen,struct drawarea drawa, PangoLayout *layout)
{
  cairo_surface_t *shape;
  cairo_t *cr;
  xcb_pixmap_t shape_id= xcb_generate_id(xc);
  xcb_create_pixmap(xc, 1, shape_id, screen->root,  drawa.width, drawa.height);
  shape = cairo_xcb_surface_create_for_bitmap(xc, screen, shape_id, drawa.width, drawa.height);

  cr = cairo_create(shape);
  

  if(userconf.faketransparency==0)
  {
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    draw_roundingbox(cr,drawa);
    //cairo_paint(cr);
  } else
  {
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_move_to(cr, drawa.padding/4, drawa.padding/4);
    cairo_set_line_width (cr, userconf.maxstroke);
    pango_cairo_layout_path(cr, layout);
    cairo_stroke(cr);
  }
  cairo_destroy(cr);
  cairo_surface_destroy(shape);
  
  xcb_shape_mask(xc, 
                 XCB_SHAPE_SO_INTERSECT, XCB_SHAPE_SK_BOUNDING,
                 win, 0, 0, shape_id);

  xcb_free_pixmap(xc, shape_id);
  
}

void draw_roundingbox(cairo_t *cr,  struct drawarea res)
{
  /* a custom shape that could be wrapped in a function */
  double x         = 0,        /* parameters like cairo_rectangle */
    y         = 0,
    width         = res.width,
    height        = res.height,
    aspect        = 1.0,     /* aspect ratio */
    corner_radius = userconf.padding/(userconf.rounding_height>0 ? userconf.rounding_height:1);   /* and corner curvature radius */

  double radius = corner_radius / aspect;
  double degrees = M_PI / 180.0;

  cairo_new_sub_path (cr);
  cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
  cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
  cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
  cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
  cairo_close_path (cr);
  cairo_set_source_rgb(cr, userconf.background_color.r, userconf.background_color.g, userconf.background_color.b );
  cairo_fill_preserve (cr);
  
  cairo_set_source_rgb(cr, userconf.border_color.r, userconf.border_color.g, userconf.border_color.b );
  cairo_set_line_width (cr, userconf.roundingbox_linewidth);
  cairo_stroke (cr);

}

static void do_drawing(cairo_t *cr, PangoLayout *layout, struct drawarea res)
{
  if(userconf.faketransparency==0)
  {
  
    /* cairo_rectangle(cr, 0, 0, res.width, res.height); */
    /* cairo_fill(cr); */
  
    /* cairo_rectangle(cr, 0+1, 0+1, res.width-2, res.height-2); */
    /* cairo_fill(cr);  */
    draw_roundingbox(cr,res);

    //cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);cairo_paint(cr);
  }
  else {
    cairo_move_to(cr, res.padding/4, res.padding/4);
    cairo_set_source_rgb(cr, userconf.background_color.r, userconf.background_color.g, userconf.background_color.b ); 
    //cairo_stroke_preserve(cr);
    cairo_set_line_width (cr, userconf.maxstroke);
    pango_cairo_layout_path(cr, layout);
    cairo_stroke(cr);
    //pango_cairo_update_layout(cr, layout);
    //pango_cairo_show_layout(cr, layout);
    //cairo_restore(cr);
  
    /* cairo_new_path(cr); */
    /* cairo_move_to(cr, res.padding/4, res.padding/4); */
    /* cairo_set_source_rgb(cr, userconf.font_color.r, userconf.font_color.g, userconf.font_color.b); */
    /* //cairo_stroke_preserve(cr); */
    /* cairo_set_line_width (cr, 1); */
    /* pango_cairo_layout_path(cr, layout); */
    /* //pango_cairo_update_layout(cr, layout); */
    /* //pango_cairo_show_layout(cr, layout); */
    /* cairo_stroke(cr); */
    //cairo_restore(cr);
  }  
  cairo_new_path(cr);
  cairo_move_to(cr, res.padding/4, res.padding/4);
  cairo_set_source_rgb(cr, userconf.font_color.r, userconf.font_color.g, userconf.font_color.b);
  pango_cairo_update_layout(cr, layout);
  pango_cairo_show_layout(cr, layout);
  
  /* a custom shape that could be wrapped in a function */
  /* double x         = res.padding*0,        /\* parameters like cairo_rectangle *\/ */
  /*   y         = res.padding*0, */
  /*   width         = res.width/1, */
  /*   height        = res.height/1, */
  /*   aspect        = 1.0,     /\* aspect ratio *\/ */
  /*   corner_radius = height / 5.0;   /\* and corner curvature radius *\/ */

  /* double radius = corner_radius / aspect; */
  /* double degrees = M_PI / 180.0; */

  /* cairo_new_sub_path (cr); */
  /* cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees); */
  /* cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees); */
  /* cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees); */
  /* cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees); */
  /* cairo_close_path (cr); */

  /* cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); */
  /* cairo_fill_preserve (cr); */
  /* cairo_set_source_rgba (cr, 0.5, 0, 0, 0.5); */
  /* cairo_set_line_width (cr, 10.0); */
  /* cairo_stroke (cr); */
  /* pango_cairo_update_layout(cr, layout); */
  /* pango_cairo_show_layout(cr, layout); */

}

static xcb_visualtype_t *
get_root_visual_type(xcb_screen_t *s)
{
  xcb_visualtype_t *visual_type = NULL;
  xcb_depth_iterator_t depth_iter;

  for ( depth_iter = xcb_screen_allowed_depths_iterator(s) ; depth_iter.rem ; xcb_depth_next(&depth_iter) )
  {
    xcb_visualtype_iterator_t visual_iter;
    for ( visual_iter = xcb_depth_visuals_iterator(depth_iter.data) ; visual_iter.rem ; xcb_visualtype_next(&visual_iter) )
    {
      if ( s->root_visual == visual_iter.data->visual_id )
      {
        visual_type = visual_iter.data;
        break;
      }
    }
  }

  return visual_type;
}

char *second_to_mins(uint32_t seconds)
{
  char *buf = malloc(10);
  div_t result;
  result = div(seconds, 60);
  sprintf(buf,"%02d:%02d", result.quot,result.rem);
  return buf;
}


#define Mc(tag,mpdtag)  temp=mpd_song_get_tag(song, MPD_TAG_##mpdtag, 0); \
  if(temp != NULL) {                                                    \
    mpd_info.tag = malloc(strlen(temp)+1);                              \
    strcpy( mpd_info.tag ,  mpd_song_get_tag(song,MPD_TAG_##mpdtag, 0)); \
  } else {mpd_info.tag = malloc(strlen("NA")+1); strcpy(mpd_info.tag,"NA");}                                        // Freeed in Main

char *loadBar(uint32_t x, uint32_t n, int w )
{
  char *result = malloc(w+10);


  if ( (x == n) && (x % (n/100+1) != 0) ) return("100%");

  float ratio  =  x/(float)n;
  int   c      =  ratio * w;


  sprintf(result,"%d%% [" , (int)(ratio*100));
  for (int x=0; x<c; x++) strcat(result, "#");
  for (int x=c; x<w; x++) strcat(result, "-");
  strcat(result, "]");

  return(result);
}

//const char mpdstatedesc[4][10]=  {"UNKNOWN"  ,"STOP"  ,"PLAY"  ,"PAUSE"};
const char mpdstatedesc[4][10]=  {"U","&#xf04d;","&#xf04b;", "&#xf04c;"};

int get_mpd_status()
{
  uint32_t time = 0;
  struct mpd_connection *conn;
  struct mpd_status *status;
   
  struct mpd_song *song;
  const char *temp;

  conn = mpd_connection_new(NULL, 0, 0);
  status = mpd_run_status(conn);
  if (!status) return 0;
  time = mpd_status_get_elapsed_time(status);
  mpd_info.state = mpd_status_get_state(status);
  song = mpd_run_current_song(conn);
  //fprintf(stderr,"Time: %d", time);
  //sleep(3);
  mpd_status_free(status);
  mpd_connection_free(conn);
    

  if(!song) return 0;
  if(song)
  {
    //dbg_printf("length:%u", strlen(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0)));
    Mc(artist,ARTIST)
    Mc(title,TITLE)
    Mc(date, DATE)
    Mc(album, ALBUM)
      
    mpd_info.time_elapsed =  second_to_mins(time);
    mpd_info.time_total =  second_to_mins(mpd_song_get_duration(song));
    mpd_info.progressbar = loadBar(time, mpd_song_get_duration(song),  10);


    //      asprintf(&mpd_info.text, "<span>artist| %s\nalbum | %s\ntitle | %s\nDate  | %s \nTime  | %s / %s\n%s {%s} </span><span font=\"awesome\">&#xf04c</span>", //freeed in Main
    asprintf(&mpd_info.text, "<span>artist| %s\n"
             "album | %s\ntitle | %s\n"
             "Date  | %s\n"
             "Time  | %s/%s\n"
             "%s</span><span font=\"awesome\">%s</span>", //freeed in Main
             mpd_info.artist,
             mpd_info.album,
             mpd_info.title,
             mpd_info.date,mpd_info.time_elapsed,
             mpd_info.time_total,
             mpd_info.progressbar,mpdstatedesc[mpd_info.state]);
    log_info("text:\n%s",mpd_info.text);
      
    /* dbg_printf("title : %s \n",mpd_info.title); */
    /* dbg_printf("album : %s \n",mpd_info.album); */
    /* dbg_printf("artist: %s \n",mpd_info.artist);     */
    /* dbg_printf("Date  : %s \n",mpd_info.date); */
    /* dbg_printf("Time  : %s / %s \n",mpd_info.time_elapsed, mpd_info.time_total);          */
    mpd_song_free(song);
  }

  //fprintf(stderr,"album: %s \n",mpd_info.album);

  return 1;
}
