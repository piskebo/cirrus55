#include QMK_KEYBOARD_H
#include "pointing_device.h"
#include <math.h>

// ■ M_PI（円周率）が定義されていない場合のための安全策
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ■ 重要：Vial用であっても、コンパイルを通すために空の配列は許されません。
// ここに実際のキーレイアウトを記述するか、最低1レイヤー分は中身が必要です。
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = {
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},
        {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS}
    }
};

static bool is_scroll_mode = false;

// 角度補正（10.0度反時計回り）
#define ROTATION_ANGLE 10.0
// 左右感度
#define X_SENSITIVITY 3.0
// スクロール感度
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
    // 1. 軸の入れ替え（物理的な配置に合わせる）
    float raw_x = mouse_report.y;
    float raw_y = -mouse_report.x;

    // 2. 角度回転の計算（10度）
    float rad = ROTATION_ANGLE * M_PI / 180.0;
    float cos_a = cos(rad);
    float sin_a = sin(rad);

    float rotated_x = raw_x * cos_a + raw_y * sin_a;
    float rotated_y = -raw_x * sin_a + raw_y * cos_a;

    // 3. 左右（X軸）の追従性改善：端数蓄積
    cursor_x_accumulated += rotated_x * X_SENSITIVITY;
    mouse_report.x = (int8_t)cursor_x_accumulated;
    cursor_x_accumulated -= (int8_t)cursor_x_accumulated;

    // 4. 上下（Y軸）
    mouse_report.y = (int8_t)rotated_y;

    // 5. スクロールモード処理
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
