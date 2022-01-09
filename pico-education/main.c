#include <stdio.h>
#include "lcd_1602a.h"
#include "calendar.h"
#include "thermometer.h"
#include "pico/stdlib.h"

#define BUTTON_GPIO     14

/* カレンダー情報を表示する */
void disp_calendar()
{
    calendar_t *cal;
    char str_year[4];
    char str_month[2];
    char str_day[2];
    char str_hour[2];
    char str_minute[2];
    char str_second[2];
    char *str_weekofdays;
    
    /* カレンダー情報取得 */
    cal = cal_getinfo();

    /* カレンダー情報を文字データに変換 */
    sprintf(str_year, "%04d", cal->cal_date.year);
    sprintf(str_month, "%02d", cal->cal_date.month);
    sprintf(str_day, "%02d", cal->cal_date.day);

    sprintf(str_hour, "%02d", cal->cal_time.hour);
    sprintf(str_minute, "%02d", cal->cal_time.minute);
    sprintf(str_second, "%02d", cal->cal_time.second);

    /* 曜日情報を取得 */
    str_weekofdays = cal_getweekofdays_str(cal->cal_date.dayofweek);

    /* 日付情報を表示(LCD1行目) */
    lcd_setcursor(LCD_LINE_1, 0);
    lcd_print(str_year);
    lcd_print("/");
    lcd_print(str_month);
    lcd_print("/");
    lcd_print(str_day);

    /* 曜日情報を表示(LCD1行目) */
    lcd_print(str_weekofdays);

    /* 時刻情報を表示(LCD2行目) */
    lcd_setcursor(LCD_LINE_2, 0);
    lcd_print(str_hour);
    lcd_print(":");
    lcd_print(str_minute);
    lcd_print(":");
    lcd_print(str_second);

    lcd_cursorhome();
}

/* 温度情報を表示する */
void disp_temperature()
{
    float temp;
    char str_temp[4];

    /* 温度情報を取得して文字データに変換 */
    temp = tem_gettemp_cel();
    sprintf(str_temp, "%.1f", temp);

    lcd_setcursor(LCD_LINE_2, 10);
    lcd_print(str_temp);
    lcd_print("C");

    lcd_cursorhome();
}

int main(void)
{
    bool gpio_sw_input;
    bool gpio_led_out = false;
    calendar_t *cal;

    /* USB使用の為の初期化 */
    stdio_init_all();

    /* GPIO初期化 */
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, gpio_led_out);

    /* LCD初期化 */
    lcd_init();

    /* カレンダー初期化 */
    cal_init();

    /* 温度測定初期化 */
    tem_init();

    while(1)
    {
        /* カレンダー情報表示 */
        disp_calendar();

        /* 温度情報表示 */
        disp_temperature();

        gpio_sw_input = gpio_get(BUTTON_GPIO);
        sleep_ms(50);
        if(!gpio_sw_input)
        {
            gpio_led_out = !gpio_led_out;
            gpio_put(PICO_DEFAULT_LED_PIN, gpio_led_out);
        }
    }

    return 0;
}
