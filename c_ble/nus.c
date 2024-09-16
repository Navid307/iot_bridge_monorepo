#include <assert.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "gattlib.h"

#include "nus.h"


static pthread_cond_t m_connection_terminated = PTHREAD_COND_INITIALIZER;

// declaring mutex
static pthread_mutex_t m_connection_terminated_lock = PTHREAD_MUTEX_INITIALIZER;

static gattlib_connection_t* m_connection;

static void notification_handler(const uuid_t* uuid, const uint8_t* data, size_t data_length, void* user_data) {
	uintptr_t i;
	
	for(i = 0; i < data_length; i++) {
		printf("%c", data[i]);
	}
    fflush(stdout);
}

static void int_handler(int dummy) {
	gattlib_disconnect(m_connection, false /* wait_disconnection */);
	exit(0);
}

static void on_device_connect(gattlib_adapter_t* adapter, const char *dst, gattlib_connection_t* connection, int error, void* user_data) {
	uuid_t nus_characteristic_tx_uuid;
	uuid_t nus_characteristic_rx_uuid;
	char input[256];
	char* input_ptr;
	int ret, total_length, length = 0;

	// Convert characteristics to their respective UUIDs
	ret = gattlib_string_to_uuid(NUS_CHARACTERISTIC_TX_UUID, strlen(NUS_CHARACTERISTIC_TX_UUID) + 1, &nus_characteristic_tx_uuid);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Fail to convert characteristic TX to UUID.");
		goto EXIT;
	}
	ret = gattlib_string_to_uuid(NUS_CHARACTERISTIC_RX_UUID, strlen(NUS_CHARACTERISTIC_RX_UUID) + 1, &nus_characteristic_rx_uuid);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Fail to convert characteristic RX to UUID.");
		goto EXIT;
	}

	// Look for handle for NUS_CHARACTERISTIC_TX_UUID
	gattlib_characteristic_t* characteristics;
	int characteristic_count;
	ret = gattlib_discover_char(connection, &characteristics, &characteristic_count);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Fail to discover characteristic.");
		goto EXIT;
	}

	//
	// Confirm the Nordic UART RX and TX GATT characteristics are present
	//
	uint16_t tx_handle = 0, rx_handle = 0;
	for (int i = 0; i < characteristic_count; i++) {
		if (gattlib_uuid_cmp(&characteristics[i].uuid, &nus_characteristic_tx_uuid) == 0) {
			tx_handle = characteristics[i].value_handle;
		} else if (gattlib_uuid_cmp(&characteristics[i].uuid, &nus_characteristic_rx_uuid) == 0) {
			rx_handle = characteristics[i].value_handle;
		}
	}
	if (tx_handle == 0) {
		GATTLIB_LOG(GATTLIB_ERROR, "Fail to find NUS TX characteristic.");
		goto FREE_GATT_CHARACTERISTICS;
	} else if (rx_handle == 0) {
		GATTLIB_LOG(GATTLIB_ERROR, "Fail to find NUS RX characteristic.");
		goto FREE_GATT_CHARACTERISTICS;
	}

	//
	// Listen for Nordic UART TX GATT characteristic
	//
	ret = gattlib_register_notification(connection, notification_handler, NULL);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Fail to register notification callback.");
		goto FREE_GATT_CHARACTERISTICS;
	}

	ret = gattlib_notification_start(connection, &nus_characteristic_rx_uuid);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Fail to start notification.");
		goto FREE_GATT_CHARACTERISTICS;
	}

	// Register handler to catch Ctrl+C
	m_connection = connection;
	signal(SIGINT, int_handler);

	while(1) {
		fgets(input, sizeof(input), stdin);

		// NUS TX can only receive 20 bytes at a time
		input_ptr = input;
		for (total_length = strlen(input) + 1; total_length > 0; total_length -= length) {
			length = MIN(total_length, 20);
			ret = gattlib_write_without_response_char_by_handle(m_connection, tx_handle, input_ptr, length);
			if (ret) {
				GATTLIB_LOG(GATTLIB_ERROR, "Fail to send data to NUS TX characteristic.");
				goto FREE_GATT_CHARACTERISTICS;
			}
			input_ptr += length;
		}
	}

FREE_GATT_CHARACTERISTICS:
	free(characteristics);

EXIT:
	gattlib_disconnect(connection, false /* wait_disconnection */);

	pthread_mutex_lock(&m_connection_terminated_lock);
	pthread_cond_signal(&m_connection_terminated);
	pthread_mutex_unlock(&m_connection_terminated_lock);
}

static void ble_discovered_device(gattlib_adapter_t* adapter, const char* addr, const char* name, void *user_data) {
	int ret;
	int16_t rssi;

	if(name == NULL){
		return;
	}
	
	if (strcmp(name, "Nordic_UART_Service") == 0){
	
	ret = gattlib_get_rssi_from_mac(adapter, addr, &rssi);

	ret = gattlib_connect(adapter, addr, GATTLIB_CONNECTION_OPTIONS_NONE, on_device_connect, NULL);
	if (ret != GATTLIB_SUCCESS) {
		GATTLIB_LOG(GATTLIB_ERROR, "Failed to connect to the bluetooth device '%s'", addr);
	}
	}
}

static void* ble_task(void* arg) {
	char* addr = arg;
	gattlib_adapter_t* adapter;
	int ret;

	ret = gattlib_adapter_open(NULL, &adapter);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Failed to open adapter.");
		return NULL;
	}

	ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device, BLE_SCAN_TIMEOUT, NULL);
	if (ret) {
		GATTLIB_LOG(GATTLIB_ERROR, "Failed to scan.");
		return NULL;
	}

	// Wait for the device to be connected
	pthread_mutex_lock(&m_connection_terminated_lock);
	pthread_cond_wait(&m_connection_terminated, &m_connection_terminated_lock);
	pthread_mutex_unlock(&m_connection_terminated_lock);

	return NULL;
}

int nus_scan_connect(void) {
	int ret;
	
	ret = gattlib_mainloop(ble_task, NULL);
	if (ret != GATTLIB_SUCCESS) {
		GATTLIB_LOG(GATTLIB_ERROR, "Failed to create gattlib mainloop");
	}

	return 0;
}