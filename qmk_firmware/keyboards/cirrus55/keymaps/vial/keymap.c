#include QMK_KEYBOARD_H
#include "pointing_device.h"

// ■ 55キーレイアウトの定義（ビルドエラー回避用）
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};

static bool is_scroll_mode = false;

// ■ 左右の感度補正（もっさり解消用）
#define X_SENSITIVITY 3.0
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
    // 1. 物理的な入力を取得
    int8_t raw_x = mouse_report.x;
    int8_t raw_y = mouse_report.y;

    // 2. 左右（X軸）の追従性改善：端数蓄積処理
    cursor_x_accumulated += (float)raw_y * X_SENSITIVITY;
    mouse_report.x = (int8_t)cursor_x_accumulated;
    cursor_x_accumulated -= (int8_t)cursor_x_accumulated;

    // 3. 上下（Y軸）：元の安定した状態（90度入れ替え）
    mouse_report.y = -raw_x;

    // 4. スクロールモード処理（元の正常な向き）
    if (is_scroll_mode) {
        scroll_accumulated_h += (float)mouse_report.x /
