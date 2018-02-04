CC = gcc
CFLAGS = -g -O3 -I./ \
	 -Wall \
	 -Wextra \
	 -Wno-unused-parameter \
	 $(shell pkg-config --cflags glib-2.0)
LDFLAGS = $(shell pkg-config --libs glib-2.0)

PROG = dtreematch
HDRS = dtreematch.h
SRCS = dtreematch.c

OBJS = $(SRCS:.c=.o)

all : $(PROG)

$(PROG) : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(PROG)

dtreematch.o : dtreematch.c dtreematch.h

.PHONY : clean install uninstall

clean :
	rm -f core $(PROG) $(OBJS)

install : uninstall
	mkdir -p $(DESTDIR)/usr/bin
	install -m 0755 dtreematch $(DESTDIR)/usr/bin/dtreematch

uninstall :
	rm -f $(DESTDIR)/usr/bin/dtreematch
