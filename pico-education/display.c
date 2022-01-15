#include "display.h"

extern MODE mode_state;                         /* モード状態 */
extern MODE_SETTING_ITEM mode_setting_item;     /* 設定モード時の現在設定項目 */
extern bool isdisp_setting_item[MODE_SETTING_NUM];

/* カレンダー情報を表示する */
static void display_calendar()
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
    
    if (isdisp_setting_item[MODE_SETTING_YEAR] == true)
    {
        lcd_print(str_year);
    }
    else
    {
        lcd_print("    ");
    }

    lcd_print("/");

    if (isdisp_setting_item[MODE_SETTING_MONTH] == true)
    {
        lcd_print(str_month);
    }
    else
    {
        lcd_print("  ");
    }
    lcd_print("/");

    if (isdisp_setting_item[MODE_SETTING_DAY] == true)
    {
        lcd_print(str_day);
    }
    else
    {
        lcd_print("  ");
    }

    /* 曜日情報を表示(LCD1行目) */
    lcd_print(str_weekofdays);

    /* 時刻情報を表示(LCD2行目) */
    lcd_setcursor(LCD_LINE_2, 0);

    if (isdisp_setting_item[MODE_SETTING_HOUR] == true)
    {
        lcd_print(str_hour);
    }
    else
    {
        lcd_print("  ");
    }
    lcd_print(":");

    if (isdisp_setting_item[MODE_SETTING_MINUTE] == true)
    {
        lcd_print(str_minute);
    }
    else
    {
        lcd_print("  ");
    }
    lcd_print(":");
    lcd_print(str_second);
}

/* 温度情報を表示する */
static void display_temperature()
{
    float temp;
    char str_temp[4];

    /* 温度情報を取得して文字データに変換 */
    temp = tem_gettemp_cel();
    sprintf(str_temp, "%.1f", temp);

    lcd_setcursor(LCD_LINE_2, 10);
    lcd_print(str_temp);
    lcd_print("C");
}

/* LCD表示メイン処理 */
void display_main()
{ 
    display_calendar();
    display_temperature();
     
    if (mode_state == MODE_SETTING)
    {
       
    }
}
