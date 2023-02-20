//
// Created by jewoo on 2023-01-31.
//

#include "SerialPort.h"
#include "Utility.h"
#include "AssemblyUtility.h"

/// 시리얼 포트를 담당하는 자료구조
static SERIAL_MANAGER gs_stSerialManager;


///시리얼 포트 초기화
void kInitializeSerialPort(void) {
    WORD wPortBaseAddress;

    /// 뮤텍스 초기화
    kInitializeMutex(&(gs_stSerialManager.stLock));

    /// COM1 시리얼 포트 (0x3F8)를 선택하여 초기화
    wPortBaseAddress = SERIAL_PORT_COM1;

    /// 인터럽트 활성화 레지스터 (0x3F9)에 0을 전송하여 모든 인터럽트를 비활성화
    kOutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_INTERRUPT_ENABLE, 0);
    /// 통신속도를 115200으로 설정
    /// 라인 제어 레지스터(0x3FB)의 DLAB 비트(비트 7)를 1로 설정하여 제수 래치 레지스터에 접근
    kOutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_LINE_CONTROL,
                 SERIAL_LINE_CONTROL_DLAB);

    /// LSB 제수 래치 레지스터(0x3F8)에 재수의 하위 8비트를 전송
    kOutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_DIVISOR_LATCH_LSB, SERIAL_DIVISOR_LATCH_115200);
    /// MSB 제수 래치 레지스터(0x3F9)에 재수의 상위 8비트를 전송
    kOutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_DIVISOR_LATCH_MSB,
                 SERIAL_DIVISOR_LATCH_115200 >> 8);

    /// 송수신 방법을 설정
    /// 라인 제어 레지스터(0x3FB)에 통신 방법을 8비트, 패리티 없음
    /// 1 Stop 비트로 설정하고, 제수 래치 레지스터 사용이 끝났으므로 DLAB 비트를 0으로 설정
    kOutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_LINE_CONTROL,
                 SERIAL_LINE_CONTROL_8BIT | SERIAL_LINE_CONTROL_NO_PARITY | SERIAL_LINE_CONTROL_1BIT_STOP);

    /// FIFO의 인터럽트 발생 시점을 14바이트로 설정
    kOutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_FIFO_CONTROL,
                 SERIAL_FIFO_CONTROL_FIFO_ENABLE | SERIAL_FIFO_CONTROL_14BYTE_FIFO);
}

/// 송신 FIFO가 비어 있는지를 확인
static BOOL kIsSerialTransmitterBufferEmpty(void) {
    BYTE bData;

    /// 라인 상태 레지스터(포트 0x3FD)을 읽은 뒤 TBE 비트(비트 1)를 확인하여 송신 FIFO가 비어 있는지를 확인
    bData = kInPortByte(SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_LINE_STATUS);
    if ((bData & SERIAL_LINE_STATUS_TRANSMIT_BUFFER_EMPTY) == SERIAL_LINE_STATUS_TRANSMIT_BUFFER_EMPTY) {
        return TRUE;
    }

    return FALSE;
}

/// 시리얼 포트로 데이터를 송신
void kSendSerialData(BYTE *pbBuffer, int iSize) {
    int iSentByte;
    int iTempSize;
    int j;

    /// 동기화
    kLock(&(gs_stSerialManager.stLock));

    /// 요청한 바이트 수만큼 보낼 때가지 반복
    iSentByte = 0;
    while (iSentByte < iSize) {
        /// 송신 FIFO에 데이터가 남아 있다면 다 전송될 때까지 대기
        while (kIsSerialTransmitterBufferEmpty() == FALSE) {
            kSleep(0);
        }

        /// 전송할 데이터 중에서 남은 크기와 FIFO의 최대 크기(16바이트)를 비교한 후
        /// 작은 것을 선택하여 송신 시리얼 포트를 채움
        iTempSize = MIN(iSize - iSentByte, SERIAL_FIFO_MAX_SIZE);
        for (j = 0; j < iTempSize; j++) {
            /// 송신 버퍼 레지스터 (0x3F8)에 한 바이트를 전송
            kOutPortByte(SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_TRANSMIT_BUFFER,
                         pbBuffer[iSentByte + j]);
        }

        iSentByte += iTempSize;
    }

    /// 동기화
    kUnlock(&(gs_stSerialManager.stLock));
}

/// 수신 FIFO가 데이터가 있는지를 확인
static BOOL kIsSerialReceiveBufferFull(void) {
    BYTE bData;
    /// 라인 상태 레지스터(포트 0x3FD)을 읽은 뒤 RxRD 비트(비트 0)를 확인하여 수신 FIFO가 데이터가 있는지를 확인
    bData = kInPortByte(SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_LINE_STATUS);
    if ((bData & SERIAL_LINE_STATUS_RECEIVED_DATA_READY) == SERIAL_LINE_STATUS_RECEIVED_DATA_READY) {
        return TRUE;
    }

    return FALSE;
}

/// 시리얼 포트에서 데이터를 읽음
int kReceiveSerialData(BYTE *pbBuffer, int iSize) {
    int i;

    /// 동기화
    kLock(&(gs_stSerialManager.stLock));

    for (i = 0; i < iSize; i++) {
        /// 버퍼에 데이터가 없으면 중지
        if (kIsSerialReceiveBufferFull() == FALSE) {
            break;
        }

        /// 수신 버퍼에 레지스터(0x3F8_에서 한 바이트를 읽음
        pbBuffer[i] = kInPortByte(SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_RECEIVE_BUFFER);
    }

    /// 동기화
    kUnlock(&(gs_stSerialManager.stLock));

    /// 읽은 데이터 개수를 반환
    return i;
}

/// 시리얼 포트 컨트롤러의 FIFO를 초기화
void kClearSerialFIFO(void) {
    /// 동기화
    kLock(&(gs_stSerialManager.stLock));

    /// 송수신 FIFO를 모두 비우고 버퍼에 데이터가 14바이트 찼을 때 인터럽트가 발생하도록 FIFO 제어 레지스터(0x3FA)에 설정값 정송
    kOutPortByte(SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_FIFO_CONTROL,
                 (SERIAL_FIFO_CONTROL_FIFO_ENABLE | SERIAL_FIFO_CONTROL_14BYTE_FIFO
                  | SERIAL_FIFO_CONTROL_CLEAR_RECEIVE_FIFO | SERIAL_FIFO_CONTROL_CLEAR_TRASNMIT_FIFO));

    /// 동기화
    kUnlock(&(gs_stSerialManager.stLock));
}
