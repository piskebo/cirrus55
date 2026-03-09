#include QMK_KEYBOARD_H
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {};
#include "pointing_device.h"

// 状態を記憶しておく変数
bool is_scroll_mode = false;

// 左右の感度補正用倍率
#define X_SENSITIVITY 2.5

// ■ スクロールの速度調整（数値を大きくするとさらに遅くなる）
#define SCROLL_DIVISOR_X 16.0
#define SCROLL_DIVISOR_Y 16.0

// QMK公式準拠：スクロール蓄積用変数
float scroll_accumulated_h = 0;
float scroll_accumulated_v = 0;

// ■ 左右移動の「もっさり」解消用：端数蓄積変数を追加
static float cursor_x_accumulated = 0;

// CPI（カーソル感度）のリストと初期値
uint16_t cpi_array[] = {200, 400, 500, 800, 1600, 3200};
uint8_t cpi_index = 2; // 初期値 500

// ボタンが押された時のルール
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_F24: // スクロール起動キー
            is_scroll_mode = record->event.pressed;
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

// トラックボールが動いた時のルール
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // A. 軸の入れ替えとY軸反転 ＋ 左右(X軸)の感度増幅
    int8_t temp_x = mouse_report.x;
    
    // ■ X軸のデータに倍率を掛け、切り捨てられていた端数を蓄積する
    cursor_x_accumulated += (float)mouse_report.y * X_SENSITIVITY;
    mouse_report.x = (int8_t)cursor_x_accumulated;
    cursor_x_accumulated -= (int8_t)cursor_x_accumulated; // 適用した整数分だけ引く
    
    mouse_report.y = -temp_x;

    // B. スクロールモードの適用
    if (is_scroll_mode) {
        // スクロール量の計算（速度調整の割り算を適用）
        scroll_accumulated_h += (float)mouse_report.x / SCROLL_DIVISOR_X;
        
        // 上下スクロールの方向を反転させるためマイナスを追加
        scroll_accumulated_v += -(float)mouse_report.y / SCROLL_DIVISOR_Y;

        mouse_report.h = (int8_t)scroll_accumulated_h;
        mouse_report.v = (int8_t)scroll_accumulated_v;

        scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
        scroll_accumulated_v -= (int8_t)scroll_accumulated_v;

        // カーソル自体は動かさない
        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    
    return mouse_report;
}
