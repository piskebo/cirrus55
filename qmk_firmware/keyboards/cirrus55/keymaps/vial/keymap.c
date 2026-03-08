#include QMK_KEYBOARD_H
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {};
#include "pointing_device.h"

// 状態を記憶しておく変数
bool is_scroll_mode = false;

// ■ 左右の感度補正用倍率（まずは2倍でテスト）
#define X_SENSITIVITY 2.0

// QMK公式準拠：スクロール蓄積用変数
float scroll_accumulated_h = 0;
float scroll_accumulated_v = 0;

// CPI（カーソル感度）のリストと初期値
uint16_t cpi_array[] = {200, 400, 500, 800, 1600, 3200};
uint8_t cpi_index = 2; // 初期値 500

// ボタンが押された時のルール
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // ■ 確実な解決策：Vialの「F24」キーをスクロール起動キーとして代用する
        case KC_F24: 
            is_scroll_mode = record->event.pressed;
            return false;
            
        case 0x7F50: // User 0 (もしこれも効かない場合は後日 F23 等に変更します)
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
    
    // ■ 左右の動きが鈍い問題の解決：X軸のデータに倍率（2.0倍）を掛ける
    float adjusted_x = (float)mouse_report.y * X_SENSITIVITY;
    mouse_report.x = (int8_t)adjusted_x;
    
    mouse_report.y = -temp_x;

    // B. スクロールモードの適用
    if (is_scroll_mode) {
        // 動きをそのままスクロールとして蓄積（重さ機能は一旦外し、確実な動作を優先）
        scroll_accumulated_h += (float)mouse_report.x / 1.0;
        scroll_accumulated_v += (float)mouse_report.y / 1.0;

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
