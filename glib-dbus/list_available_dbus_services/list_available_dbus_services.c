#include <gio/gio.h>
#include <unistd.h>
#include <stdio.h>


void list_available_dbus_services()
{
    GDBusProxy *proxy;
    GDBusConnection *connection;
    GError *error = NULL;
    GVariant *result;

    /*
     * We can list the dbus services for the session G_BUS_TYPE_SESSION
     * or the system G_BUS_TYPE_SYSTEM.
     * G_BUS_TYPE_SESSION requires an existing machine-id, which is not
     * in the expected location, if you use FreeBSD. A symlink do the trick
     * sudo ln -s /var/lib/dbus/machine-id /etc
     */
    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);

    g_assert_no_error (error);
    error = NULL;
    proxy = g_dbus_proxy_new_sync (connection,
                                G_DBUS_PROXY_FLAGS_NONE,
                                NULL,                               /* GDBusInterfaceInfo */
                                "org.freedesktop.DBus",             /* name */
                                "/org/freedesktop/DBus",            /* object path */
                                "org.freedesktop.DBus.ListNames",   /* interface */
                                NULL,                               /* GCancellable */
                                &error);

    g_assert_no_error (error);

    result = g_dbus_proxy_call_sync (proxy,
                                   "org.freedesktop.DBus.ListNames",
                                   NULL,
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);

    g_assert_no_error (error);
    g_assert (result != NULL);

    printf("DEBUG: result data type id %s\n", g_variant_get_type_string(result));

    {
        gchar *str;
        GVariantIter *iter;
        g_variant_get (result, "(as)", &iter);
        while (g_variant_iter_loop (iter, "s", &str))
            g_print ("%s\n", str);
        g_variant_iter_free (iter);
    }
    g_variant_unref (result);
    g_object_unref (proxy);
    g_object_unref (connection);

}

int main (int   argc, char *argv[])
{
    GMainLoop* loop = NULL;
    loop = g_main_loop_new (NULL, FALSE);

    list_available_dbus_services();

    g_main_loop_unref (loop);

    return 0;
}
