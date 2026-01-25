#include "s3_geek_graphics.h"

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
}

void loop()
{
    graphics.loop();
    yield();
}
