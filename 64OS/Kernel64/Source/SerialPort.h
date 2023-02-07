//
// Created by jewoo on 2023-01-31.
//

#ifndef CRAFTING_SERIALPORT_H
#define CRAFTING_SERIALPORT_H

#include "Types.h"
#include "Queue.h"
#include "Synchronization.h"


// 매크로

/// 시리얼 포트의 I/O 포트 기준 어드레스
#define SERIAL_PORT_COM1                                            0x3F8
#define SERIAL_PORT_COM2                                            0x2F8
#define SERIAL_PORT_COM3                                            0x3E8
#define SERIAL_PORT_COM4                                            0x2E8

/// 각 레지스터의 오프셋
#define SERIAL_PORT_INDEX_RECEIVE_BUFFER                                0x00
#define SERIAL_PORT_INDEX_TRANSMIT_BUFFER                               0x00
#define SERIAL_PORT_INDEX_INTERRUPT_ENABLE                              0x01
#define SERIAL_PORT_INDEX_DIVISOR_LATCH_LSB                             0x00
#define SERIAL_PORT_INDEX_DIVISOR_LATCH_MSB                             0x01
#define SERIAL_PORT_INDEX_INTERRUPT_IDENTIFICATION                      0x02
#define SERIAL_PORT_INDEX_FIFO_CONTROL                                  0x02
#define SERIAL_PORT_INDEX_LINE_CONTROL                                  0x03
#define SERIAL_PORT_INDEX_MODEM_CONTROL                                 0x04
#define SERIAL_PORT_INDEX_LINE_STATUS                                   0x05
#define SERIAL_PORT_INDEX_MODEM_STATUS                                  0x06

/// 인터럽트 활성화 레지스터에 관한 매크로
#define SERIAL_INTERRUPT_ENABLE_RECEIVE_BUFFER_FULL         0x01
#define SERIAL_INTERRUPT_ENABLE_TRANSIMITTER_BUFFER_EMPTY   0x02
#define SERIAL_INTERRUPT_ENABLE_LINE_STATUS                 0x04
#define SERIAL_INTERRUPT_ENABLE_DELTA_STATUS                0x08

/// FIFO 제어 레지스터에 관한 매크로
#define SERIAL_FIFO_CONTROL_FIFO_ENABLE                         0x01
#define SERIAL_FIFO_CONTROL_CLEAR_RECEIVE_FIFO                  0x01
#define SERIAL_FIFO_CONTROL_CLEAR_TRASNMIT_FIFO                 0x04
#define SERIAL_FIFO_CONTROL_ENABLE_DMA                          0x08
#define SERIAL_FIFO_CONTROL_1BYTE_FIFO                          0x00
#define SERIAL_FIFO_CONTROL_4BYTE_FIFO                          0x40
#define SERIAL_FIFO_CONTROL_8BYTE_FIFO                          0x80
#define SERIAL_FIFO_CONTROL_14BYTE_FIFO                         0xC0


//// 라인 제어 레지스터에 관한 매크로
#define SERIAL_LINE_CONTROL_8BIT                        0x03
#define SERIAL_LINE_CONTROL_1BIT_STOP                   0x00
#define SERIAL_LINE_CONTROL_NO_PARITY           0x00
#define SERIAL_LINE_CONTROL_ODD_PARITY      0x08
#define SERIAL_LINE_CONTROL_EVEN_PARITY         0x18
#define SERIAL_LINE_CONTROL_MARK_PARITY         0x28
#define SERIAL_LINE_CONTROL_SPACE_PARITY            0x38
#define SERIAL_LINE_CONTROL_DLAB                0x80

/// 라인 상태 레지스터에 관한 매크로
#define SERIAL_LINE_STATUS_RECEIVED_DATA_READY          0x01
#define SERIAL_LINE_STATUS_OVER_RUN_ERROR               0x02
#define SERIAL_LINE_STATUS_PARITY_ERROR                 0x04
#define SERIAL_LINE_STATUS_FRAMING_ERROR                0X08
#define SERIAL_LINE_STATUS_BREAK_INDICATOR              0x10
#define SERIAL_LINE_STATUS_TRANSMIT_BUFFER_EMPTY        0x20
#define SERIAL_LINE_STATUS_TRANSMIT_EMPTY               0x40
#define SERIAL_LINE_STATUS_RECEIVED_CHARACTOR_ERROR     0x80


/// 제수 래치 레지스터에 관한 매크로
#define SERIAL_DIVISOR_LATCH_115200             1
#define SERIAL_DIVISOR_LATCH_57600             2
#define SERIAL_DIVISOR_LATCH_38400             3
#define SERIAL_DIVISOR_LATCH_19200             6
#define SERIAL_DIVISOR_LATCH_9600             12
#define SERIAL_DIVISOR_LATCH_4800             24
#define SERIAL_DIVISOR_LATCH_2400             48


/// FIFO이 최대 크기
#define SERIAL_FIFO_MAX_SIZE                    16

// 구조체
/// 시리얼 포트를 담당하는 자료구조
typedef struct kSerialPortManager {
    /// 동기화 객체
    MUTEX stLock;
} SERIAL_MANAGER;

// 함수
void kInitializeSerialPort(void);

void kSendSerialData(BYTE *pbBuffer, int iSize);

int kReceiveSerialData(BYTE *pbBuffer, int iSize);

void kClearSerialFIFO(void);

static BOOL kIsSerialTransmitterBufferEmpty(void);

static BOOL kIsSerialReceiveBufferFull(void);


#endif //CRAFTING_SERIALPORT_H
