#include <lvgl.h>
#include "Setup45_TTGO_T_Watch.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

/* Small LVGL buffer: 10 lines */
static lv_color_t buf[TFT_WIDTH * 10];
static lv_disp_draw_buf_t draw_buf;

/* LVGL flush callback (LVGL 8.x) */
void my_disp_flush(lv_disp_drv_t *disp,
                   const lv_area_t *area,
                   lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

static void btn_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        Serial.println("Button clicked");
    }
}

void setup()
{
    Serial.begin(115200);

    // /* Backlight MUST be enabled */
    // pinMode(TFT_BL, OUTPUT);
    // digitalWrite(TFT_BL, HIGH);

    /* TFT init */
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    /* LVGL init */
    lv_init();

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* ---- TEST OBJECT: BUTTON ---- */
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn, 120, 50);
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    /* Button label */
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Click me");
    lv_obj_center(label);
}

void loop()
{
    lv_timer_handler();
    delay(5);
}
