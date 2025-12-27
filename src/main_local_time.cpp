#include "local_time.h"

void setup_local_time()
{
    const char *ntpServer = "pool.ntp.org";

    // Bucharest / Romania timezone
    const char *tz = "EET-2EEST,M3.5.0/3,M10.5.0/4";

    configTzTime(tz, ntpServer);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 10000))
    {
        Serial.println("Failed to obtain time");
        return;
    }

    Serial.println(&timeinfo, "Local time: %Y-%m-%d %H:%M:%S");
}

bool wifi_connect()
{
    const char ssid[] = WIFI_SSID;
    const char password[] = WIFI_PASSWORD;

    uint32_t c = 0;
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
        ++c;

        if (c >= 5)
        {
            Serial.println("Failed to connect to WiFi");
            return false;
        }
    }

    Serial.println("WiFi connected");
    return true;
}

void wifi_diconnect()
{
    WiFi.disconnect(true, true);
}