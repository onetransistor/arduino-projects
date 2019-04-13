/*  HTTPS Web server on ESP8266
    https://www.onetransistor.eu/2019/04/https-server-on-esp8266-nodemcu.html
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>

const char *ssid = "ssid";
const char *password = "password";
const char *dname = "esp8266";

BearSSL::ESP8266WebServerSecure server(443);
ESP8266WebServer serverHTTP(80);

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIC6jCCAlOgAwIBAgIUZIw0cBcWDPJZe8ZIDu6bDqdwwvwwDQYJKoZIhvcNAQEL
BQAwejELMAkGA1UEBhMCUk8xCjAIBgNVBAgMAUIxEjAQBgNVBAcMCUJ1Y2hhcmVz
dDEbMBkGA1UECgwST25lVHJhbnNpc3RvciBbUk9dMRYwFAYDVQQLDA1PbmVUcmFu
c2lzdG9yMRYwFAYDVQQDDA1lc3A4MjY2LmxvY2FsMB4XDTE5MDQxMzE1NTMzOFoX
DTIwMDQxMjE1NTMzOFowejELMAkGA1UEBhMCUk8xCjAIBgNVBAgMAUIxEjAQBgNV
BAcMCUJ1Y2hhcmVzdDEbMBkGA1UECgwST25lVHJhbnNpc3RvciBbUk9dMRYwFAYD
VQQLDA1PbmVUcmFuc2lzdG9yMRYwFAYDVQQDDA1lc3A4MjY2LmxvY2FsMIGfMA0G
CSqGSIb3DQEBAQUAA4GNADCBiQKBgQCiZmrefwe6AwQc5BO+T/18IVyJJ007EASn
HocT7ODkL2HNgIKuQCnPimiysLh29tL1rRoE4v7qtpV4069BrMo2XqFvZkfbZo/c
qMcLJi43jSvWVUaWvk8ELlXNR/PX4627MilhC4bLD57VB7Q2AF4jrAVhBLzClqg0
RyCS1yab+wIDAQABo20wazAdBgNVHQ4EFgQUYvIljCgcnOfeRn1CILrj38c7Ke4w
HwYDVR0jBBgwFoAUYvIljCgcnOfeRn1CILrj38c7Ke4wDwYDVR0TAQH/BAUwAwEB
/zAYBgNVHREEETAPgg1lc3A4MjY2LmxvY2FsMA0GCSqGSIb3DQEBCwUAA4GBAI+L
mejdOgSCmsmhT0SQv5bt4Cw3PFdBj3EMFltoDsMkrJ/ot0PumdPj8Mukf0ShuBlL
alf/hel7pkwMbXJrQyt3+EN/u4SjjZZJT21Zbxbmo1BB/vy1fkugfY4F3JavVAQ/
F49UaclGs77AVkDYwKlRh5VWhmnfuXPN6NXkfV+z
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAKJmat5/B7oDBBzk
E75P/XwhXIknTTsQBKcehxPs4OQvYc2Agq5AKc+KaLKwuHb20vWtGgTi/uq2lXjT
r0GsyjZeoW9mR9tmj9yoxwsmLjeNK9ZVRpa+TwQuVc1H89fjrbsyKWELhssPntUH
tDYAXiOsBWEEvMKWqDRHIJLXJpv7AgMBAAECgYA5Syqu3mAKdt/vlWOFw9CpB1gP
JydvC+KoVvPOysY4mqLFjm4MLaTSjIENcZ1SkxewBubkDHVktw+atgvhfqVD4xnC
ewMpuN6Rku5A6EELhUoDrgMEt6M9D/0/iPaMm3VDtLXJq5SuKTpnM+vyE4/uM2Gu
4COfL4GQ0A5KWTzGcQJBANfpU/kwdZf8/oaOvpNZPGRsryjIXXuWMzKKM+M1RqSA
UQV596MGXjo8k8YG/A99rTmVhbeTMC2/7gIyGTePe/kCQQDAjZg2Ujz7wY3gf1Fi
ZETL7DHsss74sZyWZI490yIX0TQqKpXqEIKlkV+UZTOoSZiAaUyPjokblPmTkKfu
uMyTAkBIBjfS+o1fxC+L53Y/ZRc2UOMlcaFtpq8xftTMSGtmWL+uWf93zJoGR0rs
VkwjRsNQYEaY9Gqv+ESHSvsKg7zRAkEAoOLuhpzqVZThHe5jqumKzjS5dkPlScjl
xIeaji/msa3cf0r73goTj5HLIev5YKi1or3Y+a4oA4LTkifxGTcRvwJBAJB+qUE6
y8y+4yxStsWu362tn2o4EjyPL2UGc40wtlQng2GzPZ20+xVYcLxsJXE5/Jqg8IeI
elVVC46RfjDK9G0=
-----END PRIVATE KEY-----
)EOF";

bool connectToWifi() {
  byte timeout = 50;

  Serial.println("\n\n");

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  for (int i = 0; i < timeout; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi");
      Serial.print("Server can be accessed at https://");
      Serial.print(WiFi.localIP());
      if (MDNS.begin(dname)) {
        // https://superuser.com/questions/491747/how-can-i-resolve-local-addresses-in-windows
        Serial.print(" or at https://");
        Serial.print(dname);
        Serial.println(".local");
      }
      return true;
    }
    delay(5000);
    Serial.print(".");
  }

  Serial.println("\nFailed to connect to WiFi");
  Serial.println("Check network status and access data");
  Serial.println("Push RST to try again");
  return false;
}

void showWebpage() {
  int ledStatus = digitalRead(D0);

  if (server.hasArg("led")) {
    String status = server.arg("led");
    if (status == "on") {
      digitalWrite(D0, HIGH);
      ledStatus = 1;
    }
    else if (status == "off") {
      digitalWrite(D0, LOW);
      ledStatus = 0;
    }
  }

  String content = "<!DOCTYPE html><html>";
  content += "<head><title>ESP8266 Test Server</title>";
  content += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  content += "<link rel=\"shortcut icon\" href=\"data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAA/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARAiIgEQAiAQASAAEAEgAhABAgAQASACARAAIBEQIiAQASACAQACABABIAIAEAAgARACIAABAAIAAAAAAAAAAAAAAAAAAAAAEREAERAAEAAQAAAAAQAQABAAAAABABAAERAAARAAERAQAAAQAAAQARAAABAAABABEREAAREAERCQmQAAZ2YAAGtmAACdEQAAZrsAAGbdAACZ7gAA//8AAP//AAAMdwAAf7cAAH+3AAAecQAAffYAAH32AAAOMQAA\" />";
  content += "<style>body {font-family: sans-serif;} </style>";
  content += "</head><body>";
  content += "<h1>ESP8266 HTTPS Server</h1><form action=\"/\" method=\"get\"><p><b>D0</b> LED Status:";

  if (ledStatus) content += " <i>Turned on</i>";
  else content += " <i>Turned off</i>";

  content += "</p>";

  if (ledStatus) content += "<input type=\"submit\" value=\"off\" name=\"led\">";
  else content += "<input type=\"submit\" value=\"on\" name=\"led\">";

  content += "</form>";
  
  content += "<p>";
  time_t now = time(NULL);
  content += ctime(&now);
  content += "</p>";
  
  content += "<p>For more information see <a href=\"https://www.onetransistor.eu/2019/04/https-server-on-esp8266-nodemcu.html\" target=\"_blank\">OneTransistor</a>.</p></body></html>";

  server.send(200, "text/html", content);
}

void secureRedirect() {
  serverHTTP.sendHeader("Location", String("https://esp8266.local"), true);
  serverHTTP.send(301, "text/plain", "");
}

void setup() {
  pinMode(D0, OUTPUT);
  Serial.begin(115200);

  if (!connectToWifi()) {
    delay(60000);
    ESP.restart();
  }

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  serverHTTP.on("/", secureRedirect);
  serverHTTP.begin();

  server.setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
  server.on("/", showWebpage);
  server.begin();
  
  Serial.println("Server is ready");
}

void loop() {
  serverHTTP.handleClient();
  server.handleClient();
  MDNS.update();
}
