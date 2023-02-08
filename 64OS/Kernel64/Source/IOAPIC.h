//
// Created by jewoo on 2023-02-01.
//

#ifndef CRAFTING_IOAPIC_H
#define CRAFTING_IOAPIC_H

#include "Types.h"

//============= I/O 리다이렉션 테이블을 초기화하는 과정 =================
/*
 * MP 설정 테이블의 버스 엔트리를 검색하여 ISA버스 엔트리를 구함
 *                        |
 * MP 설정 테이블의 I/O 인터럽트 지정 엔트리를 검색하여 ISA 버스에서 인터럽트가 전달되는 엔트리를 구함
 *                        |
 * I/O APIC의 I/O 리다이렉션 테이블을 모드 마스크하여 인터럽트가 발생하지 못하도록 설정
 *                        |
 *  |------------------------------------------------------------------------
 *  | MP 설정 테이블의 I/O 인터럽트 지정 엔트리 재검색
 *  |------------------------------------------------------------------------
 *  | I/O 인터럽트 지정 엔트리의 버스 ID필드가 ISA 버스와 일치하는 것만 검색
 *  |                     |
 *  | I/O 인터럽트 지정 엔트리의 IRQ 필드와 극성. 트리거 모드를 참고하여 IO APIC의 IO 리다이렉션 테이블 설정
 *  |-----------------------------------------------------------------------------
 *  |                     |
 *  |                     | 모든 엔트리를 검색
 *  |                     |
 *  | I/O 리다이렉션 테이블 설정 완료
 *  |-------------------------------------------------------------------------------
 * */


// 매크로

/// I/O APIC 레지스터 오프셋 관련 매크로
#define IOAPIC_REGISTER_IO_REGISTER_SELECTOR                        0x00
#define IOAPIC_REGISTER_IO_WINDOW                                   0x10

/// 위의 두 레지스터로 접근할 때 사용하는 레지스터의 인덱스
#define IOAPIC_REGISTER_INDEX_IOAPIC_ID                             0x00
#define IOAPIC_REGISTER_INDEX_IOAPIC_VERSION                        0x01
#define IOAPIC_REGISTER_INDEX_IOAPIC_ARBID                          0x02
#define IOAPIC_REGISTER_INDEX_LOW_IO_REDIRECTION_TABLE              0x10
#define IOAPIC_REGISTER_INDEX_HIGH_IO_REDIRECTION_TABLE             0x11

/// I/O 리다이렉션 테이블의 최대 개수
#define IOAPIC_MAXIO_REDIRECTION_TABLE_COUNT                        24

/// 인터럽트 마스크 (Interrupt Mask)
#define IOAPIC_INTERRUPT_MASK                                       0x01

/// 트리거 모드 (Trigger Mode)
#define IOAPIC_TRIGGERMODE_LEVEL                                    0x80
#define IOAPIC_TRIGGERMODE_EDGE                                     0x00

/// 리모트 IRR (Remote IRR)
#define IOAPIC_REMOTE_IRR_EOI                                       0x40
#define IOAPIC_REMOTE_IRR_ACCEPT                                    0x00

/// 인터럽트 입력 핀 극성 (Interrupt Input Pin Polarity)
#define IOAPIC_POLARITY_ACTIVE_LOW                      0x20
#define IOAPIC_POLARITY_ACTIVE_HIGH                     0x00

/// 전달 상태 (Delivery Status)
#define IOAPIC_DELIVERY_STATUS_SEND_PENDING             0x10
#define IOAPIC_DELIVERY_STATUS_IDLE 0x00

/// 목적지 모드 (Destination Mode)
#define IOAPIC_DESTINATIONMODE_LOGICAL_MODE             0x08
#define IOAPIC_DESTINATIONMODE_PHYSICAL_MODE            0x00

/// 전달모드 (Delivery Mode)
#define IOAPIC_DELIVERYMODE_FIXED                   0x00
#define IOAPIC_DELIVERYMODE_LOWEST_PRIORITY         0x01
#define IOAPIC_DELIVERYMODE_SMI                     0x02
#define IOAPIC_DELIVERYMODE_NMI                     0x04
#define IOAPIC_DELIVERYMODE_INIT                    0x05
#define IOAPIC_DELIVERYMODE_EXTINT                  0x07

/// IRQ를 I/O APIC의 인터럽트 입력 핀으로 대응시키는 테이블의 최대 크기
#define IOAPIC_MAX_IRQ_TO_INTIN_MAP_COUNT               16

// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)
/// I/O 리다이렉션 테이블의 자료구조
typedef struct kIORedirectionTableStruct {

    /// 인터럽트 벡터
    BYTE bVector;

    /// 트리거 모드, 리모트 IRR, 인터럽트 입력 핀 극성, 전달 상태, 목적지 모드, 전달 모드를 담당하는 필드
    BYTE bFlagsAndDeliveryMode;

    /// 인터럽트 마스크
    BYTE bInterruptMask;

    /// 예약된 영역
    BYTE vbReserved[4];

    /// 인터럽트를 전달할 APIC ID
    BYTE bDestination;

} IOREDIRECTION_TABLE;

#pragma pack(pop)

/// I/O APIC를 관리하는 자료구조
typedef struct kIOAPICManagerStruct {
    /// ISA 버스가 연결된 I/O APIC의 메모리 맴 어드레스
    QWORD qwIOAPICBaseAddressOfISA;

    /// IRQ와 I/O APIC의 인터럽트 입력 핀 간의 연결관계를 저장하는 테이블
    BYTE vbIRAToINTINMap[IOAPIC_MAX_IRQ_TO_INTIN_MAP_COUNT];

} IOAPIC_MANAGER;


// 함수
QWORD kGetIOAPICBaseAddressOfISA(void);

void kSetIOAPICRedirectionEntry(IOREDIRECTION_TABLE *pstEntry, BYTE bAPICID,
                                BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode,
                                BYTE bVector);

void kReadIOAPICRedirectionTable(int iINTIN, IOREDIRECTION_TABLE *pstEntry);

void kWriteIOAPICRedirectionTable(int iINTIN, IOREDIRECTION_TABLE *pstEntry);

void kMaskAllInterruptInIOAPIC(void);

void kInitializeIORedirectionTable(void);

void kPrintIRQToINTINMap(void);

void kRoutingIRQToAPICID(int iIRQ, BYTE bAPICID);

#endif //CRAFTING_IOAPIC_H
