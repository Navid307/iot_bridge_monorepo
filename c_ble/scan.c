#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/queue.h>

#include <syslog.h>

#include "gattlib.h"

#define BLE_SCAN_TIMEOUT   10

static const char* adapter_name;

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static void ble_discovered_device(gattlib_adapter_t* adapter, const char* addr, const char* name, void *user_data) {
	int ret;

	if (name == NULL){
		return;
	}

	if(strcmp(name, "Nordic_UART_Service") == 0){
		printf("Discovered %s - '%s'\n", addr, name);
	}
}

void* ble_task() {
	gattlib_adapter_t* adapter;
	int ret;

	ret = gattlib_adapter_open(adapter_name, &adapter);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Failed to open adapter.");
		return NULL;
	}

	pthread_mutex_lock(&g_mutex);
	ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, NULL);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Failed to scan.");
		goto EXIT;
	}

	gattlib_adapter_scan_disable(adapter);

	pthread_mutex_unlock(&g_mutex);
EXIT:
	gattlib_adapter_close(adapter);
	return NULL;
}

int scan(void) {
	int ret;
	
	ret = gattlib_mainloop(ble_task, NULL);
	if (ret != GATTLIB_SUCCESS) {
		GATTLIB_LOG(GATTLIB_ERROR, "Failed to create gattlib mainloop");
	}

	return ret;
}