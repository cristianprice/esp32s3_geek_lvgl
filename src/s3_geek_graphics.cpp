#include "s3_geek_graphics.h"

/* Static instance */
S3GeekGraphics *S3GeekGraphics::instance = nullptr;

S3GeekGraphics::S3GeekGraphics()
    : display(nullptr),
      drawBuf(nullptr),
      counterLabel(nullptr),
      counter(0),
      currentMs(0)
{
    instance = this;
}

void S3GeekGraphics::begin()
{
    initDisplay();
    initLVGL();
    createUI();
    startTickTask();
}

void S3GeekGraphics::loop()
{
    lv_timer_handler();
}

/* --------------------------------------------------------------------------
 * Initialization
 * -------------------------------------------------------------------------- */

void S3GeekGraphics::initDisplay()
{
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
}

void S3GeekGraphics::initLVGL()
{
    lv_init();

    display = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
    lv_display_set_flush_cb(display, displayFlushCb);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);

    drawBuf = static_cast<lv_color_t *>(ps_malloc(BUF_SIZE));
    while (!drawBuf)
    {
        Serial.println("Failed to allocate LVGL buffer in PSRAM");
        delay(100);
    }

    lv_display_set_buffers(
        display,
        drawBuf,
        nullptr,
        BUF_SIZE,
        LV_DISPLAY_RENDER_MODE_PARTIAL);
}

void S3GeekGraphics::createUI()
{
    lv_obj_t *scr = lv_screen_active();

    counterLabel = lv_label_create(scr);
    lv_label_set_text(counterLabel, "Count: 0");
    lv_obj_align(counterLabel, LV_ALIGN_CENTER, 0, 0);

    lv_timer_create(counterTimerCb, 1000, nullptr);
}

void S3GeekGraphics::startTickTask()
{
    xTaskCreate(
        lvTickTask,
        "lv_tick_task",
        2048,
        nullptr,
        1,
        nullptr);
}

/* --------------------------------------------------------------------------
 * Static callbacks
 * -------------------------------------------------------------------------- */

void S3GeekGraphics::displayFlushCb(lv_display_t *disp,
                                    const lv_area_t *area,
                                    uint8_t *px_map)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    instance->tft.startWrite();
    instance->tft.setAddrWindow(area->x1, area->y1, w, h);
    instance->tft.pushColors(
        reinterpret_cast<uint16_t *>(px_map),
        w * h,
        true);
    instance->tft.endWrite();

    lv_display_flush_ready(disp);
}

void S3GeekGraphics::counterTimerCb(lv_timer_t *timer)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "Count: %lu", instance->counter++);
    lv_label_set_text(instance->counterLabel, buf);
}

void S3GeekGraphics::lvTickTask(void *param)
{
    instance->currentMs = millis();

    while (true)
    {
        uint64_t now = millis();
        lv_tick_inc(now - instance->currentMs);
        instance->currentMs = now;

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
