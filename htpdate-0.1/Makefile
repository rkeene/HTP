CC = gcc
CFLAGS = -Wall -W
CPPFLAGS =
LDFLAGS =

prefix = /usr/local
bindir = ${prefix}/bin

.PHONY: clean all install distclean
all: htpdate

htpdate: htpdate-0.1.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o htpdate htpdate-0.1.c

install: all
	-mkdir -p $(bindir)
	cp -p htpdate $(bindir)/htpdate
	chmod 755 $(bindir)/htpdate

clean:
	rm -f htpdate *.o

distclean: clean
