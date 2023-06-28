CFLAGS += $(shell pkg-config --cflags libevdev)
LIBS   += $(shell pkg-config --libs   libevdev)

DESTDIR ?= /usr/local

all: yinput

yinput: yinput.c record.c replay.c
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

install: yinput
	install -m 755 yinput $(DESTDIR)/bin/yinput

uninstall: yinput
	rm $(DESTDIR)/bin/yinput
