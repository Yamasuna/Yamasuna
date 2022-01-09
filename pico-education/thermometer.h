#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define THERMO_PIN          28          /* サーミスタが接続されたピン番号 */
#define THERMO_INTERVAL     60*1000     /* 温度取得周期(ms)             */
#define THERMO_ADC_CHANNEL  2           /* ADC入力チャンネル番号(GPIO28) */

#define THERMO_VCC          3.3         /* 供給電圧(V)                  */
#define THERMO_RESISTOR     10*1000     /* サーミスタ抵抗値(Ohm)        */
#define THERMO_KELVIN_TEMP  273.15+25   /* ケルビン温度(K)              */
#define THERMO_MATERIAL_CONSTANT  3950  /* サーミスタ物質定数            */

/* 関数定義 */
void tem_init();            /* 温度測定の初期化を行う */
float tem_gettemp_cel();    /* 温度を取得する(℃)    */
