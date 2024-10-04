#ifndef DBUS_H
#define DBUS_H

void dbus_send_message(char* message, size_t size);
void dbus_init(void);
void dbus_clean(void);

#endif /* DBUS_H */