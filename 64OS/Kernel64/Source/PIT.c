//
// Created by jewoo on 2023-01-09.
//
// 컨트롤러를 제어
// (Programmable Interval Timer) 컨트롤러의 레지스터에 주기를 설정함으로써 발생 간격을 조절

#include "PIT.h"
#include "AssemblyUtility.h"

// PIT를 초기화
void kInitializePIT(WORD wCount, BOOL bPeriodic) {

    // PIT 컨트롤 레지스터(포트 0x43)에 값을 초기화해서 카운트를 멈툰 뒤에 모드 0에 바이너리 카운터로 설정
    kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_ONCE);

    // 만약 일정한 주기로 반복하는 타이머라면 모드 2로 설정
    if (bPeriodic == TRUE) {
        // PIT 컨트롤 레지스터(포트 0x43)에 모드 2에 바이터리 카운터로 설정
        kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC);
    }

    // 카운터0(포트 0X40)에 LSB -> MSB 순으로 카운터 초기 값을 설정
    kOutPortByte(PIT_PORT_COUNTER0, wCount);
    kOutPortByte(PIT_PORT_COUNTER0, wCount >> 8);
}

// 카운터 0의 현재 값을 반환
WORD kReadCounter0(void) {
    BYTE bHighByte, bLowByte;
    WORD wTemp = 0;

    // PIT 컨트롤 레지스터(포트 0x43)에 래치 커맨드를 전송하여 카운터 0에 있는 현재 값을 읽음
    kOutPortByte(PIT_PORT_CONTROL, PIT_COUNTER0_LATCH);


    // 카운터 0(포트 0x40)에서 LSB -> MSB 순으로 카운터 값을 읽음
    bLowByte = kInPortByte(PIT_PORT_COUNTER0);
    bHighByte = kInPortByte(PIT_PORT_COUNTER0);

    // 읽은 값을 16비트로 합하여 반환
    wTemp = bHighByte;
    wTemp = (wTemp << 8) | bLowByte;
    return wTemp;
}


// 카운터 0을 직접 설정하여 일정 시간 이상 대기
//      함수를 호출하려면 PIT 컨트롤러의 설저이 바뀌므로, 이후에 PIT 컨트롤러를 재설정해야 함
//      정확하게 측정하려면 함수 사용전에 인터럽트를 비활성화 하는 것이 좋음
//      약 50ms 까지 측정 가능
void kWaitUsingDirectPIT(WORD wCount) {
    WORD wLastCounter0;
    WORD wCurrentCounter0;

    // PIT 컨트롤러 0-0xFFFF까지 반복해서 카운팅하도록 설정
    kInitializePIT(0, TRUE);

    // 지금부터 wCount 이상 증가할 때까지 대기
    wLastCounter0 = kReadCounter0();
    while (1) {
        // 현재 카운터 0의 값을 반환
        wCurrentCounter0 = kReadCounter0();
        if (((wLastCounter0 - wCurrentCounter0) & 0xFFFF) >= wCount) {
            break;
        }
    }
}
