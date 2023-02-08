//
// Created by jewoo on 2023-02-01.
//

#ifndef CRAFTING_MULTIPROCESSOR_H
#define CRAFTING_MULTIPROCESSOR_H

#include "Types.h"

/// MultiProcessor 관련 매크로  (부팅 과정을 지원하는 코어)
#define BOOTSTRAP_PROCESSOR_FLAG_ADDRESS 0x7C09


/// 지원 가능한 최대 프로세서 또는 코어의 개수
#define MAX_PROCESSOR_COUNT             16

// 함수
BOOL kStartUpApplicationProcessor(void);

BYTE kGetAPICID(void);

static BOOL kWakeUpApplicationProcessor(void);

#endif //CRAFTING_MULTIPROCESSOR_H
