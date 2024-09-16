#ifndef NUS_H
#define NUS_H

#define NUS_CHARACTERISTIC_TX_UUID	"6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_CHARACTERISTIC_RX_UUID	"6e400003-b5a3-f393-e0a9-e50e24dcca9e"

#define BLE_SCAN_TIMEOUT   10

#define MIN(a,b)	((a)<(b)?(a):(b))

int nus_scan_connect(void);

#endif /* NUS_H */

