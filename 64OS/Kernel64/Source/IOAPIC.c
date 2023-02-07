//
// Created by jewoo on 2023-02-01.
//

#include "IOAPIC.h"
#include "MPConfigurationTable.h"
#include "PIC.h"
#include "Utility.h"
#include "Console.h"

/// I/O APIC를 관리하는 자료구조
static IOAPIC_MANAGER gs_stIOAPICManager;

/// ISA 버스가 연결된 I/O APIC의 기준 어드레스를 반환
QWORD kGetIOAPICBaseAddressOfISA(void) {
    MP_CONFIGURATION_MANAGER *pstMPManager;
    IO_APIC_ENTRY *pstIOAPICEntry;

    /// I/O APIC의 어드레스가 저장되어 있지 않으면 엔트리를 찾아서 저장
    if (gs_stIOAPICManager.qwIOAPICBaseAddressOfISA == NIL) {
        pstIOAPICEntry = kFindIOAPICEntryForISA();
        if (pstIOAPICEntry != NIL) {
            gs_stIOAPICManager.qwIOAPICBaseAddressOfISA = pstIOAPICEntry->dwMemoryMapAddress & 0xFFFFFFFF;
        }
    }
    /// I/O APIC의 기준 어드레스를 찾아서 저장한 다음 반환
    return gs_stIOAPICManager.qwIOAPICBaseAddressOfISA;
}

/// I/O 리다이렉션 테이블 자료구조에 값을 저장
void kSetIOAPICRedirectionEntry(IOREDIRECTION_TABLE *pstEntry, BYTE bAPICID,
                                BYTE bInterruptMask, BYTE bFlagsAndDeliveryMode,
                                BYTE bVector) {
    kMemSet(pstEntry, 0, sizeof(IOREDIRECTION_TABLE));

    pstEntry->bDestination = bAPICID;
    pstEntry->bFlagsAndDeliveryMode = bFlagsAndDeliveryMode;
    pstEntry->bInterruptMask = bInterruptMask;
    pstEntry->bVector = bVector;
}

/// 인터럽트 입력핀에 해당하는  I/O 리다이렉션 테이블에서 값을 읽음
void kReadIOAPICRedirectionTable(int iINTIN, IOREDIRECTION_TABLE *pstEntry) {
    QWORD *pqwData;
    QWORD qwIOAPICBaseAddress;

    /// ISA 버스가 연결된 I/O APIC의 메모리 맵  I/O 어드레스
    qwIOAPICBaseAddress = kGetIOAPICBaseAddressOfISA();

    /// IO 리다이렉션 테이블은 8바이트이므로 8바이트 정수로 변환해서 처리
    pqwData = (QWORD *) pstEntry;

    //=========================================================================================
    /// I/O 리다이렉션 테이블의 상위 4바이트를 읽음
    /// I/O 리다이렉션 테이블은 상위 레지스터와 하위 레지스터가 한 쌍이므로 INTIN에 2을 곱하여
    /// 해당 I/O 리다이렉션 테이블 레지스터의 인덱스를 계산
    //=========================================================================================
    /// I/O 레지스터 선택 레지스터(0xFEC00000)에 상위 IO 리다이렉션 테이블 레지스터의 인덱스를 전송
    *(DWORD *) (qwIOAPICBaseAddress + IOAPIC_REGISTER_IO_REGISTER_SELECTOR) =
            IOAPIC_REGISTER_INDEX_HIGH_IO_REDIRECTION_TABLE + iINTIN * 2;
    /// I/O 레지스터 선택 레지스터(0xFEC00010)에 상위 IO 리다이렉션 테이블 레지스터의 값을 읽음
    *pqwData = *(DWORD *) (qwIOAPICBaseAddress + IOAPIC_REGISTER_IO_WINDOW);
    *pqwData = *pqwData << 32;

    //=========================================================================================
    /// I/O 리다이렉션 테이블의 하위 4바이트를 읽음
    /// I/O 리다이렉션 테이블은 상위 레지스터와 하위 레지스터가 한 쌍이므로 INTIN에 2을 곱하여
    /// 해당 I/O 리다이렉션 테이블 레지스터의 인덱스를 계산
    //=========================================================================================
    /// I/O 레지스터 선택 레지스터(0xFEC00000)에 하위 IO 리다이렉션 테이블 레지스터의 인덱스를 전송
    *(DWORD *) (qwIOAPICBaseAddress + IOAPIC_REGISTER_IO_REGISTER_SELECTOR) =
            IOAPIC_REGISTER_INDEX_LOW_IO_REDIRECTION_TABLE + iINTIN * 2;

    /// I/O 윈도우 레지스터(0xFEC00010)에 하위 IO 리다이렉션 테이블 레지스터의 값을 읽음
    *pqwData |= *(DWORD *) (qwIOAPICBaseAddress + IOAPIC_REGISTER_IO_WINDOW);
}

/// 인터럽트 입력 핀에 해당하는 I/O 리다이렉션 테이블에 값을 씀
void kWriteIOAPICRedirectionTable(int iINTIN, IOREDIRECTION_TABLE *pstEntry) {
    QWORD *pqwData;
    QWORD qwIOAPICBaseAddress;

    /// ISA 버스가 연결된 I/O APIC의 메모리 맵 I/O 어드레스
    qwIOAPICBaseAddress = kGetIOAPICBaseAddressOfISA();

    /// IO 리다이렉션 테이블은 8바이트이므로 8바이트 정수로 변환해서 처리
    pqwData = (QWORD *) pstEntry;

    //=========================================================================================
    /// I/O 리다이렉션 테이블의 상위 4바이트를 씀
    /// I/O 리다이렉션 테이블은 상위 레지스터와 하위 레지스터가 한 쌍이므로 INTIN에 2을 곱하여
    /// 해당 I/O 리다이렉션 테이블 레지스터의 인덱스를 계산
    //=========================================================================================
    /// I/O 레지스터 선택 레지스터(0xFEC00000)에 상위 IO 리다이렉션 테이블 레지스터의 인덱스를 전송
    *(DWORD *) (qwIOAPICBaseAddress + IOAPIC_REGISTER_IO_REGISTER_SELECTOR) =
            IOAPIC_REGISTER_INDEX_HIGH_IO_REDIRECTION_TABLE + iINTIN * 2;
    /// I/O 레지스터 선택 레지스터(0xFEC00010)에 상위 IO 리다이렉션 테이블 레지스터의 값을 씀
    *(DWORD *) (qwIOAPICBaseAddress + IOAPIC_REGISTER_IO_WINDOW) = *pqwData >> 32;

    //=========================================================================================
    /// I/O 리다이렉션 테이블의 하위 4바이트를 씀
    /// I/O 리다이렉션 테이블은 상위 레지스터와 하위 레지스터가 한 쌍이므로 INTIN에 2을 곱하여
    /// 해당 I/O 리다이렉션 테이블 레지스터의 인덱스를 계산
    //=========================================================================================
    /// I/O 레지스터 선택 레지스터(0xFEC00000)에 하위 IO 리다이렉션 테이블 레지스터의 인덱스를 전송
    *(DWORD *) (qwIOAPICBaseAddress + IOAPIC_REGISTER_IO_REGISTER_SELECTOR) =
            IOAPIC_REGISTER_INDEX_LOW_IO_REDIRECTION_TABLE + iINTIN * 2;

    /// I/O 윈도우 레지스터(0xFEC00010)에 하위 IO 리다이렉션 테이블 레지스터의 값을 씀
    *(DWORD *) (qwIOAPICBaseAddress + IOAPIC_REGISTER_IO_WINDOW) = *pqwData;
}

/// I/O APIC에 연결된 모든 인터럽트 핀을 마스크하여 인터럽트가 전달되지 않도록 함
void kMaskAllInterruptInIOAPIC(void) {
    IOREDIRECTION_TABLE stEntry;
    int i;
    /// 모든 인터럽트를 비활성화
    for (i = 0; i < IOAPIC_MAXIO_REDIRECTION_TABLE_COUNT; i++) {
        /// I/O 리다이렉션 테이블을 읽어서 인터럽트 마스크 필드(비트 0)를 1로 설정하여 다시 저장
        kReadIOAPICRedirectionTable(i, &stEntry);
        stEntry.bInterruptMask = IOAPIC_INTERRUPT_MASK;
        kWriteIOAPICRedirectionTable(i, &stEntry);
    }
}

/// I/O APIC의 I/O 리다이렉션 테이블을 초기화
void kInitializeIORedirectionTable(void) {

    MP_CONFIGURATION_TABLE_HEADER *pstMPHeader;
    MP_CONFIGURATION_MANAGER *pstMPManager;
    IO_INTERRUPT_ASSIGNMENT_ENTRY *pstIOAssignmentEntry;
    IOREDIRECTION_TABLE stIORedirectionEntry;
    QWORD qwEntryAddress;
    BYTE bEntryType;
    BYTE bDestination;
    int i;


    //==========================================================================================
    /// I/O APIC를 관리하는 자료구조를 초기화
    //==========================================================================================
    kMemSet(&gs_stIOAPICManager, 0, sizeof(gs_stIOAPICManager));

    /// I/O APIC의 메모리 맵 I/O 어드레스 저장, 아래 함수에서 내부적으로 처리함
    kGetIOAPICBaseAddressOfISA();

    /// IRQ를 I/O APIC의 INTIN 핀과 연결한 테이블(IRQ->INTIN 매핑 테이블)을 초기화
    for (i = 0; i < IOAPIC_MAX_IRQ_TO_INTIN_MAP_COUNT; i++) {
        gs_stIOAPICManager.vbIRAToINTINMap[i] = 0xFF;
    }

    //==========================================================================================
    /// I/O APIC를 마스크하여 인터럽트가 발생하지 않도록 하고 I/O 리다이렉션 테이블 초기화
    //==========================================================================================
    /// 먼저 I/O APIC의 인터럽트를 마스크 하여 인터럽트가 발생하지 않도록 함
    kMaskAllInterruptInIOAPIC();

    /// I/O 인터럽트 지정 엔트리 중에 ISA 버스와 관련된 인터럽트만 추려서 I/O 리다이렉션 테이블에 설정
    /// MP 설정 테이블 헤더의 시작 어드레스와 엔트리의 시작 어드레스를 저장
    pstMPManager = kGetMPConfigurationManager();
    pstMPHeader = pstMPManager->pstMPConfigurationTableHeader;
    qwEntryAddress = pstMPManager->qwBaseEntryStartAddress;

    /// 모든 엔트리를 확인하여 ISA 버스와 관련된 I/O 인터럽트 지정 엔트리를 검색
    for (i = 0; i < pstMPHeader->wEntryCount; i++) {
        bEntryType = *(BYTE *) qwEntryAddress;
        switch (bEntryType) {
            /// I/O 인터럽트에 지정 엔트리라면 ISA 버스인지 확인하여 I/O 리다이렉션
            /// 테이블에 설정하고 IRQ->INTLIN 매핑 테이블을 구성
            case MP_ENTRY_TYPE_IO_INTERRUPT_ASSIGNMENT:
                pstIOAssignmentEntry = (IO_INTERRUPT_ASSIGNMENT_ENTRY *) qwEntryAddress;

                /// 인터럽트 타입이 인터럽트인 것만 처리
                if ((pstIOAssignmentEntry->bSourceBUSID == pstMPManager->bISABusID) &&
                    (pstIOAssignmentEntry->bInterruptType == MP_INTERRUPT_TYPE_INT)) {
                    /// 목적지 필드는 IRQ 0을 제외하고 0x00으로 설정하여  Bootstrap Processor만 전달
                    /// IRQ 0는 스케줄러에 사용해야 하므로 0xFF로 설정하여 모든 코어로 전달
                    if (pstIOAssignmentEntry->bSourceBUSIRQ == 0) {
                        bDestination = 0xFF;
                    } else {
                        bDestination = 0x00;
                    }

                    /// ISA 버스는 엦시 트리거와 1일 때 활성화(Activate High)를 사용
                    /// 목적지 모드는 물리 모드, 잔전달 모드는 고정으로 할당
                    /// 인터럽트 벡터는 PIC 컨트롤러의 벡터와 같이 0x20 + IRQ로 설정
                    kSetIOAPICRedirectionEntry(&stIORedirectionEntry, bDestination,
                                               0x00,
                                               IOAPIC_TRIGGERMODE_EDGE | IOAPIC_POLARITY_ACTIVE_HIGH |
                                               IOAPIC_DESTINATIONMODE_PHYSICAL_MODE | IOAPIC_DELIVERYMODE_FIXED,
                                               PIC_IRQ_START_VECTOR + pstIOAssignmentEntry->bSourceBUSIRQ);

                    /// ISA 버스에서 전달된 IRQ는 I/O APIC의 INTIN 핀에 있으므로, INTIN 값을 이용하여 처리
                    kWriteIOAPICRedirectionTable(pstIOAssignmentEntry->bDestinationIOAPICLINTIN, &stIORedirectionEntry);

                    /// IRQ와 인터럽트 입력 핀의 관계를 저장(IRQ->INTIN 매핑 테이블 구성)
                    gs_stIOAPICManager.vbIRAToINTINMap[pstIOAssignmentEntry->bSourceBUSIRQ] = pstIOAssignmentEntry->bDestinationIOAPICLINTIN;
                }
                qwEntryAddress += sizeof(IO_INTERRUPT_ASSIGNMENT_ENTRY);
                break;

                /// 프로세서 엔트리는 무시
            case MP_ENTRY_TYPE_PROCESSOR:
                qwEntryAddress += sizeof(PROCESSOR_ENTRY);
                break;

                /// 버스 엔트리, I/O APIC 엔트리, 로컬 인터럽트 지정 엔트리는 무시
            case MP_ENTRY_TYPE_BUS:
            case MP_ENTRY_TYPE_IO_APIC:
            case MP_ENTRY_TYPE_LOCAL_INTERRUPT_ASSIGNMENT:
            default:
                qwEntryAddress += 8;
                break;
        }
    }
}


/// IRQ와 I/O APIC의 인터럽트 핀 간의 매핑 관계를 추출
void kPrintIRQToINTINMap(void) {
    int i;
    kPrintf("=========== IRQ To I/O APIC INT IN Mapping Table =============");
    for (i = 0; i < IOAPIC_MAX_IRQ_TO_INTIN_MAP_COUNT; i++) {
        kPrintf("IRQ[%d] -> INTIN [%d]\n",
                i, gs_stIOAPICManager.vbIRAToINTINMap[i]);
    }
}

/// IRQ를 로컬 APIC ID로 전달하도록 변경
void kRoutingIRQToAPICID(int iIRQ, unsigned char bAPICID) {
    int i;
    IOREDIRECTION_TABLE stEntry;

    /// 범위 검사
    if (iIRQ > IOAPIC_MAX_IRQ_TO_INTIN_MAP_COUNT) {
        return;
    }

    /// 설정된 I/O 리다이렉션 테이블을 읽어서 목적지 필드만 수정
    kReadIOAPICRedirectionTable(gs_stIOAPICManager.vbIRAToINTINMap[iIRQ], &stEntry);
    stEntry.bDestination = bAPICID;
    kWriteIOAPICRedirectionTable(gs_stIOAPICManager.vbIRAToINTINMap[iIRQ],
                                 &stEntry);

}
