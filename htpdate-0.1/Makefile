CC = gcc
CFLAGS = -Wall -W -g3
CPPFLAGS =
LDFLAGS =

prefix = /usr/local
bindir = ${prefix}/bin

.PHONY: clean all install distclean
all: htpdate htpd

htpdate: htpdate.o htp.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o htpdate htpdate.o htp.o
htpd: htpd.o htp.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o htpd htpd.o htp.o

htp.o: htp.c htp.h
htpdate.o: htpdate.c htp.h
htpd.o: htpd.c htp.h

install: all
	-mkdir -p $(bindir)
	cp -p htpdate $(bindir)/htpdate
	cp -p htpdate $(bindir)/htpd
	chmod 755 $(bindir)/htpdate
	chmod 755 $(bindir)/htpd

clean:
	rm -f htpd htpdate *.o

distclean: clean
