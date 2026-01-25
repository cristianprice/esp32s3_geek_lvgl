#include "s3_geek_local_time.h"
#include "Arduino.h"
#include <time.h>
#include "s3_geek_graphics.h"

extern S3GeekGraphics graphics;

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

    char buffer[64];
    strftime(buffer, sizeof(buffer), "Local time: %Y-%m-%d %H:%M:%S", &timeinfo);
    graphics.postMessage("%s", buffer);
}
