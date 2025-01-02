/*
  MIT License

  Copyright (c) 2018 Antonio Alexander Brewer (tonton81) - https://github.com/tonton81

  Designed and tested for PJRC Teensy 4.0.

  Forum link : https://forum.pjrc.com/threads/56035-FlexCAN_T4-FlexCAN-for-Teensy-4?highlight=flexcan_t4

  Thanks goes to skpang, mjs513, and collin for tech/testing support

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#if !defined(_FLEXCAN_T4_H_) && defined(NATIVE)
#define _FLEXCAN_T4_H_

#include <Arduino.h>

typedef struct CAN_error_t {
  char state[30] = "Idle";
  bool BIT1_ERR = 0;
  bool BIT0_ERR = 0;
  bool ACK_ERR = 0;
  bool CRC_ERR = 0;
  bool FRM_ERR = 0;
  bool STF_ERR = 0;
  bool TX_WRN = 0;
  bool RX_WRN = 0;
  char FLT_CONF[14] = { 0 };
  uint8_t RX_ERR_COUNTER = 0;
  uint8_t TX_ERR_COUNTER = 0;
  uint32_t ESR1 = 0;
  uint16_t ECR = 0;
} CAN_error_t;

typedef struct CAN_message_t {
  uint32_t id = 0;          // can identifier
  uint16_t timestamp = 0;   // FlexCAN time when message arrived
  uint8_t idhit = 0; // filter that id came from
  struct {
    bool extended = 0; // identifier is extended (29-bit)
    bool remote = 0;  // remote transmission request packet type
    bool overrun = 0; // message overrun
    bool reserved = 0;
  } flags;
  uint8_t len = 8;      // length of data
  uint8_t buf[8] = { 0 };       // data
  int8_t mb = 0;       // used to identify mailbox reception
  uint8_t bus = 0;      // used to identify where the message came from when events() is used.
  bool seq = 0;         // sequential frames
} CAN_message_t;

typedef void (*_MB_ptr)(const CAN_message_t &msg); /* mailbox / global callbacks */

typedef enum FLEXCAN_PINS {
  ALT = 0,
  DEF = 1,
} FLEXCAN_PINS;

typedef enum FLEXCAN_MAILBOX {
  MB0 = 0,
  MB1 = 1,
  MB2 = 2,
  MB3 = 3,
  MB4 = 4,
  MB5 = 5,
  MB6 = 6,
  MB7 = 7,
  MB8 = 8,
  MB9 = 9,
  MB10 = 10,
  MB11 = 11,
  MB12 = 12,
  MB13 = 13,
  MB14 = 14,
  MB15 = 15,
  MB16 = 16,
  MB17 = 17,
  MB18 = 18,
  MB19 = 19,
  MB20 = 20,
  MB21 = 21,
  MB22 = 22,
  MB23 = 23,
  MB24 = 24,
  MB25 = 25,
  MB26 = 26,
  MB27 = 27,
  MB28 = 28,
  MB29 = 29,
  MB30 = 30,
  MB31 = 31,
  MB32 = 32,
  MB33 = 33,
  MB34 = 34,
  MB35 = 35,
  MB36 = 36,
  MB37 = 37,
  MB38 = 38,
  MB39 = 39,
  MB40 = 40,
  MB41 = 41,
  MB42 = 42,
  MB43 = 43,
  MB44 = 44,
  MB45 = 45,
  MB46 = 46,
  MB47 = 47,
  MB48 = 48,
  MB49 = 49,
  MB50 = 50,
  MB51 = 51,
  MB52 = 52,
  MB53 = 53,
  MB54 = 54,
  MB55 = 55,
  MB56 = 56,
  MB57 = 57,
  MB58 = 58,
  MB59 = 59,
  MB60 = 60,
  MB61 = 61,
  MB62 = 62,
  MB63 = 63,
  FIFO = 99
} FLEXCAN_MAILBOX;

typedef enum FLEXCAN_RXTX {
  TX,
  RX,
  LISTEN_ONLY
} FLEXCAN_RXTX;

typedef enum FLEXCAN_CLOCK {
  CLK_OFF,
  CLK_8MHz = 8,
  CLK_16MHz = 16,
  CLK_20MHz = 20,
  CLK_24MHz = 24,
  CLK_30MHz = 30,
  CLK_40MHz = 40,
  CLK_60MHz = 60,
  CLK_80MHz = 80
} FLEXCAN_CLOCK;

typedef enum FLEXCAN_IDE {
  NONE = 0,
  EXT = 1,
  RTR = 2,
  STD = 3,
  INACTIVE
} FLEXCAN_IDE;

typedef enum FLEXCAN_FLTEN {
  ACCEPT_ALL = 0,
  REJECT_ALL = 1
} FLEXCAN_FLTEN;

typedef enum FLEXCAN_FILTER_TABLE {
  FLEXCAN_MULTI = 1,
  FLEXCAN_RANGE = 2,
  FLEXCAN_TABLE_B_MULTI = 3,
  FLEXCAN_TABLE_B_RANGE = 4,
  FLEXCAN_USERMASK = 5
} FLEXCAN_FILTER_TABLE;

typedef enum FLEXCAN_FIFOTABLE {
  A = 0,
  B = 1,
  C = 2
} FLEXCAN_FIFOTABLE;

typedef enum FLEXCAN_RXQUEUE_TABLE {
  RX_SIZE_2 = (uint16_t)2,
  RX_SIZE_4 = (uint16_t)4,
  RX_SIZE_8 = (uint16_t)8,
  RX_SIZE_16 = (uint16_t)16,
  RX_SIZE_32 = (uint16_t)32,
  RX_SIZE_64 = (uint16_t)64,
  RX_SIZE_128 = (uint16_t)128,
  RX_SIZE_256 = (uint16_t)256,
  RX_SIZE_512 = (uint16_t)512,
  RX_SIZE_1024 = (uint16_t)1024
} FLEXCAN_RXQUEUE_TABLE;

typedef enum FLEXCAN_DLC_SIZE {
  DLC_SIZE_8 = (uint16_t)8,
  DLC_SIZE_12 = (uint16_t)12,
  DLC_SIZE_16 = (uint16_t)16,
  DLC_SIZE_20 = (uint16_t)20,
  DLC_SIZE_24 = (uint16_t)24,
  DLC_SIZE_32 = (uint16_t)32,
  DLC_SIZE_48 = (uint16_t)48,
  DLC_SIZE_64 = (uint16_t)64
} FLEXCAN_DLC_SIZE;

typedef enum FLEXCAN_RFFN_TABLE {
  RFFN_8 = (uint8_t)0,
  RFFN_16 = (uint8_t)1,
  RFFN_24 = (uint8_t)2,
  RFFN_32 = (uint8_t)3,
  RFFN_40 = (uint8_t)4,
  RFFN_48 = (uint8_t)5,
  RFFN_56 = (uint8_t)6,
  RFFN_64 = (uint8_t)7,
  RFFN_72 = (uint8_t)8,
  RFFN_80 = (uint8_t)9,
  RFFN_88 = (uint8_t)10,
  RFFN_96 = (uint8_t)11,
  RFFN_104 = (uint8_t)12,
  RFFN_112 = (uint8_t)13,
  RFFN_120 = (uint8_t)14,
  RFFN_128 = (uint8_t)15
} FLEXCAN_RFFN_TABLE;

typedef enum FLEXCAN_TXQUEUE_TABLE {
  TX_SIZE_2 = (uint16_t)2,
  TX_SIZE_4 = (uint16_t)4,
  TX_SIZE_8 = (uint16_t)8,
  TX_SIZE_16 = (uint16_t)16,
  TX_SIZE_32 = (uint16_t)32,
  TX_SIZE_64 = (uint16_t)64,
  TX_SIZE_128 = (uint16_t)128,
  TX_SIZE_256 = (uint16_t)256,
  TX_SIZE_512 = (uint16_t)512,
  TX_SIZE_1024 = (uint16_t)1024
} FLEXCAN_TXQUEUE_TABLE;

typedef enum CAN_DEV_TABLE {
  CAN0 = (uint32_t)0x0,
  CAN1 = (uint32_t)0x401D0000,
  CAN2 = (uint32_t)0x401D4000,
  CAN3 = (uint32_t)0x401D8000
} CAN_DEV_TABLE;

#define FCTP_CLASS template<CAN_DEV_TABLE _bus, FLEXCAN_RXQUEUE_TABLE _rxSize = RX_SIZE_16, FLEXCAN_TXQUEUE_TABLE _txSize = TX_SIZE_16>
#define FCTP_FUNC template<CAN_DEV_TABLE _bus, FLEXCAN_RXQUEUE_TABLE _rxSize, FLEXCAN_TXQUEUE_TABLE _txSize>
#define FCTP_OPT FlexCAN_T4<_bus, _rxSize, _txSize>

#define SIZE_LISTENERS 4

inline bool operator==(const CAN_message_t &lhs, const CAN_message_t &rhs) {
    // Two CAN messages are equal if their ID and buffer match
    return (lhs.id == rhs.id) && (std::memcmp(lhs.buf, rhs.buf, sizeof(lhs.buf)) == 0);
}

FCTP_CLASS class FlexCAN_T4 {
public:
    FlexCAN_T4() = default;
    virtual ~FlexCAN_T4() = default;
    virtual void begin() = 0;
    virtual void setBaudRate(uint32_t baud = 1000000) = 0;
    virtual int read(CAN_message_t &msg) = 0;
    virtual int write(const CAN_message_t &msg) = 0;
};

#endif