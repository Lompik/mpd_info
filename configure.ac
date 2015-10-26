#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(mpd_info, 0.1, lompik)
AC_CONFIG_SRCDIR([src/mpd_info.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
AM_SILENT_RULES 

AC_USE_SYSTEM_EXTENSIONS
AC_GNU_SOURCE #asprintf

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h signal.h])

# libraries
PKG_CHECK_MODULES([xcbproto], xcb-proto >= 1.7.1)
PKG_CHECK_MODULES([xcb], xcb > 1.5 xcb-shape xcb-aux xcb-randr cairo-xcb)
PKG_CHECK_MODULES([x11], [x11])
PKG_CHECK_MODULES([pango],    [pango pangocairo])
PKG_CHECK_MODULES([libmpdclient],	  [libmpdclient])
PKG_CHECK_MODULES([cairo],	  [cairo cairo-xlib])

AC_SUBST([EXTRA_CFLAGS], ["-Wall -Wextra -Wparentheses -Winline -pedantic"])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC

# add debug

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")


AC_CONFIG_FILES([Makefile])
AC_OUTPUT



echo ""
echo "  Package: ${PACKAGE_NAME} ${PACKAGE_VERSION}"
echo ""
echo "  Used CFLAGS:"
echo "    CFLAGS..............: ${CFLAGS}"
echo "    Warning CFLAGS......: ${CWARNFLAGS}"
echo ""
echo "  Installation:"
echo "    Prefix..............: ${prefix}"
echo ""
echo "  Debug: " ${debug} 
