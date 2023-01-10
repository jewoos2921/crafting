//
// Created by jewoo on 2023-01-02.
//

#ifndef CRAFTING_PIC_H
#define CRAFTING_PIC_H

#include "Types.h"

// 매크로
// I/O 포트 정의

#define PIC_MASTER_PORT1                                0x20
#define PIC_MASTER_PORT2                                0x21
#define PIC_SLAVE_PORT1                                 0xA0
#define PIC_SLAVE_PORT2                                 0xA1

// IDT 테이블에서 인터럽트 벡터가 시작되는 위치, 0x20
#define PIC_IRQ_START_VECTOR                            0x20

// 함수
void kInitializePIC(void);

void kMaskPICInterrupt(WORD wIRQBitmask);

void kSendEOIToPIC(int iIRQNumber);


#endif //CRAFTING_PIC_H
