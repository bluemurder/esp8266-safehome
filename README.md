# esp8266-safehome
ESP8266 (NodeMCU v1.0) safe home system with RFID authentication and multiple wireless sensors

## Cybersecurity notes
Within version 2.4.0 of *Arduino core for ESP8266 WiFi chip* (released Jan 2, 2018) such project is secure against KRACK WPA2 attack.

## Description
The presented project is a modular wireless intrusion detection system for home and office. The intrusion detection is performed exploiting infrared sensors (PIR), and the alarm is activated and deactivated by the owners via physical devices (RFID keys).

It is based on several NodeMCU 1.0 units connected through a dedicated hidden wireless network protected via WPA2 PSK security. The wireless units communicate via HTTP requests/responses, and in this way it can scale with different sensor positioned in any way with respect to the central access point.

The access point act as IP router for the other alarm modules, collecting data from remote sensors and activating the siren. The following image shows a possible deployment of presented system.

![alt text](https://github.com/bluemurder/esp8266-safehome/blob/master/images/plant1.png "Deploy example")

* **accesspoint**: it is the central part of the system. It act as access point, router and default gateway of the dedicated wireless network. It stores the information on allowed RFID tags able to activate or not the alarm, and presents a backup battery power circuit that allows operations without main power presence. It brings a PIR sensor, and it controls a siren. Finally, it is connected to an RFID reader to enable alarm activation. 
* **rfidstation**: it is a secondary point where the owner can activate or deactivate the alarm. This unit is connected to the main power supply, and it is deployed externally with respect to the monitored area, No presence sensors are hosted by such component. Some leds are used to show to the used the security system status (component connested to the access point, alarm active or not).
* **pirstation**: the purpose of that component is to host presence sensors and to inform the access point in case of intrusion.
* **sirenstation**: this module can be used to drive a secondary external siren. 