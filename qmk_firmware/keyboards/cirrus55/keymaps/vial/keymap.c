#include QMK_KEYBOARD_H
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {};
#include "pointing_device.h"
#include <math.h>

// 状態を記憶しておく変数
bool is_scroll_mode = false;
uint8_t scrl_divisor = 1; // スクロールの重さ（1〜3）

// CPI（カーソル感度）のリストと初期値
uint16_t cpi_array[] = {200, 400, 500, 800, 1600, 3200};
uint8_t cpi_index = 2; // 初期値を 500 に設定

// QMK公式準拠：スクロール量を小数のまま蓄積する変数
float scroll_accumulated_h = 0;
float scroll_accumulated_v = 0;

// ボタンが押された時のルール（Vialの生データで判定）
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case 0x7F54: // Vialの「User 4」キー (スクロールモード)
            is_scroll_mode = record->event.pressed;
            return false;
            
        case 0x7F50: // Vialの「User 0」キー (CPI切替)
            if (record->event.pressed) {
                cpi_index = (cpi_index + 1) % 6;
                pointing_device_set_cpi(cpi_array[cpi_index]);
            }
            return false;
            
        case 0x7F51: // Vialの「User 1」キー (スクロール重さ切替)
            if (record->event.pressed) {
                scrl_divisor = (scrl_divisor >= 3) ? 1 : scrl_divisor + 1;
            }
            return false;

        // ※角度補正(User 2, 3)は一旦無効化するため処理を削除しました

        default:
            break;
    }
    return true;
}

// トラックボールが動いた時のルール
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // A. センサーの実装向き補正: X軸とY軸を入れ替え、かつY軸の進行方向を反転
    int8_t temp_x = mouse_report.x;
    mouse_report.x = mouse_report.y;
    mouse_report.y = -temp_x;

    // B. スクロールモードの適用（QMK公式の蓄積方式）
    if (is_scroll_mode) {
        // 重さ(scrl_divisor)で割った小数を蓄積
        scroll_accumulated_h += (float)mouse_report.x / (float)scrl_divisor;
        scroll_accumulated_v += (float)mouse_report.y / (float)scrl_divisor;

        // 1を超えた整数部分だけをマウスレポートに代入して送信
        mouse_report.h = (int8_t)scroll_accumulated_h;
        mouse_report.v = (int8_t)scroll_accumulated_v;

        // 送信した整数分を蓄積値から引く
        scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
        scroll_accumulated_v -= (int8_t)scroll_accumulated_v;

        // カーソル自体は動かさない
        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    
    return mouse_report;
}
