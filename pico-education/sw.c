#include <stdio.h>
#include "sw.h"

#define SW_NUM                  SW_TYPE_NUM     /* 使用するスイッチの個数     */
#define SW_MODE_GPIO            14              /* MODEスイッチ用GPIOピン番号 */
#define SW_ENTER_GPIO           15              /* ENTERスイッチ用GPIOピン番号 */
#define SW_INPUT_INTERVAL_MS    10              /* SW状態取得周期(ms)         */

#define SW_PUSH_COUNTER_TIMEOUT 500             /* SW押下時間タイムアウト */
#define SW_PUSH_COUNTER_SHORT   490             /* SW押下時間10カウント経過(短押し) */
#define SW_PUSH_COUNTER_LONG    200             /* SW押下時間300カウント経過(長押し) */

/* SW入力回路によってtrue/falseは見直すこと */
#define SW_PUSH                false            /* SW押下状態 */
#define SW_RELEASE             true             /* SW離し状態 */

/* グローバル変数 */
struct repeating_timer ms_timer;                /* msタイマー */
static uint16_t sw_timer_cnt[SW_NUM];           /* SW押下時間カウンタ(1カウント=SW状態取得周期) */
static PUSH_STATE sw_state[SW_NUM];             /* SW押下確定状態 */
static uint sw_gpio[SW_NUM] = {SW_MODE_GPIO,
                               SW_ENTER_GPIO};  /* SW用GPIOピン番号 */

/* SW状態取得周期毎のタイマー割込み発生時の処理 */
bool repeating_timer_sw_callback(struct repeating_timer *t)
{
    uint8_t idx_sw;

    /* SW押下時間カウンタを更新 */
    for(idx_sw = 0; idx_sw < SW_NUM; idx_sw++)
    {
        if(sw_timer_cnt[idx_sw] != 0)
        {
            sw_timer_cnt[idx_sw]--;
        }
        else
        {
            /* 何もしない */
        }
    }

    return true;
}

/* SW設定初期化 */
void sw_init()
{
    uint8_t idx_sw;

    /* スイッチ入力用GPIO設定 */
    for(idx_sw = 0; idx_sw < SW_NUM; idx_sw++)
    {
        gpio_init(sw_gpio[idx_sw]);
        gpio_set_dir(sw_gpio[idx_sw], GPIO_IN);
        gpio_pull_up(sw_gpio[idx_sw]);
    }

    /* SW状態初期化 */
    for(idx_sw = 0; idx_sw < SW_NUM; idx_sw++)
    {
        sw_timer_cnt[idx_sw] = 0;
        sw_state[idx_sw] = PUSH_NONE;
    }

    /* タイマーコールバック設定 */
    add_repeating_timer_ms(SW_INPUT_INTERVAL_MS, repeating_timer_sw_callback, NULL, &ms_timer);
}

/* SW入力処理 */
void sw_input()
{
    static bool gpio_sw_input_old[SW_NUM] = {SW_RELEASE}; /* SW押下状態生値(前回) */
    static bool gpio_sw_input_new[SW_NUM] = {SW_RELEASE}; /* SW押下状態生値(今回) */
    uint8_t idx_sw;

    /* SW確定状態初期化 */
    for(idx_sw = 0; idx_sw < SW_NUM; idx_sw++)
    {
        sw_state[idx_sw] = PUSH_NONE;
    }

    /* SW状態を取得し、確定状態を判定する */
    for(idx_sw = 0; idx_sw < SW_NUM; idx_sw++)
    {
        /* SW状態を取得 */
        gpio_sw_input_new[idx_sw] = gpio_get( sw_gpio[idx_sw] );

        /* SW押下状態判定 */
        if( (gpio_sw_input_old[idx_sw] == SW_RELEASE) &&
            (gpio_sw_input_new[idx_sw] == SW_PUSH) )
        {
            /* 押下なし→押下ありに変化した時(SWを押下した時) */
            sw_timer_cnt[idx_sw] = SW_PUSH_COUNTER_TIMEOUT;
        }
        else if ( (gpio_sw_input_old[idx_sw] == SW_PUSH) &&
                  (gpio_sw_input_new[idx_sw] == SW_RELEASE) )
        {
            /* 押下あり→押下なしに変化した時(SWを離した時) */
            if( (sw_timer_cnt[idx_sw] > 0) &&
                (sw_timer_cnt[idx_sw] <= SW_PUSH_COUNTER_SHORT) )
            {
                /* SW押下タイマカウントが短押し判定時間を経過しており、
                　長押し判定時間未満の場合、短押しと判定する */
                sw_state[idx_sw] = PUSH_SHORT;
                sw_timer_cnt[idx_sw] = 0;
            }
            else
            {
                /* SW離しとして確定する */
                sw_state[idx_sw] = PUSH_NONE;
            }
        }
        else if (gpio_sw_input_old[idx_sw] == SW_PUSH &&
                 gpio_sw_input_new[idx_sw] == SW_PUSH)
        {
            /* 押下あり継続中(SW押下中) */
            if ( (sw_timer_cnt[idx_sw] > 0) &&
                 (sw_timer_cnt[idx_sw] <= SW_PUSH_COUNTER_LONG) )
            {
                /* 長押し分のタイマカウント満了として長押し確定 */
                sw_state[idx_sw] = PUSH_LONG;
                sw_timer_cnt[idx_sw] = 0;
            }
            else
            {
                /* 何もしない */
            }
        }
        else
        {
            /* 押下なし継続中は何もしない */
        }

        /* SW押下状態更新 */
        gpio_sw_input_old[idx_sw] = gpio_sw_input_new[idx_sw];
    }
}

/* 指定したSW種類の押下確定状態を返す */
PUSH_STATE sw_getstate(SW_TYPE swtype)
{
    return sw_state[swtype];
}
