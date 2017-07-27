// PINOUT:
// RC522 MODULE    NodeMCU 1.0
// SDA             15
// SCK             14
// MOSI            13
// MISO            12
// IRQ             N/A
// GND             GND
// RST             5
// 3.3V            3.3V

//////////////////////////////////////////////////////////////////////////////
// Includes
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

//////////////////////////////////////////////////////////////////////////////
// Defines
#define SS_PIN 15 // SPI pin connected to local RFID reader
#define RST_PIN 5 // SPI RST pin connected to local RFID reader
#define REDLED_PIN D2 // D2
#define GREENLED_PIN D4 // D4

//////////////////////////////////////////////////////////////////////////////
// Globals
MFRC522 mfrc522(SS_PIN, RST_PIN);
IPAddress accesspoint_ip(42, 42, 42, 42);
int accesspoint_port(42501);
WiFiClient wifiClient;
WiFiServer httpServer(15763);

//////////////////////////////////////////////////////////////////////////////
// Setup
void setup()
{
  // Status leds
  pinMode(REDLED_PIN, OUTPUT);
  pinMode(GREENLED_PIN, OUTPUT);
  
  // Initiate  SPI bus
  SPI.begin();
  
  // Initiate MFRC522
  mfrc522.PCD_Init();

  // Connect with AP
  IPAddress ip(42, 42, 42, 43);
  IPAddress gateway(42, 42, 42, 42);
  IPAddress subnet(255, 0, 0, 0);
  WiFi.config(ip, gateway, subnet);

  const char* ssid = "GuaglioWifi";
  const char* password = "test";
  //const char* password = "Kk$fptf#kMUgH$-fAZN4p^9";
  WiFi.begin(ssid, password);

  // Set automatic reconnection
  WiFi.setAutoReconnect(true);

  // Start http server
  httpServer.begin();

  // Debug
  Serial.begin(9600);
}

//////////////////////////////////////////////////////////////////////////////
// Loop
void loop() {

  // Set green led according to connection
  bool connectionOk = CheckConnection();
  digitalWrite(GREENLED_PIN, connectionOk ? HIGH : LOW);

  // Set red led according to alarm state
  bool armedState = CheckAlarmState();
  digitalWrite(REDLED_PIN, armedState ? HIGH : LOW);
  
  // Read from local RFID reader 
  CheckLocalRFIDReader();
}

//////////////////////////////////////////////////////////////////////////////
// Check connection
bool CheckConnection()
{
  return WiFi.status() == WL_CONNECTED;
}

//////////////////////////////////////////////////////////////////////////////
// Check alarm state
bool CheckAlarmState()
{
  if(!wifiClient.connected())
  {
    if (!wifiClient.connect(accesspoint_ip, accesspoint_port))
    {
      return false;
    }
  }

  // This will send the request to the server
  wifiClient.print("GET /armed HTTP/1.1\r\nHost: AP\r\nConnection: close\r\n\r\n");

  // Read response
  String resp = wifiClient.readStringUntil('\r');
  Serial.println("armed: " + resp);
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

  // Send data to access point
  if(!wifiClient.connected())
  {
    if (!wifiClient.connect(accesspoint_ip, accesspoint_port))
    {
      digitalWrite(GREENLED_PIN, HIGH);
      delay(200);
      digitalWrite(GREENLED_PIN, LOW);
      delay(200);
      digitalWrite(GREENLED_PIN, HIGH);
      delay(200);
      digitalWrite(GREENLED_PIN, LOW);
      return;
    }
  }

  // This will send the request to the server
  wifiClient.print("TAG:" + content);
}
