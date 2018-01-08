// Create a WiFi access point and provide a web server on it.

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

// Access point credentials
const char *ssid = "ESPap";
const char *password = "thereisnospoon";

// Web server on port 80
ESP8266WebServer server(80);

// Handler for home page, printing a simple web page
void handleRoot() {
  server.send(200, "text/html", "<h1>Connected</h1>");
}

// Init routine
void setup() {
  delay(1000);
  Serial.begin(9600);
  Serial.println();
  Serial.print("Configuring access point...");
  
  // Configure access point
  WiFi.softAP(ssid, password);

  // Get current IP address and print it
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Assign handler
  server.on("/", handleRoot);

  // Start web server
  server.begin();
  Serial.println("HTTP server started");
}

// Main loop routine
void loop() {
  // Manage handlers
  server.handleClient();
}
