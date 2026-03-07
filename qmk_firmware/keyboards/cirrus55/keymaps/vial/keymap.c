#include QMK_KEYBOARD_H
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {};
#include "pointing_device.h"
#include <math.h>

// 円周率の定義（環境によって無い場合のエラー回避）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 1. オリジナルボタンの定義（画像と同じ順番で並べます）
enum custom_keycodes {
    CPI_SW = SAFE_RANGE, // これが Vialの「User 0」になる
    SCRL_SW,             // これが Vialの「User 1」になる
    ROT_R15,             // これが Vialの「User 2」になる
    ROT_L15,             // これが Vialの「User 3」になる
    SCRL_MO,             // これが Vialの「User 4」になる
};

// 状態を記憶しておく変数
bool is_scroll_mode = false;
uint8_t scrl_divisor = 1; // スクロールの重さ（数値が大きいほど小さく動く）
int16_t rot_angle = 0;    // センサーの回転角度

// CPI（カーソル感度）のリスト: 200 -> 400 -> 800 -> 1600 -> 3200
uint16_t cpi_array[] = {200, 400, 800, 1600, 3200};
uint8_t cpi_index = 1; // 初期値は 400 に設定

// 2. ボタンが押された時のルール
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SCRL_MO:
            is_scroll_mode = record->event.pressed;
            return false;
            
        case CPI_SW:
            if (record->event.pressed) {
                // 配列を順番に切り替え（最後まで行ったら最初に戻る）
                cpi_index = (cpi_index + 1) % 5;
                pointing_device_set_cpi(cpi_array[cpi_index]);
            }
            return false;
            
        case SCRL_SW:
            if (record->event.pressed) {
                // 1 -> 2 -> 3 -> 1... の順でスクロールの重さを切り替え
                scrl_divisor = (scrl_divisor >= 3) ? 1 : scrl_divisor + 1;
            }
            return false;
            
        case ROT_R15:
            if (record->event.pressed) {
                rot_angle = (rot_angle + 15) % 360;
            }
            return false;
            
        case ROT_L15:
            if (record->event.pressed) {
                rot_angle = (rot_angle - 15 + 360) % 360;
            }
            return false;

        default:
            break;
    }
    return true;
}

// 3. トラックボールが動いた時のルール
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // A. センサーの回転補正を適用
    if (rot_angle != 0 && (mouse_report.x != 0 || mouse_report.y != 0)) {
        float theta = rot_angle * M_PI / 180.0;
        int8_t rx = (int8_t)(mouse_report.x * cos(theta) - mouse_report.y * sin(theta));
        int8_t ry = (int8_t)(mouse_report.x * sin(theta) + mouse_report.y * cos(theta));
        mouse_report.x = rx;
        mouse_report.y = ry;
    }

    // B. スクロールモードの適用
    if (is_scroll_mode) {
        // 重さ（scrl_divisor）で割ってスクロール量を調整
        mouse_report.h = mouse_report.x / scrl_divisor;
        mouse_report.v = mouse_report.y / scrl_divisor;
        
        // カーソル自体は動かないようにする
        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    
    return mouse_report;
}
