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

}

void kWaitUsingDirectPIT(WORD wCount) {

}
