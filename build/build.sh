#! /bin/sh

autoconf; autoheader
rm -rf autom4te.cache

if [ -z "${UTIL}" ]; then
        exit 0
fi

if [ ! "${SNAPSHOT}" = "1" ]; then
	VER_MAJ=`echo $VERS | cut -f 1 -d .`
	VER_MIN=`echo $VERS | cut -f 2 -d .`
	VER_REV=`echo $VERS | cut -f 3 -d .`
	printf "%03i%03i%03i\n" $VER_MAJ $VER_MIN $VER_REV > VERSION
else
	echo "SNAPSHOT." > VERSION
fi

WIN32="${HOME}/root/windows-i386"
CFLAGS="-I${WIN32}/include"
CPPFLAGS="${CFLAGS}"
LDFLAGS="-L${WIN32}/lib -static"
LIBS="-lopennet"
DATE="`date +%Y%m%d%H%M`"
CROSS=i586-mingw32msvc
if [ ! -z "${CROSS}" ]; then
	CROSSCMD="${CROSS}-"
fi
export CFLAGS LDFLAGS CPPFLAGS LIBS
make distclean
./configure --host=${CROSS} --prefix=${HOME}/root/windows-i386/ && \
make || exit 1
${CROSSCMD}strip htpd.exe htpdate.exe
if [ ! "${SNAPSHOT}" = "1" ]; then
	cp htpd.exe "/web/rkeene/devel/htp/win32/stable/htpd-${VERS}.exe"
	cp htpdate.exe "/web/rkeene/devel/htp/win32/stable/htpdate-${VERS}.exe"
else
	cp htpd.exe "/web/rkeene/devel/htp/win32/devel/htpd-${VERS}.exe"
	cp htpdate.exe "/web/rkeene/devel/htp/win32/devel/htpdate-${VERS}.exe"
fi
