
# Rust BLE Sub-Project

A desktop application that communicates with an SPP device by sending messages and subscribing to notifications.

## Current Issues:
1. **Library Choice**: Should you use `BlueR` instead of `btleplug`?
2. **Peripheral Reset Handling**: The application does not respond to peripheral resets.
3. **Concurrency**: Writing to the device and subscribing to notifications currently require separate threads.
