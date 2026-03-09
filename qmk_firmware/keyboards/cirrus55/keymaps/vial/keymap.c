#include QMK_KEYBOARD_H
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {};
#include "pointing_device.h"

static bool is_scroll_mode = false;

// 左右の感度補正倍率
#define X_SENSITIVITY 2.5

// ■ 8.0（速い）と 16.0（動かない）の中間である 10.0 を設定
#define SCROLL_DIVISOR_X 10.0
#define SCROLL_DIVISOR_Y 10.0

static float scroll_accumulated_h = 0;
static float scroll_accumulated_v = 0;

uint16_t cpi_array[] = {200, 400, 500, 800, 1600, 3200};
uint8_t cpi_index = 2; 

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case 0x7F54: // User 4
        case KC_F24: // F24
            is_scroll_mode = record->event.pressed;
            scroll_accumulated_h = 0;
            scroll_accumulated_v = 0;
            return false;
            
        case 0x7F50: // User 0 (CPI切替)
            if (record->event.pressed) {
                cpi_index = (cpi_index + 1) % 6;
                pointing_device_set_cpi(cpi_array[cpi_index]);
            }
            return false;

        default:
            break;
    }
    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    int8_t temp_x = mouse_report.x;
    
    // X軸（左右）のデータ増幅
    float adjusted_x = (float)mouse_report.y * X_SENSITIVITY;
    mouse_report.x = (int8_t)adjusted_x;
    mouse_report.y = -temp_x;

    if (is_scroll_mode) {
        scroll_accumulated_h += (float)mouse_report.x / SCROLL_DIVISOR_X;
        // 反転修正済みの上下スクロール
        scroll_accumulated_v += (float)mouse_report.y / SCROLL_DIVISOR_Y;

        mouse_report.h = (int8_t)scroll_accumulated_h;
        mouse_report.v = (int8_t)scroll_accumulated_v;

        scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
        scroll_accumulated_v -= (int8_t)scroll_accumulated_v;

        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    
    return mouse_report;
}
