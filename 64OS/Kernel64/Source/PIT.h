//
// Created by jewoo on 2023-01-09.
//

#ifndef CRAFTING_PIT_H
#define CRAFTING_PIT_H

#include "Types.h"

#define PIT_FREQUENCY                               1193182
#define MSTOCOUNT(x)                                (PIT_FREQUENCY * (x) / 100)
#define USTOCOUNT(x)                                (PIT_FREQUENCY * (x) / 100000)

// I/O 포트
#define PIT_PORT_CONTROL                            0x43
#define PIT_PORT_COUNTER0                           0x40
#define PIT_PORT_COUNTER1                           0x41
#define PIT_PORT_COUNTER2                           0x42

// 모드
#define PIT_CONTROL_COUNTER0                        0x00
#define PIT_CONTROL_COUNTER1                        0x40
#define PIT_CONTROL_COUNTER2                        0x80
#define PIT_CONTROL_LSBMSBRW                        0x30
#define PIT_CONTROL_LATCH                           0x00
#define PIT_CONTROL_MODE0                           0x00
#define PIT_CONTROL_MODE2                           0x04

// Binary or BCD
#define PIT_CONTROL_BINARY_COUNTER                  0x00
#define PIT_CONTROL_BCD_COUNTER                     0x01

#define PIT_COUNTER0_ONCE                           (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LSBMSBRW | PIT_CONTROL_MODE0 | PIT_CONTROL_BINARY_COUNTER)
#define PIT_COUNTER0_PERIODIC                       (PIT_CONTROL_COUNTER0 |PIT_CONTROL_LSBMSBRW | PIT_CONTROL_MODE2 | PIT_CONTROL_BINARY_COUNTER)
#define PIT_COUNTER0_LATCH                          (PIT_CONTROL_COUNTER0 | PIT_CONTROL_LATCH)

// 함수
void kInitializePIT(WORD wCount, BOOL bPeriodic);

WORD kReadCounter0(void);

void kWaitUsingDirectPIT(WORD wCount);


#endif //CRAFTING_PIT_H
