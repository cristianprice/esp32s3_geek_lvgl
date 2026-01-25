#include "s3_geek_graphics.h"
#include "s3_geek_local_time.h"
#include "s3_geek_sd.h"
#include "s3_geek_ftp.h"
#include <WiFi.h>

extern S3GeekGraphics graphics;
bool wifiConnected = false;
const char *ssid = "my net";        // Replace with your WiFi SSID
const char *password = "hidden01#"; // Replace with your WiFi password

static void onWiFiEvent(WiFiEvent_t event)
{
    String ipMessage;
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        graphics.postMessage("WiFi connected to AP");
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        ipMessage = "IP address: " + WiFi.localIP().toString();
        graphics.postMessage(ipMessage.c_str());
        wifiConnected = true;

        // Wifi is connected , we start all operations.
        setup_local_time();
        setup_sdcard();
        setup_ftp();
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        graphics.postMessage("WiFi disconnected");
        wifiConnected = false;
        WiFi.reconnect();
        break;

    default:
        break;
    }
}

void setup_wifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(onWiFiEvent);
    WiFi.begin(ssid, password);
}
