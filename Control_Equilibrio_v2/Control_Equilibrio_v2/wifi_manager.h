#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

const char    WIFI_AP_SSID[]   = "Balancin-ESP32";
const char    WIFI_AP_PASS[]   = "equilibrio123";   // mínimo 8 caracteres
const uint8_t WIFI_AP_CHANNEL  = 6;
const uint8_t WIFI_AP_MAX_CLI  = 2;

AsyncWebServer servidorWeb(80);

AsyncWebSocket ws("/ws");

bool wifiBegin()
{
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);   // menor latencia

    IPAddress ip(192, 168, 4, 1), mask(255, 255, 255, 0);
    WiFi.softAPConfig(ip, ip, mask);

    if (!WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS, WIFI_AP_CHANNEL, 0, WIFI_AP_MAX_CLI))
    {
        return false;
    }

    servidorWeb.addHandler(&ws);

    if (LittleFS.begin(false))   // false = no formatear automáticamente
    {
    servidorWeb.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    }
    else
    {
        servidorWeb.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
            r->send(200, "text/plain", "LittleFS no montado");
        });
    }
    
    servidorWeb.onNotFound([](AsyncWebServerRequest *r) {
        r->send(404, "text/plain", "No encontrado");
    });

    servidorWeb.begin();
    return true;
}

#endif