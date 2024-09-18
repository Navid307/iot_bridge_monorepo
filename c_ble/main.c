#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nus.h"

#ifdef GATTLIB_LOG_BACKEND_SYSLOG
#include <syslog.h>
#endif

int main() {
#ifdef GATTLIB_LOG_BACKEND_SYSLOG
  openlog("gattlib_nordic_uart", LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_USER);
  setlogmask(LOG_UPTO(LOG_INFO));
#endif

  nus_scan_connect();

  return 0;
}