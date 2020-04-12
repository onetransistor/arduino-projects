/****************************************************************
 * 
 * ESP8266 WiFi Analyzer 1.0
 * Modes: AP list
 *        Single AP scan
 *        
 * https://www.onetransistor.eu/2020/04/wifi-analyzer-esp8266-ili9341-lcd.html
 ****************************************************************/

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <FS.h>
#include <TFT_eSPI.h>
#include <user_interface.h>

#define TEXT_FONT 1
#define MAX_AP_COUNT 64

TFT_eSPI tft = TFT_eSPI();

bool singleNetworkDisplay = false;

struct idx_rssi_t {
  int idx;
  int32_t rssi;
} idx_rssi[MAX_AP_COUNT];

int nNet = 0; // number of APs found
int selectedAP = 0; // selected AP in list screen

uint8_t selectedBSSID[6];
uint8_t selectedChannel;
bool scanKnownChannelOnly = true, customScanDone = true;

long scanTime = -20000, current, buttonTime, scanInterval = 5000;

int yoff = 16;

static const uint8_t lock_bitmap[] PROGMEM  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x04, 0x20, 0x0f, 0xf0, 0x1f, 0xf8, 0x1e, 0x78, 0x1e, 0x78, 0x1f, 0xf8, 0x0f, 0xf0, 0x00, 0x00 };
static const uint8_t unlock_bitmap[] PROGMEM  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x48, 0x00, 0x84, 0x00, 0x84, 0x00, 0x84, 0x00, 0x04, 0x00, 0x0f, 0xf0, 0x1f, 0xf8, 0x1e, 0x78, 0x1e, 0x78, 0x1f, 0xf8, 0x0f, 0xf0, 0x00, 0x00 };
static const uint8_t reload_bitmap[] PROGMEM  = { 0x00, 0x00, 0x00, 0x10, 0x00, 0x18, 0x1f, 0xfc, 0x3f, 0xfe, 0x7f, 0xfc, 0x60, 0x18, 0x00, 0x10, 0x08, 0x00, 0x18, 0x06, 0x3f, 0xfe, 0x7f, 0xfc, 0x3f, 0xf8, 0x18, 0x00, 0x08, 0x00, 0x00, 0x00 };


/******************************************************************
    ACCESS POINT LIST FUNCTIONS
 ******************************************************************/

/***** Signal indicator on network list screen *****/
void signalIndicator(int32_t s, int xpos, int ypos, uint16_t color) {
  tft.fillRect(xpos, ypos + 12, 2, 3, color);

  if (s < -88) tft.fillRect(xpos + 3, ypos + 9, 2, 6, TFT_DARKGREY);
  else tft.fillRect(xpos + 3, ypos + 9, 2, 6, color);

  if (s < -78) tft.fillRect(xpos + 6, ypos + 6, 2, 9, TFT_DARKGREY);
  else tft.fillRect(xpos + 6, ypos + 6, 2, 9, color);

  if (s <= -66) tft.fillRect(xpos + 9, ypos + 3, 2, 12, TFT_DARKGREY);
  else tft.fillRect(xpos + 9, ypos + 3, 2, 12, color);

  if (s < -55) tft.fillRect(xpos + 12, ypos, 2, 15, TFT_DARKGREY);
  else tft.fillRect(xpos + 12, ypos, 2, 15, color);
}

/**** Network display as list item *****/
void displayNetwork(int i, int pos) {
  int ypos = 20 + (pos * 32);
  const int xoff = 4;
  uint16_t color;

  String buf;

  // ROW 1
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  buf = WiFi.SSID(i) + " (" + WiFi.BSSIDstr(i) + ")";
  tft.drawString(buf.c_str(), xoff, ypos, TEXT_FONT);

  // ROW 2
  int32_t rssi = WiFi.RSSI(i);
  if (rssi < -88) color = TFT_RED;
  else if (rssi > -66) color = TFT_GREEN;
  else color = TFT_YELLOW;

  signalIndicator(rssi, xoff, ypos + 12, color);
  tft.setTextColor(color, TFT_BLACK);
  buf = String(rssi) + "dBm";
  tft.drawString(buf.c_str(), xoff + 16, ypos + 16, TEXT_FONT);

  // ROW 2, channel
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("CH", xoff + 64, ypos + 16, TEXT_FONT);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  buf = String(WiFi.channel(i));
  tft.drawString(buf, xoff + 80, ypos + 16, TEXT_FONT);

  // ROW 2, encryption
  uint8_t enc = WiFi.encryptionType(i);
  switch (enc) {
    case 2: buf = "WPA-PSK"; break;
    case 5: buf = "WEP"; break;
    case 4: buf = "WPA2-PSK"; break;
    case 8: buf = "WPA/WPA2-PSK"; break;
    case 7:
    default: buf = "OPEN"; break;
  }

  if (enc == 7) tft.drawBitmap(xoff + 96, ypos + 10, unlock_bitmap, 16, 16, TFT_OLIVE);
  else tft.drawBitmap(xoff + 96, ypos + 10, lock_bitmap, 16, 16, TFT_MAROON);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(buf, xoff + 112, ypos + 16, TEXT_FONT);
}

void sortNetworksByRSSI() {
  for (int i = 0; i < nNet; i++) {
    idx_rssi[i].idx = i;
    idx_rssi[i].rssi = WiFi.RSSI(i);
  }

  for (int i = 0; i < nNet; i++) {
    for (int j = i + 1; j < nNet; j++) {
      if (idx_rssi[i].rssi < idx_rssi[j].rssi) {
        // switch indexes
        int ti = idx_rssi[i].idx;
        idx_rssi[i].idx = idx_rssi[j].idx;
        idx_rssi[j].idx = ti;

        // switch rssi values
        int32_t tr = idx_rssi[i].rssi;
        idx_rssi[i].rssi = idx_rssi[j].rssi;
        idx_rssi[j].rssi = tr;
      }
    }
  }
}

void displaySelectionMarker(int pos) {
  pos = pos % 9;
  int prevPos = pos - 1;
  if (prevPos == -1) prevPos = 8;

  int yPos = 16 + (32 * pos);
  int yPrevPos = 16 + (32 * prevPos);

  int xPos = 227;

  // clear previous selection marker
  tft.fillTriangle(xPos, yPrevPos + 10, xPos, yPrevPos + 22, xPos + 12, yPrevPos + 16, TFT_BLACK);

  // set current selection marker
  tft.fillTriangle(xPos, yPos + 10, xPos, yPos + 22, xPos + 12, yPos + 16, TFT_LIGHTGREY);
}

void networksListScreen(bool sort = true) {
  int displayedAPs = 0;
  String msg;

  if (sort) sortNetworksByRSSI();

  tft.setTextDatum(L_BASELINE);
  tft.setTextPadding(0);

  displayedAPs = (selectedAP / 9) * 9;

  int maxCnt = min(displayedAPs + 10, nNet);
  if (displayedAPs + 1 == maxCnt)
    msg = "Showing network " + String(maxCnt) + " of " + String(nNet) + ".";
  else
    msg = "Showing networks " + String(displayedAPs + 1) + '-' + String(maxCnt) + " of " + String(nNet) + ".";
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setTextPadding(220);
  tft.drawString(msg.c_str(), 4, 308, TEXT_FONT);

  tft.fillRect(0, 16, 240, 288, TFT_BLACK);

  for (int i = 0; i < 9; i++) {
    int remaining = nNet - displayedAPs;
    if (i >= remaining) break;
    displayNetwork(idx_rssi[i + displayedAPs].idx, i);
  }
}

void selectNextNetwork() {
  bool loadMoreAPs = false; // true when reached the end of current list and new APs should be plotted

  selectedAP++;
  if (selectedAP >= nNet) {
    selectedAP = 0;
    loadMoreAPs = true;
  }

  if (selectedAP % 9 == 0) loadMoreAPs = true;
  if (loadMoreAPs) networksListScreen(false);

  displaySelectionMarker(selectedAP);
}

void scanAllDone(int n) {
  tft.fillRect(224, 304, 16, 16, TFT_LIGHTGREY);
  nNet = n;
  //dispAt = 0;
  scanTime = current; // readjust scan time

  // plot network list
  selectedAP = 0;
  networksListScreen(true);
  displaySelectionMarker(selectedAP);
}

/************************************************************
    SINGLE NETWORK DISPLAY FUNCTIONS
 ***********************************************************/

String identifyManufacturer(String bssidStr) {
  if (!SPIFFS.exists("/manuf.txt"))
    return "Unknown manufacturer (1)";

  File f = SPIFFS.open("/manuf.txt", "r");
  if (!f) return "Unknown manufacturer (2)";

  String ln = "";

  while (f.available()) {
    char c = f.read();
    if (c != '\n') ln += String(c);
    else {
      String mac = ln.substring(0, 8);
      if (bssidStr.startsWith(mac) == true) {
        f.close();
        return ln.substring(9);
      }
      ln = "";
    }
  }
  f.close();
  return "Unknown manufacturer (3)";
}

void approxDistanceToAP(int rssi) {
  // See: https://stackoverflow.com/a/18359639
  int f = selectedChannel * 5 + 2407;
  if (selectedChannel == 14) f = 2484;
  rssi = abs(rssi);
  double ex = (27.55 - (20 * log10(f)) + rssi) / 20;
  float dist = pow(10, ex);
  String sdist = String(dist, 1);
  if (rssi == 100) sdist = "--";

  tft.setTextDatum(R_BASELINE);
  tft.setTextPadding(64);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(sdist, 226, 136, 4);
  tft.setTextDatum(R_BASELINE);
  tft.setTextPadding(0);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("m", 236, 129, TEXT_FONT);
}

void bigSignalIndicator(int rssi) {
  uint16_t color = TFT_GREEN;
  int rssi2 = rssi;

  if (rssi <= -66) color = TFT_YELLOW;
  if (rssi < -88) color = TFT_RED;
  if (rssi <= -95) color = TFT_DARKGREY;

  rssi2 = abs(rssi);
  rssi2 = (rssi2 - 35) * 2;
  rssi2 = 144 - rssi2;
  if (rssi2 < 24) rssi2 = 24;
  if (rssi2 > 144) rssi2 = 144;

  for (int i = 0; i < 120; i += 2) {
    if ((i < rssi2) && (i % 10 != 0)) tft.fillRect(120 - i, 144 - i, i * 2 , 2, color);
    else tft.fillRect(120 - i, 144 - i, i * 2 , 2, TFT_DARKGREY);
  }

  String srssi = String(rssi);
  if (rssi == -100) srssi = "  --";

  tft.setTextDatum(L_BASELINE);
  tft.setTextPadding(50);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(srssi, 4, 136, 4);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("dBm", 44, 129, TEXT_FONT);

  approxDistanceToAP(rssi);
}

static void ICACHE_FLASH_ATTR single_scan_done(void *arg, STATUS status) {
  customScanDone = true;
  tft.fillRect(224, 304, 16, 16, TFT_LIGHTGREY);
  scanTime = current; // readjust scan time

  if  (status == OK) {
    struct bss_info *bss_link = (struct bss_info*)arg;

    if (bss_link != NULL) {
      String msg;

      char ssid_copy[33]; // Ensure space for maximum len SSID (32) plus trailing 0
      memcpy(ssid_copy, bss_link->ssid, sizeof(bss_link->ssid));
      ssid_copy[32] = 0; // Potentially add 0-termination if none present earlier

      bigSignalIndicator(bss_link->rssi);

      tft.setTextDatum(BC_DATUM);

      tft.setTextPadding(240);
      if (bss_link->is_hidden == 0) {
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString(ssid_copy, 120, 172, 4);
      }
      else {
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString("Hidden", 120, 172, 4);
      }

      tft.setTextDatum(L_BASELINE);
      int frq = bss_link->channel * 5 + 2407;
      if (frq == 2477) frq = 2484;
      msg = "Channel " + String(bss_link->channel) + " (" + String(frq) + "MHz)";
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(msg, 4, 200, TEXT_FONT);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      msg = "802.11";
      if (bss_link->phy_11b) msg += 'b';
      if (bss_link->phy_11g) msg += 'g';
      if (bss_link->phy_11n) msg += 'n';
      tft.drawString(msg, 136, 200, TEXT_FONT);

      if (bss_link->wps) {
        tft.fillRoundRect(216, 198, 21, 11, 3, TFT_LIGHTGREY);
        tft.setTextPadding(0);
        tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
        tft.drawString("WPS", 218, 200, TEXT_FONT);
      }
      else
        tft.fillRoundRect(220, 184, 20, 16, 3, TFT_BLACK);

      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft.drawString("Security:", 4, 216, TEXT_FONT);

      switch (bss_link->authmode) {
        case AUTH_OPEN: msg = "None"; break;
        case AUTH_WEP: msg = "WEP"; break;
        case AUTH_WPA_PSK: msg = "WPA-PSK"; break;
        case AUTH_WPA2_PSK: msg = "WPA2-PSK"; break;
        case AUTH_WPA_WPA2_PSK: msg = "WPA/WPA2-PSK"; break;
        case AUTH_MAX: msg = "MAX"; break;
      }

      if (bss_link->authmode != 0) {
        switch (bss_link->pairwise_cipher) {
          case CIPHER_WEP40: msg += " (WEP40)"; break;
          case CIPHER_WEP104: msg += " (WEP104)"; break;
          case CIPHER_TKIP: msg += " (TKIP)"; break;
          case CIPHER_CCMP: msg += " (CCMP)"; break;
          case CIPHER_TKIP_CCMP: msg += " (TKIP/CCMP)"; break;
          case CIPHER_UNKNOWN: msg += " (Unknown)"; break;
        }
      }

      tft.setTextPadding(180);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString(msg, 60, 216, TEXT_FONT);

      msg = "Freq. offset=" + String(bss_link->freq_offset) + " (Calibration=" + String(bss_link->freqcal_val) + ")";
      tft.setTextPadding(240);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft.drawString(msg, 4, 264, TEXT_FONT);
    }
    else {
      tft.setTextDatum(BC_DATUM);
      tft.setTextPadding(240);
      tft.setTextColor(TFT_MAROON, TFT_BLACK);
      tft.drawString("Out of range", 120, 172, 4);
      tft.setTextDatum(L_BASELINE);

      bigSignalIndicator(-100);
    }
  }
}

void displaySingleNetwork() {
  // store data about selected network
  for (int i = 0; i < 6; i++)
    selectedBSSID[i] = WiFi.BSSID(idx_rssi[selectedAP].idx)[i];
  selectedChannel = WiFi.channel(idx_rssi[selectedAP].idx);

  // draw interface
  tft.fillRect(0, 16, 240, 288, TFT_BLACK);
  tft.setTextDatum(L_BASELINE);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  tft.setTextPadding(240);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Scanning...", 120, 172, 4);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(WiFi.BSSIDstr(idx_rssi[selectedAP].idx), 120, 184, TEXT_FONT);
  tft.setTextDatum(L_BASELINE);

  tft.drawString(identifyManufacturer(WiFi.BSSIDstr(idx_rssi[selectedAP].idx)), 4, 232, TEXT_FONT);
  tft.setTextPadding(0);

  tft.fillRect(0, 304, 240, 16, TFT_LIGHTGREY);
}

void setup() {
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);

  SPIFFS.begin();

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("WiFi Analyzer", 120, 172, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Version 1.0", 120, 192, TEXT_FONT);
  tft.setTextDatum(L_BASELINE);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 240, 16, TFT_LIGHTGREY);
  tft.fillRect(0, 304, 240, 16, TFT_LIGHTGREY);
  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.drawString("ESP8266 WiFi Analyzer 1.0", 4, 4, TEXT_FONT);

  tft.fillRect(0, 16, 240, 288, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  tft.setTextPadding(240);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Scanning...", 120, 172, 4);
  tft.setTextDatum(L_BASELINE);
}

void loop() {
  current = millis();

  if (current > scanTime + scanInterval) {
    tft.drawBitmap(224, 304, reload_bitmap, 16, 16, TFT_DARKGREEN);

    if (singleNetworkDisplay) {
      struct scan_config config;
      memset(&config, 0, sizeof(config));

      config.bssid = &selectedBSSID[0];
      if (scanKnownChannelOnly) config.channel = selectedChannel;
      config.show_hidden = 1;
      config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
      config.scan_time.active.min = 250;
      config.scan_time.active.max = 1000;

      if (wifi_station_scan(&config, single_scan_done)) {
        customScanDone = false;
        yield();
      }
    }
    else {
      WiFi.scanDelete();
      WiFi.scanNetworksAsync(scanAllDone, true);
    }

    scanTime = current + 25000;
  }

  if (current > buttonTime + 200) {
    if ((digitalRead(D1) == LOW) && (!singleNetworkDisplay)) {
      if (WiFi.scanComplete() >= 0) {
        selectNextNetwork();
        scanTime = current + 5000;
        buttonTime = current;
      }
    }

    if (digitalRead(D2) == LOW) {
      /***** LIST DISPLAY *****/
      if (singleNetworkDisplay && customScanDone) { // switch back to list mode
        scanInterval = 5000; // increase refresh interval

        networksListScreen(true);
        displaySelectionMarker(selectedAP);
        singleNetworkDisplay = false;
        scanTime = current + 5000;
      }
      /***** SINGLE AP SCAN *****/
      else if (WiFi.scanComplete() >= 0) {
        scanInterval = 2000; // decrease refresh interval
        singleNetworkDisplay = true;
        scanTime = current + 200; // new scan

        displaySingleNetwork();
      }
      buttonTime = current;
    }
  }
}
