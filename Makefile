NAME = notif

include config.mk

all: options config.h notif

options:
	@echo notif build options:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"

config.h:
	cp notifd/config.def.h notifd/config.h

$(OBJ): $(SRCDIR)/config.h

.c.o:
	$(CC) $(CFLAGS) -c $< -o $(<:.c=.o)

notif: $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	rm -rf $(NAME) $(OBJ)
