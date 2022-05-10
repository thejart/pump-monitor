# Pump Monitor

Once upon a time, there was a basement with below grade plumbing. Thanks to a the wonderful technology of ejector pumps, this posed no immediate problems for the basement in question. One day a terrible thought occurred to the resident of said basement, "How can I be certain that the pump is working? How can I guarantee advanced knowledge of a potential shit flood?" The resident thought about this for a day or so, and finally determined that this was a job well-suited for an accelerometer attached to an Arduino board. And thus the shit pump monitor was born.

## Logic Overview
1. Connect to a wireless network
2. Begin and continuously monitor the LSM6DS3 sensor's gyroscopic data
3. When a threshold has been reached, make an HTTP call to a web endpoint
4. Healthcheck: Periodically call out to webserver, regardless of gyro data

## Getting Started
1. Purchase a Nano 33 IoT arduino (Is this strictly necessary? No, but you'll probably need to customize this sketch to support a different board)
2. Create a file `arduino_secrets.h` in the same directory as the sketch file in the following format (See below)
3. Make some adjustments as needed. (eg. The default gyroscopic threshold is 5, which may or may not be valid for your plumbing situation)
4. Upload the sketch to your arduino board
5. Snugly attach the arduino board to the egress plumbing of your ejector pump
6. Power the board

---
### arduino_secrets.h
```
#define SECRET_SSID "Network SSID"
#define SECRET_PASS "Network password"
#define WEBSERVER "domain.name.of.webserver"
```
*Note:* The final endpoint will be https://&lt;WEBSERVER&gt;/poop.txt along with some query params that distinguish healthchecks and pump notifications.
