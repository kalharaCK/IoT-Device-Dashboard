// ---------------------------
// main.cpp  (ESP32 / Arduino)
// ---------------------------
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// UI (PROGMEM) ----------------
#include "dashboard_html.h"   // serves /index.html

// ---------- Captive DNS ----------
#define DNS_PORT 53
DNSServer dnsServer;

// ---------- HTTP server ----------
WebServer server(80);

// ---------- Defaults / files ----------
static const char* DEFAULT_AP_SSID = "ESP32-AccessPoint";
static const char* DEFAULT_AP_PASS = "12345678";      // >= 8 chars
static const char* WIFI_FILE = "/wifi.json";
static const char* GSM_FILE  = "/gsm.json";
static const char* USER_FILE = "/user.json";

// ---------- Helpers ----------
String ipToStr(const IPAddress &ip) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  return String(buf);
}

void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Cache-Control", "no-store");
}
void sendJson(int code, const String& body) {
  addCORS();
  server.send(code, "application/json", body);
}
void sendText(int code, const String& body, const String& ctype = "text/plain") {
  addCORS();
  server.send(code, ctype, body);
}

// ---------------------------
// Per-tab config "classes"
// ---------------------------
struct WifiConfig {
  String staSsid, staPass;
  String apSsid, apPass;

  bool load() {
    if (!SPIFFS.exists(WIFI_FILE)) return false;
    File f = SPIFFS.open(WIFI_FILE, "r"); if (!f) return false;
    DynamicJsonDocument doc(1024);
    auto err = deserializeJson(doc, f); f.close();
    if (err) return false;
    staSsid = doc["staSsid"] | "";
    staPass = doc["staPass"] | "";
    apSsid  = doc["apSsid"]  | DEFAULT_AP_SSID;
    apPass  = doc["apPass"]  | DEFAULT_AP_PASS;
    return true;
  }
  bool save() const {
    DynamicJsonDocument doc(1024);
    doc["staSsid"] = staSsid;
    doc["staPass"] = staPass;
    doc["apSsid"]  = apSsid.length() ? apSsid : DEFAULT_AP_SSID;
    doc["apPass"]  = apPass.length() ? apPass : DEFAULT_AP_PASS;
    File f = SPIFFS.open(WIFI_FILE, "w"); if (!f) return false;
    serializeJson(doc, f); f.close(); return true;
  }
} wifiCfg;

struct GsmConfig {
  String carrierName, apn, apnUser, apnPass;
  bool load() {
    if (!SPIFFS.exists(GSM_FILE)) return false;
    File f = SPIFFS.open(GSM_FILE, "r"); if (!f) return false;
    DynamicJsonDocument doc(1024); if (deserializeJson(doc, f)) { f.close(); return false; }
    f.close();
    carrierName = doc["carrierName"] | ""; apn = doc["apn"] | "";
    apnUser = doc["apnUser"] | ""; apnPass = doc["apnPass"] | "";
    return true;
  }
  bool save() const {
    DynamicJsonDocument doc(1024);
    doc["carrierName"] = carrierName; doc["apn"] = apn;
    doc["apnUser"] = apnUser; doc["apnPass"] = apnPass;
    File f = SPIFFS.open(GSM_FILE, "w"); if (!f) return false;
    serializeJson(doc, f); f.close(); return true;
  }
} gsmCfg;

struct UserConfig {
  String name, email, phone;
  bool load() {
    if (!SPIFFS.exists(USER_FILE)) return false;
    File f = SPIFFS.open(USER_FILE, "r"); if (!f) return false;
    DynamicJsonDocument doc(1024); if (deserializeJson(doc, f)) { f.close(); return false; }
    f.close();
    name = doc["name"] | ""; email = doc["email"] | ""; phone = doc["phone"] | "";
    return true;
  }
  bool save() const {
    DynamicJsonDocument doc(1024);
    doc["name"] = name; doc["email"] = email; doc["phone"] = phone;
    File f = SPIFFS.open(USER_FILE, "w"); if (!f) return false;
    serializeJson(doc, f); f.close(); return true;
  }
} userCfg;

// ---------- WiFi helpers ----------
void startAP(const String& ssid, const String& pass) {
  WiFi.mode(WIFI_AP_STA);
  bool ap_ok = WiFi.softAP(ssid.c_str(), pass.length() >= 8 ? pass.c_str() : NULL);
  if (!ap_ok) {
    WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASS);
  }
  delay(100);
  // Captive DNS: resolve everything to AP IP
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

void connectSTA(const String& ssid, const String& pass) {
  if (!ssid.length()) return;
  WiFi.begin(ssid.c_str(), pass.c_str());
}

const char* rssiToStrength(int rssi) {
  if (rssi >= -60) return "strong";
  if (rssi >= -75) return "medium";
  return "weak";
}

// ---------- JSON builders ----------
String buildStatusJson() {
  DynamicJsonDocument doc(1024);
  doc["mode"] = "AP+STA";

  JsonObject ap = doc.createNestedObject("ap");
  ap["ssid"] = wifiCfg.apSsid.length() ? wifiCfg.apSsid : DEFAULT_AP_SSID;
  ap["ip"]   = ipToStr(WiFi.softAPIP());
  ap["mac"]  = WiFi.softAPmacAddress();

  JsonObject sta = doc.createNestedObject("sta");
  bool staConnected = (WiFi.status() == WL_CONNECTED);
  sta["ssid"]      = staConnected ? WiFi.SSID() : "";
  sta["connected"] = staConnected;
  sta["ip"]        = staConnected ? ipToStr(WiFi.localIP()) : "0.0.0.0";
  sta["rssi"]      = staConnected ? WiFi.RSSI() : 0;
  sta["hostname"]  = WiFi.getHostname() ? WiFi.getHostname() : "";
  
  // Add connection status string for UI display
  if (staConnected) {
    sta["status"] = "Connected to " + WiFi.SSID();
    sta["statusClass"] = "status-connected";
  } else {
    sta["status"] = "Not connected";
    sta["statusClass"] = "status-disconnected";
  }

  String out; serializeJson(doc, out); return out;
}

// ---------- Routing ----------
void handleRoot() {
  // Redirect common captive-portal probes to index
  String host = server.hostHeader();
  if (host.startsWith("connectivitycheck.") || host.startsWith("captive.apple.com") ||
      host.startsWith("msftconnecttest.") || host.startsWith("detectportal.")) {
    addCORS();
    server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/index.html");
    server.send(302, "text/plain", "");
    return;
  }
  // Serve dashboard UI from PROGMEM
  addCORS();
  server.send_P(200, "text/html", dashboard_html, dashboard_html_len);
}

void handleOptions() {
  addCORS();
  server.send(204);
}

void handleNotFound() {
  // Captive fallback: always send the app
  handleRoot();
}

void setup() {
  Serial.begin(115200); delay(200);
  Serial.println("Starting ESP32 Configuration Panel...");

  // Filesystem
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
  } else {
    Serial.println("SPIFFS mounted successfully");
  }

  // Load configs
  wifiCfg.load(); gsmCfg.load(); userCfg.load();

  // Start AP and optional STA
  String apSsid = wifiCfg.apSsid.length() ? wifiCfg.apSsid : DEFAULT_AP_SSID;
  String apPass = wifiCfg.apPass.length() ? wifiCfg.apPass : DEFAULT_AP_PASS;
  startAP(apSsid, apPass);
  
  Serial.println("Access Point started:");
  Serial.println("SSID: " + apSsid);
  Serial.println("IP: " + ipToStr(WiFi.softAPIP()));
  
  // Try to connect to saved WiFi if available
  if (wifiCfg.staSsid.length()) {
    Serial.println("Attempting to connect to saved WiFi: " + wifiCfg.staSsid);
    connectSTA(wifiCfg.staSsid, wifiCfg.staPass);
  }

  // Routes ---------------------
  server.on("/", HTTP_GET, handleRoot);
  server.on("/index.html", HTTP_GET, handleRoot);

  // CORS preflight
  server.onNotFound(handleNotFound);
  server.on("/", HTTP_OPTIONS, handleOptions);
  server.on("/index.html", HTTP_OPTIONS, handleOptions);

  // ---- API: Status
  server.on("/api/status", HTTP_GET, []() {
    sendJson(200, buildStatusJson());
  });
  server.on("/api/status", HTTP_OPTIONS, handleOptions);

  // ---- API: WiFi Scan
  server.on("/api/wifi/scan", HTTP_GET, []() {
    Serial.println("Starting WiFi scan...");
    int n = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);
    Serial.println("Found " + String(n) + " networks");
    
    DynamicJsonDocument doc(8192);
    // Deduplicate by SSID keeping best RSSI
    struct Row { String ssid; int rssi; int channel; wifi_auth_mode_t auth; };
    std::vector<Row> rows;
    rows.reserve(n);
    for (int i = 0; i < n; i++) {
      Row r{ WiFi.SSID(i), WiFi.RSSI(i), WiFi.channel(i), WiFi.encryptionType(i) };
      if (r.ssid.length() == 0) continue;
      // keep only best RSSI per SSID
      bool replaced = false;
      for (auto &e : rows) {
        if (e.ssid == r.ssid) { if (r.rssi > e.rssi) e = r; replaced = true; break; }
      }
      if (!replaced) rows.push_back(r);
    }
    // sort by RSSI desc
    std::sort(rows.begin(), rows.end(), [](const Row&a, const Row&b){ return a.rssi > b.rssi; });

    JsonArray arr = doc.to<JsonArray>();
    for (auto &r : rows) {
      JsonObject o = arr.add<JsonObject>();
      o["ssid"]     = r.ssid;
      o["rssi"]     = r.rssi;
      o["channel"]  = r.channel;
      o["auth"]     = (int)r.auth;
      o["security"] = (r.auth == WIFI_AUTH_OPEN) ? "Open" : "Secure";
      o["strength"] = rssiToStrength(r.rssi);
    }
    String out; serializeJson(doc, out);
    sendJson(200, out);
  });
  server.on("/api/wifi/scan", HTTP_OPTIONS, handleOptions);

  // ---- API: WiFi Connect (save STA credentials + attempt connect)
  server.on("/api/wifi/connect", HTTP_POST, []() {
    if (!server.hasArg("plain")) { sendText(400, "Invalid JSON"); return; }
    DynamicJsonDocument doc(1024);
    auto err = deserializeJson(doc, server.arg("plain"));
    if (err) { sendText(400, "Invalid JSON"); return; }
    String ssid = doc["ssid"] | "";
    String pass = doc["password"] | "";
    if (!ssid.length()) { sendText(400, "SSID required"); return; }

    Serial.println("Attempting to connect to: " + ssid);
    
    // Save credentials first
    wifiCfg.staSsid = ssid; 
    wifiCfg.staPass = pass; 
    wifiCfg.save();

    // Disconnect from current network cleanly
    WiFi.disconnect(true);
    delay(1000);
    
    // Connect to new network
    connectSTA(ssid, pass);

    // Wait up to ~15s for connection
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    DynamicJsonDocument resp(256);
    bool ok = (WiFi.status() == WL_CONNECTED);
    resp["success"] = ok;
    
    if (ok) {
      resp["ip"]   = ipToStr(WiFi.localIP());
      resp["ssid"] = WiFi.SSID();
      resp["rssi"] = WiFi.RSSI();
      Serial.println("Connected successfully!");
      Serial.println("IP: " + ipToStr(WiFi.localIP()));
    } else {
      resp["ip"]   = "0.0.0.0";
      resp["ssid"] = "";
      resp["error"] = "Connection timeout or authentication failed";
      Serial.println("Connection failed!");
    }
    
    String out; serializeJson(resp, out);
    sendJson(ok ? 200 : 500, out);
  });
  server.on("/api/wifi/connect", HTTP_OPTIONS, handleOptions);

  // ---- API: WiFi Disconnect
  server.on("/api/wifi/disconnect", HTTP_POST, []() {
    Serial.println("Disconnecting from WiFi...");
    
    // Clear saved credentials
    wifiCfg.staSsid = "";
    wifiCfg.staPass = "";
    wifiCfg.save();
    
    // Disconnect from WiFi
    WiFi.disconnect(true);
    delay(1000);
    
    // Ensure we're still in AP+STA mode for the web interface
    WiFi.mode(WIFI_AP_STA);
    
    Serial.println("WiFi disconnected and credentials cleared");
    
    DynamicJsonDocument resp(256);
    resp["success"] = true;
    resp["message"] = "Disconnected from WiFi network";
    
    String out; serializeJson(resp, out);
    sendJson(200, out);
  });
  server.on("/api/wifi/disconnect", HTTP_OPTIONS, handleOptions);

  // ---- API: Save GSM
  server.on("/api/save/gsm", HTTP_POST, []() {
    if (!server.hasArg("plain")) { sendText(400, "Invalid JSON"); return; }
    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, server.arg("plain"))) { sendText(400, "Invalid JSON"); return; }
    gsmCfg.carrierName = doc["carrierName"] | "";
    gsmCfg.apn         = doc["apn"] | "";
    gsmCfg.apnUser     = doc["apnUser"] | "";
    gsmCfg.apnPass     = doc["apnPass"] | "";
    bool ok = gsmCfg.save();
    sendText(ok ? 200 : 500, ok ? "OK" : "SAVE_FAILED");
  });
  server.on("/api/save/gsm", HTTP_OPTIONS, handleOptions);

  // ---- API: Load GSM
  server.on("/api/load/gsm", HTTP_GET, []() {
    DynamicJsonDocument doc(1024);
    doc["carrierName"] = gsmCfg.carrierName;
    doc["apn"]         = gsmCfg.apn;
    doc["apnUser"]     = gsmCfg.apnUser;
    doc["apnPass"]     = gsmCfg.apnPass;
    String out; serializeJson(doc, out);
    sendJson(200, out);
  });
  server.on("/api/load/gsm", HTTP_OPTIONS, handleOptions);

  // ---- API: Save USER
  server.on("/api/save/user", HTTP_POST, []() {
    if (!server.hasArg("plain")) { sendText(400, "Invalid JSON"); return; }
    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, server.arg("plain"))) { sendText(400, "Invalid JSON"); return; }
    userCfg.name  = doc["name"]  | "";
    userCfg.email = doc["email"] | "";
    userCfg.phone = doc["phone"] | "";
    bool ok = userCfg.save();
    sendText(ok ? 200 : 500, ok ? "OK" : "SAVE_FAILED");
  });
  server.on("/api/save/user", HTTP_OPTIONS, handleOptions);

  // ---- API: Load USER
  server.on("/api/load/user", HTTP_GET, []() {
    DynamicJsonDocument doc(1024);
    doc["name"]  = userCfg.name;
    doc["email"] = userCfg.email;
    doc["phone"] = userCfg.phone;
    String out; serializeJson(doc, out);
    sendJson(200, out);
  });
  server.on("/api/load/user", HTTP_OPTIONS, handleOptions);

  // ---- API: Save AP settings (optional)
  server.on("/api/save/wifi", HTTP_POST, []() {
    if (!server.hasArg("plain")) { sendText(400, "Invalid JSON"); return; }
    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, server.arg("plain"))) { sendText(400, "Invalid JSON"); return; }
    String apSsid = doc["apSsid"] | wifiCfg.apSsid;
    String apPass = doc["apPass"] | wifiCfg.apPass;
    if (apSsid.length()) wifiCfg.apSsid = apSsid;
    if (apPass.length()) wifiCfg.apPass = apPass;
    bool ok = wifiCfg.save();
    sendText(ok ? 200 : 500, ok ? "OK" : "SAVE_FAILED");
  });
  server.on("/api/save/wifi", HTTP_OPTIONS, handleOptions);

  // ---- API: Reboot
  server.on("/api/reboot", HTTP_POST, []() {
    sendText(200, "OK");
    delay(200);
    ESP.restart();
  });
  server.on("/api/reboot", HTTP_OPTIONS, handleOptions);

  // Start server
  server.begin();
  Serial.println("[HTTP] server started");
  Serial.println("Configuration panel available at: http://" + ipToStr(WiFi.softAPIP()));
}

void loop() {
  dnsServer.processNextRequest(); // captive portal DNS
  server.handleClient();          // HTTP
  
  // Optional: Print WiFi status periodically for debugging
  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint > 30000) { // Every 30 seconds
    lastStatusPrint = millis();
    Serial.println("Status - AP: " + ipToStr(WiFi.softAPIP()) + 
                   ", STA: " + (WiFi.status() == WL_CONNECTED ? ipToStr(WiFi.localIP()) : "Not connected"));
  }
}