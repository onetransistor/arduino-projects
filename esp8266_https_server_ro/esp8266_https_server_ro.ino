/*  Server web pe ESP8266
    https://ro.onetransistor.eu/2019/04/server-web-securizat-pe-nodemcu-esp8266.html
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>

const char *ssid = "retea";
const char *password = "parola";
const char *dname = "esp8266";

BearSSL::ESP8266WebServerSecure server(443);
ESP8266WebServer serverHTTP(80);

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIC6jCCAlOgAwIBAgIUEjzbG4k1ObFehlTkgPWEqZr8CuAwDQYJKoZIhvcNAQEL
BQAwejELMAkGA1UEBhMCUk8xCjAIBgNVBAgMAUIxEjAQBgNVBAcMCUJ1Y2hhcmVz
dDEbMBkGA1UECgwST25lVHJhbnNpc3RvciBbUk9dMRYwFAYDVQQLDA1PbmVUcmFu
c2lzdG9yMRYwFAYDVQQDDA1lc3A4MjY2LmxvY2FsMB4XDTE5MDQxMzEyMTAyNloX
DTI5MDQxMDEyMTAyNlowejELMAkGA1UEBhMCUk8xCjAIBgNVBAgMAUIxEjAQBgNV
BAcMCUJ1Y2hhcmVzdDEbMBkGA1UECgwST25lVHJhbnNpc3RvciBbUk9dMRYwFAYD
VQQLDA1PbmVUcmFuc2lzdG9yMRYwFAYDVQQDDA1lc3A4MjY2LmxvY2FsMIGfMA0G
CSqGSIb3DQEBAQUAA4GNADCBiQKBgQDR6hyJ058zjqIZe9yO7DYBs4EH2TEJQxoT
xsNM/i6PbOgL1lLJoqDhUBktxiQcWKKv3z8uYTqIz6PXEzCBrOt0FLJlrcy0Eyiz
B/r+C4MYX1VkadWP7e1TLJcYcOCXwy3vy/jxzLc5or/S4n+3dtoPHDBYu4+EpkKx
6XtnVEAGwQIDAQABo20wazAdBgNVHQ4EFgQUpGGd2qsOiaMao8hF0QTatNV3E0Aw
HwYDVR0jBBgwFoAUpGGd2qsOiaMao8hF0QTatNV3E0AwDwYDVR0TAQH/BAUwAwEB
/zAYBgNVHREEETAPgg1lc3A4MjY2LmxvY2FsMA0GCSqGSIb3DQEBCwUAA4GBAKKB
LxzSEIF+1a1IlarRyM+IE5dcfmlXUDeCRIIRVWZio3P6Hm16QB0iPfRM+VUJTHez
vUdIBpsaSs9T8r9TsZ0bxd37QHpOQtfzoX59pdYBK9eAy5Jbn0HfrE7nZVtexN4c
owPfrzl0N45zvb8ke92f/Ixjr+RouHG52O9hFDyb
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBANHqHInTnzOOohl7
3I7sNgGzgQfZMQlDGhPGw0z+Lo9s6AvWUsmioOFQGS3GJBxYoq/fPy5hOojPo9cT
MIGs63QUsmWtzLQTKLMH+v4LgxhfVWRp1Y/t7VMslxhw4JfDLe/L+PHMtzmiv9Li
f7d22g8cMFi7j4SmQrHpe2dUQAbBAgMBAAECgYAs4wDjQAtk/PUQufm53izWREot
IXZo4p5q6C/PAPN32/PqxBPD5jC8vWIE9b+3CAUR0o0yH7xikPQaA5J1yEI4lt6t
1jLsy3YAqOhdH1jM2MSIQe3fmZsp4SyzidVpcrDkMnK3e1kVZicSfv9QjFS6RfPt
f0IigIOUrt/lMSaCoQJBAPooLv/e/jupifb7V0GJ+A+QLbAd7puQKrFfijhJfTXO
/TBnYXJ6DRQcSC9L35YydV6gYztF/cX/BFp1/dsCod0CQQDW0UwuszyAmihRruiq
qgZOgMPjSqe5j59JX3BKCVyISU2EwQ9+JHZ7MZPedpnPrd+eimETIK9q/GBjf11A
eFQ1AkEAwsaO8cNbCHFVbuz8X5dhghystjhYFOAHndvZ70GpMEBee1XDVjMaA9KR
keHt0TCwmmEfYoN4uLV7WkQMyH4gMQJAKI5pOKPkN09jb1B7YsUo3adX1FCi69ie
tQaMt52e16gnN3oPh7wwlj+c8DIqBdiI0HDFtQvFsoglVoOUQni1RQJBAMDbBbiS
fB8Sk2zYVXF8qqowhkuYzu1lqHUvOL5MDgUVfsTBUQpspSUKQMND3qOImvNNiTtX
L62hExfVNLPIuds=
-----END PRIVATE KEY-----
)EOF";

bool connectToWifi() {
  byte timeout = 50;

  Serial.println("\n\n");

  Serial.print("Se incearca conectarea la ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  for (int i = 0; i < timeout; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectat la retea");
      Serial.print("Acceseaza serverul la https://");
      Serial.print(WiFi.localIP());
      if (MDNS.begin(dname)) {
        // https://superuser.com/questions/491747/how-can-i-resolve-local-addresses-in-windows
        Serial.print(" sau la https://");
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
  content += "<style>body {font-family: sans-serif;} </style>";
  content += "</head><body>";
  content += "<h1>Server ESP8266</h1><form action=\"/\" method=\"get\"><p>Stare LED conectat la pinul <b>D0</b>:";

  if (ledStatus) content += " <i>Pornit</i>";
  else content += " <i>Oprit</i>";

  content += "</p>";

  if (ledStatus) content += "<input type=\"submit\" value=\"off\" name=\"led\">";
  else content += "<input type=\"submit\" value=\"on\" name=\"led\">";

  content += "</form>";
  
  content += "<p>";
  time_t now = time(NULL);
  content += ctime(&now);
  content += "</p>";
  
  content += "<p>Viziteaza <a href=\"https://ro.onetransistor.eu/2019/04/server-web-securizat-pe-nodemcu-esp8266.html\" target=\"_blank\">OneTransistor</a> pentru detalii.</p></body></html>";

  server.send(200, "text/html", content);
}

void redirectionareHTTPS() {
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

  configTime(3 * 3600, 0, "ro.pool.ntp.org", "time.nist.gov");

  serverHTTP.on("/", redirectionareHTTPS);
  serverHTTP.begin();

  server.setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));
  server.on("/", afisareSite);
  server.begin();
  
  Serial.println("Serverul a fost pornit");
}

void loop() {
  serverHTTP.handleClient();
  server.handleClient();
  MDNS.update();
}
