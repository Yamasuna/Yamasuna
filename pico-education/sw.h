#ifndef _HEADER_SW
#define _HEADER_SW

#include "pico/stdlib.h"

/* SWの種類 */
typedef enum{
    MODE_SW,
    ENTER_SW,
    SW_TYPE_NUM
} SW_TYPE;

/* SW押下確定状態 */
typedef enum{
    PUSH_NONE,      /* 押下無し */
    PUSH_SHORT,     /* SW短押し */
    PUSH_LONG       /* SW長押し */
} PUSH_STATE;

/* 関数定義 */
void sw_init();                             /* SW設定初期化 */
void sw_input();                            /* SW入力処理 */
PUSH_STATE sw_getstate(SW_TYPE swtype);     /* 指定したSW種類の押下状態を返す */

#endif  //_HEADER_SW
