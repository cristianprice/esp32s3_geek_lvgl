#include <lvgl.h>
#include <TFT_eSPI.h>
#include "Arduino.h"
#include "local_time.h"

static TFT_eSPI tft = TFT_eSPI();
static lv_color_t buf[240 * 135]; // Full screen buffer

#define TFT_X_OFFSET 0
#define TFT_Y_OFFSET 0

static lv_obj_t *hour_arc, *hour_label;
static lv_obj_t *min_arc, *min_label;
static lv_obj_t *sec_arc, *sec_label;

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    tft.startWrite();
    tft.setAddrWindow(area->x1 + TFT_X_OFFSET, area->y1 + TFT_Y_OFFSET, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();
    lv_display_flush_ready(disp);
}

static void create_arc_with_label(lv_obj_t **arc, lv_obj_t **label, lv_coord_t size, lv_color_t color, int max_value, lv_coord_t x, lv_coord_t y)
{
    *arc = lv_arc_create(lv_scr_act());
    lv_obj_set_size(*arc, size, size);
    lv_obj_set_pos(*arc, x, y);
    lv_arc_set_range(*arc, 0, max_value);
    lv_arc_set_rotation(*arc, 270);
    lv_arc_set_bg_angles(*arc, 0, 360);
    lv_obj_remove_flag(*arc, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_style_arc_width(*arc, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(*arc, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(*arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(*arc, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(*arc, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(*arc, LV_OPA_TRANSP, LV_PART_KNOB);

    *label = lv_label_create(*arc);
    lv_label_set_text(*label, "00");
    lv_obj_center(*label);
    lv_obj_set_style_text_font(*label, LV_FONT_DEFAULT, 0);
}

void create_lvgl_view()
{
    const lv_coord_t arc_size = 70;
    const lv_coord_t spacing = 10;

    /* Create a container that will hold all arcs */
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(cont);

    /* Horizontal layout */
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        cont,
        LV_FLEX_ALIGN_CENTER, /* main axis */
        LV_FLEX_ALIGN_CENTER, /* cross axis */
        LV_FLEX_ALIGN_CENTER  /* track align */
    );

    lv_obj_set_style_pad_gap(cont, spacing, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);

    /* Create arcs inside container */
    create_arc_with_label(&hour_arc, &hour_label, arc_size,
                          lv_palette_main(LV_PALETTE_GREEN), 11,
                          0, 0);
    lv_obj_set_parent(hour_arc, cont);

    create_arc_with_label(&min_arc, &min_label, arc_size,
                          lv_palette_main(LV_PALETTE_BLUE), 59,
                          0, 0);
    lv_obj_set_parent(min_arc, cont);

    create_arc_with_label(&sec_arc, &sec_label, arc_size,
                          lv_palette_main(LV_PALETTE_RED), 59,
                          0, 0);
    lv_obj_set_parent(sec_arc, cont);
}

void setup()
{
    Serial.begin(115200);
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    lv_init();
    lv_display_t *disp = lv_display_create(240, 135);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_FULL);

    if (wifi_connect())
    {
        setup_local_time();
    }

    create_lvgl_view();
}

void loop()
{
    static uint32_t last_ms = 0;
    static uint32_t last_update = 0;

    uint32_t now = millis();

    /* Advance LVGL internal time */
    lv_tick_inc(now - last_ms);
    last_ms = now;

    /* Update UI once per second using local time */
    if (now - last_update >= 1000)
    {
        last_update = now;

        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            uint8_t sec = timeinfo.tm_sec;
            uint8_t min = timeinfo.tm_min;
            uint8_t hour = timeinfo.tm_hour % 12;

            lv_arc_set_value(hour_arc, hour);
            lv_arc_set_value(min_arc, min);
            lv_arc_set_value(sec_arc, sec);

            char buf[8];

            snprintf(buf, sizeof(buf), "%02d", hour == 0 ? 12 : hour);
            lv_label_set_text(hour_label, buf);

            snprintf(buf, sizeof(buf), "%02d", min);
            lv_label_set_text(min_label, buf);

            snprintf(buf, sizeof(buf), "%02d", sec);
            lv_label_set_text(sec_label, buf);
        }
        else
        {
            /* Optional: indicate time not available */
            Serial.println("Local time not available");
        }
    }

    lv_timer_handler();
    delay(5);
}
