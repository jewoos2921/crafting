//
// Created by jewoo on 2023-01-31.
//

// MP 설정 테이블
#ifndef CRAFTING_MPCONFIGURATIONTABLE_H
#define CRAFTING_MPCONFIGURATIONTABLE_H

#include "Types.h"

/* 확장 BIOS 데이터 영역의 시작 어드레스에서 1KB 범위 내에 존재
 * - 확장 BIOS 데이터 영역의 시작 어드레스는 물리 메모리 어드레스 0x040E에 2바이트로 저장되어 있음
 * - 저장된 값은 확장 BIOS 데이터 영역이 시작하는 세그먼트의 어드레스임, 따라서 저장된 값에 16을 곱해야 실제 확장 BIOS 데이터 영역이 시작하는 실제 물리 어드레스가 됨
 * 시스템 기본 메모리의 끝부분에서 1KB 이하 범위에 존재
 * - 640KB의 기본 메모리를 가지고 있다면 639KB~640KB 범위 내에 존재
 * - 시스템 기본 메모리의 크기는 물리 메모리 어드레스 0x0413에 2바이트로 저장되어 있음
 * - 저장된 값의 단위는 KB이므로 1024를 곱하여 실제 크기로 변환해야 함
 * - BIOS의 롬 영역 중에서 0x0F0000~0x0FFFFF 범위 내에 존재
 */


// 매크로
/// MP 플로팅 포인터의 특성 바이트(Feature Byte)
#define MP_FLOATING_POINTER_FEATURE_BYTE1_USE_MP_TABLE      0x00
#define MP_FLOATING_POINTER_FEATURE_BYTE2_PIC_MODE          0x80

/// 엔트리 타입(Entry Type)
#define MP_ENTRY_TYPE_PROCESSOR                             0
#define MP_ENTRY_TYPE_BUS                                   1
#define MP_ENTRY_TYPE_IO_APIC                               2
#define MP_ENTRY_TYPE_IO_INTERRUPT_ASSIGNMENT               3
#define MP_ENTRY_TYPE_LOCAL_INTERRUPT_ASSIGNMENT            4

/// 프로세서 CPU 플래그
#define MP_PROCESSOR_CPU_FLAGS_ENABLE                       0x01
#define MP_PROCESSOR_CPU_FLAGS_BSP                          0x02


/// 버스 타입 스트링(Bus Type String)
#define MP_BUS_TYPE_STRING_ISA                             "ISA"
#define MP_BUS_TYPE_STRING_PCI                              "PCI"
#define MP_BUS_TYPE_STRING_PCMCIA                           "PCMCIA"
#define MP_BUS_TYPE_STRING_VESALOCALBUS                     "VL"

/// 인터럽트 타입(Interrupt Type)
#define MP_INTERRUPT_TYPE_INT                               0
#define MP_INTERRUPT_TYPE_NMI                               1
#define MP_INTERRUPT_TYPE_SMI                               2
#define MP_INTERRUPT_TYPE_EXTINT                            3

/// 인터럽트 플래그(Interrupt Flags)
#define MP_INTERRUPT_FLAGS_CONFORM_POLARITY                 0x00
#define MP_INTERRUPT_FLAGS_ACTIVE_HIGH                      0x01
#define MP_INTERRUPT_FLAGS_ACTIVE_LOW                       0x03
#define MP_INTERRUPT_FLAGS_CONFORM_TRIGGER                  0x00
#define MP_INTERRUPT_FLAGS_EDGE_TRIGGERED                   0x04
#define MP_INTERRUPT_FLAGS_LEVEL_TRIGGERED                  0x0C

// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

/// MP 플로팅 포인터 자료구조(MP Floating Pointer Data Structure)
typedef struct kMPFloatingPointerStruct {

    /// 시그너처, _MP_
    char vcSignature[4];

    /// MP 설정 테이블이 위치하는 어드레스
    DWORD dwMPConfigurationTableAddress;

    /// MP 플로팅 포인터 자료구조의 길이, 16바이트
    BYTE bLength;

    /// MultiProcessor Specification 버전
    BYTE bRevision;

    /// 체크섬
    BYTE bCheckSum;

    /// MP 특성 바이트 1~5
    BYTE vbMPFeatureByte[5];
} MP_FLOATING_POINTER;

/// MP 설정 테이블 헤더 (MP Configuration Table Header) 자료구조
typedef struct kMPConfigurationTableHeaderStruct {
    /// 시그너처, PCMP
    char vcSignature[4];

    /// 기본 테이블 길이
    WORD wBaseTableLength;

    /// MultiProcessor Specification 버전
    BYTE bRevision;

    /// 체크섬
    BYTE bCheckSum;

    /// OEM ID 문자열
    char vcOEMIDString[8];

    /// PRODUCT ID 문자열
    char vcProductIDString[12];

    /// OEM이 정의한 테이블의 어드레스
    DWORD dwOEMTablePointerAddress;

    /// OEM이 정의한 테이블의 크기
    WORD wOEMTableSize;

    /// 기본 MP 설정 테이블 엔트리의 개수
    WORD wEntryCount;

    /// 로컬 APIC의 메모리 맵 I/O 어드레스
    DWORD dwMemoryMapIOAddressOfLocalAPIC;

    /// 확장 테이블의 길이
    WORD wExtendedTableLength;

    /// 확장 테이블의 체크섬
    BYTE bExtendedTableChecksum;

    /// 예약됨
    BYTE bReserved;

} MP_CONFIGURATION_TABLE_HEADER;

/// 프로세서 엔트리 자료구조(Processor Entry)
typedef struct kProcessorEntryStruct {
    /// 엔트리 타입 코드, 0
    BYTE bEntryType;

    /// 프로세서에 포함된 로컬 APIC의 ID
    BYTE bLocalAPICID;

    /// 로컬 APIC의 버전
    BYTE bLocalAPICVersion;

    /// CPU 플래그
    BYTE bCPUFlags;

    /// CPU 시그너처
    BYTE vbCPUSignature[4];

    /// 특성 플래그
    DWORD dwFeatureFlags;

    /// 예약됨
    DWORD vdwReserved[2];
} PROCESSOR_ENTRY;

/// 버스 엔트리 자료구조(Bus Entry)
typedef struct kBusEntryStruct {

    /// 엔트리 타입 코드, 1
    BYTE bEntryType;

    /// 버스 ID
    BYTE bBusID;

    /// 버스 타입 문자열
    char vcBusTypeString[6];

} BUS_ENTRY;

/// I/O APIC 엔트리 자료구조
typedef struct kIOAPICEntryStruct {
    /// 엔트리 타입 코드, 2
    BYTE bEntryType;

    /// I/O APIC ID
    BYTE bIOAPICID;

    /// I/O APIC 버전
    BYTE bIOAPICVersion;

    /// I/O APIC 플래그
    BYTE bIOAPICFlags;

    /// 메모리 맵 I/O 어드레스
    DWORD dwMemoryMapAddress;
} IO_APIC_ENTRY;

/// I/O 인터럽트 지정 엔트리 자료구조 (I/O Interrupt Assignment Entry)
typedef struct kIOInterruptAssignmentEntryStruct {
    /// 엔트리 타입 코드, 3
    BYTE bEntryType;

    /// 인터럽트 타입
    BYTE bInterruptType;

    /// 로컬 인터럽트 플래그
    WORD wInterruptFlags;

    /// 발생한 버스 ID
    BYTE bSourceBUSID;

    /// 발생한 버스 IRQ
    BYTE bSourceBUSIRQ;

    /// 전달할 I/O APIC ID
    BYTE bDestinationIOAPICID;

    /// 전달할 I/O APIC INTIN
    BYTE bDestinationIOAPICLINTIN;
} IO_INTERRUPT_ASSIGNMENT_ENTRY;

/// 로컬 인터럽트 지정 엔트리 자료구조 (Local Interrupt Assignment Entry)
typedef struct kLocalInterruptEntryStruct {
    /// 엔트리 타입 코드, 4
    BYTE bEntryType;

    /// 인터럽트 타입
    BYTE bInterruptType;

    /// 로컬 인터럽트 플래그
    WORD wInterruptFlags;

    /// 발생한 버스 ID
    BYTE bSourceBUSID;

    /// 발생한 버스 IRQ
    BYTE bSourceBUSIRQ;

    /// 전달할 로컬 APIC ID
    BYTE bDestinationLocalAPICID;

    /// 전달할 로컬 APIC INTIN
    BYTE bDestinationLocalAPICLINTIN;

} LOCAL_INTERRUPT_ASSIGNMENT_ENTRY;

#pragma pack(pop)

/// MP 설정 테이블을 관리하는 자료구조
typedef struct kMPConfigurationManagerStruct {

    /// MP 플로팅 테이블
    MP_FLOATING_POINTER *pstMPFloatingPointer;

    /// MP 설정 테이블 헤더
    MP_CONFIGURATION_TABLE_HEADER *pstMPConfigurationTableHeader;

    /// 기본 MP 설정 테이블 엔트리의 시작 어드레스
    QWORD qwBaseEntryStartAddress;

    /// 프로세서 또는 코어 수
    int iProcessorCount;

    /// PIC 모드 지원 여부
    BOOL bUsePICMode;

    /// ISA 버스의 ID
    BYTE bISABusID;

} MP_CONFIGURATION_MANAGER;


// 함수
BOOL kFindMPFloatingPointerAddress(QWORD *pstAddress);

BOOL kAnalysisMPConfigurationTable(void);

MP_CONFIGURATION_MANAGER *kGetMPConfigurationManager(void);

void kPrintMPConfigurationTable(void);

int kGetProcessorCount(void);

IO_APIC_ENTRY *kFindIOAPICEntryForISA(void);

#endif //CRAFTING_MPCONFIGURATIONTABLE_H
