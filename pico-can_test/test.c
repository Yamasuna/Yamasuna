#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

/* 共通 */
#define PIN_HIGH            (1u)
#define PIN_LOW             (0u)

/* PINアサイン */
#define PIN_MISO            (0u)
#define PIN_CS              (1u)
#define PIN_SCK             (2u)
#define PIN_MOSI            (3u)
#define PIN_LED_ON_BOARD    (25u)

/* MCP2515設定値  */
#define SPI_BAUDRATE_1MHZ   (1000000u)  /* ボーレート1MHz */
#define MCP2515_RX0BF_HIGH  (0x14u)     /* RX0BF端子 High出力 */
#define MCP2515_RX0BF_LOW   (0x04u)     /* RX0BF端子 Low出力 */
#define MCP2515_RX1BF_HIGH  (0x28u)     /* RX1BF端子 High出力 */
#define MCP2515_RX1BF_LOW   (0x08u)     /* RX1BF端子 Low出力  */

/* MCP2515レジスタアドレス */
#define MCP2515_REG_ADDR_BFPCTRL    (0x0Cu)     /* BFPCTRL */
#define MCP2515_REG_ADDR_TXRTSCTRL  (0x0Du)     /* TXRTSCTRL */
#define MCP2515_REG_ADDR_TEC        (0x1Cu)     /* TEC */
#define MCP2515_REG_ADDR_REC        (0x1Du)     /* REC */
#define MCP2515_REG_ADDR_CNF3       (0x28u)     /* CNF3 */
#define MCP2515_REG_ADDR_CNF2       (0x29u)     /* CNF2 */
#define MCP2515_REG_ADDR_CNF1       (0x2Au)     /* CNF1 */
#define MCP2515_REG_ADDR_CANINTE    (0x2Bu)     /* CANINTE */
#define MCP2515_REG_ADDR_CANINTF    (0x2Cu)     /* CANINTF */
#define MCP2515_REG_ADDR_EFLG       (0x2Du)     /* EFLG */
#define MCP2515_REG_ADDR_TXB0CTRL   (0x30u)     /* TXB0CTRL */
#define MCP2515_REG_ADDR_TXB0SIDH   (0x31u)     /* TXB0SIDH */
#define MCP2515_REG_ADDR_TXB0SIDL   (0x32u)     /* TXB0SIDL */
#define MCP2515_REG_ADDR_TXB0DLC    (0x35u)     /* TXB0DLC  */
#define MCP2515_REG_ADDR_TXB0D0     (0x36u)     /* TXB0D0   */
#define MCP2515_REG_ADDR_TXB1CTRL   (0x40u)     /* TXB1CTRL */
#define MCP2515_REG_ADDR_TXB1SIDH   (0x41u)     /* TXB1SIDH */
#define MCP2515_REG_ADDR_TXB1SIDL   (0x42u)     /* TXB1SIDL */
#define MCP2515_REG_ADDR_TXB1DLC    (0x45u)     /* TXB1DLC  */
#define MCP2515_REG_ADDR_TXB1D0     (0x46u)     /* TXB1D0   */
#define MCP2515_REG_ADDR_TXB2CTRL   (0x50u)     /* TXB2CTRL */
#define MCP2515_REG_ADDR_TXB2SIDH   (0x51u)     /* TXB2SIDH */
#define MCP2515_REG_ADDR_TXB2SIDL   (0x52u)     /* TXB2SIDL */
#define MCP2515_REG_ADDR_TXB2DLC    (0x55u)     /* TXB2DLC  */
#define MCP2515_REG_ADDR_TXB2D0     (0x56u)     /* TXB2D0   */
#define MCP2515_REG_ADDR_RXB0CTRL   (0x60u)     /* RXB0CTRL */
#define MCP2515_REG_ADDR_RXB1CTRL   (0x70u)     /* RXB1CTRL */

/* MCP2515 SPI命令種類 */
typedef enum MCP2515_INSTRUCTION {
  MCP2515_RESET,
  MCP2515_REG_READ,
  MCP2515_RX_BUF_READ_RXB0SIDH,
  MCP2515_RX_BUF_READ_RXB0D0,
  MCP2515_RX_BUF_READ_RXB1SIDH,
  MCP2515_RX_BUF_READ_RXB1D0,
  MCP2515_REG_WRITE,
  MCP2515_TX_BUF_WRITE_TXB0SIDH,
  MCP2515_TX_BUF_WRITE_TXB0D0,
  MCP2515_TX_BUF_WRITE_TXB1SIDH,
  MCP2515_TX_BUF_WRITE_TXB1D0,
  MCP2515_TX_BUF_WRITE_TXB2SIDH,
  MCP2515_TX_BUF_WRITE_TXB2D0,
  MCP2515_TX_REQUEST_TXB0,
  MCP2515_TX_REQUEST_TXB1,
  MCP2515_TX_REQUEST_TXB2,
  MCP2515_STATE_READ,
  MCP2515_RX_STATE_READ,
  MCP2515_REG_BIT_CHANGE,
  MCP2515_INSTRUCTION_MAX
} MCP2515_INSTRUCTION_T;

/* MCP2515 SPI命令セット */
typedef struct MCP2515_INSTRUCTION_SET
{
    MCP2515_INSTRUCTION_T  inst_type;   /* 命令種類   */
    uint8_t                inst_code;   /* 命令コード */
    uint8_t                inst_size;   /* 命令サイズ */
} MCP2515_INSTRUCTION_SET_T;

/* MCP2515 SPI命令セット定義 */
MCP2515_INSTRUCTION_SET_T mcp2515_instruction[MCP2515_INSTRUCTION_MAX] =
{
  {MCP2515_RESET,                   0b11000000, 1u},  /* リセット                              */
  {MCP2515_REG_READ,                0b00000011, 2u},  /* レジスタ読み込み                       */
  {MCP2515_RX_BUF_READ_RXB0SIDH,    0b10010000, 1u},  /* RXB0SIDHバッファ読み込み               */
  {MCP2515_RX_BUF_READ_RXB0D0,      0b10010010, 1u},  /* RXB0D0バッファ読み込み                 */
  {MCP2515_RX_BUF_READ_RXB1SIDH,    0b10010100, 1u},  /* RXB1SIDHバッファ読み込み               */
  {MCP2515_RX_BUF_READ_RXB1D0,      0b10010110, 1u},  /* RXB1D0バッファ読み込み                 */
  {MCP2515_REG_WRITE,               0b00000010, 3u},  /* レジスタ書き込み                       */
  {MCP2515_TX_BUF_WRITE_TXB0SIDH,   0b01000000, 2u},  /* TXB0SIDHから送信バッファ書き込み       */
  {MCP2515_TX_BUF_WRITE_TXB0D0,     0b01000001, 2u},  /* TXB0D0から送信バッファ書き込み         */
  {MCP2515_TX_BUF_WRITE_TXB1SIDH,   0b01000010, 2u},  /* TXB1SIDHから送信バッファ書き込み       */
  {MCP2515_TX_BUF_WRITE_TXB1D0,     0b01000011, 2u},  /* TXB1D0から送信バッファ書き込み         */
  {MCP2515_TX_BUF_WRITE_TXB2SIDH,   0b01000100, 2u},  /* TXB2SIDHから送信バッファ書き込み       */
  {MCP2515_TX_BUF_WRITE_TXB2D0,     0b01000101, 2u},  /* TXB2D0から送信バッファバッファ書き込み  */
  {MCP2515_TX_REQUEST_TXB0,         0b10000001, 1u},  /* TXB0バッファ送信要求                   */
  {MCP2515_TX_REQUEST_TXB1,         0b10000010, 1u},  /* TXB1バッファ送信要求                   */
  {MCP2515_TX_REQUEST_TXB2,         0b10000100, 1u},  /* TXB2バッファ送信要求                   */
  {MCP2515_STATE_READ,              0b10100000, 1u},  /* 状態読み込み                           */
  {MCP2515_RX_STATE_READ,           0b10110000, 1u},  /* RX状態読み込み                         */
  {MCP2515_REG_BIT_CHANGE,          0b00000101, 4u}   /* ビット変更                             */
};

/* バッファ */
static uint8_t     mcp2515_inst_buf[256u];   /* MCP2515命令送信用バッファ */

/* タイマ */
struct repeating_timer sec_timer;  /* 1秒タイマー     */

/* 関数プロトタイプ */
static void mcp2515_init();                                     /* MCP2515の初期化を行う */
static void mcp2515_cs_enable();                                /* CS信号のEnableを行う  */
static void mcp2515_cs_disable();                               /* CS信号のDisableを行う */
static void mcp2515_reset();                                    /* MCP2515のリセットを行う */
static void mcp2515_reg_write(uint8_t reg_addr, uint8_t write_val);   /* MCP2515のレジスタ書き込み */
static void mcp2515_tx_buf_write(uint8_t buf_no, uint8_t *data, uint8_t size);  /* MCP2515送信バッファ書き込み */
static void mcp2515_reg_read(uint8_t reg_addr, uint8_t *read_val);   /* MCP2515のレジスタ読み込み */
static void mcp2515_reg_bit_change(uint8_t reg_addr, uint8_t mask, uint8_t val);
static void mcp2515_can_send();                                 /* MCP2515でCAN送信を行う */

static bool repeating_timer1sec_callback(struct repeating_timer *t); /* 1秒毎のタイマー割込み発生時の処理 */

static void mcp2515_init()
{
    uint8_t buf_idx;
    uint8_t buf_size;
    
    /* バッファ初期化 */
    buf_size = sizeof(mcp2515_inst_buf) / sizeof(mcp2515_inst_buf[0]);
    for (buf_idx = 0u; buf_idx < buf_size; buf_size++)
    {
        mcp2515_inst_buf[buf_idx] = 0x00u;
    }

    /* SPI初期化 */
    spi_init(spi0, SPI_BAUDRATE_1MHZ);

    // Set SPI format
    spi_set_format( spi0,   // SPI instance
                    8,      // Number of bits per transfer
                    0,      // Polarity (CPOL)
                    0,      // Phase (CPHA)
                    SPI_MSB_FIRST);

    /* ポートの初期化 */
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    /* CSピン設定 */
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);

    /* CS無効化 */
    mcp2515_cs_disable();

    /* リセット */
    mcp2515_reset();
}

static void mcp2515_cs_enable()
{
    gpio_put(PIN_CS, PIN_LOW);
}

static void mcp2515_cs_disable()
{
    gpio_put(PIN_CS, PIN_HIGH);
}

static void mcp2515_reset()
{
    uint8_t inst_size;
    mcp2515_inst_buf[0] = mcp2515_instruction[MCP2515_RESET].inst_code;
    inst_size           = mcp2515_instruction[MCP2515_RESET].inst_size;

    mcp2515_cs_enable();
    spi_write_blocking(spi0, &mcp2515_inst_buf[0], inst_size);
    mcp2515_cs_disable();
    sleep_ms(10u);
}

static void mcp2515_reg_write(uint8_t reg_addr, uint8_t write_val)
{
    uint8_t inst_size;
    mcp2515_inst_buf[0] = mcp2515_instruction[MCP2515_REG_WRITE].inst_code;
    mcp2515_inst_buf[1] = reg_addr;
    mcp2515_inst_buf[2] = write_val;
    inst_size           = mcp2515_instruction[MCP2515_REG_WRITE].inst_size;

    mcp2515_cs_enable();
    spi_write_blocking(spi0, &mcp2515_inst_buf[0], inst_size);
    mcp2515_cs_disable();
}

static void mcp2515_tx_buf_write(uint8_t buf_no, uint8_t *data, uint8_t size)
{
    mcp2515_cs_disable();
    spi_write_blocking(spi0, )
}

static void mcp2515_reg_read(uint8_t reg_addr, uint8_t *read_val)
{
    uint8_t inst_size;
    mcp2515_inst_buf[0] = mcp2515_instruction[MCP2515_REG_READ].inst_code;
    mcp2515_inst_buf[1] = reg_addr;
    inst_size           = mcp2515_instruction[MCP2515_REG_READ].inst_size;

    mcp2515_cs_enable();
    spi_write_read_blocking(spi0, &mcp2515_inst_buf[0], read_val, inst_size);
    mcp2515_cs_disable();
}

static void mcp2515_reg_bit_change(uint8_t reg_addr, uint8_t mask, uint8_t val)
{
    uint8_t inst_size;
    mcp2515_inst_buf[0] = mcp2515_instruction[MCP2515_REG_BIT_CHANGE].inst_code;
    mcp2515_inst_buf[1] = reg_addr;
    mcp2515_inst_buf[2] = mask;
    mcp2515_inst_buf[3] = val;
    inst_size           = mcp2515_instruction[MCP2515_REG_BIT_CHANGE].inst_size;

    mcp2515_cs_enable();
    spi_write_blocking(spi0, &mcp2515_inst_buf[0], inst_size);
    mcp2515_cs_disable();
}

static void mcp2515_can_send()
{
    uint8_t reg_TxB0CTRL;
    uint8_t inst_size;
 
    reg_TxB0CTRL = 0x00u;

    /* 送信バッファ設定 */
    mcp2515_inst_buf[0] = mcp2515_instruction[MCP2515_TX_BUF_WRITE_TXB0SIDH].inst_code;
    mcp2515_inst_buf[1] = 0x20u;    /* TXB0SIDH */
    mcp2515_inst_buf[2] = 0x00u;    /* TXB0SIDL */
    mcp2515_inst_buf[3] = 0x00u;    /* TXB0EID8 */
    mcp2515_inst_buf[4] = 0x00u;    /* TXB0EID0 */
    mcp2515_inst_buf[5] = 0x08u;    /* TXB0DLC  */
    mcp2515_inst_buf[6] = 0x11u;    /* TXB0D0   */
    mcp2515_inst_buf[7] = 0x22u;    /* TXB0D1   */
    mcp2515_inst_buf[8] = 0x33u;    /* TXB0D2   */
    mcp2515_inst_buf[9] = 0x44u;    /* TXB0D3   */
    mcp2515_inst_buf[10] = 0x55u;   /* TXB0D4   */
    mcp2515_inst_buf[11] = 0x66u;   /* TXB0D5   */
    mcp2515_inst_buf[12] = 0x77u;   /* TXB0D6   */
    mcp2515_inst_buf[13] = 0x88u;   /* TXB0D7   */
    inst_size = 14u;

    mcp2515_cs_enable();
    spi_write_blocking(spi0, &mcp2515_inst_buf[0], inst_size);
    mcp2515_cs_disable();


    /* TXB0CTRL.TXREQビット=1 */
    mcp2515_reg_bit_change(MCP2515_REG_ADDR_TXB0CTRL, 0x08u, 0x08u);

    /* TXB0CTRL.TXREQビットが0(送信完了)になるまで待つ */
    while ( (reg_TxB0CTRL & 0x08u) == 0x08u)
    {
        mcp2515_reg_read(MCP2515_REG_ADDR_TXB0CTRL, &reg_TxB0CTRL);
    }

}
    

static bool repeating_timer1sec_callback(struct repeating_timer *t)
{
    static uint8_t out_RX1BF = PIN_LOW;
           uint8_t output_state;

    if (out_RX1BF == PIN_LOW)
    {
        output_state = MCP2515_RX0BF_LOW | MCP2515_RX1BF_HIGH;
        
        out_RX1BF = PIN_HIGH;
    }
    else
    {
        output_state = MCP2515_RX0BF_HIGH | MCP2515_RX1BF_LOW;
        out_RX1BF = PIN_LOW;
    }

    mcp2515_reg_write(MCP2515_REG_ADDR_BFPCTRL, output_state);

    return true;
}

void main(void)
{
    uint8_t out_RX1BF;

    out_RX1BF = PIN_LOW;

    /* UART用に有効化 */
    stdio_init_all();

    /* To use GPIO 25pin */
    gpio_init(PIN_LED_ON_BOARD);
    gpio_set_dir(PIN_LED_ON_BOARD, GPIO_OUT);
    gpio_put(PIN_LED_ON_BOARD, 1);

    /* MCP2515初期化 */
    mcp2515_init();

    /* 1秒タイマ起動 */
    add_repeating_timer_ms(1000, repeating_timer1sec_callback, NULL, &sec_timer);

    //Loop
    while (true)
    {
        #if 0
        if(out_RX1BF == PIN_LOW)
        {
            mcp2515_reg_write(MCP2515_REG_ADDR_BFPCTRL, MCP2515_RX1BF_HIGH);
            out_RX1BF = PIN_HIGH;
        }
        else
        {
            mcp2515_reg_write(MCP2515_REG_ADDR_BFPCTRL, MCP2515_RX1BF_LOW);
            out_RX1BF = PIN_LOW;
        }
        
        sleep_ms(1000u);
        #endif
    }
}
