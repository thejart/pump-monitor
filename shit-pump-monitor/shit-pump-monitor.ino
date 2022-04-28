/*
 * This script should:
 * 1. Connect a wireless network
 * 2. Begin and continuously monitor the LSM6DS3 sensor for gyroscopic data
 * 3. When a threshold has been reached, make an HTTP call a webserver
 * 
 * This frankenstein of a script was created by thejart in April 2022 with some help by the following:
 * - dlf (Metodo2 srl), 13 July 2010
 * - Tom Igoe, 31 May 2012
 * - Riccardo Rizzo, 10 Jul 2019
 */
 
#include <SPI.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DS3.h>
#include "arduino_secrets.h"      // Please enter your sensitive data in the Secret tab/arduino_secrets.h
 
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;      // the WiFi radio's status
char webserver[] = WEBSERVER;
char endpoint[] = "/poop.txt";

//WiFiClient client;
WiFiSSLClient client;

void setup() {
  // initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. needed for native USB port only
  }
  
  Serial.println("================================");
  initializeWifi();
  printCurrentNet();
  printWifiData();
  
  initializeGyro();
  httpCallout(0,0,0);
}

void loop() {
  monitorGyroscope();
}

/** ==================================== **/

// Gyroscope Methods
void initializeGyro() {
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  Serial.println("Initialized Gyro");
}

void monitorGyroscope() {
  float x, y, z;
  float threshold = 5.0;
  float neg_threshold = -5.0;

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);
//    Serial.print(x);
//    Serial.print('\t');
//    Serial.print(y);
//    Serial.print('\t');
//    Serial.println(z);

    if (x > threshold || y > threshold || z > threshold || x < neg_threshold || y < neg_threshold || z < neg_threshold) {
      httpCallout(x,y,z);
    }
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
void httpCallout(float xvalue, float yvalue, float zvalue) {
  Serial.print("Notifying ");
  Serial.print(webserver);
  Serial.println(" of movement");

  if (client.connect(webserver, 443)) {
//    Serial.println("connected to server");
    // Make a HTTP request:
//    client.println("GET /poop.txt HTTP/1.0");
    client.print("GET ");
    client.print(endpoint);
    client.print("?x=");
    client.print(xvalue);
    client.print("&y=");
    client.print(yvalue);
    client.print("&z=");
    client.print(zvalue);
    client.println(" HTTP/1.1");
    client.println("User-Agent: Arduino Shit Pump");
    client.println("Host: irk.evergreentr.com");
    client.println("Connection: close");
    client.println();
  }
 
//  // if there are incoming bytes available
//  // from the server, read them and print them:
//  while (client.available()) {
//    char c = client.read();
//    Serial.write(c);
//  }
//
//  // if the server's disconnected, stop the client:
//  if (!client.connected()) {
//    Serial.println();
//    Serial.println("disconnecting from server.");
//    client.stop();
//  }

  delay(5000);
}
