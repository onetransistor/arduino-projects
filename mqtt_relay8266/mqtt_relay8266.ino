/*
   MQTT Relay with Timer
   https://www.onetransistor.eu

*/

#define MCU_ESP8266 /* MCU_ESP8266 for ESP8266 boards or MCU_ESP32 for ESP32 */
#define DEV_NAME "Pump"
#define DEV_TYPE "Switch"

#ifdef MCU_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#else
#include <WiFi.h>
#include <ESPmDNS.h>
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <TM1637Display.h>

#define PIN_RLY D5 /* Relay control pin (output) */
#define PIN_BTN D6 /* Button pin (input) */
#define PIN_CLK D2 /* TM1637 clock pin (output) */
#define PIN_DIO D1 /* TM1637 data pin (output) */
#define PIN_ERR D7 /* Error LED pin (output) */

/* WiFi settings */
const char *ssid = "...";
const char *password = "...";
const char *mdns_name = DEV_TYPE "-" DEV_NAME;
// #define USE_DHCP /* when enabled the following IP config is ignored */
const IPAddress ip(192, 168, 1, 5);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);

/* MQTT Server settings */
const IPAddress mqttIP(192, 168, 1, 4);
const unsigned int mqttPort = 1883;
const char *mqtt_user = "...";
const char *mqtt_pass = "...";

/* OTA Firmware update */
const char* otaPassword = "admin";

/* Publishes to: */
const char *mqttStatusCheck = DEV_TYPE "/" DEV_NAME "/Status/Check";
const char *mqttStatusText = DEV_TYPE "/" DEV_NAME "/Status/Text";
const char *mqttTimerRemain = DEV_TYPE "/" DEV_NAME "/Timer/Remaining";

/* Subscribes to: */
const char *mqttStatusRequest = DEV_TYPE "/" DEV_NAME "/Status/Request";
const char *mqttTimerSet = DEV_TYPE "/" DEV_NAME "/Timer/Set";

/* Strings */
#define WILL_MSG "Not connected."
#define OFF_MSG "The pump is off."
#define OFF_TIME_MSG "The pump is off. \nPrevious on time is %d minutes."
#define ON_MSG "The pump is on since %d minutes."
#define ON_TIMER_MSG "The pump is on for %d minutes out of %d minutes in total."
#define FWUPD_MSG "Disconnected for firmware update."

WiFiClient espClient;
PubSubClient mqttClient(espClient);

TM1637Display display(PIN_CLK, PIN_DIO);
uint8_t dispData[] = {0xff, 0xff, 0xff, 0xff};

/* time intervals for periodic routines */
long timeNow = 0;
long timeConnAndServer = 0;
long timeErrorLed = 0;
long timePushButton = 0;
long timeStatusChange = -30000;
long timeKeeper = -60000;

/* error status indicator */
bool errorIndicator = false;

/* switch statuses */
bool switchOn = false, mqttOn = false, relayOn = false;

/* previous on time */
uint16_t totalMinutes = 0, lastOnMinutes = 0, limitMinutes = 0;

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, mqttStatusRequest) == 0) {
    if (payload[0] == '1') mqttOn = true;
    else mqttOn = false;
  }

  if (strcmp(topic, mqttTimerSet) == 0) {
    int setVal = atoi((const char*)payload);
    limitMinutes = setVal;
    if (mqttOn == true) limitMinutes += lastOnMinutes; // add to existing time
    else if (limitMinutes > 0) mqttOn = true; // trigger start

    updateServer();
  }
}

void setupWifiAndMqtt() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifndef USE_DHCP
  WiFi.config(ip, gateway, subnet);
#endif

  if (MDNS.begin(mdns_name))
    MDNS.addService("http", "tcp", 80); // in a future version, add http server

  ArduinoOTA.setPassword(otaPassword);
  ArduinoOTA.setHostname(mdns_name);
  ArduinoOTA.onStart([]() {
    if (mqttClient.connected()) {
      mqttClient.publish(mqttStatusText, (const unsigned char *)FWUPD_MSG, strlen(FWUPD_MSG), true);
      mqttClient.disconnect();
    }
  });
  ArduinoOTA.begin();

  mqttClient.setServer(mqttIP, mqttPort);
  mqttClient.setCallback(mqttCallback);
}

void checkConnAndServer(long tn) {
  if (tn <= timeConnAndServer + 500) return;
  timeConnAndServer = tn;

  /* check wifi, set error and return when not connected */
  if (WiFi.status() != WL_CONNECTED) {
    errorIndicator = true;
    return;
  }

  /* check mqtt, clear error and return when connected */
  if (mqttClient.connected()) {
    errorIndicator = false;
    return;
  }

  /* mqtt is not connected; attempt connection */
  errorIndicator = true;
  timeConnAndServer = tn + 5000; // delay next connection check by 5 s

  if (mqttClient.connect(mdns_name, mqtt_user, mqtt_pass, mqttStatusText, 1, true, WILL_MSG)) {
    mqttClient.subscribe(mqttStatusRequest);
    mqttClient.subscribe(mqttTimerSet);

    updateServer();

    errorIndicator = false;
  }
  else {
    errorIndicator = true;
  }
}

void blinkAtError(long tn) {
  if (!errorIndicator) {
    if (digitalRead(PIN_ERR) == LOW)
      digitalWrite(PIN_ERR, HIGH); // make sure LED stays off
    return; // no error
  }
  if (tn <= timeErrorLed + 500) return;
  timeErrorLed = tn;

  /* Toggle LED */
  digitalWrite(PIN_ERR, !digitalRead(PIN_ERR));
}

void displayShowOFF() {
  display.clear();
  display.showNumberHexEx(0x0ff, 0, true, 3);
}

void displayShowMinutes(uint16_t totalMinutes) {
  uint8_t h, m;
  m = totalMinutes % 60;
  h = totalMinutes / 60;

  dispData[0] = display.encodeDigit(h / 10);
  dispData[1] = display.encodeDigit(h % 10);
  dispData[2] = display.encodeDigit(m / 10);
  dispData[3] = display.encodeDigit(m % 10);
  dispData[1] |= 0x80;
  dispData[2] |= 0x80;

  display.setSegments(dispData);
}

void handlePushButton(long tn) {
  if (digitalRead(PIN_BTN) == HIGH) return;
  if (tn <= timePushButton + 500) return;
  timePushButton = tn;

  switchOn = !switchOn;
}

void updateServer() {
  if (!mqttClient.connected()) return;

  char str[255], str2[255]; // string buffers

  // relay status
  itoa(relayOn, str, 2);
  mqttClient.publish(mqttStatusCheck, (const unsigned char *)str, strlen(str), true);

  // OFF text messages
  if (!relayOn) {
    if (lastOnMinutes <= 0)
      strcpy(str, OFF_MSG);
    else {
      sprintf(str, OFF_TIME_MSG, lastOnMinutes);
    }

    itoa(0, str2, 10); // no remaining time
  }

  // ON text messages
  else {
    if (limitMinutes <= lastOnMinutes) {
      sprintf(str, ON_MSG, lastOnMinutes);
      itoa(0, str2, 10);
    }
    else {
      sprintf(str, ON_TIMER_MSG, lastOnMinutes, limitMinutes);
      itoa(limitMinutes - lastOnMinutes, str2, 10);
    }
  }

  mqttClient.publish(mqttStatusText, (const unsigned char *)str, strlen(str), true);
  mqttClient.publish(mqttTimerRemain, (const unsigned char *)str2, strlen(str2), true);
}

void updateDisplay(long tn) {
  if (tn <= timeKeeper + 60000) return;
  timeKeeper = tn;

  if (!relayOn) {
    displayShowOFF();
    return;
  }

  if ((limitMinutes > 0) && (totalMinutes == limitMinutes)) {
    mqttOn = 0; // trigger stop after some minutes
  }

  displayShowMinutes(totalMinutes);
  lastOnMinutes = totalMinutes;
  updateServer();
  totalMinutes++;
}

void setRelay(bool status) {
  digitalWrite(PIN_RLY, status);

  if (status) totalMinutes = 0; // clear previous time when turning on
  if (!status) limitMinutes = 0; // clear time limit when turning off
  timeKeeper = -60000;
}

void handleStatus(long tn) {
  // switch
  if (switchOn != relayOn) {
    if (tn > timeStatusChange + 10000) {
      timeStatusChange = tn;

      relayOn = switchOn;
      mqttOn = switchOn;

      limitMinutes = 0; // no time limit on button

      setRelay(relayOn);

      updateServer();
    }
    else {
      switchOn = relayOn; // set switchOn to actual relay state
    }
  }

  // mqtt
  if (mqttOn != relayOn) {
    if (tn > timeStatusChange + 10000) {
      timeStatusChange = tn;

      relayOn = mqttOn;
      switchOn = mqttOn;

      setRelay(relayOn);
    }
    else {
      mqttOn = relayOn;
    }
    updateServer();
  }
}

void setup() {
  /* Pins setup */
  pinMode(PIN_RLY, OUTPUT);
  digitalWrite(PIN_RLY, LOW);

  pinMode(PIN_ERR, OUTPUT);
  digitalWrite(PIN_ERR, LOW);

  pinMode(PIN_BTN, INPUT_PULLUP);

  /* clear display */
  display.setBrightness(0x0f);
  displayShowOFF();

  /* setup connection */
  setupWifiAndMqtt();
}

void loop() {
  timeNow = millis();
  checkConnAndServer(timeNow);
  blinkAtError(timeNow);
  handlePushButton(timeNow);
  handleStatus(timeNow);
  updateDisplay(timeNow);

  ArduinoOTA.handle();
#ifdef MCU_ESP8266
  MDNS.update();
#endif
  mqttClient.loop();
}
