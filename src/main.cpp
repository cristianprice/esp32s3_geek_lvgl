#include <lvgl.h>
#include <TFT_eSPI.h>
#include "Arduino.h"
#include "local_time.h"

static TFT_eSPI tft;

/* --------------------------------------------------------------------------
 * Display configuration
 * -------------------------------------------------------------------------- */

static constexpr uint16_t DISP_HOR_RES = 240;
static constexpr uint16_t DISP_VER_RES = 135;

/* Use partial buffer: 20 lines in RGB565 */
static constexpr size_t BUF_LINES = 20;
static constexpr size_t BUF_PIXELS = DISP_HOR_RES * BUF_LINES;
static constexpr size_t BUF_SIZE = BUF_PIXELS * sizeof(lv_color_t);

static lv_color_t *buf;

/* --------------------------------------------------------------------------
 * LVGL objects
 * -------------------------------------------------------------------------- */

static lv_obj_t *counter_label;
static uint32_t counter = 0;

static uint64_t current_ms = 0;
void lv_tick_task(void *pvParameter)
{
    current_ms = millis();

    while (true)
    {
        uint64_t t = millis() - current_ms;
        lv_tick_inc(t); // increment LVGL tick by elapsed ms
        current_ms = millis();

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/* --------------------------------------------------------------------------
 * Display flush callback
 * -------------------------------------------------------------------------- */

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(reinterpret_cast<uint16_t *>(px_map), w * h, true);
    tft.endWrite();

    lv_display_flush_ready(disp);
}

/* --------------------------------------------------------------------------
 * LVGL timer callback
 * -------------------------------------------------------------------------- */

static void counter_timer_cb(lv_timer_t *timer)
{
    char txt[32];
    snprintf(txt, sizeof(txt), "Count: %lu", counter++);
    lv_label_set_text(counter_label, txt);
}

/* --------------------------------------------------------------------------
 * Setup
 * -------------------------------------------------------------------------- */

void setup()
{

    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    /* Ensure PSRAM is available */
    while (!psramInit())
    {
        Serial.println("Failed to initialize PSRAM");
        delay(100);
    }

    /* Initialize display */
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    /* Initialize LVGL */
    lv_init();

    /* Create display */
    lv_display_t *disp = lv_display_create(DISP_HOR_RES, DISP_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);

    /* REQUIRED for LVGL 9.x */
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

    /* Allocate draw buffer in PSRAM */
    buf = static_cast<lv_color_t *>(ps_malloc(BUF_SIZE));
    while (!buf)
    {
        Serial.println("Failed to allocate LVGL buffer in PSRAM");
        delay(100);
    }

    /* Register draw buffer */
    lv_display_set_buffers(
        disp,
        buf,
        NULL,
        BUF_SIZE,
        LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Create UI */
    lv_obj_t *scr = lv_screen_active();

    counter_label = lv_label_create(scr);
    lv_label_set_text(counter_label, "Count: 0");
    lv_obj_align(counter_label, LV_ALIGN_CENTER, 0, 0);

    /* Update label once per second */
    lv_timer_create(counter_timer_cb, 1000, NULL);

    xTaskCreate(
        lv_tick_task,   // Task function
        "lv_tick_task", // Name
        2048,           // Stack size (bytes)
        NULL,           // Parameters
        1,              // Priority
        NULL            // Task handle
    );
}

/* --------------------------------------------------------------------------
 * Main loop
 * -------------------------------------------------------------------------- */
void loop()
{
    lv_timer_handler();
    yield();
}
