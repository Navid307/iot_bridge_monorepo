#include <gio/gio.h>
#include <glib/gprintf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static GDBusProxy *proxy;
static GDBusConnection *conn;

void dbus_send_message(char *message, size_t message_size) {
  GVariant *result;
  GError *error = NULL;
  const gchar *str;

  result = g_dbus_proxy_call_sync(proxy, "ble_message", g_variant_new("(s)", message),
                                  G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

  g_assert_no_error(error);
  g_variant_get(result, "(&s)", &str);
  g_printf("The server answered: '%s'\n", str);
  g_variant_unref(result);
}

void dbus_init() {
  GError *error = NULL;

  conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  g_assert_no_error(error);

  proxy = g_dbus_proxy_new_sync(conn, G_DBUS_PROXY_FLAGS_NONE,
                                NULL, /* GDBusInterfaceInfo */
                                "org.example.TestServer",    /* name */
                                "/org/example/TestObject",   /* object path */
                                "org.example.TestInterface", /* interface */
                                NULL,                        /* GCancellable */
                                &error);
  g_assert_no_error(error);
}

void dbus_clean() {
  g_object_unref(conn);
  g_object_unref(proxy);
}
