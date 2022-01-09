#include "calendar.h"

#define MONTHS          12      /* 1年の月の数      */

typedef enum{
    JANUARY = 1,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
} MONTH_OF_YEAR;

/* グローバル変数 */
static calendar_t calendar;         /* カレンダー情報   */
struct repeating_timer sec_timer;   /* 1秒タイマー     */

/* 各月の日数(1月～12月)(非うるう年) */
static uint8_t days_per_month[MONTHS]           = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* 各月の日数(1月～12月)(うるう年) */
static uint8_t days_per_month_leap_year[MONTHS] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* 曜日の文字列情報 */
static char *days_of_week_str[7] = {"(SAT)",
                                    "(SUN)",
                                    "(MON)",
                                    "(TUE)",
                                    "(WED)",
                                    "(THU)",
                                    "(FRI)"};

/* 閏年判定処理 */
bool cal_judge_leap_year(uint16_t year)
{
    bool isleap_year;   /* 閏年かどうか(1:閏年 0:閏年でない) */

    if( year % 400 == 0)
    {
        isleap_year = true;
    }
    else if (year % 100 == 0)
    {
        isleap_year = false;
    }
    else if (year % 4 == 0)
    {
        isleap_year = true;
    }

    return isleap_year;
}

/* 曜日計算関数 */
uint8_t cal_weekofdays(uint16_t year, uint8_t month, uint8_t day)
{
    uint8_t daysofweek;
    uint16_t calc_year;
    uint8_t calc_month;

    /* 1月は前年の13月、2月は前年の14月として扱う */
    if (month == JANUARY)
    {
        calc_year = year - 1;
        calc_month = 13;
    } 
    else if (month == FEBRUARY)
    {
        calc_year = year - 1;
        calc_month = 14;
    }
    else
    {
        calc_year = year;
        calc_month = month;
    }

    /* Zellerの公式による曜日計算 */
    daysofweek = (day + (26 * (calc_month + 1)) / 10 + (calc_year % 100) + (calc_year % 100) / 4 + 5 * (calc_year / 100) + (calc_year / 400) ) % 7;
    return daysofweek;
}

/* カレンダー時刻情報更新 */
static bool cal_update_time()
{
    bool date_update = false;   /* 日付更新発生フラグ(1:日付更新あり、0:日付更新なし) */
    calendar_time_t* cal_time;

    cal_time = &calendar.cal_time;

    cal_time->second++;
    if (cal_time->second == 60)
    {
        cal_time->second = 0;
        cal_time->minute++;
    } else { /* 何もしない */ }

    if (cal_time->minute == 60)
    {
        cal_time->minute = 0;
        cal_time->hour++;
    } else { /* 何もしない */ }

    if (cal_time->hour == 24)
    {
        cal_time->hour = 0;
        date_update = true;
    } else { /* 何もしない */ }

    return date_update;
}

/* 日付情報更新処理 */
static void cal_update_date()
{
    calendar_date_t* cal_date;
    cal_date = &calendar.cal_date;

    cal_date->day++;

    /* 閏年か判定 */
    if ( cal_judge_leap_year(cal_date->year) )
    {
        /* 日付を更新した結果、当月の日数を超えた場合(閏年) */
        if( cal_date->day > days_per_month_leap_year[cal_date->month-1])
        {
            cal_date->day = 1;
            cal_date->month++;
        }
    }
    else
    {
        /* 日付を更新した結果、当月の日数を超えた場合(閏年でない) */
        if( cal_date->day > days_per_month[cal_date->month-1])
        {
            cal_date->day = 1;
            cal_date->month++;
        }
    }

    /* 月を更新した結果、1年の月数を超えた場合 */
    if( cal_date->month > MONTHS)
    {
        cal_date->month = JANUARY;  /* 1月に戻る */
        cal_date->year++;           /* 西暦を進める */
    }

    /* 曜日を求める */
    cal_date->dayofweek = cal_weekofdays(cal_date->year, cal_date->month, cal_date->day);
}

/* 1秒毎のタイマー割込み発生時の処理 */
bool repeating_timer1sec_callback(struct repeating_timer *t)
{
    bool isdate_update = false;

    /* 時刻更新処理 */
    isdate_update = cal_update_time();

    /* 日付更新ありなら日付情報を更新する */
    if (isdate_update)
    {
        cal_update_date();
    }
    return true;
}

/* カレンダーを初期化する */
void cal_init()
{
    calendar.cal_date.year = DEFAULT_YEAR;
    calendar.cal_date.month = DEFAULT_MONTH;
    calendar.cal_date.day = DEFAULT_DAY;
    calendar.cal_date.dayofweek = cal_weekofdays(calendar.cal_date.year, 
                                                calendar.cal_date.month, 
                                                calendar.cal_date.day);

    calendar.cal_time.hour = DEFAULT_HOUR;
    calendar.cal_time.minute = DEFAULT_MINUTE;
    calendar.cal_time.second = DEFAULT_SECOND;

    /* タイマーコールバック設定(1秒毎) */
    add_repeating_timer_ms(1000, repeating_timer1sec_callback, NULL, &sec_timer);
}

/* カレンダー情報を取得する */
calendar_t *cal_getinfo()
{
    return &calendar;
}

/* 曜日情報の文字列を取得する */
char *cal_getweekofdays_str(uint8_t weekofday)
{
    return days_of_week_str[weekofday];
}
