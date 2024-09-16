#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "scan.h"
#include "nus.h"

#ifdef GATTLIB_LOG_BACKEND_SYSLOG
#include <syslog.h>
#endif

int main(){
    
#ifdef GATTLIB_LOG_BACKEND_SYSLOG
	openlog("gattlib_nordic_uart", LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_USER);
	setlogmask(LOG_UPTO(LOG_INFO));
#endif
    
    pthread_t thid;
    void *ret;

    if (pthread_create(&thid, NULL, nus_scan_connect, NULL) != 0) {
        perror("pthread_create() error");
        exit(1);
    }

    if (pthread_join(thid, &ret) != 0) {
        perror("pthread_create() error");
        exit(3);
    }

    return 0;
}