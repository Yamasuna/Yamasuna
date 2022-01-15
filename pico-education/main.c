#include <stdio.h>
#include "calendar.h"
#include "thermometer.h"
#include "sw.h"
#include "mode.h"
#include "display.h"
#include "pico/stdlib.h"

int main(void)
{
    /* USB使用の為の初期化 */
    stdio_init_all();

    /* SW初期化 */
    sw_init();

    /* モード遷移初期化 */
    mode_init();

    /* LCD初期化 */
    lcd_init();

    /* カレンダー初期化 */
    cal_init();

    /* 温度測定初期化 */
    tem_init();

    /* メインループ */
    while(1)
    {        
        /* SW入力処理 */
        sw_input();

        /* モード状態遷移 */
        mode_main();

        /* LCD表示メイン処理 */
        display_main();

        sleep_ms(10);
    }

    return 0;
}
