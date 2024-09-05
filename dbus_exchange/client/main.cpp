#include <dbus/dbus.h>
#include <iostream>
#include <string>
#include "../build/protobuf_message/message.pb.h" //TODO: add build to include directories

const char *SERVICE_NAME = "com.example.MyService1";
const char *OBJECT_PATH = "/com/example/MyObject";
const char *INTERFACE_NAME = "com.example.MyInterface";
const char *METHOD_NAME = "MyMethod";

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

  DBusMessage *message = dbus_message_new_method_call(
      SERVICE_NAME, OBJECT_PATH, INTERFACE_NAME, METHOD_NAME);
  if (message == NULL)
  {
    std::cerr << "Error creating message" << std::endl;
    return 1;
  }

  DBusMessage *reply = dbus_connection_send_with_reply_and_block(
      connection, message, -1, &error);
  if (dbus_error_is_set(&error))
  {
    std::cerr << "Error sending message: " << error.message << std::endl;
    dbus_error_free(&error);
    return 1;
  }

  DBusMessageIter iter;
  dbus_message_iter_init(reply, &iter);
  int type = dbus_message_iter_get_arg_type(&iter);
  if (type != DBUS_TYPE_STRING)
  {
    std::cerr << "Unexpected reply type: " << (char)type << std::endl;
    return 1;
  }

  const char *serialized;
  dbus_message_iter_get_basic(&iter, &serialized);
  std::string data(serialized);

  example::Message msg;
  if (!msg.ParseFromString(data))
  {
    std::cerr << "Error parsing message" << std::endl;
    return 1;
  }

  std::cout << "Received message: " << msg.text1() << std::endl;
  return 0;
}