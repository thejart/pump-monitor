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

//#define DEBUG
#ifdef DEBUG
#define DPRINT(...)    Serial.print(__VA_ARGS__)
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)     //blank line
#define DPRINTLN(...)   //blank line
#endif


char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;      // the WiFi radio's status
char webserver[] = WEBSERVER;
char endpoint[] = "/shitty.php";

bool gyroDebug = false;
bool httpDebug = false;

unsigned long oneMinute = 60000; // 60k milliseconds
// The Nano 33IoT's clock frequency can change depending on power source due to it not having a crystal-based clock.
// In my experience this results in a clock that runs ~2% slower than advertised
// https://forum.arduino.cc/t/nano-33-iot-millis-rate-varies-with-usb-power-source/939392/2
unsigned long twelveHours = 43200000 - 885000; // 43,200 seconds * 1000 milliseconds (minus ~2% per the above comment)
unsigned long startTimeMark;

WiFiSSLClient client;

void setup() {
  // initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial && millis() < oneMinute) {
    ; // wait for serial port to connect. needed for native USB port only
  }
  
  DPRINTLN("================================");
  initializeWifi();
  printCurrentNet();
  printWifiData();
  
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
    DPRINTLN("Failed to initialize IMU!");
    while (1);
  }

  DPRINTLN("Initialized Gyro");

  if (gyroDebug) {
    DPRINT("Gyroscope sample rate = ");
    DPRINT(IMU.gyroscopeSampleRate());
    DPRINTLN(" Hz");
    DPRINTLN();
    DPRINTLN("Gyroscope in degrees/second");
    DPRINTLN("X\tY\tZ");
  }
}

void monitorGyroscope() {
  float x, y, z;
  float threshold = 5.0;
  float neg_threshold = -5.0;

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);
    if (gyroDebug) {
      DPRINT(x);
      DPRINT('\t');
      DPRINT(y);
      DPRINT('\t');
      DPRINTLN(z);
    }

    if (x > threshold || y > threshold || z > threshold || x < neg_threshold || y < neg_threshold || z < neg_threshold) {
      httpCallout(x,y,z,false);
    }

    if (millis() - startTimeMark > twelveHours) {
      startTimeMark = millis();
      httpCallout(x,y,z,true);
    }
  }
}

// WiFi Methods
void initializeWifi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    DPRINTLN("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    DPRINTLN("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    DPRINT("Attempting to connect to WPA SSID: ");
    DPRINTLN(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  DPRINTLN("You're connected to the network");
}

void printCurrentNet() {
  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  DPRINT("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  DPRINT("signal strength (RSSI):");
  DPRINTLN(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  DPRINT("Encryption Type:");
  DPRINTLN(encryption, HEX);
  DPRINTLN();
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  DPRINT("IP Address: ");
  DPRINTLN(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  DPRINT("MAC address: ");
  printMacAddress(mac);
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      DPRINT("0");
    }
    DPRINT(mac[i], HEX);
    if (i > 0) {
      DPRINT(":");
    }
  }
  DPRINTLN();
}

// HTTP Methods
void httpCallout(float xvalue, float yvalue, float zvalue, bool isHealthCheck) {
  if (!isHealthCheck) {
    DPRINT("Notifying ");
    DPRINT(webserver);
    DPRINTLN(" of movement");
  }

  if (client.connect(webserver, 443)) {
    if (httpDebug) {
      DPRINTLN("connected to server");
    }
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

  client.stop();
}
