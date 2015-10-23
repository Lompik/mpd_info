#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H


struct RGBcolor {
  float r, g,b;
};

#define MAX_USER_CONF_LINE_READ 100


struct user_config{
  char font[MAX_USER_CONF_LINE_READ];
  int x_pos, y_pos;
  int rounding_height;
  int padding;
  struct RGBcolor background_color;
  struct RGBcolor border_color;
  struct RGBcolor font_color;
  int maxstroke;
  int faketransparency;
  int roundingbox_linewidth;
  int debug_level;
};

const struct user_config defaultconf;
struct user_config userconf;

int get_user_config( struct user_config *uc, const char *path) ;

#endif
