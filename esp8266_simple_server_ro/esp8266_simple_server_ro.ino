/*  Server web pe ESP8266
    https://ro.onetransistor.eu/2018/11/server-simplu-pe-nodemcu-esp8266.html
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

const char *ssid = "retea";
const char *password = "parola";
const char *dname = "esp8266";

ESP8266WebServer server(80);

bool connectToWifi() {
  byte timeout = 50;

  Serial.println("\n\n");

  Serial.print("Se incearca conectarea la ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  for (int i = 0; i < timeout; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectat la retea");
      Serial.print("Acceseaza serverul la http://");
      Serial.print(WiFi.localIP());
      if (MDNS.begin(dname)) {
        // https://superuser.com/questions/491747/how-can-i-resolve-local-addresses-in-windows
        Serial.print(" sau la http://");
        Serial.print(dname);
        Serial.println(".local");
      }
      return true;
    }
    delay(5000);
    Serial.print(".");
  }

  Serial.println("\nNu s-a reusit conectarea la WiFi");
  Serial.println("Verifica starea retelei si datele de acces");
  Serial.println("Apasa RST pentru a reincerca conectarea");
  return false;
}

void afisareSite() {
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
  content += "</head><body>";
  content += "<h1>Server ESP8266</h1><form action=\"/\" method=\"get\"><p>Stare LED conectat la pinul <b>D0</b>:";

  if (ledStatus) content += " <i>Pornit</i>";
  else content += " <i>Oprit</i>";

  content += "</p>";

  if (ledStatus) content += "<input type=\"submit\" value=\"off\" name=\"led\">";
  else content += "<input type=\"submit\" value=\"on\" name=\"led\">";

  content += "</form>";
  content += "<p>Viziteaza <a href=\"https://ro.onetransistor.eu/2018/11/server-simplu-pe-nodemcu-esp8266.html\" target=\"_blank\">OneTransistor</a> pentru detalii.</p></body></html>";

  server.send(200, "text/html", content);
}

void setup() {
  pinMode(D0, OUTPUT);
  Serial.begin(115200);

  if (!connectToWifi()) {
    delay(60000);
    ESP.restart();
  }

  server.on("/", afisareSite);
  server.begin();
  Serial.println("Serverul a fost pornit");
}

void loop() {
  server.handleClient();
}
