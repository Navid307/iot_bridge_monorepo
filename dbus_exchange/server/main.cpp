#include <dbus/dbus.h>
#include <iostream>
#include <string>
#include "../build/protobuf_message/message.pb.h" //TODO: add build to include directories

const char *SERVICE_NAME = "com.example.MyService1";
const char *OBJECT_PATH = "/com/example/MyObject";
const char *INTERFACE_NAME = "com.example.MyInterface";
const char *METHOD_NAME = "MyMethod";

DBusHandlerResult handle_method_call(DBusConnection *connection,
                                     DBusMessage *message,
                                     void *user_data)
{
  std::cout << "Received method call" << std::endl;
  DBusMessage *reply = dbus_message_new_method_return(message);
  if (reply == NULL)
  {
    std::cerr << "Error creating reply" << std::endl;
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  example::Message msg;
  msg.set_text1("Hello from server new");
  std::string serialized;
  msg.SerializeToString(&serialized);

  DBusMessageIter iter;
  dbus_message_iter_init_append(reply, &iter);
  if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &serialized))
  {
    std::cerr << "Error appending message to reply" << std::endl;
    dbus_message_unref(reply);
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  if (!dbus_connection_send(connection, reply, NULL))
  {
    std::cerr << "Error sending reply" << std::endl;
  }
  dbus_connection_flush(connection);
  dbus_message_unref(reply);
  return DBUS_HANDLER_RESULT_HANDLED;
}

int main()
{
  DBusError error;
  dbus_error_init(&error);

  DBusConnection *connection = dbus_bus_get(DBUS_BUS_SESSION, &error);

  if (dbus_error_is_set(&error))
  {
    std::cerr << "Error connecting to D-Bus: " << error.message << std::endl;
    dbus_error_free(&error);
    return 1;
  }

  dbus_bus_request_name(connection, SERVICE_NAME, DBUS_NAME_FLAG_REPLACE_EXISTING,
                        &error);
  if (dbus_error_is_set(&error))
  {
    std::cerr << "Error requesting name: " << error.message << std::endl;
    dbus_error_free(&error);
    return 1;
  }

  DBusObjectPathVTable vtable = {NULL, &handle_method_call, NULL, NULL, NULL,
                                 NULL};
  if (!dbus_connection_register_object_path(connection, OBJECT_PATH, &vtable,
                                            NULL))
  {
    std::cerr << "Error registering object path" << std::endl;
    return 1;
  }

  while (dbus_connection_read_write_dispatch(connection, -1))
  {
  }

  return 0;
}