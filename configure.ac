dnl configure.ac -- source of ./configure for DominionMUD.
dnl
dnl Regenerate ./configure after editing this file with:  autoconf
dnl (autoconf 2.69 or later).  src/conf.h.in is kept by hand: add an
dnl "#undef NAME" line there for anything new that this file defines.
dnl
dnl This replaces the CircleMUD 3.0 configure.in (autoconf 2.13), whose
dnl generated script no longer worked with modern compilers: clang (the
dnl default compiler on macOS and FreeBSD) and GCC 14 reject the implicit
dnl int and implicit function declarations in its test programs, so it
dnl stopped at "C compiler cannot create executables" or silently got
dnl checks wrong.

AC_PREREQ([2.69])
AC_INIT([DominionMUD], [3.0])
AC_CONFIG_SRCDIR([src/comm.c])
AC_CONFIG_HEADERS([src/conf.h])

dnl ---------------------------------------------------------------- compiler
AC_PROG_CC
AC_C_CONST

dnl gcc and clang both set GCC=yes.  The code is kept free of warnings at
dnl -Wall (see AGENTS.md), so that is all we ask for.
if test "$GCC" = yes; then
  MYFLAGS="-Wall"
else
  MYFLAGS=""
fi
AC_SUBST([MYFLAGS])

AC_DEFINE([CIRCLE_UNIX], [1],
  [Define if we're compiling CircleMUD under any type of UNIX system.])

dnl ---------------------------------------------------------------- libraries
dnl Solaris-style systems keep the resolver and sockets in separate
dnl libraries; Linux, macOS and FreeBSD need nothing extra.
AC_SEARCH_LIBS([gethostbyaddr], [nsl])
AC_SEARCH_LIBS([socket], [socket])
NETLIB="$LIBS"
AC_SUBST([NETLIB])
dnl Passwords are hashed with PBKDF2 (password.c), not crypt(3); the legacy
dnl src/util/Makefile.in still refers to @CRYPTLIB@.
CRYPTLIB=""
AC_SUBST([CRYPTLIB])

dnl ---------------------------------------------------------------- headers
AC_DEFINE([STDC_HEADERS], [1], [Define if you have the ANSI C header files.])
AC_CHECK_HEADERS([sys/wait.h fcntl.h sys/fcntl.h errno.h net/errno.h \
  string.h strings.h limits.h sys/time.h sys/select.h sys/types.h unistd.h \
  memory.h assert.h arpa/telnet.h arpa/inet.h sys/stat.h sys/socket.h \
  sys/resource.h netinet/in.h netdb.h signal.h sys/uio.h])

AC_CACHE_CHECK([whether time.h and sys/time.h may both be included],
  [circle_cv_header_time],
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <sys/time.h>
#include <time.h>]], [[struct tm *tp = 0; (void) tp;]])],
    [circle_cv_header_time=yes], [circle_cv_header_time=no])])
if test "$circle_cv_header_time" = yes; then
  AC_DEFINE([TIME_WITH_SYS_TIME], [1],
    [Define if you can safely include both <sys/time.h> and <time.h>.])
fi

dnl ---------------------------------------------------------------- types
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_TYPE([struct in_addr],
  [AC_DEFINE([HAVE_STRUCT_IN_ADDR], [1], [Define if the system has struct in_addr.])],
  [], [[#include <sys/types.h>
#include <netinet/in.h>]])
AC_CHECK_TYPE([socklen_t], [],
  [AC_DEFINE([socklen_t], [int], [Define to `int' if <sys/socket.h> doesn't define.])],
  [[#include <sys/types.h>
#include <sys/socket.h>]])
AC_DEFINE([RETSIGTYPE], [void], [Define as the return type of signal handlers.])

dnl ---------------------------------------------------------------- functions
AC_CHECK_FUNCS([vprintf gettimeofday select snprintf strcasecmp strdup strerror \
  stricmp strlcpy strncasecmp strnicmp strstr vsnprintf inet_addr inet_aton])

dnl ---------------------------------------------------------------- prototypes
dnl sysdep.h declares these itself (NEED_<NAME>_PROTO) on systems whose
dnl headers don't.  Each is checked with the same headers sysdep.h includes.
m4_define([circle_proto_includes], [[
#define NO_LIBRARY_PROTOTYPES
#define __COMM_C__
#define __ACT_OTHER_C__
#include <sysdep.h>
]])
circle_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS -I$srcdir/src"
m4_foreach_w([circle_fn],
  [accept atoi atol bind bzero chdir close fclose fcntl fflush fprintf fputc
   fputs fread fscanf fseek fwrite getpeername getpid getrlimit getsockname
   gettimeofday htonl htons inet_addr inet_aton inet_ntoa listen ntohl perror
   printf qsort read remove rewind select setitimer setrlimit setsockopt
   snprintf socket sprintf sscanf strcasecmp strdup strerror stricmp strlcpy
   strncasecmp strnicmp system time unlink vsnprintf write],
  [AC_CHECK_DECL(circle_fn, [],
     [AC_DEFINE(m4_toupper([NEED_]circle_fn[_PROTO]), [],
        [Define if the system headers have no prototype for ]circle_fn[.])],
     [circle_proto_includes])
])
CPPFLAGS="$circle_save_CPPFLAGS"

dnl ---------------------------------------------------------------- make
dnl src/Makefile uses GNU make features (pattern rules, $(patsubst), -include).
dnl On FreeBSD and the other BSDs that is "gmake" (pkg install gmake).
AC_MSG_CHECKING([for GNU make])
GNUMAKE=""
for circle_make in make gmake; do
  if $circle_make --version 2>/dev/null | grep 'GNU Make' >/dev/null; then
    GNUMAKE=$circle_make
    break
  fi
done
if test -n "$GNUMAKE"; then
  AC_MSG_RESULT([$GNUMAKE])
else
  AC_MSG_RESULT([no])
  AC_MSG_WARN([GNU make is needed to build DominionMUD; install it (e.g. 'pkg install gmake' on FreeBSD) and use gmake.])
  GNUMAKE=gmake
fi

AC_CONFIG_FILES([src/Makefile src/util/Makefile])
AC_OUTPUT

echo ""
echo "Configuration completed.  To compile, type:  $GNUMAKE"
