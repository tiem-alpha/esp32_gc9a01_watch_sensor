#include "wifi_config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "log.h"

Preferences preferences;
WebServer server(80);

// Keys in flash
const char *PREF_NAMESPACE = "wifi";
const char *KEY_SSID = "ssid";
const char *KEY_PASS = "pass";

// AP info
const char *AP_SSID = "ESP32_Watch";
const char *AP_PASS = ""; // >= 8 ký tự

static String ssid;
static String pass;
// Thời gian timeout khi thử connect WiFi
const unsigned long WIFI_CONNECT_TIMEOUT = 15000;
uint8_t connectWifi = 0;
static unsigned long startStamp;
uint8_t isStartAPMode =0; 
static void saveWifiConfig();

callBack wifiConfigDone;
// -------------------------------
// HÀM CLEAR THÔNG TIN WIFI
// -------------------------------
void clearWifiConfig()
{
    preferences.begin(PREF_NAMESPACE, false);
    preferences.remove(KEY_SSID);
    preferences.remove(KEY_PASS);
    preferences.end();
    connectWifi = 0;
    setConnected(0);
    Serial.println("Đã xóa thông tin WiFi khỏi bộ nhớ!");
}

// -------------------------------
// Hàm hiển thị FORM cấu hình WiFi
// -------------------------------
String wifiConfigHTML()
{
    return R"=====(
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
      <input type="password" name="pass" placeholder="Mật khẩu Wi-Fi" required>
      <button type="submit">Lưu cấu hình</button>
    </form>
    <div class="footer">© 2025 ESP32 Config Portal</div>
  </div>
</body>
</html>
  )=====";
}

// -------------------------------
// Xử lý trang chủ
// -------------------------------
void handleRoot()
{
    // if(connectWifi ==0)
    server.send(200, "text/html", wifiConfigHTML());
}

// -------------------------------
// Lưu thông tin WiFi & thử kết nối
// -------------------------------
void handleSave()
{
    ssid = server.arg("ssid");
    pass = server.arg("pass");
    ssid.trim();
    pass.trim();
    if(pass.length() ==0 || ssid.length() ==0) {
       handleRoot();
      return; 
    }
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.print(" pass ");
    Serial.println(pass);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    startStamp = millis();
    wifiConfigDone();
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
    if (millis() - startStamp > 300)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            setConnected(1);
            saveWifiConfig();
            Serial.println("ket noi thanh cong");
            return 1; // connect success
        }
    }
    return 0; 
}

void saveWifiConfig()
{
    Serial.println("Kết nối WiFi thành công!");
    // Lưu vào bộ nhớ
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
    if(isStartAPMode ==0 ){
        isStartAPMode =1; 
         Serial.println("==> Không có WiFi hoặc kết nối thất bại, bật AP mode");
    connectWifi = 0;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.begin();
    // WriteLog("http://192.168.4.1", 18);
    Serial.println("WebServer đã chạy, mở: http://192.168.4.1");
    }
   
}

// -------------------------------
// Thử connect WiFi dùng thông tin lưu trước đó
// -------------------------------
bool tryConnectSavedWiFi()
{
    preferences.begin(PREF_NAMESPACE, true);
    String ssid = preferences.getString(KEY_SSID, "");
    String pass = preferences.getString(KEY_PASS, "");
    preferences.end();

    if (ssid == "")
    {
        Serial.println("Không có thông tin WiFi trong bộ nhớ.");
        return false;
    }

    Serial.printf("Thử kết nối WiFi: %s\n", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

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
    if (connectWifi == 0)
        server.handleClient();
}
