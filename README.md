# esp8266-safehome
ESP8266 (NodeMCU v1.0) safe home system with RFID authentication and multiple wireless sensors

## Cybersecurity notes
Within version 2.4.0 of *Arduino core for ESP8266 WiFi chip* (released Jan 2, 2018) such project is secure against KRACK WPA2 attack.

## Description
The presented project is a modular wireless intrusion detection system for home and office. The intrusion detection is performed exploiting infrared sensors (PIR), and the alarm is activated and deactivated by the owners via physical devices (RFID keys).

It is based on several NodeMCU 1.0 units connected through a dedicated hidden wireless network protected via WPA2 PSK security. The wireless units communicate via HTTP requests/responses, and in this way it can scale with different sensor positioned in any way with respect to the central access point.

The access point act as IP router for the other alarm modules, collecting data from remote sensors and activating the siren.

![alt text](https://github.com/bluemurder/esp8266-safehome/images/plant1.png "Deploy example")

