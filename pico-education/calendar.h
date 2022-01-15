#ifndef _HEADER_CALENDAR
#define _HEADER_CALENDAR

#include <stdio.h>
#include "pico/stdlib.h"

/* 構造体定義 */
typedef struct CalendarDate{
    uint16_t year;       /* 西暦 */
    uint8_t  month;      /* 月 */
    uint8_t day;         /* 日付 */
    uint8_t dayofweek;   /* 曜日 */
} calendar_date_t;

typedef struct CalendarTime{
    uint8_t hour;       /* 時間 */
    uint8_t minute;     /* 分   */
    uint8_t second;     /* 秒   */
} calendar_time_t;

typedef struct Calendar {
    calendar_date_t cal_date;   /* 日付情報 */
    calendar_time_t cal_time;   /* 時刻情報 */
} calendar_t;

/* 初期設定データ */
#define DEFAULT_YEAR    2022    /* 初期データ(西暦) */
#define DEFAULT_MONTH   1       /* 初期データ(月)   */
#define DEFAULT_DAY     1       /* 初期データ(日)   */
#define DEFAULT_HOUR    12      /* 初期データ(時)   */
#define DEFAULT_MINUTE  0       /* 初期データ(分)   */
#define DEFAULT_SECOND  0       /* 初期データ(秒)   */

/* 設定値範囲 */
#define YEAR_MIN        2020    /* 西暦の最小設定値 */
#define YEAR_MAX        2040    /* 西暦の最大設定値 */

/* 関数定義 */
void cal_init();                                /* カレンダーを初期化する     */
calendar_t *cal_getinfo();                      /* カレンダー情報を取得する   */
char *cal_getweekofdays_str(uint8_t weekofday); /* 曜日情報の文字列を取得する */
void cal_startcalendar();                       /* カレンダーのタイマーを開始する */
void cal_stopcalendar();                        /* カレンダーのタイマーを止める */
void cal_increment_year();                      /* カレンダー設定用　西暦設定を1年進める */
void cal_increment_month();                     /* カレンダー設定用　月設定を1月進める */
void cal_increment_day();                       /* カレンダー設定用　日設定を1日進める */
void cal_increment_hour();                      /* カレンダー設定用　時間設定を1時間進める */
void cal_increment_minute();                    /* カレンダー設定用　分設定を1分進める */

#endif  //_HEADER_CALENDAR
