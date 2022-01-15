#ifndef _HEADER_LCD_1602A
#define _HEADER_LCD_1602A

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

/**************/
/*  定数定義   */
/**************/
#define LCD_LINE_1  0   /* LCD画面1行目 */
#define LCD_LINE_2  1   /* LCD画面2行目 */

/**************/
/* LCD関数定義 */
/**************/
void lcd_init();                                         /* LCDの初期化 */
void lcd_clear();                                        /* LCD表示をクリア */
void lcd_cursorhome();                                   /* LCDのカーソルを初期位置に戻す */
void lcd_setcursor(int line, int position);              /* 指定した位置にカーソルを移動する */
void lcd_setblink(int line, int position, bool isblink); /* 指定した位置のカーソル位置の桁をブリンク設定を行う */
void lcd_print(const char* str);                         /* strをLCDに表示する */

#endif  //_HEADER_LCD_1602A
