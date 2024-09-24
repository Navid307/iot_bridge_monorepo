#include <gattlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbus.h"
#include "nus.h"

//#define GATTLIB_LOG_BACKEND_SYSLOG 

#ifdef GATTLIB_LOG_BACKEND_SYSLOG
#include <syslog.h>
#endif

#define BUFFER_SIZE 1024

uint8_t buffer[BUFFER_SIZE];
static int buffer_index = 0;

void cb(const uuid_t *uuid, const uint8_t *data, size_t data_length,
        void *user_data) {
  for (int i = 0; i < data_length; i++) {
    buffer[buffer_index] = data[i];
    buffer_index++;
    if (data[i] == 0x0D) {
      dbus_send_message(buffer, buffer_index);
      buffer_index = 0;
    }
  }
}

int main() {
#ifdef GATTLIB_LOG_BACKEND_SYSLOG
  openlog("gattlib_nordic_uart", LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_USER);
  setlogmask(LOG_UPTO(LOG_DEBUG));
#endif

  dbus_init();
  nus_scan_connect(cb);
  dbus_clean();

  return 0;
}