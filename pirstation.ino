//////////////////////////////////////////////////////////////////////////////
// Includes
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 

//////////////////////////////////////////////////////////////////////////////
// Defines
#define PIR_PIN D5      // Sensor
#define REDLED_PIN D2   // D2
#define GREENLED_PIN D4 // D4

//////////////////////////////////////////////////////////////////////////////
// Globals
IPAddress accesspoint_ip(42, 42, 42, 42);
int accesspoint_port(42501);
WiFiClient wifiClient;
bool g_connected(false);
bool g_armed(false);

//////////////////////////////////////////////////////////////////////////////
// Setup
void setup()
{
  // Debug
  Serial.begin(9600);
  
  // Status leds
  pinMode(REDLED_PIN, OUTPUT);
  pinMode(GREENLED_PIN, OUTPUT);
  
  // Setup sensor pin
  pinMode(PIR_PIN, INPUT);

  // Configure wifi node
  bool stationConfigured = WiFi.config(
    IPAddress(42, 42, 42, 44),  // local_ip
    IPAddress(42, 42, 42, 42),  // gateway
    IPAddress(255, 255, 255, 0) // subnet
    );

  if(!stationConfigured)
  {
    Serial.println("Error, WiFi.config() returned false.");
  }

  // Connect to wifi
  bool stationConnected = WiFi.begin(
    "GuaglioWifi",
    "testtesttest");

  if(!stationConnected)
  {
    Serial.println("Error, WiFi.begin() returned false.");
  }

  // Set automatic reconnection
  bool autoreconnect = WiFi.setAutoReconnect(true);

  if(!autoreconnect)
  {
    Serial.println("Error, WiFi.setAutoReconnect() returned false.");
  }

  Serial.println("Setup completed.");
}

//////////////////////////////////////////////////////////////////////////////
// Loop
void loop()
{
  // Loop every 1 second.
  // Check connetion only every 
  CheckConnection();
  digitalWrite(GREENLED_PIN, g_connected ? HIGH : LOW);
  
  // If connected, check alarm state and read from local RFID reader 
  if(g_connected)
  {
    CheckAlarmState();
    
    // Set red led according to alarm state
    digitalWrite(REDLED_PIN, g_armed ? HIGH : LOW);

    // If armed, loop fast on pir sensor
    CheckLocalPIR();
  }

  // Wait for a while 
  delay(1000);
}

//////////////////////////////////////////////////////////////////////////////
// Check connection
void CheckConnection()
{
  g_connected = WiFi.status() == WL_CONNECTED;
}

//////////////////////////////////////////////////////////////////////////////
// Check alarm state
void CheckAlarmState()
{
  if(!wifiClient.connected())
  {
    if (!wifiClient.connect(accesspoint_ip, accesspoint_port))
    {
      return;
    }
  }

  // This will send the request to the server
  wifiClient.print("/armed\n");

  // Wait response
  String resp = wifiClient.readStringUntil('\n');

  if(resp.length() != 0)
  {
    //Serial.println("armed: " + resp + "_" + resp.length());
    if(resp.charAt(0) == '1')
    {
      g_armed = true;
    }
    else if(resp.charAt(0) == '0')
    {
      g_armed = false;
    }
    // else, does not change g_armed value
  }
  wifiClient.flush();
}

//////////////////////////////////////////////////////////////////////////////
// Check local PIR sensor
void CheckLocalPIR()
{
  // If no signal on sensor, all ok, return
  if(digitalRead(PIR_PIN) == LOW)
  {
    return;
  }

  // Else, send alarm to access point
  if(!wifiClient.connected())
  {
    if (!wifiClient.connect(accesspoint_ip, accesspoint_port))
    {
      return;
    }
  }

  // This will send the request to the server
  wifiClient.print("ALARM44\n");
  Serial.println("ALARM44");
}
