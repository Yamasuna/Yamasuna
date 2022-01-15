#include "mode.h"

MODE mode_state;                                  /* モード状態 */
MODE_SETTING_ITEM mode_setting_item;              /* 設定モード時の現在設定項目 */

bool isdisp_setting_item[MODE_SETTING_NUM];       /* 設定項目表示可否 */
struct repeating_timer ms_timer_disp_setting;     /* 0.5秒タイマー     */

/* 1秒毎のタイマー割込み発生時の処理 */
bool repeating_timer500ms_mode_callback(struct repeating_timer *t)
{
    isdisp_setting_item[mode_setting_item] = !isdisp_setting_item[mode_setting_item];
    return true;
}

/* カレンダー設定処理 */
void mode_calendar_setting()
{
    PUSH_STATE sw_mode;
    PUSH_STATE sw_enter;
    MODE_SETTING_ITEM mode_setting_item_old;      /* 設定モード時の現在設定項目(更新前) */

    /* スイッチ状態を取得 */
    sw_mode = sw_getstate(MODE_SW);
    sw_enter = sw_getstate(ENTER_SW);

    /* カレンダー設定モード時にMODE SW短押し */
    if (sw_mode == PUSH_SHORT)
    {
        /* 更新前の設定項目を保存 */
        /* 割込みによって更新前の設定項目が更新されたままとなってしまうことを防止する為 */
        mode_setting_item_old = mode_setting_item;

        /* 設定項目を更新する */
        mode_setting_item = (mode_setting_item + 1) % MODE_SETTING_NUM;

        /* 更新前の設定項目は表示設定にする */
        isdisp_setting_item[mode_setting_item_old] = true;
    }

    /* カレンダー設定モード時にENTER_SW短押し */
    if (sw_enter == PUSH_SHORT)
    {
        /* ENTER_SWを短押しする毎に現在の設定項目の設定値を1つ進める */
        switch(mode_setting_item)
        {
            case MODE_SETTING_YEAR:
                /* 設定項目が西暦の場合 */
                cal_increment_year();
                break;
            case MODE_SETTING_MONTH:
                /* 設定項目が月の場合 */
                cal_increment_month();
                break;
            case MODE_SETTING_DAY:
                /* 設定項目が日の場合 */
                cal_increment_day();
                break;
            case MODE_SETTING_HOUR:
                /* 設定項目が時間の場合 */
                cal_increment_hour();
                break;
            case MODE_SETTING_MINUTE:
                /* 設定項目が分の場合 */
                cal_increment_minute();
                break;
            default:
                /* 想定しない設定値 */
                break;
        }
    }
}

/* モード毎の処理 */
static void mode_proc()
{
    switch(mode_state)
    {
        case MODE_SETTING:
            /* カレンダー設定処理 */
            mode_calendar_setting();
            break;
        default:
            /* 特に処理無し */
            break;
    }
}

/* モード設定中タイマースタート */
static void mode_starttimer()
{
    /* タイマーコールバック設定(0.5秒毎) */
    add_repeating_timer_ms(500, repeating_timer500ms_mode_callback, NULL, &ms_timer_disp_setting);
}

/* モード設定中タイマースタート */
static void mode_stoptimer()
{
    uint8_t idx_item;

    /* タイマーコールバック設定(0.5秒毎) */
    cancel_repeating_timer(&ms_timer_disp_setting);

    /* 全ての表示項目を表示ありに設定 */
    for(idx_item = 0; idx_item < MODE_SETTING_NUM; idx_item++)
    {
        isdisp_setting_item[idx_item] = true;
    }
}

/* モード遷移初期化処理 */
void mode_init()
{
    uint8_t idx_item;

    mode_state = MODE_DEFAULT;
    mode_setting_item = MODE_SETTING_DEFAULT;

    /* 全ての表示項目を表示ありに設定 */
    for(idx_item = 0; idx_item < MODE_SETTING_NUM; idx_item++)
    {
        isdisp_setting_item[idx_item] = true;
    }

    /* GPIO初期化 */
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
}

/* モード遷移メイン処理 */
void mode_main()
{
    PUSH_STATE sw_mode;

    /* モードスイッチ状態を取得 */
    sw_mode = sw_getstate(MODE_SW);

    if ( (mode_state == MODE_NORMAL) &&
         (sw_mode == PUSH_LONG) )
    {
        /* 通常モードでモードスイッチ長押し発生時は設定モードに遷移 */
        mode_state = MODE_SETTING;
        mode_setting_item = MODE_SETTING_DEFAULT;
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        cal_stopcalendar(); /* カレンダー停止 */
        tem_stop();         /* 温度取得停止 */
        mode_starttimer();  /* モード設定中タイマースタート */
    }
    else if ( (mode_state == MODE_SETTING) &&
              (sw_mode == PUSH_LONG) )
    {
        /* 設定モードでモードスイッチ長押し発生時は通常モードに遷移 */
        mode_state = MODE_NORMAL;
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        cal_startcalendar();    /* カレンダー開始 */
        tem_start();            /* 温度取得開始 */
        mode_stoptimer();       /* モード設定中タイマー停止 */
    }
    else
    {
        /* モード遷移なし */
    }

    /* モード毎の処理 */
    mode_proc();
}
