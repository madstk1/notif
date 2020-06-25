#ifndef NOTIF_X11_H
#define NOTIF_X11_H

#include <gio/gio.h>

#define NOTIF_NAME      "notif"

int  notif_x11_initialize(void);
void notif_x11_cleanup(void);

void notif_x11_show(void);
void notif_x11_hide(void);

gboolean notif_x11_prepare(GSource *, int *);
gboolean notif_x11_check(GSource *);
gboolean notif_x11_dispatch(GSource *, GSourceFunc, void *);

#endif /* NOTIF_XCB_H */
