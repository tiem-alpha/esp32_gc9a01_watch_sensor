#include "server.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "filesystem.h"
#include <ESPmDNS.h>
#include "temp_sensor.h"
#include "watch.h"
#include "custom_led.h"

#define SERVER_DNS "watch"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

extern MYLED led;
uint8_t isConnected = 0;
uint8_t clientConnect = 0;

uint8_t getConnected()
{
    return isConnected;
}

void setConnected(uint8_t connected)
{
    isConnected = connected;
}

void ping()
{
    ws.textAll("ping");
}

void keepConnect()
{
    // if(clientConnect)
    ws.cleanupClients();
}

void handleColor(JsonObject color)
{
    uint16_t hourColor = color["hour"];
    uint16_t minuteColor = color["minute"];
    uint16_t secondColor = color["second"];
    setHourColor(hourColor);
    setMinColor(minuteColor);
    setSecColor(secondColor);
    Serial.printf("Hour: %u, Minute: %u, Second: %u\n", hourColor, minuteColor, secondColor);
}

void notifyClients()
{
    // Serial.println("notifyClients");
    StaticJsonDocument<128> doc;
    doc["temp"] = GetTempSensor();
    doc["humi"] = GetHumSensor();
    //   doc["brightness"] = led.current;
    //   Serial.printf("update temp %d, hum %d, brign %d\n",GetTempSensor(),  GetHumSensor(),  led.current);
    String json;
    serializeJson(doc, json);
    ws.textAll(json);
    ws.textAll("{\"brightness\": " + String(led.target) + "}");
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, data);
        if (!error)
        {
            serializeJsonPretty(doc, Serial);
            Serial.println();
            if (doc.containsKey("brightness"))
            {

                int brightness = doc["brightness"];
                MYLEDSet(&led, brightness);
            }
            if (doc.containsKey("epoch"))
            {
                time_t epoch = doc["epoch"];
                // struct timeval now = { .tv_sec = epoch };
                setTime(epoch);
            }
            if (doc.containsKey("analog"))
            {
                uint8_t an = doc["analog"];

                setAnalog(an);
            }

            if (doc.containsKey("color"))
            {
                handleColor(doc["color"]);
            }
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.println("Client connected");
        clientConnect = 1;
        notifyClients();
        ws.textAll("{\"analog\": " + String(getAnalog()) + "}");
    }
    else if (type == WS_EVT_DATA)
    {
        handleWebSocketMessage(arg, data, len);
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.println("Client disconencted ");
        clientConnect = 0;
    }
}

void initServer()
{
    if (isConnected == 0)
        return;
    server.end();
    if (MDNS.begin(SERVER_DNS))
    { // tên DNS là esp32.local
        Serial.print("mDNS responder started: http://");
        Serial.print(SERVER_DNS);
        Serial.println(".local");
    }
    else
    {
        Serial.println("Error setting up MDNS responder!");
    }

    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("control.html");

    server.on("/img_begin", HTTP_POST, [](AsyncWebServerRequest *req)
              {
  openFile(BACK_GROUND_FILE);
  req->send(200); });

    server.on(
        "/img_chunk",
        HTTP_POST,
        [](AsyncWebServerRequest *req) {},
        nullptr,
        [](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total)
        {
            writeFile(data, len, index, total);
        });

    server.on("/img_end", HTTP_POST, [](AsyncWebServerRequest *req)
              {
    closeFile();
  req->send(200); });

    server.begin();
}
