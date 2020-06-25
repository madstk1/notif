#ifndef NOTIF_NOTIFICATION_H
#define NOTIF_NOTIFICATION_H

#include <gio/gio.h>
#include <notifd/common.h>

typedef struct {
    u32         id;
    const char *summary;
    const char *body;
    double      timeout;
} notif_notification_t;

notif_notification_t   *notif_notification_create();
void                    notif_notification_free(notif_notification_t *);
void                    notif_notification_show(notif_notification_t *);
gboolean                notif_notification_loop(void *);

#endif /* NOTIF_NOTIFICATION_H */
