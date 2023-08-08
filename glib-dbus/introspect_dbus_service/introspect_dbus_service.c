#include <gio/gio.h>
#include <unistd.h>
#include <stdio.h>


void introspect_dbus_services(char * path)
{
    GDBusProxy *proxy;
    GDBusConnection *connection;
    GError *error = NULL;
    GVariant *result;
    char buffer[1024+1];

    if (strlen(path) >= 1024) {
        printf("ERROR: The service name is too long\n");
        return;
    }

    // convert path A.B.C into /A/B/C
    strcpy(buffer,"/");
    strcat(buffer, path);
    buffer[strlen(path)+1]='\0';
    for (int i=0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '.') {
            buffer[i] = '/';
        }
    }

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
                                G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                NULL,                               /* GDBusInterfaceInfo */
                                path,                               /* name */
                                buffer,                               /* object path */
                                path,   /* interface */
                                NULL,                               /* GCancellable */
                                &error);

    g_assert_no_error (error);

    result = g_dbus_proxy_call_sync (proxy,
                                   "org.freedesktop.DBus.Introspectable.Introspect",
                                   NULL,
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);

    g_assert_no_error (error);
    g_assert (result != NULL);

    printf("DEBUG: result data type id %s\n", g_variant_get_type_string(result));
    {
        const gchar *xml_data;
        g_variant_get (result, "(&s)", &xml_data);
        printf("%s introspection:\n\n%s\n", path, xml_data);
    }

    g_variant_unref (result);
    g_object_unref (proxy);
    g_object_unref (connection);

}

int main (int   argc, char *argv[])
{
    GMainLoop* loop = NULL;

    if (argc == 1) {
        printf("ERROR: At least one argument is required\n");
        exit (0);
    }

    loop = g_main_loop_new (NULL, FALSE);

    introspect_dbus_services(argv[1]);

    g_main_loop_unref (loop);

    return 0;
}
