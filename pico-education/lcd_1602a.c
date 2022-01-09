#include "lcd_1602a.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

/******************/
/* LCDコマンド定義 */
/******************/
#define LCD_CLEARDISPLAY    0x01    /* 表示クリア */
#define LCD_CURSORHOME      0x02    /* カーソルホーム */
#define LCD_ENTRYMODESET    0x04    /* エントリーモードセット */
#define LCD_DISPLAYCONTROL  0x08    /* 表示オン・オフコントロール */
#define LCD_CURDISPSHIFT    0x10    /* カーソル・表示シフト */
#define LCD_FUNCTIONSET     0x20    /* ファンクションセット */
#define LCD_CGRAMADDRSET    0x40    /* CG RAMアドレスセット */
#define LCD_DDRAMADDRSET    0x80    /* DD RAMアドレスセット */

/***************************/
/* LCDコマンドオプション定義 */
/***************************/
/* エントリーモードセット */
#define LCD_ENTRYSHIFTINCREMENT    0x02    /* インクリメント */

/* 表示オン・オフコントロール */
#define LCD_DISPOFF         0x00    /* 表示オフ */
#define LCD_DISPON          0x04    /* 表示オン */

#define LCD_CURSOROFF       0x00    /* カーソルオフ */
#define LCD_CURSORON        0x02    /* カーソルオン */

#define LCD_BLINKOFF        0x00    /* ブリンクオフ */
#define LCD_BLINKON         0x01    /* ブリンクオン */

/* カーソル・表示シフト */
#define LCD_CURSORSHIFT     0x00    /* カーソルシフト */
#define LCD_DISPSHIFT       0x08    /* 表示シフト */

#define LCD_CURSORSHIFT_L   0x00    /* カーソル左シフト */
#define LCD_CURSORSHIFT_R   0x04    /* カーソル右シフト */

/* ファンクションセット */
#define LCD_2LINE           0x08    /* 2行表示 */
#define LCD_8BITMODE        0x10    /* 8ビットモード */
#define LCD_5X10DOT         0x04    /* 5X10ドットマトリクス */

/*****************************/
/* LCDバックライトコントロール */
/*****************************/
#define LCD_ENABLE_BIT      0x04
#define LCD_BACKLIGHT       0x08

/*****************************/
/*         I2C設定関連        */
/*****************************/
#define LCD_BUSADDR         0x27    /* LCDドライバのバスアドレス */
#define LCD_I2C_PORT        i2c_default
#define LCD_I2C_BAUDRATE    100*1000
#define LCD_I2C_DELAY_US    600

// Modes for lcd_send_byte
#define LCD_I2C_COMMAND     0
#define LCD_I2C_CHARACTER   1

/* I2C設定を初期化する */
static void lcd_i2c_init()
{
    i2c_init(LCD_I2C_PORT, LCD_I2C_BAUDRATE);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
}

/* I2Cデータ書き込み */
static void lcd_i2c_write_byte(uint8_t val)
{
    i2c_write_blocking(LCD_I2C_PORT, LCD_BUSADDR, &val, 1, false);
}

static void lcd_toggle_enable(uint8_t val) {
    // Toggle enable pin on LCD display
    // We cannot do this too quickly or things don't work
    sleep_us(LCD_I2C_DELAY_US);
    lcd_i2c_write_byte(val | LCD_ENABLE_BIT);
    sleep_us(LCD_I2C_DELAY_US);
    lcd_i2c_write_byte(val & ~LCD_ENABLE_BIT);
    sleep_us(LCD_I2C_DELAY_US);
}

/* LCDへのバイトデータ送信 */
static void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    /* Bit7～Bit4分を送信 */
    lcd_i2c_write_byte(high);
    lcd_toggle_enable(high);

    /* Bit3～Bit0分を送信 */
    lcd_i2c_write_byte(low);
    lcd_toggle_enable(low);
}

/* 1文字分をLCDに出力する */
static void lcd_char(char character)
{
    lcd_send_byte(character, LCD_I2C_CHARACTER);
}

/* LCDの初期化を行う */
void lcd_init()
{
    /* LCDに接続するI2Cの初期化 */
    lcd_i2c_init();

    /* LCDの初期化処理 */
    lcd_send_byte(0x03, LCD_I2C_COMMAND);
    lcd_send_byte(0x03, LCD_I2C_COMMAND);
    lcd_send_byte(0x03, LCD_I2C_COMMAND);
    lcd_send_byte(0x02, LCD_I2C_COMMAND);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYSHIFTINCREMENT, LCD_I2C_COMMAND);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_I2C_COMMAND);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPON, LCD_I2C_COMMAND);
    lcd_clear();
}

/* LCD表示をクリアする */
void lcd_clear()
{
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_I2C_COMMAND);
}

/* LCDのカーソルを初期位置に戻す */
void lcd_cursorhome()
{
    lcd_send_byte(LCD_CURSORHOME, LCD_I2C_COMMAND);
}

/* 指定した位置にカーソルを移動する */
void lcd_setcursor(int line, int position)
{
    int val;

    /* 指定された行が1行目か */
    if(line == LCD_LINE_1)
    {
        val = 0x80 + position;
    }
    else
    {
        val = 0xC0 + position;
    }

    lcd_send_byte(val, LCD_I2C_COMMAND);
}

/* LCDに文字列を出力する */
void lcd_print(const char* str)
{
    while(*str != '\0')
    {
       lcd_char(*str);
       str++;
    }
}
