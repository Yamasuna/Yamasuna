#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "class/cdc/cdc_device.h"

/* DEBUG用 */
#define MCP2515_DEBUG       (1u)        /* 1:MCP2515用デバッグ出力を行う 0:MCP2515デバッグ用出力を行わない */

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
#define MCP2515_REG_ADDR_RXF0SIDH       (0x00u)     /* RXF0SIDH */
#define MCP2515_REG_ADDR_RXF0SIDL       (0x01u)     /* RXF0SIDL */
#define MCP2515_REG_ADDR_BFPCTRL        (0x0Cu)     /* BFPCTRL */
#define MCP2515_REG_ADDR_TXRTSCTRL      (0x0Du)     /* TXRTSCTRL */
#define MCP2515_REG_ADDR_TXB0_CANSTAT   (0x0Eu)     /* TXB0_CANSTAT */
#define MCP2515_REG_ADDR_TXB0_CANCTRL   (0x0Fu)     /* TXB0_CANCTRL */
#define MCP2515_REG_ADDR_TEC            (0x1Cu)     /* TEC */
#define MCP2515_REG_ADDR_REC            (0x1Du)     /* REC */
#define MCP2515_REG_ADDR_RXM0SIDH       (0x20u)     /* RXM0SIDH */
#define MCP2515_REG_ADDR_RXM0SIDL       (0x21u)     /* RXM0SIDL */
#define MCP2515_REG_ADDR_RXM0EID8       (0x22u)     /* RXM0EID8 */
#define MCP2515_REG_ADDR_RXM0EID0       (0x23u)     /* RXM0EID0 */
#define MCP2515_REG_ADDR_CNF3           (0x28u)     /* CNF3 */
#define MCP2515_REG_ADDR_CNF2           (0x29u)     /* CNF2 */
#define MCP2515_REG_ADDR_CNF1           (0x2Au)     /* CNF1 */
#define MCP2515_REG_ADDR_CANINTE        (0x2Bu)     /* CANINTE */
#define MCP2515_REG_ADDR_CANINTF        (0x2Cu)     /* CANINTF */
#define MCP2515_REG_ADDR_EFLG           (0x2Du)     /* EFLG */
#define MCP2515_REG_ADDR_TXB0CTRL       (0x30u)     /* TXB0CTRL */
#define MCP2515_REG_ADDR_TXB0SIDH       (0x31u)     /* TXB0SIDH */
#define MCP2515_REG_ADDR_TXB0SIDL       (0x32u)     /* TXB0SIDL */
#define MCP2515_REG_ADDR_TXB0DLC        (0x35u)     /* TXB0DLC  */
#define MCP2515_REG_ADDR_TXB0D0         (0x36u)     /* TXB0D0   */
#define MCP2515_REG_ADDR_TXB1CTRL       (0x40u)     /* TXB1CTRL */
#define MCP2515_REG_ADDR_TXB1SIDH       (0x41u)     /* TXB1SIDH */
#define MCP2515_REG_ADDR_TXB1SIDL       (0x42u)     /* TXB1SIDL */
#define MCP2515_REG_ADDR_TXB1DLC        (0x45u)     /* TXB1DLC  */
#define MCP2515_REG_ADDR_TXB1D0         (0x46u)     /* TXB1D0   */
#define MCP2515_REG_ADDR_TXB2CTRL       (0x50u)     /* TXB2CTRL */
#define MCP2515_REG_ADDR_TXB2SIDH       (0x51u)     /* TXB2SIDH */
#define MCP2515_REG_ADDR_TXB2SIDL       (0x52u)     /* TXB2SIDL */
#define MCP2515_REG_ADDR_TXB2DLC        (0x55u)     /* TXB2DLC  */
#define MCP2515_REG_ADDR_TXB2D0         (0x56u)     /* TXB2D0   */
#define MCP2515_REG_ADDR_RXB0CTRL       (0x60u)     /* RXB0CTRL */
#define MCP2515_REG_ADDR_RXB0SIDH       (0x61u)     /* RXB0SIDH */
#define MCP2515_REG_ADDR_RXB0SIDL       (0x62u)     /* RXB0SIDL */
#define MCP2515_REG_ADDR_RXB0DLC        (0x65u)     /* RXB0DLC  */
#define MCP2515_REG_ADDR_RXB0D0         (0x66u)     /* RXB0D0   */
#define MCP2515_REG_ADDR_RXB1CTRL       (0x70u)     /* RXB1CTRL */

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

/* MCP2515 TXバッファ番号 */
typedef enum MCP2515_TXBUFNUM {
  MCP2515_TXBUFNUM_TXB0SIDH,   /* TXバッファ0 TXB0SIDH */
  MCP2515_TXBUFNUM_TXB0D0,     /* TXバッファ0 TXB0D0   */
  MCP2515_TXBUFNUM_TXB1SIDH,   /* TXバッファ1 TXB1SIDH */
  MCP2515_TXBUFNUM_TXB1D0,     /* TXバッファ1 TXB1D0   */
  MCP2515_TXBUFNUM_TXB2SIDH,   /* TXバッファ2 TXB2SIDH */
  MCP2515_TXBUFNUM_TXB2D0      /* TXバッファ2 TXB2D0   */
} MCP2515_TXBUFNUM_T;

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
static uint8_t     tinyusb_in_buf[256u];     /* TinyUSB用受信バッファ     */

/* タイマ */
struct repeating_timer sec_timer;  /* 1秒タイマー     */

/* 関数プロトタイプ */
static void mcp2515_init();                                                                     /* MCP2515の初期化を行う */
static void mcp2515_set_baudrate();                                                             /* MCP2515のボーレート設定を行う */
static void mcp2515_cs_enable();                                                                /* CS信号のEnableを行う  */
static void mcp2515_cs_disable();                                                               /* CS信号のDisableを行う */
static void mcp2515_reset();                                                                    /* MCP2515のリセットを行う */
static void mcp2515_reg_write(uint8_t reg_addr, uint8_t write_val);                             /* MCP2515のレジスタ書き込み */
static void mcp2515_tx_buf_write(MCP2515_TXBUFNUM_T buf_no, uint8_t *data, uint8_t size);       /* MCP2515送信バッファ書き込み */
static void mcp2515_reg_read(uint8_t reg_addr, uint8_t *read_val);                              /* MCP2515のレジスタ読み込み */
static void mcp2515_reg_bit_change(uint8_t reg_addr, uint8_t mask, uint8_t val);
static void mcp2515_can_send();                                                                 /* MCP2515でCAN送信を行う */
static void mcp2515_can_recv();                                                                 /* MCP2515でCAN受信を行う */
static void mcp2515_txb0_send_request();                                                        /* TXB0送信要求命令 */
static uint8_t mcp2515_get_tx_buf_instcode(MCP2515_TXBUFNUM_T buf_no);                          /* 指定したバッファ番号の命令コードを得る */

static bool repeating_timer1sec_callback(struct repeating_timer *t); /* 1秒毎のタイマー割込み発生時の処理 */
static bool mcp2515_is_rxb0_receive();                                                          /* RXB0バッファが受信したかチェックする */

static void mcp2515_init()
{
    uint8_t buf_idx;
    uint8_t buf_size;
    
    /* バッファ初期化 */
    buf_size = (uint8_t)(sizeof(mcp2515_inst_buf) / sizeof(mcp2515_inst_buf[0]));
    for (buf_idx = 0u; buf_idx < buf_size; buf_size++)
    {
        mcp2515_inst_buf[buf_idx] = 0x00u;
    }

    /* SPI初期化 */
    spi_init(spi0, SPI_BAUDRATE_1MHZ);

    // Set SPI format
    spi_set_format( spi0,            // SPI instance
                    8,               // Number of bits per transfer
                    SPI_CPOL_0,      // Polarity (CPOL)
                    SPI_CPHA_0,      // Phase (CPHA)
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

    sleep_ms(100u);

    /* Configurationモードに設定 */
    mcp2515_reg_write(MCP2515_REG_ADDR_TXB0_CANCTRL, 0x80u);

    /* MCP2515の送信リクエストを設定 */
    mcp2515_reg_write(MCP2515_REG_ADDR_TXRTSCTRL, 0x01);

    /* BaudRate設定 */
    mcp2515_set_baudrate();

    /* 受信フィルタ設定 */
    //mcp2515_reg_write(MCP2515_REG_ADDR_RXF0SIDH, 0x24u);    /* RXF0SIDH */
    mcp2515_reg_write(MCP2515_REG_ADDR_RXF0SIDH, 0x23u);    /* RXF0SIDH */
    mcp2515_reg_write(MCP2515_REG_ADDR_RXF0SIDL, 0x80u);    /* RXF0SIDL */
    mcp2515_reg_write(MCP2515_REG_ADDR_RXM0SIDH, 0x00u);    /* RXM0SIDH */
    mcp2515_reg_write(MCP2515_REG_ADDR_RXM0SIDL, 0x00u);    /* RXM0SIDL */
    mcp2515_reg_write(MCP2515_REG_ADDR_RXM0EID8, 0xFFu);    /* RXM0EID8 */
    mcp2515_reg_write(MCP2515_REG_ADDR_RXM0EID0, 0xFFu);    /* RXM0EID0 */

    /* CANINTE設定 */
    /* TX0IE割り込み許可 */
    /* RX0IE割り込み許可 */
    mcp2515_reg_write(MCP2515_REG_ADDR_CANINTE,  0x05u);

    /* 受信バッファ0設定 */
    /* マスク/フィルタがオフで、すべてのメッセージを受信 */
    mcp2515_reg_write(MCP2515_REG_ADDR_RXB0CTRL, 0x60u);

    /* ループバックモードに設定 */
#if 0
    mcp2515_reg_write(MCP2515_REG_ADDR_TXB0_CANCTRL, 0x40u);
#endif

#if 1
    /* 通常モードに設定 */
    mcp2515_reg_write(MCP2515_REG_ADDR_TXB0_CANCTRL, 0x00u);
#endif
}

static void mcp2515_set_baudrate()
{
    /* CANボーレート設定 */
    /* 発振周波数16MHzの状態で125kbpsを設定する */

    /* CNF1設定 */
    /* SJW = 1 */
    /* BRP = 3 TQ = 2*(BRP+1)/FOSC */
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF1, 0x03u);
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF1, 0x01u);
    mcp2515_reg_write(MCP2515_REG_ADDR_CNF1, 0x42u); //125kHz 12MHz(実績あり)
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF1, 0x41u); //125kHz 16MHz(実績あり)
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF1, 0x43u); //125kHz 16MHz(実績あり)

    /* CNF2設定 */
    /* BTLMODE = 0b(PS2の長さはPS1およびIPT(2TQ)より大きくする)  */
    /* SAM = 0b(バスラインはサンプル点で1回だけサンプリングされる) */
    /* PHSEG1 = 6 (PHSEQ1+1)*TQ */
    /* PRSEG = 5(PRSEG+1)*TQ */
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF2, 0x35u);
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF2, 0xB9u);
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF2, 0x31u);
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF2, 0xE1u);    //12MHz
    mcp2515_reg_write(MCP2515_REG_ADDR_CNF2, 0xF8u);    //125kHz 12MHz(実績あり)
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF2, 0xE5u);    //125kHz 16MHz (実績あり)



    /* CNF3設定 */
    /* SOF = 0b(CLKOUTピンはクロック出力機能として有効化される) */
    /* WAKFIL = 0b(ウェイクアップフィルタ無効) */
    /* PHSEG2 = 5( (PHSEG2+1) * TQ) */
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF3, 0x05u);
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF3, 0x84u);
    mcp2515_reg_write(MCP2515_REG_ADDR_CNF3, 0x85u);    //125kHz 12MHz(実績あり)
    //mcp2515_reg_write(MCP2515_REG_ADDR_CNF3, 0x83u);    //125kHz 16MHz(実績あり)
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

static void mcp2515_tx_buf_write(MCP2515_TXBUFNUM_T buf_no, uint8_t *data, uint8_t size)
{
    uint8_t inst_code;
    uint8_t inst_size;

    inst_code = mcp2515_get_tx_buf_instcode(buf_no);
    inst_size = size + 1u;

    mcp2515_inst_buf[0] = inst_code;

    for(uint8_t idx = 0; idx < size; idx++)
    {
        mcp2515_inst_buf[1 + idx] = data[idx];
    }
    
    mcp2515_cs_enable();
    spi_write_blocking(spi0, &mcp2515_inst_buf[0], inst_size);
    mcp2515_cs_disable();
}

static void mcp2515_reg_read(uint8_t reg_addr, uint8_t *read_val)
{
    uint8_t inst_size;
    mcp2515_inst_buf[0] = mcp2515_instruction[MCP2515_REG_READ].inst_code;
    mcp2515_inst_buf[1] = reg_addr;
    inst_size           = mcp2515_instruction[MCP2515_REG_READ].inst_size;

    mcp2515_cs_enable();
    spi_write_blocking(spi0, &mcp2515_inst_buf[0], inst_size);
    spi_read_blocking(spi0, 0, read_val, 1u);
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
    uint8_t reg_TEC;
    uint8_t reg_EFLG;
    uint8_t reg_CNF1;
    uint8_t reg_CNF2;
    uint8_t reg_CNF3;
    uint8_t inst_size;
    uint8_t reg_val;
    uint8_t tx_buf[13];
    static uint16_t send_cnt = 0u;
 
    reg_TxB0CTRL = 0x00u;

    /* 送信バッファデータ設定 */
#if 1
    tx_buf[0]  = 0x24u;    /* TXB0SIDH */
    tx_buf[1]  = 0x80u;    /* TXB0SIDL */
    tx_buf[2]  = 0x00u;    /* TXB0EID8 */
    tx_buf[3]  = 0x00u;    /* TXB0EID0 */
    tx_buf[4]  = 0x08u;    /* TXB0DLC  */
    tx_buf[5]  = 0xAAu;    /* TXB0D0   */
    tx_buf[6]  = 0xBBu;    /* TXB0D1   */
    tx_buf[7]  = 0xCCu;    /* TXB0D2   */
    tx_buf[8]  = 0xDDu;    /* TXB0D3   */
    tx_buf[9]  = 0xEEu;    /* TXB0D4   */
    tx_buf[10] = 0xFFu;    /* TXB0D5   */
    tx_buf[11] = 0x11u;    /* TXB0D6   */
    tx_buf[12] = 0x22u;    /* TXB0D7   */
#else
    tx_buf[0]  = 0x23u;    /* TXB0SIDH */
    tx_buf[1]  = 0x80u;    /* TXB0SIDL */
    tx_buf[2]  = 0x00u;    /* TXB0EID8 */
    tx_buf[3]  = 0x00u;    /* TXB0EID0 */
    tx_buf[4]  = 0x08u;    /* TXB0DLC  */
    tx_buf[5]  = 0x33u;    /* TXB0D0   */
    tx_buf[6]  = 0x44u;    /* TXB0D1   */
    tx_buf[7]  = 0x55u;    /* TXB0D2   */
    tx_buf[8]  = 0x66u;    /* TXB0D3   */
    tx_buf[9]  = 0x77u;    /* TXB0D4   */
    tx_buf[10] = 0x88u;    /* TXB0D5   */
    tx_buf[11] = 0x99u;    /* TXB0D6   */
    tx_buf[12] = 0xAAu;    /* TXB0D7   */
#endif
    inst_size = 13u;

    /* 送信バッファ設定 */
    mcp2515_tx_buf_write(MCP2515_TX_BUF_WRITE_TXB0SIDH, &tx_buf[0], inst_size);

#if MCP2515_DEBUG
    printf("****Send CAN Message.[%d]****\n", send_cnt);
    for(uint8_t idx = 0; idx < inst_size; idx++)
    {
        mcp2515_reg_read(MCP2515_REG_ADDR_TXB0SIDH + idx, &reg_val);
        printf("read_reg[%d]:0x%02x\n", idx, reg_val);
    }
    printf("******\n");
    send_cnt++;
#endif

    /* TXB0CTRL.TXREQビット=1 */
    #if 0
    reg_TxB0CTRL = 0x08u;
    //mcp2515_reg_bit_change(MCP2515_REG_ADDR_TXB0CTRL, 0x08u, 0x08u);
    mcp2515_reg_write(MCP2515_REG_ADDR_TXB0CTRL, reg_TxB0CTRL);
    printf("**before send_TXB0CTRL:0x%02x\n", reg_TxB0CTRL);
    #endif
    mcp2515_txb0_send_request();
    mcp2515_reg_read(MCP2515_REG_ADDR_TXB0CTRL, &reg_TxB0CTRL);

#if MCP2515_DEBUG
    printf("[Before Send]TXB0CTRL:0x%02x\n",reg_TxB0CTRL);
#endif

    sleep_ms(100u);

#if MCP2515_DEBUG
    mcp2515_reg_read(MCP2515_REG_ADDR_TXB0CTRL, &reg_TxB0CTRL);
    mcp2515_reg_read(MCP2515_REG_ADDR_TEC, &reg_TEC);
    mcp2515_reg_read(MCP2515_REG_ADDR_EFLG, &reg_EFLG);
    mcp2515_reg_read(MCP2515_REG_ADDR_CNF1, &reg_CNF1);
    mcp2515_reg_read(MCP2515_REG_ADDR_CNF2, &reg_CNF2);
    mcp2515_reg_read(MCP2515_REG_ADDR_CNF3, &reg_CNF3);
    printf("[After Send]TXB0CTRL:0x%02x\n",reg_TxB0CTRL);
    printf("[After Send]TEC:0x%02x\n", reg_TEC);
    printf("[After Send]EFLG:0x%02x\n", reg_EFLG);
    printf("[After Send]CNF1:0x%02x\n", reg_CNF1);
    printf("[After Send]CNF2:0x%02x\n", reg_CNF2);
    printf("[After Send]CNF3:0x%02x\n", reg_CNF3);
#endif

}

static void mcp2515_can_recv()
{
    bool    is_receive;
    uint8_t can_dlc;        /* Data Length Code */
    uint8_t can_idh;        /* CAN受信ID上位     */
    uint8_t can_idl;        /* CAN受信ID下位     */
    uint16_t can_id;         /* CAN受信ID        */
    uint8_t can_data[8];    /* CAN受信データ    */
    static uint16_t recv_cnt = 0u;

    /* RXB0バッファからの受信割り込みをチェック */
    is_receive = mcp2515_is_rxb0_receive();

    if (is_receive == true)
    {
        mcp2515_reg_read(MCP2515_REG_ADDR_RXB0SIDH, &can_idh);
        mcp2515_reg_read(MCP2515_REG_ADDR_RXB0SIDL, &can_idl);
        mcp2515_reg_read(MCP2515_REG_ADDR_RXB0DLC,  &can_dlc);
        can_id = (can_idh << 3u) | (can_idl >> 5u);

        for(int idx = 0u; idx < can_dlc; idx++)
        {
            mcp2515_reg_read(MCP2515_REG_ADDR_RXB0D0 + idx, &can_data[idx]);
        }

        /* RX0IFフラグをクリア */
        mcp2515_reg_write(MCP2515_REG_ADDR_CANINTF, 0x00u);

#if MCP2515_DEBUG
        /* 受信データを表示 */
        printf("****Received CAN Message.[%d]****\n", recv_cnt);
        printf("can_idh = 0x%02x\n", can_idh);
        printf("can_idl = 0x%02x\n", can_idl);
        printf("can_id = 0x%x\n", can_id);
        printf("can_dlc= 0x%02x\n", can_dlc);
        recv_cnt++;

        for(int idx = 0u; idx < 8; idx++)
        {
            printf("can_data[%d] = 0x%02x\n", idx, can_data[idx]);
        }
#endif
    }
    else
    {
        /* 受信割り込み無しなら何もしない */
    }
    
}

static void mcp2515_txb0_send_request()
{
    uint8_t inst_size;
    mcp2515_inst_buf[0] = mcp2515_instruction[MCP2515_TX_REQUEST_TXB0].inst_code;
    inst_size           = mcp2515_instruction[MCP2515_TX_REQUEST_TXB0].inst_size;

    mcp2515_cs_enable();
    spi_write_blocking(spi0, &mcp2515_inst_buf[0], inst_size);
    mcp2515_cs_disable();
}

static uint8_t mcp2515_get_tx_buf_instcode(MCP2515_TXBUFNUM_T buf_no)
{
    uint8_t inst_code;

    switch(buf_no)
    {
        case MCP2515_TXBUFNUM_TXB0SIDH:
            inst_code = mcp2515_instruction[MCP2515_TX_BUF_WRITE_TXB0SIDH].inst_code;
            break;
        case MCP2515_TXBUFNUM_TXB0D0:
            inst_code = mcp2515_instruction[MCP2515_TX_BUF_WRITE_TXB0D0].inst_code;
            break;
        case MCP2515_TXBUFNUM_TXB1SIDH:
            inst_code = mcp2515_instruction[MCP2515_TX_BUF_WRITE_TXB1SIDH].inst_code;
            break;
        case MCP2515_TXBUFNUM_TXB1D0:
            inst_code = mcp2515_instruction[MCP2515_TX_BUF_WRITE_TXB1D0].inst_code;
            break;
        case MCP2515_TXBUFNUM_TXB2SIDH:
            inst_code = mcp2515_instruction[MCP2515_TX_BUF_WRITE_TXB2SIDH].inst_code;
            break;
        case MCP2515_TXBUFNUM_TXB2D0:
            inst_code = mcp2515_instruction[MCP2515_TX_BUF_WRITE_TXB2D0].inst_code;
            break;
        default:
            inst_code = mcp2515_instruction[MCP2515_TX_BUF_WRITE_TXB0SIDH].inst_code;
            break;
    }

    return inst_code;
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

static bool mcp2515_is_rxb0_receive()
{
    uint8_t reg_val;
    bool ret;

    ret = false;
    mcp2515_reg_read(MCP2515_REG_ADDR_CANINTF, &reg_val);

    if ( (reg_val & 0x01u) == 0x01u)
    {
        ret = true;
    }
    else
    {
        /* do nothing */
    }

    return ret;
}

void main(void)
{
    uint8_t out_RX1BF;
    uint8_t can_stat;

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
    //add_repeating_timer_ms(1000, repeating_timer1sec_callback, NULL, &sec_timer);

    //Loop
    while (true)
    {
        #if 0
        mcp2515_reg_read(MCP2515_REG_ADDR_TXB0_CANSTAT, &can_stat);
        printf("CANSTAT:0x%02x\n", can_stat);

        if(out_RX1BF == PIN_LOW)
        {
            mcp2515_reg_write(MCP2515_REG_ADDR_BFPCTRL, MCP2515_RX0BF_LOW | MCP2515_RX1BF_HIGH);
            out_RX1BF = PIN_HIGH;
        }
        else
        {
            mcp2515_reg_write(MCP2515_REG_ADDR_BFPCTRL, MCP2515_RX0BF_HIGH | MCP2515_RX1BF_LOW);
            out_RX1BF = PIN_LOW;
        }
        #endif

        /* USB受信バッファにデータが存在する分を受信 */
        #if 0
        if (tud_cdc_available() > 0u)
        {
            tud_cdc_read(tinyusb_in_buf, 1u);
            busy_wait_us_32(10000);
            printf("****Tiny USB Received :%c\n", tinyusb_in_buf[0u]);
        }
        #endif
        mcp2515_can_recv();     /* CAN受信処理 */
        mcp2515_can_send();     /* CAN送信処理 */
        sleep_ms(1000u);
    }
}
