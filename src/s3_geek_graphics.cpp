#include "s3_geek_graphics.h"

/* Static instance */
S3GeekGraphics *S3GeekGraphics::instance = nullptr;

S3GeekGraphics::S3GeekGraphics()
    : display(nullptr),
      drawBuf(nullptr),
      currentMs(0)
{
    instance = this;
    for (int i = 0; i < MAX_LINES; i++)
        lineBuffer[i] = nullptr;
}

void S3GeekGraphics::begin()
{
    initDisplay();
    initLVGL();
    createUI();
    startTickTask();

    logQueue = xQueueCreate(16, sizeof(char *));
    configASSERT(logQueue);
}

void S3GeekGraphics::loop()
{
    char *msg;
    bool hasNew = false;

    // Check if there is a message
    while (xQueueReceive(logQueue, &msg, 0) == pdTRUE)
    {
        hasNew = true;

        // If the buffer is full, clear all messages
        if (lineCount >= MAX_LINES)
        {
            // Free all old lines
            for (int i = 0; i < lineCount; i++)
            {
                int idx = (lineStart + i) % MAX_LINES;
                free(lineBuffer[idx]);
                lineBuffer[idx] = nullptr;
            }

            // Reset ring buffer
            lineStart = 0;
            lineCount = 0;

            // Clear LVGL textarea
            lv_textarea_set_text(terminal, "");
        }

        // Add the new message to the buffer
        lineBuffer[(lineStart + lineCount) % MAX_LINES] = msg;
        lineCount++;

        // Append to LVGL terminal
        lv_textarea_add_text(terminal, msg);
        lv_textarea_add_text(terminal, "\n");
    }

    // Scroll to bottom if there was new text
    if (hasNew)
    {
        lv_obj_scroll_to_y(
            terminal,
            lv_obj_get_scroll_bottom(terminal),
            LV_ANIM_OFF);
    }

    // Handle LVGL timers
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

    terminal = lv_textarea_create(scr);
    lv_obj_set_size(terminal, DISP_HOR_RES, DISP_VER_RES);
    lv_obj_align(terminal, LV_ALIGN_CENTER, 0, 0);

    lv_textarea_set_one_line(terminal, false);
    lv_obj_set_scrollbar_mode(terminal, LV_SCROLLBAR_MODE_OFF);
    lv_textarea_set_cursor_click_pos(terminal, false);
    lv_textarea_set_text_selection(terminal, false);
    lv_textarea_set_password_mode(terminal, false);

    /* Hide cursor via style (LVGL 9.x correct method) */
    lv_obj_set_style_opa(terminal, LV_OPA_0, LV_PART_CURSOR);

    /* Font */
    lv_obj_set_style_text_font(
        terminal,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_textarea_set_text(terminal, "Terminal ready...\n");
}

void S3GeekGraphics::postMessage(const char *fmt, ...)
{
    if (!logQueue)
        return;

    char *msg = static_cast<char *>(malloc(LINE_LEN));
    if (!msg)
        return;

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, LINE_LEN, fmt, args);
    va_end(args);

    if (xQueueSend(logQueue, &msg, 0) != pdPASS)
    {
        free(msg); // queue full
    }
}

void S3GeekGraphics::startTickTask()
{
    xTaskCreate(
        lvTickTask,
        "lv_tick_task",
        1024,
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
