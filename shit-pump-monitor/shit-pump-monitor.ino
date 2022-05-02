/*
 * This script should:
 * 1. Connect to a wireless network
 * 2. Begin and continuously monitor the LSM6DS3 sensor's gyroscopic data
 * 3. When a threshold has been reached, make an HTTP call to a web endpoint
 * 4. Healthcheck: Periodically call out to webserver, regardless of gyro data
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

unsigned long oneMinute = 60000; // 60k millisecondes
unsigned long startTimeMark;

WiFiSSLClient client;

void setup() {
  // initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial && millis() < oneMinute) {
    ; // wait for serial port to connect. needed for native USB port only
  }
  
  initializeWifi();
  initializeGyro();

  httpCallout(0,0,0,true);
  startTimeMark = millis();
}

void loop() {
  monitorGyroscope();
}

/** ==================================== **/

// Gyroscope Methods
void initializeGyro() {
  if (!IMU.begin()) {
    while (1);
  }
}

void monitorGyroscope() {
  float x, y, z;
  float threshold = 5.0;
  float neg_threshold = -5.0;

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);

    if (x > threshold || y > threshold || z > threshold || x < neg_threshold || y < neg_threshold || z < neg_threshold) {
      httpCallout(x,y,z,false);
    }

    if (millis() - startTimeMark > oneMinute) {
      startTimeMark = millis();
      httpCallout(x,y,z,true);
    }
  }
}

// WiFi Methods
void initializeWifi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
}

// HTTP Methods
void httpCallout(float xvalue, float yvalue, float zvalue, bool isHealthCheck) {
  if (client.connect(webserver, 443)) {
    // Make a HTTP request:
    client.print("GET ");
    client.print(endpoint);
    client.print("?x=");
    client.print(xvalue);
    client.print("&y=");
    client.print(yvalue);
    client.print("&z=");
    client.print(zvalue);
    if (isHealthCheck) {
      client.print("&healthcheck=1");
    } else {
      client.print("&shitstorm=1");
    }
    client.println(" HTTP/1.1");
    client.println("User-Agent: Arduino Shit Pump");
    client.println("Host: irk.evergreentr.com");
    client.println("Connection: close");
    client.println();

    if (!isHealthCheck) {
      delay(5000);
    }
  }

  // Is this necessary?
  if (!client.connected()) {
    client.stop();
  }
}
