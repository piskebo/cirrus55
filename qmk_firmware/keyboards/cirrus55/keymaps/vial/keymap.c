// --- 変数定義エリア ---
static bool is_scroll_mode = false;

// 左右の感度補正（もっさり解消用）
#define X_SENSITIVITY 3.0
#define SCROLL_DIVISOR_X 10.0
#define SCROLL_DIVISOR_Y 10.0

static float scroll_accumulated_h = 0;
static float scroll_accumulated_v = 0;
static float cursor_x_accumulated = 0;

// --- 処理関数 ---
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
