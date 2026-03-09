
#include QMK_KEYBOARD_H
#include "pointing_device.h"
#include <math.h> // 回転計算に使用

static bool is_scroll_mode = false;

// ■ 角度補正（11時方向を真上にしたい場合は 30.0 前後、まずはご指定の 10.0 から）
#define ROTATION_ANGLE 10.0

// 左右の感度補正（3.0を維持）
#define X_SENSITIVITY 3.0

// スクロール速度（10.0を維持）
#define SCROLL_DIVISOR_X 10.0
#define SCROLL_DIVISOR_Y 10.0

static float scroll_accumulated_h = 0;
static float scroll_accumulated_v = 0;
static float cursor_x_accumulated = 0;

uint16_t cpi_array[] = {200, 400, 500, 800, 1600, 3200};
uint8_t cpi_index = 2; 

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case 0x7F54: 
        case KC_F24:
            is_scroll_mode = record->event.pressed;
            scroll_accumulated_h = 0;
            scroll_accumulated_v = 0;
            return false;
        case 0x7F50: 
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
    // 1. 物理的な入力を取得（現在の90度入れ替え込み）
    float raw_x = mouse_report.y;
    float raw_y = -mouse_report.x;

    // 2. 角度補正の計算
    float rad = ROTATION_ANGLE * M_PI / 180.0;
    float cos_a = cos(rad);
    float sin_a = sin(rad);

    // 回転行列を適用（手の傾きを相殺）
    float rotated_x = raw_x * cos_a + raw_y * sin_a;
    float rotated_y = -raw_x * sin_a + raw_y * cos_a;

    // 3. 左右（X軸）の感度増幅と端数蓄積
    cursor_x_accumulated += rotated_x * X_SENSITIVITY;
    mouse_report.x = (int8_t)cursor_x_accumulated;
    cursor_x_accumulated -= (int8_t)cursor_x_accumulated;

    // 4. 上下（Y軸）の適用
    mouse_report.y = (int8_t)rotated_y;

    // スクロールモードの処理
    if (is_scroll_mode) {
        scroll_accumulated_h += (float)mouse_report.x / SCROLL_DIVISOR_X;
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
