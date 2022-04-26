/*
 * This script should:
 * 1. Connect a wireless network
 * 2. Begin and continuously monitor the LSM6DS3 sensor for gyroscopic data
 * 3. When a threshold has been reached, make an HTTP call a webserver
 * 
 * This frankenstein of a script was created by thejart in April 2022 with some help by the following:
 * - dlf (Metodo2 srl), 13 July 2010
 * - Tom Igoe, 31 May 2012
 */
 
#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"      // Please enter your sensitive data in the Secret tab/arduino_secrets.h
 
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;      // the WiFi radio's status
char webserver[] = WEBSERVER;

WiFiClient client;

void setup() {
  // initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. needed for native USB port only
  }

  initializeWifi();
  printCurrentNet();
  printWifiData();
}

void loop() {
  //httpCallout();

  monitorGyroscope();
}

/** ==================================== **/

// Gyroscope Methods
void monitorGyroscope() {
  while(true) {
    delay(10000);
    Serial.println("I'm totally checking the gyroscope bro");
  }
}

// WiFi Methods
void initializeWifi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("You're connected to the network");
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  //Serial.print("SSID: ");
  //Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

// HTTP Methods
void httpCallout() {
  /**
 * - call out to evergreentr every 1 second
 * - gather data during the remainder of that second
 * - publish high/low/med (and raw data, if possible)
 */
 
  Serial.println("\nStarting connection to web server...");
  // if you get a connection, report back via serial:
  if (client.connect(webserver, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /poop.txt HTTP/1.1");
    client.println("Host: irk.evergreentr.com");
    client.println("Connection: close");
    client.println();
  }
  
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}
