# CAN-Bus-ESP32
**CAN** atau **Controller Area Network** merupakan sebuah **protokol komunikasi multi-master** yang digunakan pada otomotif sehingga komunikasi yang kompleks antar ECU (Electronic Control Unit) dapat berjalan secara cepat dan real time dengan penggunaan kabel yang sedikit. CAN bus ini menggunakan dua kabel **CAN High** dan **CAN Low** serta menggunakan metode transmisi data **half-duplex**.
OSI layer yang digunakan pada CAN bus adalah** physical layer (layer 1), data link layer (layer 2), dan application layer (layer 7)**, yang berarti implementasi CAN menghubungkan dari data link layer hingga application layer.

## Penggunaan Komponen
1. ESP32 devkitc v4 (CAN Controller)
![ESP32 DevKitC V4 Pinout](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/_images/esp32-devkitC-v4-pinout.png)
2. MCP2551 (CAN Transceiver)
![MCP2551](https://store.fut-electronics.com/cdn/shop/products/MCP2551-CAN-transceiver_1024x1024.jpg?v=1668800479)

## CAN Bus Sniffing
Device = CANABLE/CANdleLight USB to CAN module
![CANABLE](https://m.media-amazon.com/images/I/41PwRFTGYWL._AC_UF894,1000_QL80_.jpg)
Software = Cangaroo [(Download in Windows)] (https://canable.io/utilities/cangaroo-win32-0363ce7.zip)

## Reverse Engineering Parking Sensor System
![Parking Sensor](https://www.static-src.com/wcsstore/Indraprastha/images/catalog/full//100/MTA-49134961/oem_oem_full01.jpg)
- 1 paket data diawali dengan pulse HIGH selama 1800-1860 µs
- 1 paket data berisi 32 bit data dengan pembagian: 
- 1 byte pertama = data sensor A
- 1 byte kedua = data sensor D
- 1 byte ketiga = data sensor C
- 1 byte keempat = data sensor B
- Bit 1 ditandai dengan pulse HIGH selama 150-200 µs
- Bit 0 ditandai dengan pulse HIGH selama 50-120 µs
