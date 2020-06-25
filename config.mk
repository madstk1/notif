CC = gcc
VERSION = 0.2

SRCDIR = notifd

LIBS = $(shell pkg-config --libs dbus-1) \
	   $(shell pkg-config --libs gio-2.0) \
	   $(shell pkg-config --libs x11) \
	   $(shell pkg-config --libs xinerama) \
	   $(shell pkg-config --libs cairo) \
	   $(shell pkg-config --libs xft)

INCS = $(shell pkg-config --cflags dbus-1) \
	   $(shell pkg-config --cflags gio-2.0) \
	   $(shell pkg-config --cflags x11) \
	   $(shell pkg-config --cflags xinerama) \
	   $(shell pkg-config --cflags cairo) \
	   $(shell pkg-config --cflags xft)

SRC = $(SRCDIR)/main.c \
	  $(SRCDIR)/dbus.c \
	  $(SRCDIR)/notification.c \
	  $(SRCDIR)/x11.c
OBJ = $(SRC:.c=.o)

CFLAGS = $(INCS) -Wall -DVERSION='"$(VERSION)"' -D_XOPEN_SOURCE=700 -I. -I$(SRCDIR)
LDFLAGS = $(LIBS)
