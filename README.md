# NTUEE-ES-2024-fall
The repository for the NTUEE embedded system labs course.

### Lab 1: LED blinking and RTOS API
This lab incorporates the basics of user interrupt, CMSIS OS and GPIO configuration to perform different behaviors under the three scenarios:
1. User button single tap.
2. User button long press.
3. A fixed-interval timer callback.

### Lab 2: Wifi and Sensor data
This lab focuses on the Wifi and the LSM6DSL modules. The accelerometer sensor values are continously transmitted to a host in the local network by TCP connection and then visualized at the host. In addition, the significant motion function is enabled in the LSM6DSL module. An interrupt is implemented to send a unique signal to the host when such events are detected.

### Lab 3: BLE programming
In this lab, a BLE central is established on a RPI to communicate with the BLE testing app on a phone. The server also supports setting the CCCD value on the client side.