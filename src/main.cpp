#include "s3_geek_graphics.h"
#include "s3_geek_wifi.h"
#include "s3_geek_sd.h"
#include "s3_geek_ftp.h"

S3GeekGraphics graphics;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    while (!psramInit())
    {
        Serial.println("Failed to initialize PSRAM");
        delay(100);
    }

    graphics.begin();
    setup_wifi();
}

uint32_t counter = 0;
void loop()
{
    graphics.loop();
    loop_ftp();
    yield();
}
