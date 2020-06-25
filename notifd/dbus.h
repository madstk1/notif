#include <stdio.h>
#include <stdlib.h>

#include <gio/gio.h>

extern GQueue *notificationQueue;

int notif_dbus_init(void);
int notif_dbus_cleanup(void);
