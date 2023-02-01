//
// Created by jewoo on 2023-01-01.
//

#ifndef CRAFTING_DESCRIPTOR_H
#define CRAFTING_DESCRIPTOR_H

#include "Types.h"

// 매크로
//==============================================================================
// GDT
//==============================================================================
// 조합에 사용할 기본 매크로
#define GDT_TYPE_CODE                       0x0A
#define GDT_TYPE_DATA                       0x02
#define GDT_TYPE_TSS                        0X09
#define GDT_FLAGS_LOWER_S                   0x10
#define GDT_FLAGS_LOWER_DPL0                0x00
#define GDT_FLAGS_LOWER_DPL1                0x20
#define GDT_FLAGS_LOWER_DPL2                0x40
#define GDT_FLAGS_LOWER_DPL3                0x60
#define GDT_FLAGS_LOWER_P                   0x80
#define GDT_FLAGS_UPPER_L                   0x20
#define GDT_FLAGS_UPPER_DB                  0x40
#define GDT_FLAGS_UPPER_G                   0x80

// 실제로 사용할 매크로
// LOWER FLAGS는 CODE/DATA/TSS, DPL0, Present로 설정
#define GDT_FLAGS_LOWER_KERNELCODE          (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_KERNELDATA          (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_TSS                 (GDT_FLAGS_LOWER_DPL0 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_USERCODE            (GDT_TYPE_CODE | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P)
#define GDT_FLAGS_LOWER_USERDATA            (GDT_TYPE_DATA | GDT_FLAGS_LOWER_S | GDT_FLAGS_LOWER_DPL3 | GDT_FLAGS_LOWER_P)

// UPPER FLAGS 는 Granualty로 설정하고 코드 및 데이터는 64비트 추가
#define GDT_FLAGS_UPPER_CODE                (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
#define GDT_FLAGS_UPPER_DATA                (GDT_FLAGS_UPPER_G | GDT_FLAGS_UPPER_L)
#define GDT_FLAGS_UPPER_TSS                 (GDT_FLAGS_UPPER_G)

// 세그먼트 디스크립터 오프셋
#define GDT_KERNEL_CODE_SEGMENT                 0x08
#define GDT_KERNEL_DATA_SEGMENT                 0x10
#define GDT_TSS_SEGMENT                         0x18

// 기타 GDT에 관련된 매크로
// GDTR의 시작 어드레스, 1MB에서 256KB까지는 페이지 테이블 영역
#define GDTR_STAR_ADDRESS                       0x142000
// 8바이트 엔트리의 개수, 널 디스크립터/커널 코드/커널 데이터
#define GDT_MAX_ENTRY_8_COUNT                   3
// 16바이트 엔트리의 개수, 즉 TSS는 프로세서 또는 코어의 최대 개수만큼 생성
#define GDT_MAX_ENTRY_16_COUNT                  (MAX_PROCESSOR_COUNT)
// GDT 테이블의 크기
#define GDT_TABLE_SIZE                       ((sizeof(GDTENTRY8) * GDT_MAX_ENTRY_8_COUNT) + (sizeof(GDTENTRY16) * GDT_MAX_ENTRY_16_COUNT))
/// TSS 세그먼트의 전체 크기
#define TSS_SEGMENT_SIZE                     (sizeof(TSSSEGMENT) *  MAX_PROCESSOR_COUNT)


//======================================================================================================================
// IDT
//======================================================================================================================
// 조합에 사용할 기본 매크로
#define IDT_TYPE_INTERRUPT 0x0E
#define IDT_TYPE_TRAP 0x0F
#define IDT_FLAGS_DPL0 0x00
#define IDT_FLAGS_DPL1 0x20
#define IDT_FLAGS_DPL2 0x40
#define IDT_FLAGS_DPL3 0x60
#define IDT_FLAGS_P 0x80
#define IDT_FLAGS_IST0 0
#define IDT_FLAGS_IST1 1

// 실제로 사용할 매크로
#define IDT_FLAGS_KERNEL    (IDT_FLAGS_DPL0 | IDT_FLAGS_P)
#define IDT_FLAGS_USER      (IDT_FLAGS_DPL3 | IDT_FLAGS_P)

// 기타 IDT에 관련된 매트로
// IDT 엔트리의 개수
#define IDT_MAX_ENTRY_COUNT 100
// IDTR의 시작 어드레스, TSS 세그먼트의 뒤쪽에 위치
#define IDTR_START_ADDRESS       (GDTR_STAR_ADDRESS + sizeof (GDTR) + GDT_TABLE_SIZE + TSS_SEGMENT_SIZE)
// IDT 테이블의 시작 어드레스
#define IDT_START_ADDRESS        (IDTR_START_ADDRESS + sizeof(IDTR))
// IDT 테이블의 전체 크기
#define IDT_TABLE_SIZE           (IDT_MAX_ENTRY_COUNT  * sizeof(IDTENTRY))

// IST의 시작 어드레스
#define IST_START_ADDRESS 0x700000
// IST의 크기
#define IST_SIZE 0x100000

// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

// GDTR 과 IDTR 구조체
typedef struct kGDTRStruct {
    WORD wLimit;
    QWORD qwBaseAddress;
    // 16 바이트 어드레스 정렬을 위해 추가
    WORD wPading;
    DWORD dwPading;
} GDTR, IDTR;


// 8바이트 크기의 GDT 엔트리 구조
typedef struct kGDTEntry8Struct {
    WORD wLowerLimit;
    WORD wLowerBaseAddress;
    BYTE bUpperBaseAddress1;

    // 4비트 Type, 1비트 S, 2비트 DPL, 1비트 P
    BYTE bTypeAndLowerFlag;

    // 4비트 Segment Limit, 1비트 AVL, L, D/B, G
    BYTE bUpperLimitAndUpperFlag;
    BYTE bUpperBaseAddress2;
} GDTENTRY8;


// 8바이트 크기의 GDT 엔트리 구조
typedef struct kGDTEntry16Struct {
    WORD wLowerLimit;
    WORD wLowerBaseAddress;
    BYTE bMiddleBaseAddress1;
    // 4비트 Type, 1비트 0, 2비트 DPL, 1비트 P
    BYTE bTypeAndLowerFlag;

    // 4비트 Segment Limit, 1비트 AVL, 0, 0, G
    BYTE bUpperLimitAndUpperFlag;
    BYTE bMiddleBaseAddress2;
    DWORD dwUpperBaseAddress;
    DWORD dwReserved;
} GDTENTRY16;

// TSS Data 구조체
typedef struct kTSSDataStruct {
    DWORD dwReserved1;
    QWORD qwRsp[3];
    QWORD qwReserved2;
    QWORD qwIST[7];
    QWORD qwReserved3;
    WORD dwReserved;
    WORD wIOMapBaseAddress;
} TSSSEGMENT;

// IDT 게이트 디스크립터 구조체
typedef struct kIDTEntryStruct {
    WORD wLowerBaseAddress;
    WORD wSegmentSelector;
    // 3 bit IST, 5 bit 0
    BYTE bIST;
    // 4 bit Type, 1 bit 0, 2 bit DPL, 1 bit P
    BYTE bTypeAndFlags;
    WORD wMiddleBaseAddress;
    DWORD dwUpperBaseAddress;
    DWORD dwReserved;
} IDTENTRY;

#pragma pack(pop)

// 함수
void kInitializeGDTTableAndTSS(void);

void kSetGDTEntry8(GDTENTRY8 *pstEntry, DWORD dwBaseAddress, DWORD dwLimit,
                   BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType);

void kSetGDTEntry16(GDTENTRY16 *pstEntry, QWORD qwBaseAddress, DWORD dwLimit,
                    BYTE bUpperFlags, BYTE bLowerFlags, BYTE bType);

void kInitializeTSSSegment(TSSSEGMENT *pstTSS);

void kInitializeIDTTables(void);

void kSetIDTEntry(IDTENTRY *pstEntry, void *pvHandler, WORD wSelector,
                  BYTE bIST, BYTE bFlags, BYTE bType);

void kDummyHandler(void);


#endif //CRAFTING_DESCRIPTOR_H
