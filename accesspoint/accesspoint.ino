// Select NodeMCU 1.0

// PINOUT:
// ---------------------------
// RC522 MODULE    NodeMCU 1.0
// SDA             15 (D8)
// SCK             14 (D5)
// MOSI            13 (D7)
// MISO            12 (D6)
// IRQ             N/A
// GND             GND
// RST             5  (D1)
// 3.3V            3.3V
// ---------------------------
// Peripherals     NodeMcu 1.0
// Siren           4 (D2)
// Pir             2 (D4)
// Status led         D3

// IP of this access point: 42.42.42.42
// SSID: "GuaglioWifi"
// Password: "testtesttest"
// Server port: 42501

//////////////////////////////////////////////////////////////////////////////
// Include files

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 

//////////////////////////////////////////////////////////////////////////////
// Defines
#define SS_PIN 15       // SPI pin connected to local RFID reader
#define RST_PIN 5       // SPI RST pin connected to local RFID reader
#define SIREN_PIN D2    // Siren command pin (active low)
#define LOCALPIR_PIN D4 // PIR sensor input
#define ARMEDLED_PIN D3 // Led showing armed status

//////////////////////////////////////////////////////////////////////////////
// Globals
bool g_armed(false);
MFRC522 mfrc522(SS_PIN, RST_PIN);
bool g_rfidStationConnected;
WiFiServer httpServer(42501);
WiFiClient rfidStationClient;

//////////////////////////////////////////////////////////////////////////////
// Setup
void setup()
{
  // Siren control pin setup
  pinMode(SIREN_PIN, OUTPUT);
  digitalWrite(SIREN_PIN, LOW);

  // Wait one second
  delay(1000);
  
  // Init serial for debugging purpose 
  Serial.begin(9600);

  // Local sensor input pin setup
  pinMode(LOCALPIR_PIN, INPUT);

  // Led showing armed status
  pinMode(ARMEDLED_PIN, OUTPUT);

  // Led is ON when system is not armed
  digitalWrite(ARMEDLED_PIN, g_armed ? LOW : HIGH);

  // Flag used to store if the station with secondary RFID reader is present and connected
  g_rfidStationConnected = false;
  
  // Initiate  SPI bus
  SPI.begin();
  
  // Init MFRC522 interface
  mfrc522.PCD_Init();

  // Start Wifi configuring security, specific channel and hidden network mode
  bool apCreation = WiFi.softAP(
    "GuaglioWifi",                // ssid
    "testtesttest",               // password
    13,                           // channel
    1);                           // ssid_hidden

  // Check for errors
  if(!apCreation)
  {
    Serial.println("Error, WiFi.softAP() returned false.");
  }

  // Set IPv4 address
  bool apConfig = WiFi.softAPConfig(
    IPAddress(42, 42, 42, 42),         // local ip
    IPAddress(42, 42, 42, 42),         // default gateway
    IPAddress(255, 255, 255, 0)        // subnet
    );

  // Check for errors
  if(!apConfig)
  {
    Serial.println("Error, WiFi.softAPConfig() returned false.");
  }

  // Print actual IP address
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(myIP);

  // Start Webserver
  httpServer.begin();

  // Print debug messages
  Serial.println("Server started.");
  Serial.println("Setup completed.");
}

//////////////////////////////////////////////////////////////////////////////
// Main loop
void loop()
{
  // Check presence of remote RFID reader. If present, no delay will be applied
  // to alarm
  ConnectRfidStation();
  
  // Read from local RFID reader 
  CheckLocalRFIDReader();
  
  // Check local sensor
  if(digitalRead(LOCALPIR_PIN) == HIGH)
  {
    PresenceDetected();
  }

  // Handle remote requests
  HandleRemoteRequests();

  // Delay on main loop
  delay(500);
}

//////////////////////////////////////////////////////////////////////////////
// Check presence of remote RFID reader
void ConnectRfidStation()
{
  if(!rfidStationClient.connected())
  {
    if (rfidStationClient.connect(IPAddress(42, 42, 42, 43), 15763))
    {
      g_rfidStationConnected = true;
      return;
    }
    g_rfidStationConnected = false;
  }
  else
  {
    g_rfidStationConnected = true;
  }

  String strstatus = "rfidstation:";
  if(g_rfidStationConnected) strstatus += "connected\n";
  else strstatus += "not connected\n";
  Serial.print(strstatus);
}

//////////////////////////////////////////////////////////////////////////////
// Decide if start alarm immediately or with a delay
void PresenceDetected()
{
  if(g_armed)
  {
    if(g_rfidStationConnected)
    {
      ImmediateIntrusion();
    }
    else
    {
      DelayedIntrusion();
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// Handle remote requests
void HandleRemoteRequests()
{
  // Check if a client has connected
  WiFiClient wifiClient = httpServer.available();
  if (!wifiClient)
  {
    return;
  }

  // Check if there's a pending request
  if(!wifiClient.available())
  {
    return;
  }

  // Read the first line of the request
  String req = wifiClient.readStringUntil('\n');
  Serial.println("Client request:" + req);

  // Match the request
  if (req.indexOf("/armed") != -1)
  {
    // rfid station requested to get current alarm state
    wifiClient.print(g_armed ? "1" : "0");
  }
  else if (req.indexOf("/pir") != -1)
  {
    // Some sensor station detected presence
    PresenceDetected();
  }
  else if (req.indexOf("TAG:") != -1)
  {
    // Tag ID received from remote RFID station
    g_rfidStationConnected = true;

    Serial.println(">>>>>" + req.substring(5));

    if(IsKnownTagId(req.substring(5)))
    {
      Serial.println("Known tag received!!!");
      ToggleAlarmStatus();
    }
  }
  else
  {
    Serial.println("invalid request");
    //wifiClient.stop();
    return;
  }
}

//////////////////////////////////////////////////////////////////////////////
// Check local RFID tag reader
void CheckLocalRFIDReader()
{
  // Look for new cards
  if(!mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  
  // Select one of the cards
  if(!mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  String content= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }  
  content.toUpperCase();

  // Check if present tag is one of known tags
  if (IsKnownTagId(content.substring(1)))
  {
    ToggleAlarmStatus();
  }
}

//////////////////////////////////////////////////////////////////////////////
// Check if tag ID is known
bool IsKnownTagId(const String & id)
{
  if(id == "55 79 D7 2B")
  {
    return true;
  }
  else
  {
    return false;
  }
}

//////////////////////////////////////////////////////////////////////////////
// Check local RFID tag reader
void ToggleAlarmStatus()
{
  if(g_armed)
  {
    // Switch off alarm
    g_armed = false;

    // Blink
    digitalWrite(ARMEDLED_PIN, HIGH);
    delay(1000);
    digitalWrite(ARMEDLED_PIN, LOW);
    delay(1000);

    // Led is ON when system is not armed
    digitalWrite(ARMEDLED_PIN, g_armed ? LOW : HIGH);
  }
  else
  {
    // Turn on alarm
    digitalWrite(ARMEDLED_PIN, LOW);
    delay(500);
    digitalWrite(ARMEDLED_PIN, HIGH);
    delay(500);
    digitalWrite(ARMEDLED_PIN, LOW);
    delay(500);
    digitalWrite(ARMEDLED_PIN, HIGH);
    delay(500);
    digitalWrite(ARMEDLED_PIN, LOW);

    // System armed
    g_armed = true;
    
    // Led is ON when system is not armed
    digitalWrite(ARMEDLED_PIN, g_armed ? LOW : HIGH);
  }
}

//////////////////////////////////////////////////////////////////////////////
// Siren!
void ImmediateIntrusion()
{
  // TODO
  // Set siren pin high
  // Call the end siren routine after 20 minutes
  // exit
}

//////////////////////////////////////////////////////////////////////////////
// 20 seconds to turn off, then siren!
void DelayedIntrusion()
{
  //
}

