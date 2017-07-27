#include <SPI.h>
#include <MFRC522.h>

//////////////////////////////////////////////////////////////////////////////
// Defines
#define SS_PIN 15 // SPI pin connected to local RFID reader
#define RST_PIN 5 // SPI RST pin connected to local RFID reader

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
// Globals
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup()
{
  // Blue onboard led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Initiate  SPI bus
  SPI.begin();
  
  // Initiate MFRC522
  mfrc522.PCD_Init();   

  // To send rfid tag values via serial 
  Serial.begin(9600);

  // Setup completed
  delay(2000);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  CheckLocalRFIDReader();
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

  // Show UID on serial monitor
  Serial.print("UID tag :");

  String content= "";
  
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  Serial.print(content);
  Serial.println();
  Serial.println();

  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}
