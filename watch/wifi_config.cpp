#include "wifi_config.h"
#include <WiFi.h>
#include <Preferences.h>
#include "server.h"
#include <ESPAsyncWebServer.h>
#include "log.h"
#include "filesystem.h"
#include <DNSServer.h>

DNSServer dnsServer;
const byte DNS_PORT = 53;

#define FIXED_WIFI 0

#if (FIXED_WIFI)
const char *SSID = "Router2_VHA";
const char *PASS = "Hanoi@2025";
// const char *SSID = "QE Lab_AP01_2.4GHz";
// const char *PASS = "Hanoi@2025";
#endif

Preferences preferences;
extern AsyncWebServer server;

// Keys in flash
const char *PREF_NAMESPACE = "wifi";
const char *KEY_SSID = "ssid";
const char *KEY_PASS = "pass";

// AP info
const char *AP_SSID = "ESP32_Watch";
const char *AP_PASS = "";

static String ssid;
static String pass;

const unsigned long WIFI_CONNECT_TIMEOUT = 15000;
uint8_t connectWifi = 0;
static unsigned long startStamp;
uint8_t isStartAPMode = 0;
volatile bool wantConnectWiFi = false;
static void saveWifiConfig();

callBack wifiConfigDone;
// -------------------------------
// H??M CLEAR THÔNG TIN WIFI
// -------------------------------
void clearWifiConfig()
{
  preferences.begin(PREF_NAMESPACE, false);
  preferences.remove(KEY_SSID);
  preferences.remove(KEY_PASS);
  preferences.end();
  connectWifi = 0;
  setConnected(0);
  Serial.println("Đã xóa thông tin WiFi!");
}

// -------------------------------
const char html[] PROGMEM  = R"HTML(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Cấu hình Wi-Fi ESP32</title>
  <style>
    body {
      font-family: "Segoe UI", sans-serif;
      background: linear-gradient(135deg, #4facfe, #00f2fe);
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }
    .container {
      background: white;
      padding: 30px 40px;
      border-radius: 15px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.2);
      text-align: center;
      width: 90%;
      max-width: 400px;
    }
    h2 {
      color: #333;
      margin-bottom: 20px;
    }
    input[type="text"], input[type="password"] {
      width: 100%;
      padding: 12px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 8px;
      font-size: 16px;
    }
    button {
      background-color: #4facfe;
      color: white;
      border: none;
      padding: 12px 20px;
      border-radius: 8px;
      font-size: 16px;
      cursor: pointer;
      width: 100%;
      transition: background 0.3s;
    }
    button:hover {
      background-color: #00c6ff;
    }
    .footer {
      margin-top: 15px;
      font-size: 13px;
      color: #666;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Cấu hình Wi-Fi ESP32</h2>
    <form action="/save" method="POST">
      <input type="text" name="ssid" placeholder="Tên Wi-Fi (SSID)" required>
      <input type="password" name="password" placeholder="Mật khẩu Wi-Fi" required>
      <button type="submit">Lưu cấu hình</button>
    </form>
    <div class="footer">© 2025 ESP32 Config Portal</div>
  </div>
</body>
</html>
)HTML";
// -------------------------------
void handleSave(AsyncWebServerRequest *request)
{
  if (!request->hasParam("ssid", true) ||
      !request->hasParam("password", true))
  {
    request->send(400, "text/plain", "Missing SSID or PASS");
//    request->send(SPIFFS, "/config.html", "text/html");
    request->send(200, "text/html", html);
    return;
  }

  ssid = request->getParam("ssid", true)->value();
  pass = request->getParam("password", true)->value();
  Serial.printf("Connecting to %s pass %s...\n", ssid.c_str(), pass.c_str());

  if (pass.length() == 0 || ssid.length() == 0)
  {

    return;
  }
  request->send(200, "text/plain", "Connecting...");
  wantConnectWiFi = true;   // báo ý định
  // dnsServer.stop();
  // WiFi.softAPdisconnect(true);
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid.c_str(), pass.c_str());
  // startStamp = millis();
  // wifiConfigDone();
}

uint8_t waitWifiDone()
{
  isStartAPMode = 0;
  static unsigned long stamp = millis();
  if (millis() - startStamp >= WIFI_CONNECT_TIMEOUT)
  {
    Serial.println("ket noi that bai time out");
    return 2; // timeout
  }
  if (millis() - stamp > 300)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      setConnected(1);
      saveWifiConfig();
      Serial.print("IP: ");
      connectWifi = 1;
      Serial.println(WiFi.localIP());
      Serial.println("ket noi thanh cong");
      server.end();
      return 1; // connect success
    }
stamp = millis();
  }
  return 0;
}

void saveWifiConfig()
{
  Serial.println("Kết nối WiFi thành công!");

  preferences.begin(PREF_NAMESPACE, false);
  preferences.putString(KEY_SSID, ssid);
  preferences.putString(KEY_PASS, pass);
  preferences.end();
  setConnected(1);
  Serial.println("Đã lưu WiFi vào flash!");
}
// -------------------------------
// Tạo AP mode + WebServer config
// -------------------------------
void startConfigPortal()
{
  if (isStartAPMode == 0)
  {
    isStartAPMode = 1;
    connectWifi = 0;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->redirect("http://192.168.4.1"); });
    // Serial.print("AP IP: ");
    // Serial.println(WiFi.softAPIP());


     // send with config file in spiffs
//    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
//              { request->send(SPIFFS, "/config.html", "text/html"); });
server.on("/", [](AsyncWebServerRequest *request) {
  request->send(200, "text/html", html);
});

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request)
              { handleSave(request); });

    server.begin();
    Serial.println("http://192.168.4.1");
  }
}

// -------------------------------
// -------------------------------
bool tryConnectSavedWiFi()
{
#if (FIXED_WIFI)

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
#else
  Serial.println("init wifi , read from memory");
  preferences.begin(PREF_NAMESPACE, true);
  String ssid = preferences.getString(KEY_SSID, "");
  String pass = preferences.getString(KEY_PASS, "");
  preferences.end();

  if (ssid == "")
  {
    Serial.println("Không có thông tin WiFi trong Bộ nhớ");
    return false;
  }

  Serial.printf("kết nối WiFi: %s\n", ssid.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
#endif

  unsigned long start = millis();
  while (millis() - start < WIFI_CONNECT_TIMEOUT)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Kết nối WiFi thành công!");
      Serial.print("IP: ");
      connectWifi = 1;
      setConnected(1);
      Serial.println(WiFi.localIP());
      server.end();
      return true;
    }
    delay(300);
  }

  Serial.println("Kết nối thất bại.");
  return false;
}

bool WifiConfigInit(callBack wifiConfig)
{
  wifiConfigDone = wifiConfig;
  return tryConnectSavedWiFi();
}

void WifiConfigRun()
{
   if (wantConnectWiFi)
  {
    wantConnectWiFi = false;
    startStamp = millis();
    Serial.println("Stopping AP & DNS");
    dnsServer.stop();
    WiFi.softAPdisconnect(true);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

   if (wifiConfigDone)
      wifiConfigDone();
  }
}
