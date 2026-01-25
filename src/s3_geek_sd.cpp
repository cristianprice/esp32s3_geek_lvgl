#include "s3_geek_sd.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "s3_geek_graphics.h"

SPIClass sdSPI(HSPI);
extern S3GeekGraphics graphics;

void setup_sdcard()
{
    sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    sdSPI.setClockDivider(SPI_CLOCK_DIV2);
    if (!SD.begin(SD_CS, sdSPI))
    {
        graphics.postMessage("Failed to mount the memory card Procedure");
        return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        graphics.postMessage("Disconnected storage");
        return;
    }
    else if (cardType == CARD_MMC)
    {
        graphics.postMessage("The MMC card is mounted");
    }
    else if (cardType == CARD_SD)
    {
        graphics.postMessage("The SDSC card is mounted");
    }
    else if (cardType == CARD_SDHC)
    {
        graphics.postMessage("The SDHC card is mounted");
    }
    else
    {
        graphics.postMessage("An unknown memory card is mounted");
    }
}