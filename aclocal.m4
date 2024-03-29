AC_DEFUN(DC_DO_NETWORK, [
  AC_SEARCH_LIBS(inet_aton, xnet ws2_32 wsock32, [
    AC_DEFINE(HAVE_INET_ATON, [], [Have inet_aton()])
  ], [
    AC_SEARCH_LIBS(inet_addr, nsl ws2_32 wsock32, [
      AC_DEFINE(HAVE_INET_ADDR, [], [Have inet_addr()])
    ], [
      AC_MSG_WARN([could not find inet_addr or inet_aton!])
    ])
  ])
  AC_SEARCH_LIBS(inet_ntoa, socket nsl ws2_32 wsock32,, [ AC_MSG_WARN([Couldn't find inet_ntoa!]) ])
  AC_SEARCH_LIBS(connect, socket nsl ws2_32 wsock32,, [ AC_MSG_WARN([Couldn't find connect!]) ])
])

AC_DEFUN(DC_DO_WIN32, [
  AC_CHECK_HEADERS(windows.h windowsx.h winsock2.h)
  AC_CHECK_LIB(wsock32, main, [
    AC_DEFINE(HAVE_LIBWSOCK32, [], [Have libwsock32])
      LIBS="${LIBS} -lwsock32"
  ])
])

AC_DEFUN(DC_ASK_OPTLIB, [
  AC_ARG_WITH($5, [  --with-$5 $4], [
# Specified
    LIBSPEC=$withval
  ], [
# Not specified
    LIBSPECFLAGS=`pkg-config --libs $5 2>/dev/null`
    LIBSPECCFLAGS=`pkg-config --cflags $5 2>/dev/null`
    AC_CHECK_LIB($1, $2, [
      OLDCPPFLAGS="$CPPFLAGS"
      OLDCFLAGS="$CFLAGS"
      CPPFLAGS="$CPPFLAGS $LIBSPECCFLAGS"
      CFLAGS="$CFLAGS $LIBSPECCFLAGS"
      AC_CHECK_HEADER($3, [
        LIBSPEC=yes
      ], [
        LIBSPEC=no
      ])
      CPPFLAGS="$OLDCPPFLAGS"
      CFLAGS="$OLDCFLAGS"
    ], [
      LIBSPEC=no
      $8
      AC_MSG_WARN(Didn't find $5)
    ], $LIBSPECFLAGS)
  ])
  case $LIBSPEC in
  	no)
                $8
  		AC_MSG_WARN(Support for $5 disabled)
  		;;
  	*)
  		if test "${LIBSPEC}" = "yes"; then
			true
		else
			LIBSPECFLAGS="-L${LIBSPEC}/lib ${LIBSPECFLAGS}"
			LIBSPECCFLAGS="-I${LIBSPEC}/include ${LIBSPECCFLAGS}"
  		fi
		AC_CHECK_LIB($1, $2, [
		  OLDCFLAGS="$CFLAGS"
		  OLDCPPFLAGS="$CPPFLAGS"
		  CPPFLAGS="$CPPFLAGS ${LIBSPECCFLAGS}"
		  CFLAGS="$CFLAGS ${LIBSPECCFLAGS}"
  		  AC_CHECK_HEADER($3, [
		    if test -n "$7"; then
		      AC_DEFINE($7, [1], [Define to 1 if you have the <$3> header file.])
		    fi
		    if test -n "$6"; then
		      AC_DEFINE($6, [1], [Define to 1 if you have $2 from $5])
		    fi
		    LDFLAGS="$LIBSPECFLAGS $LDFLAGS"
		    LIBS="-l$1 $LIBS"
		  ], [
		    CFLAGS="$OLDCFLAGS"
		    CPPFLAGS="$OLDCPPFLAGS"
                    $8
		    AC_MSG_ERROR(Could not find $3)
		  ])
		], [
		  AC_MSG_ERROR(Could not find $5)
		], $LIBSPECFLAGS)
  		;;
  esac
])

AC_DEFUN(DC_CHK_OS_INFO, [
	AC_CANONICAL_HOST
	AC_SUBST(CFLAGS)
	AC_SUBST(CPPFLAGS)

	AC_MSG_CHECKING(host operating system)
	AC_MSG_RESULT($host_os)

	case $host_os in
		mingw32msvc*)
			CFLAGS="$CFLAGS -mno-cygwin -mms-bitfields"
			CPPFLAGS="$CPPFLAGS -mno-cygwin -mms-bitfields"
			;;
		cygwin*)
			CFLAGS="$CFLAGS -mms-bitfields"
			CPPFLAGS="$CPPFLAGS -mms-bitfields"
			;;

	esac
])
