#! /bin/sh

if [ ! -x configure ]; then cd ../; fi
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
