//
// Created by jewoo on 2023-02-01.
//

#ifndef CRAFTING_LOCALAPIC_H
#define CRAFTING_LOCALAPIC_H

#include "Types.h"

/// 로컬 APIC 레지스터 오프셋 관련 매크로
#define APIC_REGISTER_EOI                                       0x0000B0
#define APIC_REGISTER_SVR                                       0x0000F0
#define APIC_REGISTER_APICID                                    0x000020
#define APIC_REGISTER_TASK_PRIORITY                             0x000080
#define APIC_REGISTER_TIMER                                     0x000320
#define APIC_REGISTER_THERMAL_SENSOR                            0x000330
#define APIC_REGISTER_PERFORMANCE_MONITORING_COUNTER            0x000340
#define APIC_REGISTER_LINT0                                     0x000350
#define APIC_REGISTER_LINT1                                     0x000360
#define APIC_REGISTER_ERROR                                     0x000370
#define APIC_REGISTER_ICR_LOWER                                 0x000300
#define APIC_REGISTER_ICR_UPPER                                 0x000310


/// 전달 모드(Delivery Mode) 관련 매크로
#define APIC_DERIVERYMODE_FIXED                                 0x000000
#define APIC_DERIVERYMODE_LOWEST_PRIORITY                       0x000100
#define APIC_DERIVERYMODE_SMI                                   0x000200
#define APIC_DERIVERYMODE_NMI                                   0x000400
#define APIC_DERIVERYMODE_INIT                                  0x000500
#define APIC_DERIVERYMODE_STARTUP                               0x000600
#define APIC_DERIVERYMODE_EXTINT                                0x000700



/// 목적지 모드(Destination Mode) 관련 매크로
#define APIC_DESTINATION_MODE_PHYSICAL                               0x000000
#define APIC_DESTINATION_MODE_LOGICAL                                0x000800

/// 전달 상태(Delivery Status) 관련 매크로
#define APIC_DERIVERY_STATUS_IDLE                                   0x000000
#define APIC_DERIVERY_STATUS_PENDING                                0x001000

/// 레벨(Level) 관련 매크로
#define APIC_LEVEL_DEASSERT                                     0x000000
#define APIC_LEVEL_ASSERT                                       0x004000

/// 트리거 모드 (Trigger Mode) 관련 매크로
#define APIC_TRIGGER_MODE_EDGE                                  0x000000
#define APIC_TRIGGER_MODE_LEVEL                                 0x008000

/// 목적지 약어 (Destination Shorthand) 관련 매크로
#define APIC_DESTINATION_SHORTHAND_NO_SHORTHAND                 0x000000
#define APIC_DESTINATION_SHORTHAND_SELF                         0x040000
#define APIC_DESTINATION_SHORTHAND_ALL_INCLUDING_SELF           0x080000
#define APIC_DESTINATION_SHORTHAND_ALL_EXCLUDING_SELF           0x0C0000

/// 인터럽트 마스크 관련 매크로
#define APIC_INTERRUPT_MASK                                     0x010000

/// 타이머 모드 관련 매크로
#define APIC_TIMERMODE_PREIODIC                                 0x020000
#define APIC_TIMERMODE_ONESHOT                                  0x000000

/// 인터럽트 입력 핀 극성 관련 매크로
#define APIC_POLARITY_ACTIVE_LOW                                0x002000
#define APIC_POLARITY_ACTIVE_HIGH                               0x000000


// 함수
QWORD kGetLocalAPICBaseAddress(void);

void kEnableSoftwareLocalAPIC(void);

void kSendEOIToLocalAPIC(void);

void kSetTaskPriority(BYTE bPriority);

void kInitializeLocalVectorTable(void);

#endif //CRAFTING_LOCALAPIC_H
