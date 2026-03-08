#include QMK_KEYBOARD_H
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {};
#include "pointing_device.h"
#include <math.h>

// 円周率の定義
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 状態を記憶しておく変数
bool is_scroll_mode = false;
int16_t rot_angle = 0;  // 補正を削除（0度スタート）に変更

// CPI（カーソル感度）のリストと初期値
uint16_t cpi_array[] = {200, 400, 500, 800, 1600, 3200};
uint8_t cpi_index = 2; // 初期値を 500 に設定

// ボタンが押された時のルール（生データで直接判定）
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
            
        case 0x7F52: // Vialの「User 2」キー (角度+15)
            if (record->event.pressed) {
                rot_angle = (rot_angle + 15) % 360;
            }
            return false;
            
        case 0x7F53: // Vialの「User 3」キー (角度-15)
            if (record->event.pressed) {
                rot_angle = (rot_angle - 15 + 360) % 360;
            }
            return false;

        default:
            break;
    }
    return true;
}

// トラックボールが動いた時のルール
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // A. センサーの実装向き補正: X軸とY軸を入れ替え、かつY軸を反転させる
    int8_t temp_x = mouse_report.x;
    mouse_report.x = mouse_report.y;
    mouse_report.y = -temp_x;

    // B. センサーの回転補正を適用
    if (rot_angle != 0 && (mouse_report.x != 0 || mouse_report.y != 0)) {
        float theta = rot_angle * M_PI / 180.0;
        int8_t rx = (int8_t)(mouse_report.x * cos(theta) - mouse_report.y * sin(theta));
        int8_t ry = (int8_t)(mouse_report.x * sin(theta) + mouse_report.y * cos(theta));
        mouse_report.x = rx;
        mouse_report.y = ry;
    }

    // C. スクロールモードの適用（割り算処理を外し、ダイレクトに出力）
    if (is_scroll_mode) {
        mouse_report.h = mouse_report.x;
        mouse_report.v = mouse_report.y;
        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    
    return mouse_report;
}
