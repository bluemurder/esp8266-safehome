// PINOUT:
// RC522 MODULE    NodeMCU 1.0
// SDA             15 (D8)
// SCK             14 (D5)
// MOSI            13 (D7)
// MISO            12 (D6)
// IRQ             N/A
// GND             GND
// RST             5  (D1)
// 3.3V            3.3V

//////////////////////////////////////////////////////////////////////////////
// Includes
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 

//////////////////////////////////////////////////////////////////////////////
// Defines
#define SS_PIN 15       // SPI pin connected to local RFID reader
#define RST_PIN 5       // SPI RST pin connected to local RFID reader
#define REDLED_PIN D2   // D2
#define GREENLED_PIN D4 // D4

//////////////////////////////////////////////////////////////////////////////
// Globals
MFRC522 mfrc522(SS_PIN, RST_PIN);
IPAddress accesspoint_ip(42, 42, 42, 42);
int accesspoint_port(42501);
WiFiClient wifiClient;
WiFiServer httpServer(15763);
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
  
  // Initiate  SPI bus
  SPI.begin();
  
  // Initiate MFRC522
  mfrc522.PCD_Init();

  // Configure wifi node
  bool stationConfigured = WiFi.config(
    IPAddress(42, 42, 42, 43),  // local_ip
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

  // Start http server
  httpServer.begin();

  Serial.println("Server started on port 15763");

  Serial.println("Setup completed.");
}

//////////////////////////////////////////////////////////////////////////////
// Loop
void loop()
{
  // Set green led according to connection
  CheckConnection();
  digitalWrite(GREENLED_PIN, g_connected ? HIGH : LOW);
  
  // If connected, check alarm state and read from local RFID reader 
  if(g_connected)
  {
    CheckAlarmState();
    
    // Set red led according to alarm state
    digitalWrite(REDLED_PIN, g_armed ? HIGH : LOW);

    // Read if tag present
    CheckLocalRFIDReader();
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
  //String resp = wifiClient.readStringUntil('\r');
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

  // Send data to access point and blink green led
  if(!wifiClient.connected())
  {
    if (!wifiClient.connect(accesspoint_ip, accesspoint_port))
    {
      return;
    }
  }

  // This will send the request to the server
  wifiClient.print("TAG:" + content + "\n");
  Serial.println("TAG:" + content);

  // Assuming here if connection ok and green led ON.
  // Blink green led to inform user that card info was sent 
  digitalWrite(GREENLED_PIN, LOW);
  delay(200);
  digitalWrite(GREENLED_PIN, HIGH);
  delay(200);
  digitalWrite(GREENLED_PIN, LOW);
  delay(200);
  digitalWrite(GREENLED_PIN, HIGH);
  delay(200);
  digitalWrite(GREENLED_PIN, LOW);
  delay(200);
  digitalWrite(GREENLED_PIN, HIGH);
}
