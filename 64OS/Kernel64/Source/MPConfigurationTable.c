//
// Created by jewoo on 2023-01-31.
//

#include "MPConfigurationTable.h"
#include "Console.h"
#include "Utility.h"

/// MP 설정 테이블을 관리하는 자료구조
static MP_CONFIGURATION_MANAGER gs_stMPConfigurationManager = {0,};

/// BIOS의 영역에서 MP Floating Header를 찾아서 그 주소를 반환
BOOL kFindMPFloatingPointerAddress(QWORD *pstAddress) {
    char *pcMPFloatingPointer;
    QWORD qwEBFAAddress;
    QWORD qwSystemBaseMemory;

    /// 확장 BIOS 데이터 영역과 시스템 기본 메모리를 출력
    kPrintf("Extended BIOS Data Area = [0x%X] \n",
            (DWORD) (*(WORD *) 0x040E) * 16);
    kPrintf("Sysyem Base Address = [0x%X] \n",
            (DWORD) (*(WORD *) 0x0413) * 124);

    /// 확장 BIOS 데이터 영역을 검색하여 MP 플로팅 포인터를 찾음
    /// 확장 BIOS 데이터 영역은 0x040E에서 세그먼트의 시작 어드레스를 찾을 수 있음
    qwEBFAAddress = *(WORD *) (0x040E);

    /// 세그먼트의 시작 어드레스 이므로 16을 곱하여 실제 물리 어드레스로 변환
    qwEBFAAddress *= 16;
    for (pcMPFloatingPointer = (char *) qwEBFAAddress;
         (QWORD) pcMPFloatingPointer <= (qwEBFAAddress + 1024); pcMPFloatingPointer++) {
        if (kMemCmp(pcMPFloatingPointer, "_MP_", 4) == 0) {
            kPrintf("MP Floating Pointer is In EBDA, [0x%X] Address\n",
                    (QWORD) pcMPFloatingPointer);
            *pstAddress = (QWORD) pcMPFloatingPointer;
            return TRUE;
        }
    }

    /// 시스템 기본 메모리의 끝부분에서 1KB미만인 영역을 검색하여 MP 플로팅 포인터를 찾음
    /// 시스템 기본 메모리는 0x0413에서 KB 단위로 정렬된 값을 찾을 수 있음
    qwSystemBaseMemory = *(WORD *) (0x0413);
    /// KB 단위로 저장된 값이므로 1024를 곱해 실제 물리 어드레스로 변환
    qwSystemBaseMemory *= 1024;


    for (pcMPFloatingPointer = (char *) (qwSystemBaseMemory - 1024);
         (QWORD) pcMPFloatingPointer <= qwSystemBaseMemory;
         pcMPFloatingPointer++) {
        if (kMemCmp(pcMPFloatingPointer, "_MP_", 4) == 0) {
            kPrintf("MP Floating Pointer Is In System Base Memory, [0x%X] Address\n",
                    (QWORD) pcMPFloatingPointer);
            *pstAddress = (QWORD) pcMPFloatingPointer;
            return TRUE;
        }
    }

    /// BIOS의 ROM 영역을 검색하여 MP 플로팅 포인터를 찾음
    for (pcMPFloatingPointer = (char *) 0x0F0000;
         (QWORD) pcMPFloatingPointer <= 0x0FFFFF;
         pcMPFloatingPointer++) {
        if (kMemCmp(pcMPFloatingPointer, "_MP_", 4) == 0) {
            kPrintf("MP Floating Pointer Is In ROM, [0x%X] Address\n",
                    (QWORD) pcMPFloatingPointer);
            *pstAddress = (QWORD) pcMPFloatingPointer;
            return TRUE;
        }
    }

    return FALSE;
}

/// MP 설정 테이블을 분석해서 필요한 정보를 저장
BOOL kAnalysisMPConfigurationTable(void) {
    QWORD qwMPFloatingPointerAddress;
    MP_FLOATING_POINTER *pstMPFloatingPointer;
    MP_CONFIGURATION_TABLE_HEADER *pstMPConfigurationHeader;
    BYTE bEntryType;
    WORD i;
    QWORD qwEntryAddress;
    PROCESSOR_ENTRY *pstProcessEntry;
    BUS_ENTRY *pstBusEntry;

    /// 자료구조 초기화
    kMemSet(&gs_stMPConfigurationManager, 0, sizeof(MP_CONFIGURATION_MANAGER));
    gs_stMPConfigurationManager.bISABusID = 0xFF;

    /// MP 플로팅 포인터의 어드레스를 구함
    if (kFindMPFloatingPointerAddress(&qwMPFloatingPointerAddress) == FALSE) {
        return FALSE;
    }

    /// MP 플로팅 테이블 설정
    pstMPFloatingPointer = (MP_FLOATING_POINTER *) qwMPFloatingPointerAddress;
    gs_stMPConfigurationManager.pstMPFloatingPointer = pstMPFloatingPointer;
    pstMPConfigurationHeader = (MP_CONFIGURATION_TABLE_HEADER *) (
            (QWORD) pstMPFloatingPointer->dwMPConfigurationTableAddress & 0xFFFFFFFF);

    /// PIC 모드 지원 여부 저장
    if (pstMPFloatingPointer->vbMPFeatureByte[1] & MP_FLOATING_POINTER_FEATURE_BYTE2_PIC_MODE) {
        gs_stMPConfigurationManager.bUsePICMode = TRUE;
    }

    /// MP 설정 테이블 헤더와 기본 MP 설정 테이블 엔트리의 시작 어드레스 설정
    gs_stMPConfigurationManager.pstMPConfigurationTableHeader = pstMPConfigurationHeader;
    gs_stMPConfigurationManager.qwBaseEntryStartAddress =
            pstMPFloatingPointer->dwMPConfigurationTableAddress + sizeof(MP_CONFIGURATION_TABLE_HEADER);

    /// 모든 엔트리를 돌면서 프로세서의 코어수를 계산 하고 ISA 버스를 검색하여 ID를 저장
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;
    for (i = 0; i < pstMPConfigurationHeader->wEntryCount; i++) {
        bEntryType = *(BYTE *) qwEntryAddress;
        switch (bEntryType) {

            /// 프로세서 엔트리면 프로세서의 수를 하나 증가시킴
            case MP_ENTRY_TYPE_PROCESSOR:
                pstProcessEntry = (PROCESSOR_ENTRY *) qwEntryAddress;
                if (pstProcessEntry->bCPUFlags & MP_PROCESSOR_CPU_FLAGS_ENABLE) {
                    gs_stMPConfigurationManager.iProcessorCount++;
                }
                qwEntryAddress += sizeof(PROCESSOR_ENTRY);
                break;

                /// 버스 엔트리이면 ISA 버스인지 확인하여 저장
            case MP_ENTRY_TYPE_BUS:
                pstBusEntry = (BUS_ENTRY *) qwEntryAddress;
                if (kMemCmp(pstBusEntry->vcBusTypeString, MP_BUS_TYPE_STRING_ISA,
                            kStrLen(MP_BUS_TYPE_STRING_ISA)) == 0) {
                    gs_stMPConfigurationManager.bISABusID = pstBusEntry->bBusID;
                }
                qwEntryAddress += sizeof(BUS_ENTRY);
                break;

                /// 기타 엔트리는 무시하고 이동
            case MP_ENTRY_TYPE_IO_APIC:
            case MP_ENTRY_TYPE_IO_INTERRUPT_ASSIGNMENT:
            case MP_ENTRY_TYPE_LOCAL_INTERRUPT_ASSIGNMENT:
            default:
                qwEntryAddress += 8;
                break;
        }
    }
    return TRUE;
}

/// MP 설정 테이블을 관리하는 자료구조를 반환
MP_CONFIGURATION_MANAGER *kGetMPConfigurationManager(void) {
    return &gs_stMPConfigurationManager;
}

/// MP 설정 테이블의 정보를 모두 화면에 출력
void kPrintMPConfigurationTable(void) {

}

/// 프로세서 또는 코어의 개수를 반환
int iGetProcessorCount(void) {
    /// MP 설정 테이블이 없을 수도 있으므로 0으로 설정되면 1을 반환
    if (gs_stMPConfigurationManager.iProcessorCount == 0) {
        return 1;
    }
    return gs_stMPConfigurationManager.iProcessorCount;
}
