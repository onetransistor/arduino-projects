#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <FS.h>

const char *ssid = "wifi";
const char *password = "pass";
const char *dname = "sensor";

const int dhtPin = D3;

bool actt = 0, acth = 0; // action: 0=disabled, 1=enabled
bool cont = 0, conh = 0; // condition: 0=smaller, 1=greater
int thrt = 20, thrh = 50; // thresholds
float currt, currh;


ESP8266WebServer server(80);
DHT dht(dhtPin, DHT22);

void temperatureAction(bool conditionMet) {
  if (conditionMet) Serial.println("Temperature threshold reached");
}

void humidityAction(bool conditionMet) {
  if (conditionMet) Serial.println("Humidity threshold reached");
}

bool connectToWifi() {
  byte timeout = 50;

  Serial.println("\n\n");

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  for (int i = 0; i < timeout; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi");
      Serial.print("Server can be accessed at http://");
      Serial.print(WiFi.localIP());
      if (MDNS.begin(dname)) {
        // https://superuser.com/questions/491747/how-can-i-resolve-local-addresses-in-windows
        Serial.print(" or at http://");
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

void mainPage() {
  String content;
  content += "<!DOCTYPE html><html lang=\"en\"><head><title>ESP8266 Sensor Server</title>";
  content += "<meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  content += "<link rel=\"shortcut icon\" href=\"data:image/x-icon;base64,AAABAAEAEBAQAAEABAAoAQAAFgAAACgAAAAQAAAAIAAAAAEABAAAAAAAgAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAA/4QAAAAA/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARAiIgEQAiAQASAAEAEgAhABAgAQASACARAAIBEQIiAQASACAQACABABIAIAEAAgARACIAABAAIAAAAAAAAAAAAAAAAAAAAAEREAERAAEAAQAAAAAQAQABAAAAABABAAERAAARAAERAQAAAQAAAQARAAABAAABABEREAAREAERCQmQAAZ2YAAGtmAACdEQAAZrsAAGbdAACZ7gAA//8AAP//AAAMdwAAf7cAAH+3AAAecQAAffYAAH32AAAOMQAA\" />";
  content += "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"><script src=\"script.js\"></script>";
  content += "</head><body><div class=\"main\"><h1>Sensor Server</h1><h2>Parameters</h2><div id=\"readings\">Loading...</div>";

  content += "<h2>Actions</h2><form action=\"/set\" method=\"get\">";
  content += "<p class=\"rd\"><label class=\"switch\"><input type=\"checkbox\" name=\"tc\" value=\"1\"";
  if (actt) content += " checked";
  content += "><span class=\"slider round\"></span></label>&nbsp;Temperature<br/>Condition: <select name=\"to\"><option value=\"0\"";
  if (cont == 0) content += " selected";
  content += ">smaller</option><option value=\"1\"";
  if (cont == 1) content += " selected";
  content += ">greater</option></select> than<br/>Threshold: <input type=\"number\" name=\"tt\" min=\"-25\" max=\"50\" value=\"";
  content += String(thrt, DEC);
  content += "\">&nbsp;&#176;C</p>";

  content += "<p class=\"rd\"><label class=\"switch\"><input type=\"checkbox\" name=\"hc\" value=\"1\"";
  if (acth) content += " checked";
  content += "><span class=\"slider round\"></span></label>&nbsp;Humidity<br/>Condition: <select name=\"ho\"><option value=\"0\"";
  if (conh == 0) content += " selected";
  content += ">smaller</option><option value=\"1\"";
  if (conh == 1) content += " selected";
  content += ">greater</option></select> than<br/>Threshold: <input type=\"number\" name=\"ht\" min=\"0\" max=\"100\" value=\"";
  content += String(thrh, DEC);
  content += "\">&nbsp%</p>";
  content += "<div class=\"btn\"><input type=\"submit\" value=\"Save\"></div></form>";

  content += "<hr/><p class=\"f\">Copyright &#169; <a href=\"https://www.onetransistor.eu\" target=\"_blank\">One Transistor</a> 2019.</p></div></body></html>";

  server.send(200, "text/html", content);
}

void setParams() {
  if (server.hasArg("tc")) {
    if (server.arg("tc") == "1") actt = 1;

    if (server.hasArg("to")) {
      if (server.arg("to") == "1") cont = 1;
      else cont = 0;
    }

    if (server.hasArg("tt")) {
      thrt = server.arg("tt").toInt();
    }
  }
  else actt = 0;

  if (server.hasArg("hc")) {
    if (server.arg("hc") == "1") acth = 1;

    if (server.hasArg("ho")) {
      if (server.arg("ho") == "1") conh = 1;
      else conh = 0;
    }

    if (server.hasArg("ht")) {
      thrh = server.arg("ht").toInt();
    }
  }
  else acth = 0;

  server.sendHeader("Location", String("/"), true);
  server.send(301, "text/plain", "");
}

void readSensor() {
  currt = dht.readTemperature();
  currh = dht.readHumidity();

  String response = "<p>Current temperature</p><p class=\"reading\">";
  response += String(currt, 1);
  response += "&nbsp;&#176;C</p><p>Relative humidity</p><p class=\"reading\">";
  response += String(currh, 0);
  response += "&nbsp;%</p>";

  //server.sendHeader("Access-Control-Allow-Origin", String("*"), true);
  server.send(200, "text/html", response);

  if (actt == 1) {
    if (cont == 0) {
      if (currt < thrt) temperatureAction(true);
      else temperatureAction(false);
    }
    else {
      if (currt > thrt) temperatureAction(true);
      else temperatureAction(false);
    }
  }
  else temperatureAction(false);

  if (acth == 1) {
    if (conh == 0) {
      if (currh < thrh) humidityAction(true);
      else humidityAction(false);
    }
    else {
      if (currh > thrh) humidityAction(true);
      else humidityAction(false);
    }
  }
  else humidityAction(false);
}

String getContentType(String filename) {
  if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  return "text/plain";
}

bool handleFileRead(String path) {
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setup() {
  pinMode(dhtPin, INPUT);
  Serial.begin(115200);

  dht.begin();

  SPIFFS.begin();

  if (!connectToWifi()) {
    delay(60000);
    ESP.restart();
  }

  server.on("/", mainPage);
  server.on("/read", readSensor);
  server.on("/set", setParams);
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: Not Found");
  });
  server.begin();
}

void loop() {
  server.handleClient();
  MDNS.update();
}
