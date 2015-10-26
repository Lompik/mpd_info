#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "parse_config.h"
#include "debug.h"

/* #define MAX_USER_CONF_LINE_READ 100 */
/* struct RGBcolor { */
/*   float r, g,b; */
/* }; */


/* struct user_config{ */
/*   char font[MAX_USER_CONF_LINE_READ]; */
/*   int x_pos, y_pos; */
/*   int rounding; */
/*   int padding; */
/*   struct RGBcolor background_color; */
/*   struct RGBcolor font_color;     */
/* }; */

/* #define log_err printf */

const struct user_config defaultconf= {"Droid Sans Mono 20", 640,640,10,10,{0.2,0.2,0.2},{0.1,0.1,0.1},{1.0,1.0,1.0}, 10, 0,10,0};

extern int errno;

int check_intconv_error(char *intstr, int base, int perform_boundchecks,int minvalue,int maxvalue){
    int  errnotmp=errno,temp=0;char *endptr;

    errno=0;
    temp = strtol(intstr, &endptr, base);
    //printf("sdsd%d",temp);
    if (errno == ERANGE) {
        switch((long)temp) {
        case LONG_MIN:
            // underflow
            log_err("Overflow:%s\n",intstr);
            break;
        case LONG_MAX:
            // overflow
            break;
            log_err("underflow:%s\n",intstr);
        default:
            exit(0); // impossible
        }
    }   
    else if (errno != 0)
    {
        log_err("Error converting int:%s\n",intstr);
    }
    else if (*endptr != '\0')
    {
        log_err("Garbage in the end string:%s\n",intstr);
    } else
    {
        if(perform_boundchecks==1)
        {
            if( temp <= minvalue)
                return(minvalue);
            if( temp >= maxvalue)
                return(maxvalue);
        }
        return(temp);
    }
    errno=errnotmp;
    return(0);
}

struct RGBcolor colorConverter(int hexValue)
{
    struct RGBcolor rgbColor;
    rgbColor.r = ((hexValue >> 16) & 0xFF) / 255.0;  // Extract the RR byte
    rgbColor.g = ((hexValue >> 8) & 0xFF) / 255.0;   // Extract the GG byte
    rgbColor.b = ((hexValue) & 0xFF) / 255.0;        // Extract the BB byte

    return rgbColor;
}

int get_user_config( struct user_config *uc, const char *path)
{
    FILE *fp;
    char *token; char *saveptr, *value;
    int count=0;char bufr[MAX_USER_CONF_LINE_READ]; int hex;

    errno=0;
    if((fp = fopen(path,"r")) != NULL){
        /* then file opened successfully. */
        log_info("Using config file: %s", path);
        while(fgets(bufr,MAX_USER_CONF_LINE_READ,fp)!=NULL){
            /* then no read error */
            count +=1;
            //log_info("%d: %s", count, bufr);

            token=strtok_r(bufr, "=", &saveptr);

            if(token == NULL)
                break;

            value = strtok_r(NULL, "\n", &saveptr); // TODO: perform bound checkings

            if (    strcmp(token,"font") == 0)
            {
                strncpy(uc->font, value, MAX_USER_CONF_LINE_READ);
            }
            else if(strcmp(token,"x_pos") == 0)
            {
                uc->x_pos = check_intconv_error(value,10,1,0,2560);
            }
            else if(strcmp(token,"y_pos") == 0){
                uc->y_pos = check_intconv_error(value,10,1,0,1440);
            }
            else if(strcmp(token,"background_color") == 0)
            {
                hex = check_intconv_error(value,16,0,1,2);
                uc->background_color=colorConverter(hex);
            }
            else if(strcmp(token,"border_color") == 0)
            {
                hex = check_intconv_error(value,16,0,1,2);
                uc->border_color=colorConverter(hex);
            }
            else if(strcmp(token,"font_color") == 0)
            {
                hex = check_intconv_error(value,16,0,1,2);
                uc->font_color=colorConverter(hex);
            }
            else if(strcmp(token,"maxstroke") == 0)
            {
                uc->maxstroke = check_intconv_error(value,10,1,1,150);
            }
            else if(strcmp(token,"faketransparency") == 0)
            {
                uc->faketransparency = check_intconv_error(value,10,1,0,1);
            }
            else if(strcmp(token,"padding") == 0)
            {
                uc->padding = check_intconv_error(value,10,1,1,150);
            }
            else if(strcmp(token,"rounding_height") == 0)
            {
                uc->rounding_height = check_intconv_error(value,10,1,1,150);
            }
            else if(strcmp(token,"roundingbox_linewidth") == 0)
            {
                uc->roundingbox_linewidth = check_intconv_error(value,10,1,0,150);
            }
            else if(strcmp(token,"debug_level") == 0)
            {
                uc->debug_level = check_intconv_error(value,10,1,0,1);
            }

        }
        /* fgets returned null */
        if(errno != 0)
        {
            log_warn("error can't read file: %s",path);
            exit(1);
        }
        return (count);                /* EOF found, normal exit */
    }
    else {                    /* there was an error on open */
        log_warn("error can't open file: %s. Using default options.",path);
        //exit(1);
    }



    return(0);
}

/* int main(int argc, char *argv[])
   { */
/*   struct user_config test; */
/*   const char file[]="./mpd_info.conf"; */

/*   int i = get_user_config(&test,file); */

/*   printf("font: %s\n Number of line parsed: %d\nx_pos:%d\nBe= %f\n", test.font,i,test.x_pos,test.background_color.r); */
/* } */
