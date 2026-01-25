#pragma once

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Arduino.h>

class S3GeekGraphics
{
public:
    /* Display configuration */
    static constexpr uint16_t DISP_HOR_RES = 240;
    static constexpr uint16_t DISP_VER_RES = 135;

    static constexpr size_t BUF_LINES = 20;
    static constexpr size_t BUF_PIXELS = DISP_HOR_RES * BUF_LINES;
    static constexpr size_t BUF_SIZE = BUF_PIXELS * sizeof(lv_color_t);

    S3GeekGraphics();

    void begin();
    void loop();
    void postMessage(const char *fmt, ...);

private:
    QueueHandle_t logQueue;
    static constexpr int MAX_LINES = 7; // max number of lines in terminal
    static constexpr int LINE_LEN = 60; // max characters per line

    char *lineBuffer[MAX_LINES]; // circular buffer of lines
    int lineStart = 0;           // oldest line index
    int lineCount = 0;           // current number of lines

    /* Singleton instance (needed for static callbacks) */
    static S3GeekGraphics *instance;

    /* Hardware */
    TFT_eSPI tft;

    /* LVGL */
    lv_display_t *display;
    lv_color_t *drawBuf;

    uint64_t currentMs;

    lv_obj_t *terminal;

    /* Internal helpers */
    void initDisplay();
    void initLVGL();
    void createUI();
    void startTickTask();

    /* Static callbacks */
    static void displayFlushCb(lv_display_t *disp,
                               const lv_area_t *area,
                               uint8_t *px_map);
    static void lvTickTask(void *param);
};
