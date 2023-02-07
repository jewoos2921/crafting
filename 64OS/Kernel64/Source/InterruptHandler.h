//
// Created by jewoo on 2023-01-02.
//

#ifndef CRAFTING_INTERRUPTHANDLER_H
#define CRAFTING_INTERRUPTHANDLER_H

#include "Types.h"
#include "MultiProcessor.h"

// 매크로

/// 인터럽트 벡터의 최대 개수, ISA 버스의 인터럽트만 처리하므로 16
#define INTERRUPT_MAX_VECTOR_COUNT                  16

/// 인터럽트 부하 분산을 수행하는 시점, 인터럽트 처리 횟수가 10의 배수가 되는 시점
#define INTERRUPT_LOAD_BALANCEING_DIVIDOR           10

// 구조체
/// 인터럽트에 관련된 정보를 저장하는 자료구조
typedef struct kInterruptManagerStruct {
    /// 코어 별 인터럽트 처리 횟수, 최대 코어 개수 X 최대 인터럽트 벡터 개수로 정의된 2차원 배열
    QWORD vvqwCoreInterruptControl[MAX_PROCESSOR_COUNT][INTERRUPT_MAX_VECTOR_COUNT];

    /// 부하 분산 기능 사용 여부
    BOOL bUseLoadBalancing;

    /// 대칭 I/O 모드 사용 여부
    BOOL bSymmetricIOMode;
} INTERRUPT_MANAGER;

// 함수

void kSetSymmetricIOMode(BOOL bSymmetricIOMode);

void kSetInterruptLoadBalancing(BOOL bUseLoadBalancing);

void kIncreaseInterruptCount(int iIRQ);

void kSendEOI(int iIRQ);

INTERRUPT_MANAGER *kGetInterruptManager(void);

void kProcessLoadBalancing(int iIRQ);


void kCommonExceptionHanlder(int iVectorNumber, QWORD qwErrorCode);

void kCommonInterruptHandler(int iVectorNumber);

void kKeyboardHandler(int iVectorNumber);

void kTimerHandler(int iVectorNumber);

void kDeviceNotAvailableHandler(int iVectorNumber);

void kHDDHandler(int iVectorNumber);

#endif //CRAFTING_INTERRUPTHANDLER_H
