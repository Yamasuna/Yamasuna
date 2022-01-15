#include "thermometer.h"

static float tem_temp_cel;          /* 温度(℃)  */

struct repeating_timer min_timer;   /* 1分タイマー     */

/* 温度測定を行う */
void tem_measure_temp()
{
    uint16_t  adc_val;
    float     vr;
    float     rt;
    float     measure_temp;

    /* ADC値を取得する */
    adc_val = adc_read();

    /* ADC値は12bit */
    vr = THERMO_VCC * (float)adc_val / (1 << 12);
    rt = (THERMO_RESISTOR) * (vr / (THERMO_VCC - vr));
    measure_temp = 1 / (((log(rt / (THERMO_RESISTOR))) / THERMO_MATERIAL_CONSTANT)
                        + (1 / (THERMO_KELVIN_TEMP)));

    tem_temp_cel = measure_temp - 273.15f;
}

/* 1毎のタイマー割込み発生時の処理 */
bool repeating_timer1min_callback(struct repeating_timer *t)
{
    /* 温度測定を行う */
    tem_measure_temp();
    return true;
}

/* 温度測定の初期化を行う */
void tem_init()
{
    adc_init();
    adc_gpio_init(THERMO_PIN);
    adc_select_input(THERMO_ADC_CHANNEL);

    /* 初回温度測定 */
    tem_measure_temp();

    /* 温度取得開始 */
    tem_start();
}

/* 温度を取得する(℃)    */
float tem_gettemp_cel()
{
    return tem_temp_cel;
}

/* 温度取得用タイマーを開始する */
void tem_start()
{
    /* タイマーコールバック設定(1分毎) */
    add_repeating_timer_ms(60*1000, repeating_timer1min_callback, NULL, &min_timer);
}

/* 温度取得用タイマーを止める */
void tem_stop()
{
    cancel_repeating_timer(&min_timer);
}
