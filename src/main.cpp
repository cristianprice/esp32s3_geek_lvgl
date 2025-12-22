#include <lvgl.h>
#include <TFT_eSPI.h>
#include "Arduino.h"

void create_lvgl_view();
static TFT_eSPI tft = TFT_eSPI();

/* LVGL draw buffer */
static lv_color_t buf[240 * 20];

/* ST7789 offsets for 240x135 panels */
#define TFT_X_OFFSET 0
#define TFT_Y_OFFSET 0

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(
        area->x1 + TFT_X_OFFSET,
        area->y1 + TFT_Y_OFFSET,
        w,
        h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();

    lv_display_flush_ready(disp);
}

/* List item event */
static void list_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
        const char *txt = lv_label_get_text(lv_obj_get_child(btn, 0));
        Serial.print("Selected: ");
        Serial.println(txt);
    }
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
    lv_display_set_buffers(
        disp,
        buf,
        NULL,
        sizeof(buf),
        LV_DISPLAY_RENDER_MODE_PARTIAL);

    create_lvgl_view();
}

void create_lvgl_view()
{
    static lv_subject_t value;
    lv_subject_init_int(&value, 30);

    lv_obj_t *arc = lv_arc_create(lv_screen_active());
    lv_obj_set_size(arc, 100, 100);
    lv_obj_center(arc);
    lv_arc_bind_value(arc, &value);

    lv_obj_set_style_arc_opa(arc, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xffffff), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(arc, lv_color_hex(0xffffff), LV_PART_KNOB);
    lv_obj_set_style_shadow_width(arc, 15, LV_PART_KNOB);
    lv_obj_set_style_shadow_opa(arc, LV_OPA_40, LV_PART_KNOB);
    lv_obj_set_style_shadow_offset_y(arc, 5, LV_PART_KNOB);

    lv_obj_t *label = lv_label_create(arc);
    lv_obj_center(label);
    lv_label_bind_text(label, &value, "%d °C");
    lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
}

void loop()
{
    Serial.println("LVGL Tick");
    lv_timer_handler();
    delay(5);
}
