#include <stdio.h>
#include <stdlib.h>

#include <notifd/notification.h>
#include <notifd/dbus.h>
#include <notifd/x11.h>

u32 notifCounter = 1;

notif_notification_t *notif_notification_create() {
    notif_notification_t *notif = calloc(1, sizeof(notif_notification_t));
    if(!notif) {
        fprintf(stderr, "NOTIF: Failed to allocate notification: %m\n");
        exit(1);
    }

    notif->id = notifCounter++;
    notif->timeout = 5.0f;

    return notif;
}

void notif_notification_free(notif_notification_t *notif) {
    free(notif);
}

void notif_notification_show(notif_notification_t *notif) {
    g_queue_push_tail(notificationQueue, notif);
}

gboolean notif_notification_loop(void *userData) {
    notif_notification_t *notif = g_queue_peek_head(notificationQueue);
    if(!notif) {
        return true;
    }

    if(notif->timeout <= 0.0f) {
        g_queue_pop_head(notificationQueue);
        notif_x11_hide();
    } else {
        notif_x11_show();
        notif->timeout -= 0.1f;
    }
    return true;
}
