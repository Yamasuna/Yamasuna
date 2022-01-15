#ifndef _HEADER_MODE
#define _HEADER_MODE

#include <stdio.h>
#include "sw.h"
#include "calendar.h"
#include "thermometer.h"

/* モード状態 */
typedef enum{
    MODE_NORMAL,     /* 通常モード */
    MODE_SETTING     /* 設定モード */
} MODE;

/* 現在設定項目 */
typedef enum{
    MODE_SETTING_YEAR,  /* 設定項目(西暦) */
    MODE_SETTING_MONTH, /* 設定項目(月)   */
    MODE_SETTING_DAY,   /* 設定項目(日)   */
    MODE_SETTING_HOUR,  /* 設定項目(時間) */
    MODE_SETTING_MINUTE,/* 設定項目(分)   */
    MODE_SETTING_NUM    /* 設定項目数     */
} MODE_SETTING_ITEM;

#define MODE_DEFAULT         MODE_NORMAL
#define MODE_SETTING_DEFAULT MODE_SETTING_YEAR

void mode_init(); /* モード遷移初期化処理 */
void mode_main(); /* モード遷移メイン処理 */

#endif  //_HEADER_MODE