#include <stdio.h>
#include <string.h>
#include <gio/gio.h>
#include <notifd/dbus.h>
#include <notifd/x11.h>
#include <notifd/notification.h>

void usage() {
    printf("notif v" VERSION "\n");
    printf("usage: notif [-v]\n");
}

int main(int argc, char **argv) {
    if(argc > 1 && !strcmp(argv[1], "-v")) {
        usage();
        exit(1);
    }

    GSource *x11_source;
    GPollFD pollfd = {0, G_IO_IN | G_IO_HUP | G_IO_ERR, 0 };
    GMainLoop *mainloop = g_main_loop_new(NULL, 0);

    notif_dbus_init();
    pollfd.fd = notif_x11_initialize();

    GSourceFuncs x11_sources = {
        notif_x11_prepare,
        notif_x11_check,
        notif_x11_dispatch,
        NULL, NULL, NULL
    };

    x11_source = g_source_new(&x11_sources, sizeof(GSource));
    g_source_add_poll(x11_source, &pollfd);
    g_source_attach(x11_source, NULL);

    g_timeout_add(100, notif_notification_loop, NULL);

    g_main_loop_run(mainloop);
    g_clear_pointer(&mainloop, g_main_loop_unref);

    notif_x11_cleanup();
    notif_dbus_cleanup();
    return 0;
}
