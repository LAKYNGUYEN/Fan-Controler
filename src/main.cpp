#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PCF8574.h>
#include <ESP8266mDNS.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WebSocketsServer.h>
#include <LiquidCrystal_I2C.h>

#include "MAIN_PAGE.h"
#define OUTPUT_ACTIVE LOW
#define I2C_ADDRESS 0x20 // Default address for PCF8574
#define DHTTYPE DHT11    // DHT 11
#define DHTPIN 15        // DHT 11 data pin

const char *ssid = "Lau_2_a";
const char *password = "A1234567893a1";
bool start = false;

ESP8266WebServer server(80);
WebSocketsServer ws = WebSocketsServer(81);
PCF8574 pcf8574(0x20);
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

bool setState(const String &device, const String &state);
void toggle_led();
void handleRoot();
void handleall();
void wsHandler(uint8_t, WStype_t, uint8_t *, size_t);
void broadcastStates();
void Sensor();

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    pinMode(DHTPIN, INPUT);

    pinMode(LED_BUILTIN, OUTPUT); // Configure LED GPIO pin as OUTPUT
    pcf8574.pinMode(P0, OUTPUT);  // Đặt chân P0 là OUTPUT
    pcf8574.pinMode(P1, OUTPUT);  // Đặt chân P1 là OUTPUT
    pcf8574.pinMode(P2, OUTPUT);  // Đặt chân P2 là OUTPUT

    pcf8574.begin(); // Khởi động PCF8574

    Serial.println("connecting to wifi");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("Connected to WiFi");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    // LCD
    lcd.begin();
    lcd.clear();
    lcd.setCursor(0, 0); // đặt con trỏ ở vị trí hàng 0 cột 0
    lcd.print(WiFi.localIP());

    handleRoot();
    handleall();
    server.begin();
    ws.begin();

    if (MDNS.begin("esp"))
    {
        Serial.println("MDNS responder started");
        MDNS.addService("http", "tcp", 80);
    }

    Serial.println("HTTP server started");
}

void loop()
{
    // random data  send to client
    float randomData1 = random(0, 100) / 1.0;
    float randomData2 = random(0, 100) / 1.0;

    
        

    MDNS.update();
    server.handleClient();
    ws.loop();
}

bool setState(const String &device, const String &state)
{
    bool success = false;
    int8_t s = -1;
    if (state == "ON")
    {
        s = OUTPUT_ACTIVE;
    }
    else if (state == "OFF")
    {
        s = !OUTPUT_ACTIVE;
    }

    if (s == -1)
    {
        Serial.println("Invalid state");
        return false;
    }

    if (device == "fan")
    {
        pcf8574.digitalWrite(P0, s);
        success = true;
    }
    else if (device == "light")
    {
        pcf8574.digitalWrite(P1, s);
        success = true;
    }
    else if (device == "ON")
    {
        pcf8574.digitalWrite(P2, s);
        // pcf8574.digitalWrite(P1, s);
        success = true;
    }
    return success;
}

void Sensor()
{
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t))
    {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    String data = "temperature:" + String(t) + "\nhumidity:" + String(h);
    ws.broadcastTXT(data.c_str());
    // Serial.print("Humidity: ");
    // Serial.print(h);
    // Serial.print(" %\t");
    // Serial.print("Temperature: ");
    // Serial.print(t);
    // Serial.println(" *C");
    // delay(500);
}

void toggle_led()
{
    // Chuyển đổi trạng thái của các chân P0, P1, P2
    pcf8574.digitalWrite(P0, !pcf8574.digitalRead(P0));
    pcf8574.digitalWrite(P1, !pcf8574.digitalRead(P1));
    pcf8574.digitalWrite(P2, !pcf8574.digitalRead(P2));
}

void handleall()
{
    ws.onEvent(wsHandler);
}

void handleRoot()
{
    server.on("/", HTTP_GET, []()
              {
        Serial.println("Client IP: " + server.client().remoteIP().toString());
        server.send(200, "text/html", MAIN_PAGE); });

    server.on("/fan", HTTP_GET, []()
              {
        if (server.hasArg("state")) {
            String state = server.arg("state");
            if (setState("fan", state)) {
                server.send(200, "text/plain", "OK");
            } else {
                server.send(200, "text/plain", "Invalid state");
            }
        } else {
            server.send(200, "text/plain", "missing state");
        } 
        broadcastStates();
         });

    server.on("/light", []()
              {
        if (server.hasArg("state")) {
            String state = server.arg("state");
            if (setState("light", state)) {
                server.send(200, "text/plain", "OK");
            } else {
                server.send(200, "text/plain", "Invalid state");
            }
        } else {
            server.send(200, "text/plain", "missing state");
        }
        broadcastStates(); });

    server.on("/ON", []()
              {
        if (server.hasArg("state")) {
            String state = server.arg("state");
            if (setState("ON", state)) {
                server.send(200, "text/plain", "OK");
            } else {
                server.send(200, "text/plain", "Invalid state");
            }
        } else {
            server.send(200, "text/plain", "missing state");
        }
        broadcastStates(); });

    server.onNotFound([]()
                      { server.send(404, "text/html", "<h1>Not found</h1>"); });
}

String getStatesString()
{
    String res = "";
    res += "fan:" + String(pcf8574.digitalRead(P0) == OUTPUT_ACTIVE ? "ON" : "OFF") + "\n";
    res += "light:" + String(pcf8574.digitalRead(P1) == OUTPUT_ACTIVE ? "ON" : "OFF") + "\n";
    res += "ON:" + String(pcf8574.digitalRead(P2) == OUTPUT_ACTIVE ? "ON" : "OFF") + "\n";
    return res;
}

void broadcastStates()
{
    ws.broadcastTXT(getStatesString().c_str());
}

/**
 * Được gọi khi nhận được dữ liệu từ client thông qua websocket
 * @param id ID của client
 * @param type Loại sự kiện
 * @param data Dữ liệu nhận được
 * @param size Kích thước dữ liệu nhận được
 */
void wsHandler(uint8_t id, WStype_t type, uint8_t *data, size_t size)
{
    String message = (char *)data;

    if (type == WStype_CONNECTED)
    {
        Serial.printf("[%u] Connected - IP: %s\n", id, ws.remoteIP(id).toString().c_str());
    }
    else if (type == WStype_TEXT)
    {
        Serial.printf("[%u] get Text: %s\n", id, message.c_str());
        if (message == "GET")
        {
            ws.sendTXT(id, getStatesString().c_str());
        }
    }
    else if (type == WStype_DISCONNECTED)
    {
        Serial.printf("[%u] Disconnected!\n", id);
    }
}