#include "wifi_config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

Preferences preferences;
WebServer server(80);

// Keys in flash
const char *PREF_NAMESPACE = "wifi";
const char *KEY_SSID = "ssid";
const char *KEY_PASS = "pass";

// AP info
const char *AP_SSID = "ESP32_Config";
const char *AP_PASS = "12345678"; // >= 8 ký tự

// Thời gian timeout khi thử connect WiFi
const unsigned long WIFI_CONNECT_TIMEOUT = 15000;
uint8_t connectWifi = 0;
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
    <html>
    <head><meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>ESP32 WiFi Config</title></head>
    <body>
      <h2>Thiết lập WiFi cho ESP32</h2>
      <form action="/save" method="POST">
        SSID:<br>
        <input type="text" name="ssid" required><br><br>
        Password:<br>
        <input type="password" name="pass"><br><br>
        <input type="submit" value="Lưu & Kết nối">
      </form>
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
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    ssid.trim();
    pass.trim();

    server.send(200, "text/html", "<h3>Đang thử kết nối WiFi...</h3>");

    delay(300);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    bool ok = false;

    while (millis() - start < WIFI_CONNECT_TIMEOUT)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            ok = true;
            setConnected(1);
            break;
        }
        delay(200);
    }

    if (ok)
    {
        Serial.println("Kết nối WiFi thành công!");

        // Lưu vào bộ nhớ
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putString(KEY_SSID, ssid);
        preferences.putString(KEY_PASS, pass);
        preferences.end();
        setConnected(1);
        Serial.println("Đã lưu WiFi vào flash!");
        delay(1000);
//        ESP.restart();
    }
    else
    {
        Serial.println("Kết nối thất bại, quay lại AP mode.");
        ESP.restart();
    }
}

// -------------------------------
// Tạo AP mode + WebServer config
// -------------------------------
void startConfigPortal()
{
    Serial.println("==> Không có WiFi hoặc kết nối thất bại, bật AP mode");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.begin();

    Serial.println("WebServer đã chạy, mở: http://192.168.4.1");
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

void WifiConfigInit()
{
    if (!tryConnectSavedWiFi())
    {
        startConfigPortal();
    }
}

void WifiConfigRun()
{
    if (connectWifi == 0)
        server.handleClient();
}
