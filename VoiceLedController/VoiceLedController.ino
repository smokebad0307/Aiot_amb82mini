/*
 * AMB82-MINI 語音 LED 控制器
 *
 * 通訊：瀏覽器 Web Speech API -> HTTP -> AMB82-MINI
 * 板載 LED：LED_B（左/藍）、LED_G（右/綠）
 */

#include <WiFi.h>
#include "wifi_config.h"
#include "web_page.h"

WiFiServer server(80);

const int BLUE_LED_PIN = LED_B;
const int GREEN_LED_PIN = LED_G;
const unsigned long HTTP_TIMEOUT_MS = 2000;
const unsigned long WIFI_RETRY_MS = 10000;

bool blueOn = false;
bool greenOn = false;
unsigned long stateSequence = 0;
unsigned long lastWifiAttempt = 0;
bool serverReady = false;
String lastCommand = "startup";
String lastMessage = "系統已啟動";

void refreshLedState()
{
    // 讀取輸出腳位的板端狀態，並透過 API 回傳給操作介面。
    blueOn = (digitalRead(BLUE_LED_PIN) == HIGH);
    greenOn = (digitalRead(GREEN_LED_PIN) == HIGH);
}

void setLed(int pin, bool on)
{
    digitalWrite(pin, on ? HIGH : LOW);
    refreshLedState();
}

String jsonEscape(const String &value)
{
    String escaped;
    escaped.reserve(value.length() + 8);
    for (unsigned int i = 0; i < value.length(); i++) {
        char c = value.charAt(i);
        if (c == '\\' || c == '"') {
            escaped += '\\';
            escaped += c;
        } else if (c == '\n') {
            escaped += "\\n";
        } else if ((unsigned char)c >= 0x20) {
            escaped += c;
        }
    }
    return escaped;
}

String makeStateJson(bool ok, const String &message)
{
    refreshLedState();
    String json = "{\"ok\":";
    json += ok ? "true" : "false";
    json += ",\"blue\":";
    json += blueOn ? "true" : "false";
    json += ",\"green\":";
    json += greenOn ? "true" : "false";
    json += ",\"sequence\":" + String(stateSequence);
    json += ",\"lastCommand\":\"" + jsonEscape(lastCommand) + "\"";
    json += ",\"message\":\"" + jsonEscape(message) + "\"}";
    return json;
}

void sendHeaders(WiFiClient &client, const char *status, const char *contentType)
{
    client.print("HTTP/1.1 ");
    client.println(status);
    client.print("Content-Type: ");
    client.println(contentType);
    client.println("Cache-Control: no-store");
    client.println("Connection: close");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Access-Control-Allow-Methods: GET, POST, OPTIONS");
    client.println("Access-Control-Allow-Headers: Content-Type");
    client.println();
}

void sendJson(WiFiClient &client, const char *status, bool ok, const String &message)
{
    sendHeaders(client, status, "application/json; charset=utf-8");
    client.print(makeStateJson(ok, message));
}

void sendNotFound(WiFiClient &client)
{
    sendHeaders(client, "404 Not Found", "application/json; charset=utf-8");
    client.print("{\"ok\":false,\"message\":\"找不到 API 路徑\"}");
}

bool applyCommand(const String &command)
{
    if (command == "left_on") {
        setLed(BLUE_LED_PIN, true);
        lastMessage = "左邊藍燈已開啟";
    } else if (command == "right_on") {
        setLed(GREEN_LED_PIN, true);
        lastMessage = "右邊綠燈已開啟";
    } else if (command == "left_off") {
        setLed(BLUE_LED_PIN, false);
        lastMessage = "左邊藍燈已關閉";
    } else if (command == "right_off") {
        setLed(GREEN_LED_PIN, false);
        lastMessage = "右邊綠燈已關閉";
    } else if (command == "all_off") {
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, LOW);
        refreshLedState();
        lastMessage = "全部燈光已關閉";
    } else {
        return false;
    }

    lastCommand = command;
    stateSequence++;
    Serial.print("Command accepted: ");
    Serial.println(command);
    return true;
}

// 讀到 HTTP headers 結束；只保留第一行，並設上限避免異常封包耗盡記憶體。
String readRequestLine(WiFiClient &client)
{
    String firstLine;
    String currentLine;
    unsigned long started = millis();

    while (client.connected() && (millis() - started < HTTP_TIMEOUT_MS)) {
        while (client.available()) {
            char c = client.read();
            started = millis();
            if (c == '\n') {
                if (firstLine.length() == 0) {
                    firstLine = currentLine;
                } else if (currentLine.length() == 0) {
                    return firstLine;
                }
                currentLine = "";
            } else if (c != '\r' && currentLine.length() < 512) {
                currentLine += c;
            }
        }
        delay(1);
    }
    return firstLine;
}

String requestMethod(const String &line)
{
    int separator = line.indexOf(' ');
    return separator > 0 ? line.substring(0, separator) : "";
}

String requestTarget(const String &line)
{
    int first = line.indexOf(' ');
    int second = line.indexOf(' ', first + 1);
    if (first < 0 || second < 0) {
        return "";
    }
    return line.substring(first + 1, second);
}

void handleClient(WiFiClient &client)
{
    String line = readRequestLine(client);
    String method = requestMethod(line);
    String target = requestTarget(line);

    Serial.print("HTTP: ");
    Serial.println(line);

    if (method == "OPTIONS") {
        sendHeaders(client, "204 No Content", "text/plain");
    } else if (method == "GET" && (target == "/" || target == "/index.html")) {
        sendHeaders(client, "200 OK", "text/html; charset=utf-8");
        client.print(WEB_PAGE);
    } else if (method == "GET" && target == "/api/state") {
        sendJson(client, "200 OK", true, lastMessage);
    } else if (method == "POST" && target.startsWith("/api/command?name=")) {
        String command = target.substring(String("/api/command?name=").length());
        int ampersand = command.indexOf('&');
        if (ampersand >= 0) {
            command = command.substring(0, ampersand);
        }
        if (applyCommand(command)) {
            sendJson(client, "200 OK", true, lastMessage);
        } else {
            // 非白名單 API 指令不執行任何 digitalWrite。
            sendJson(client, "400 Bad Request", false, "不支援的控制指令；LED 狀態未改變");
        }
    } else if (method == "GET" && target == "/favicon.ico") {
        sendHeaders(client, "204 No Content", "image/x-icon");
    } else {
        sendNotFound(client);
    }
}

void printWifiStatus()
{
    IPAddress ip = WiFi.localIP();
    Serial.println();
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(ip);
    Serial.print("Open this URL: http://");
    Serial.println(ip);
}

void connectWifiBlocking()
{
    int status = WL_IDLE_STATUS;
    while (status != WL_CONNECTED) {
        Serial.print("Connecting to Wi-Fi: ");
        Serial.println(WIFI_SSID);
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        if (status != WL_CONNECTED) {
            Serial.println("Wi-Fi connection failed; retrying in 10 seconds.");
            delay(WIFI_RETRY_MS);
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    refreshLedState();

    connectWifiBlocking();
    server.begin();
    serverReady = true;
    printWifiStatus();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        serverReady = false;
        if (millis() - lastWifiAttempt >= WIFI_RETRY_MS) {
            lastWifiAttempt = millis();
            Serial.println("Wi-Fi disconnected; reconnecting...");
            int status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            if (status == WL_CONNECTED) {
                server.begin();
                serverReady = true;
                printWifiStatus();
            }
        }
        return;
    }

    // WiFi.begin() 可能先回傳連線中，稍後才完成；在狀態轉為已連線時重啟 HTTP server。
    if (!serverReady) {
        server.begin();
        serverReady = true;
        printWifiStatus();
    }

    WiFiClient client = server.available();
    if (client) {
        handleClient(client);
        delay(2);
        client.stop();
    }
}
