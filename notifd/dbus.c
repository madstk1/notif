#include <assert.h>
#include <gio/gio.h>
#include <notifd/dbus.h>
#include <notifd/common.h>
#include <notifd/notification.h>

u32 _ownerID, _regID;
GDBusConnection *_dbusConnection = NULL;
GDBusNodeInfo *_dbusData = NULL;
GError *_error = NULL;
GQueue *notificationQueue;

static const void (* onNotify)(notif_notification_t *) = NULL;
static const char * const _introspection_xml =
#include <notifd/introspection.xml>

static void _notif_dbus_bus_acquired(GDBusConnection *, const char *, void *);
static void _notif_dbus_name_acquired(GDBusConnection *, const char *, void *);
static void _notif_dbus_name_lost(GDBusConnection *, const char *, void *);
static void _notif_dbus_handle_method(GDBusConnection *, const char *, const char *, const char *, const char *, GVariant *, GDBusMethodInvocation *, void *);

static void _notif_dbus_get_capabilites(GDBusConnection *, const char *, GVariant *, GDBusMethodInvocation *);
static void _notif_dbus_on_notify(GDBusConnection *, const char *, GVariant *, GDBusMethodInvocation *);
static void _notif_dbus_on_close_notif(GDBusConnection *, const char *, GVariant *, GDBusMethodInvocation *);
static void _notif_dbus_get_server_capabilities(GDBusConnection *, const char *, GVariant *, GDBusMethodInvocation *);

static const GDBusInterfaceVTable dbusInterfaceTable = {
        _notif_dbus_handle_method
};

int notif_dbus_init(void) {
    onNotify = (const void (*)(notif_notification_t *)) &notif_notification_show;
    notificationQueue = g_queue_new();

    _dbusData = g_dbus_node_info_new_for_xml(_introspection_xml, &_error);
    _ownerID = g_bus_own_name(
        G_BUS_TYPE_SESSION,
        "org.freedesktop.Notifications",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        _notif_dbus_bus_acquired,
        _notif_dbus_name_acquired,
        _notif_dbus_name_lost,
        NULL, NULL
    );
    
    return _dbusData == NULL;
}

int notif_dbus_cleanup(void) {
    g_clear_pointer(&_dbusData, g_dbus_node_info_unref);
    g_bus_unown_name(_ownerID);

    return 0;
}


void _notif_dbus_bus_acquired(GDBusConnection *conn, const char *name, void *userData) {
    _regID = g_dbus_connection_register_object(
        conn,
        "/org/freedesktop/Notifications",
        _dbusData->interfaces[0],
        &dbusInterfaceTable,
        NULL, NULL,
        &_error
    );

    if(_error) {
        fprintf(stderr, "NOTIF: Error registering object: %s\n", _error->message);
        exit(1);
    }

    if(_regID <= 0) {
        fprintf(stderr, "NOTIF: Unable to register interface table.\n");
        exit(1);
    }
}

void _notif_dbus_name_acquired(GDBusConnection *conn, const char *name, void *userData) {
    if(strcmp(name, "org.freedesktop.Notifications") == 0) {
        _dbusConnection = conn;
    }
}

void _notif_dbus_name_lost(GDBusConnection *conn, const char *name, void *userData) {
    if(conn) {
        fprintf(stderr, "NOTIF: Cannot acquire org.freedesktop.Notifications\n");
        exit(1);
    }
    fprintf(stderr, "NOTIF: Cannot connect to D-BUS.\n");
    exit(1);
}

void _notif_dbus_handle_method(GDBusConnection *conn, const char *sender, const char *objectPath, const char *interface, const char *method, GVariant *parameters, GDBusMethodInvocation *invocation, void *userData) {
    if (strcmp(method, "GetCapabilities") == 0) {
        _notif_dbus_get_capabilites(conn, sender, parameters, invocation);
    } else if (strcmp(method, "Notify") == 0) {
        _notif_dbus_on_notify(conn, sender, parameters, invocation);
    } else if (strcmp(method, "CloseNotification") == 0) {
        _notif_dbus_on_close_notif(conn, sender, parameters, invocation);
    } else if (strcmp(method, "GetServerInformation") == 0) {
        _notif_dbus_get_server_capabilities(conn, sender, parameters, invocation);
    } else {
        printf("NOTIF: unknown method called: %s\n", method);
    }
}

void _notif_dbus_get_capabilites(GDBusConnection *conn, const char *sender, GVariant *params, GDBusMethodInvocation *invoc) {
    GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
    g_variant_builder_add(builder, "s", "body");

    GVariant *value = g_variant_new("(as)", builder);
    g_variant_builder_unref(builder);
    g_dbus_method_invocation_return_value(invoc, value);

    g_dbus_connection_flush(conn, NULL, NULL, NULL);
    g_variant_unref(value);
}

void _notif_dbus_on_notify(GDBusConnection *conn, const char *sender, GVariant *params, GDBusMethodInvocation *invoc) {
    int i = 0;
    GVariantIter *iter = g_variant_iter_new(params);
    GVariant *content = NULL;
    notif_notification_t *notif = notif_notification_create();

    for(content = g_variant_iter_next_value(iter); content; content = g_variant_iter_next_value(iter), i++) {
        switch(i) {
        case 0: break;
        case 1: break;
        case 2: break;
        case 3:
            if (g_variant_is_of_type(content, G_VARIANT_TYPE_STRING)) {
                notif->summary = g_variant_dup_string(content, NULL);
            }
            break;
        case 4:
            if (g_variant_is_of_type(content, G_VARIANT_TYPE_STRING)) {
                notif->body = g_variant_dup_string(content, NULL);
            }
            break;
        case 5: break;
        case 6: break;
        case 7:
            if (g_variant_is_of_type(content, G_VARIANT_TYPE_INT32)) {
                notif->timeout = g_variant_get_int32(content);
            }
            break;
        }
    }
    g_variant_iter_free(iter);

    if(notif->timeout <= 1.0f) {
        notif->timeout = 5.0f;
    }

    GVariant *reply = g_variant_new("(u)", notif->id);
    g_dbus_method_invocation_return_value(invoc, reply);
    g_dbus_connection_flush(conn, NULL, NULL, NULL);

    if(onNotify) {
        onNotify(notif);
    }
}

void _notif_dbus_on_close_notif(GDBusConnection *conn, const char *sender, GVariant *params, GDBusMethodInvocation *invoc) {
    u32 id;
    g_variant_get(params, "(u)", &id);

    // TODO:
    // Close notification.

    g_dbus_method_invocation_return_value(invoc, NULL);
    g_dbus_connection_flush(conn, NULL, NULL, NULL);
}

void _notif_dbus_get_server_capabilities(GDBusConnection *conn, const char *sender, GVariant *params, GDBusMethodInvocation *invoc) {
    GVariant *value = value = g_variant_new("(ssss)", "notif", "Natamo", VERSION, "1.2");
    g_dbus_method_invocation_return_value(invoc, value);
    g_dbus_connection_flush(conn, NULL, NULL, NULL);
}
