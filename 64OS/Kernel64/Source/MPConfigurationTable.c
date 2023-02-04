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
    MP_CONFIGURATION_MANAGER *pstMPConfiguratonManager;
    QWORD qwMPFloatingPointerAddress;
    MP_FLOATING_POINTER *pstMPFloatingPointer;
    MP_CONFIGURATION_TABLE_HEADER *pstMPTableHeader;
    PROCESSOR_ENTRY *pstProcessorEntry;
    BUS_ENTRY *pstBusEntry;
    IO_APIC_ENTRY *pstIOAPICEntry;
    IO_INTERRUPT_ASSIGNMENT_ENTRY *pstIOAssignmentEntry;
    LOCAL_INTERRUPT_ASSIGNMENT_ENTRY *pstLocalAssignmentEntry;
    QWORD qwBaseEntryAddress;
    char vcStringBuffer[20];
    WORD i;
    BYTE bEntryType;

    /// 화면에 출력할 문자열
    char *vpcInterruptType[4] = {"INT", "NMI", "SMI", "ExtINT"};
    char *vpcInterruptFlagsPO[4] = {"Conform", "Active High", "Reserved", "Active Low"};
    char *vpcInterruptFlagsEL[4] = {"Conform", "Edge-Trigger", "Reserved", "Level-Trigger"};

    // ===============================================================================================
    /// MP 설정 테이블 처리 함수를 먼저 호출하여 시스템 처리에 필요한 정보를 저장
    // ===============================================================================================
    kPrintf("================ MP Configuration Table Summary ======================\n");
    pstMPConfiguratonManager = kGetMPConfigurationManager();
    if ((pstMPConfiguratonManager->qwBaseEntryStartAddress == 0) &&
        (kAnalysisMPConfigurationTable() == FALSE)) {
        kPrintf("MP Configuration Table Analysis Fail\n");
        return;
    }
    kPrintf("MP Configuration Table Analysis Success\n");

    kPrintf("MP Floating Pointer Address : 0x%Q\n",
            pstMPConfiguratonManager->pstMPFloatingPointer);
    kPrintf("PIC Mode Support : %d\n", pstMPConfiguratonManager->bUsePICMode);
    kPrintf("MP Configuration Table Header Address : 0x%Q\n",
            pstMPConfiguratonManager->pstMPConfigurationTableHeader);
    kPrintf("Base MP Configuration Table Entry Start Address : 0x%Q\n",
            pstMPConfiguratonManager->qwBaseEntryStartAddress);
    kPrintf("Processor Count : %d\n", pstMPConfiguratonManager->iProcessorCount);
    kPrintf("ISA Bus ID : %d\n", pstMPConfiguratonManager->bISABusID);

    kPrintf("Press Any Key To Continue... ('q' is exit) : ");
    if (kGetCh() == 'q') {
        kPrintf("\n");
        return;
    }
    kPrintf("\n");


    // ===============================================================================================
    /// MP 플로팅 포인터 정보를 출력
    // ===============================================================================================
    kPrintf("================ MP Floating Pointer ======================\n");
    pstMPFloatingPointer = pstMPConfiguratonManager->pstMPFloatingPointer;
    kMemCpy(vcStringBuffer, pstMPFloatingPointer->vcSignature, 4);
    vcStringBuffer[4] = '\0';
    kPrintf("Signature : %s\n", vcStringBuffer);
    kPrintf("MP Configuration Table Address : 0x%Q\n",
            pstMPFloatingPointer->dwMPConfigurationTableAddress);
    kPrintf("Length : %d * 16 Byte\n", pstMPFloatingPointer->bLength);
    kPrintf("Version : %d\n", pstMPFloatingPointer->bRevision);
    kPrintf("CheckSum : 0x%X\n", pstMPFloatingPointer->bCheckSum);
    kPrintf("Feature Byte 1 : 0x%X ", pstMPFloatingPointer->vbMPFeatureByte[0]);

    /// MP 설정 테이블 사용 여부 출력
    if (pstMPFloatingPointer->vbMPFeatureByte[0] == 0) {
        kPrintf("(Use MP Configuration Table)\n");
    } else {
        kPrintf("(Use Default Configuration)\n");
    }

    /// PIC 모드 지원 여부 출력
    kPrintf("Feature Byte 2 : 0x%X ", pstMPFloatingPointer->vbMPFeatureByte[1]);
    if (pstMPFloatingPointer->vbMPFeatureByte[2] & MP_FLOATING_POINTER_FEATURE_BYTE2_PIC_MODE) {
        kPrintf("(PIC Mode Support)\n");
    } else {
        kPrintf("(Virtual Wire Mode Support)\n");
    }

    // ===============================================================================================
    /// MP 설정 테이블 헤더 정보를 출력
    // ===============================================================================================
    kPrintf("\n================ MP Configuration Table Header ======================\n");
    pstMPTableHeader = pstMPConfiguratonManager->pstMPConfigurationTableHeader;
    kMemCpy(vcStringBuffer, pstMPTableHeader->vcSignature, 4);
    vcStringBuffer[4] = '\0';
    kPrintf("Signature : %s\n", vcStringBuffer);
    kPrintf("Length : %d * 16 Byte\n", pstMPTableHeader->wBaseTableLength);
    kPrintf("Version : %d\n", pstMPTableHeader->bRevision);
    kPrintf("CheckSum : 0x%X\n", pstMPTableHeader->bCheckSum);
    kMemCpy(vcStringBuffer, pstMPTableHeader->vcOEMIDString, 8);
    vcStringBuffer[8] = '\0';
    kPrintf("OEM ID String : %s\n", vcStringBuffer);
    kMemCpy(vcStringBuffer, pstMPTableHeader->vcOEMIDString, 12);
    vcStringBuffer[12] = '\0';
    kPrintf("Product ID String : %s\n", vcStringBuffer);
    kPrintf("OEM Table Pointer : 0x%X\n",
            pstMPTableHeader->dwOEMTablePointerAddress);
    kPrintf("OEM Table Size: %d Byte\n",
            pstMPTableHeader->wEntryCount);
    kPrintf("Entry Count: %d\n", pstMPTableHeader->wEntryCount);
    kPrintf("Memory Mapped I/O Address Of Local APIC : 0x%X\n",
            pstMPTableHeader->dwMemoryMapIOAddressOfLocalAPIC);
    kPrintf("Extended Table Length : %d Byte\n",
            pstMPTableHeader->wExtendedTableLength);
    kPrintf("Extended Table CheckSum : 0x%X\n",
            pstMPTableHeader->bExtendedTableChecksum);

    kPrintf("Press Any Key To Continue... ('q' is exit) : ");
    if (kGetCh() == 'q') {
        kPrintf("\n");
        return;
    }
    kPrintf("\n");


    // ===============================================================================================
    /// 기본 MP 설정 테이블 엔트리 정보를 출력
    // ===============================================================================================
    kPrintf("\n================ Base MP Configuration Table Entry ======================\n");
    qwBaseEntryAddress = pstMPFloatingPointer->dwMPConfigurationTableAddress + sizeof(MP_CONFIGURATION_TABLE_HEADER);
    for (i = 0; i < pstMPTableHeader->wEntryCount; i++) {
        bEntryType = *(BYTE *) qwBaseEntryAddress;
        switch (bEntryType) {
            /// 프로세스 엔트리 정보 출력
            case MP_ENTRY_TYPE_PROCESSOR:
                kPrintf("Local APIC Version : 0x%X\n", pstProcessorEntry->bLocalAPICVersion);
                kPrintf("CPU Flags : 0x%X ", pstProcessorEntry->bCPUFlags);
                /// Enable/Disable 출력
                if (pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPU_FLAGS_ENABLE) {
                    kPrintf("(Enable, ");
                } else {
                    kPrintf("(Disable, ");
                }
                /// BSP/AP 출력
                if (pstProcessorEntry->bCPUFlags & MP_PROCESSOR_CPU_FLAGS_BSP) {
                    kPrintf("BSP)\n");
                } else {
                    kPrintf("AP)\n");
                }
                kPrintf("CPU Siganture : 0x%X\n", pstProcessorEntry->vbCPUSignature);
                kPrintf("Feature Flags : 0x%X\n",
                        pstProcessorEntry->dwFeatureFlags);

                /// 프로세스 엔트리의 크기만큼 어드레스를 증가시켜서 다음 엔트리로 이동
                qwBaseEntryAddress += sizeof(PROCESSOR_ENTRY);
                break;

                /// 버스 엔트리 정보 출력
            case MP_ENTRY_TYPE_BUS:
                pstBusEntry = (BUS_ENTRY *) qwBaseEntryAddress;
                kPrintf("Entry Type : Bus\n");
                kPrintf("BUS ID : %d\n", pstBusEntry->bBusID);
                kMemCpy(vcStringBuffer, pstBusEntry->vcBusTypeString, 6);
                vcStringBuffer[6] = '\0';
                kPrintf("Bus Type String : %s\n\n", vcStringBuffer);

                /// 버스 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
                qwBaseEntryAddress += sizeof(BUS_ENTRY);
                break;

                /// I/O APIC 엔트리
            case MP_ENTRY_TYPE_IO_APIC:
                pstIOAPICEntry = (IO_APIC_ENTRY *) qwBaseEntryAddress;
                kPrintf("Entry Type : I/O APIC\n");
                kPrintf("I/O APIC ID : %d\n", pstIOAPICEntry->bIOAPICID);
                kPrintf("I/O APIC Version :0x%X\n", pstIOAPICEntry->bIOAPICVersion);
                kPrintf("I/O APIC Flags : 0x%X\n", pstIOAPICEntry->bIOAPICFlags);

                /// Enable/Disable 출력
                if (pstIOAPICEntry->bIOAPICFlags == 1) {
                    kPrintf("(Enable)\n");
                } else {
                    kPrintf("(Disable)\n");
                }
                kPrintf("Memory Mapped I/O Address : 0x%X\n\n",
                        pstIOAPICEntry->dwMemoryMapAddress);
                /// I/O APIC 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
                qwBaseEntryAddress += sizeof(IO_APIC_ENTRY);
                break;

                /// I/O 인터럽트 지정 엔트리
            case MP_ENTRY_TYPE_IO_INTERRUPT_ASSIGNMENT:
                pstIOAssignmentEntry = (IO_INTERRUPT_ASSIGNMENT_ENTRY *) qwBaseEntryAddress;
                kPrintf("Entry Type : I/O Interrupt Assignment\n");
                kPrintf("Interrupt Type : 0x%X ", pstIOAssignmentEntry->bInterruptType);

                /// 인터럽트 타입 출력
                kPrintf("(%s)\n", vpcInterruptType[pstIOAssignmentEntry->bInterruptType]);
                kPrintf("I/O Interrupt Flags : 0x%X ", pstIOAssignmentEntry->wInterruptFlags);

                /// 극성과 트리거 모드 출력
                kPrintf("(%s, %s)\n", vpcInterruptFlagsPO[pstIOAssignmentEntry->wInterruptFlags & 0x03],
                        vpcInterruptFlagsEL[(pstIOAssignmentEntry->wInterruptFlags >> 2) & 0x03]);

                kPrintf("Source BUS ID : %d\n", pstIOAssignmentEntry->bSourceBUSID);
                kPrintf("Source BUS IRQ : %d\n", pstIOAssignmentEntry->bSourceBUSIRQ);

                kPrintf("Destination I/O APIC ID %d\n", pstIOAssignmentEntry->bDestinationIOAPICID);

                kPrintf("Destination I/O APIC INTIN %d\n\n", pstIOAssignmentEntry->bDestinationIOAPICLINTIN);

                /// I/O 인터럽트 지정 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
                qwBaseEntryAddress += sizeof(IO_INTERRUPT_ASSIGNMENT_ENTRY);
                break;


                /// 로컬 인터럽트 지정 엔트리
            case MP_ENTRY_TYPE_LOCAL_INTERRUPT_ASSIGNMENT:
                pstLocalAssignmentEntry = (LOCAL_INTERRUPT_ASSIGNMENT_ENTRY *) qwBaseEntryAddress;

                kPrintf("Entry Type : Local Interrupt Assignment\n");
                kPrintf("Interrupt Type : 0x%X ", pstLocalAssignmentEntry->bInterruptType);

                /// 인터럽트 타입 출력
                kPrintf("(%s)\n", vpcInterruptType[pstLocalAssignmentEntry->bInterruptType]);
                kPrintf("I/O Interrupt Flags : 0x%X ", pstLocalAssignmentEntry->wInterruptFlags);

                /// 극성과 트리거 모드 출력
                kPrintf("(%s, %s)\n", vpcInterruptFlagsPO[pstLocalAssignmentEntry->wInterruptFlags & 0x03],
                        vpcInterruptFlagsEL[(pstLocalAssignmentEntry->wInterruptFlags >> 2) & 0x03]);

                kPrintf("Source BUS ID : %d\n", pstLocalAssignmentEntry->bSourceBUSID);
                kPrintf("Source BUS IRQ : %d\n", pstLocalAssignmentEntry->bSourceBUSIRQ);

                kPrintf("Destination Local APIC ID %d\n",
                        pstLocalAssignmentEntry->bDestinationLocalAPICID);

                kPrintf("Destination Local APIC INTIN %d\n\n",
                        pstLocalAssignmentEntry->bDestinationLocalAPICLINTIN);

                /// 로컬 인터럽트 지정 엔트리의 크기만큼 어드레스를 증가시켜 다음 엔트리로 이동
                qwBaseEntryAddress += sizeof(LOCAL_INTERRUPT_ASSIGNMENT_ENTRY);
                break;

            default:
                kPrintf("Unknown Entry Type. %d\n", bEntryType);
                break;
        }

        /// 3개를 출력하고 나면 키 입력을 대기
        if ((i != 0) && (((i + 1) % 3) == 0)) {
            kPrintf("Press any key to continue... ('q' is exit) : ");
            if (kGetCh() == 'q') {
                kPrintf("\n");
                return;
            }
            kPrintf("\n");
        }
    }
}

/// 프로세서 또는 코어의 개수를 반환
int iGetProcessorCount(void) {
    /// MP 설정 테이블이 없을 수도 있으므로 0으로 설정되면 1을 반환
    if (gs_stMPConfigurationManager.iProcessorCount == 0) {
        return 1;
    }
    return gs_stMPConfigurationManager.iProcessorCount;
}

/// ISA 버스가 연결된 I/O APIC 엔트리를 검색
/// kAnalysisMPConfigurationTable() 함수를 먼저 호출한 뒤에 사용해야 함
IO_APIC_ENTRY *kFindIOAPICEntryForISA(void) {
    MP_CONFIGURATION_MANAGER *pstMPManager;
    MP_CONFIGURATION_TABLE_HEADER *pstMPHeader;
    IO_INTERRUPT_ASSIGNMENT_ENTRY *pstIOAssignmentEntry;
    IO_APIC_ENTRY *pstIOAPICEntry;
    BYTE bEntryType;
    QWORD qwEntryAddress;
    BOOL bFind = FALSE;
    int i;


    /// MP 설전 테이블 헤더의 시작 어드레스와 엔트리의 시작 어드레스를 저장
    pstMPHeader = gs_stMPConfigurationManager.pstMPConfigurationTableHeader;
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;

    //==============================================================================================
    /// ISA 버스와 관련된 I/O 인터럽트 지정 엔트리를 검색
    //==============================================================================================
    /// 모든 엔트리를 돌면서 ISA 버스와 관련된 I/O 인터럽트 지정 엔트리만 검색
    for (i = 0; (i < pstMPHeader->wEntryCount) && (bFind == FALSE); i++) {
        bEntryType = *(BYTE *) qwEntryAddress;
        switch (bEntryType) {
            /// 프로세스 엔트리는 무시
            case MP_ENTRY_TYPE_PROCESSOR:
                qwEntryAddress += sizeof(PROCESSOR_ENTRY);
                break;

                /// 버스 엔트리, I/O APIC 엔트리, 로컬 인터럽트 지정 엔트리는 무시
            case MP_ENTRY_TYPE_BUS:
            case MP_ENTRY_TYPE_IO_APIC:
            case MP_ENTRY_TYPE_LOCAL_INTERRUPT_ASSIGNMENT:
                qwEntryAddress += 8;
                break;


                /// IO 인터럽트 지정 엔트리이면, ISA 버스에 관련된 엔트리인지 확인
            case MP_ENTRY_TYPE_IO_INTERRUPT_ASSIGNMENT:
                pstIOAssignmentEntry = (IO_INTERRUPT_ASSIGNMENT_ENTRY *) qwEntryAddress;
                /// MP Configuration Manager 자료구조에 저장된 ISA 버스와 ID와 비교
                if (pstIOAssignmentEntry->bSourceBUSID == gs_stMPConfigurationManager.bISABusID) {
                    bFind = TRUE;
                }
                qwEntryAddress += sizeof(IO_INTERRUPT_ASSIGNMENT_ENTRY);
                break;

            default:
                break;
        }
    }

    /// 여기까지 왔는데 못 찾았으면 NIL을 반환
    if (bFind == FALSE) {
        return NIL;
    }

    //==========================================================================================
    /// ISA 버스와 관련된 I/O APIC를 검색하여 I/O APIC의 엔트리를 반환
    //==========================================================================================
    /// 다시 엔트리를 돌면서 IO 인터럽트 지정 엔트리에 저장된 I/O APIC의 ID와 일치하는 엔트리를 검색
    qwEntryAddress = gs_stMPConfigurationManager.qwBaseEntryStartAddress;
    for (i = 0; i < pstMPHeader->wEntryCount; i++) {
        bEntryType = *(BYTE *) qwEntryAddress;
        switch (bEntryType) {
            /// 프로세스 엔트리는 무시
            case MP_ENTRY_TYPE_PROCESSOR:
                qwEntryAddress += sizeof(PROCESSOR_ENTRY);
                break;

                /// 버스 엔트리, IO 인터럽트 지정 엔트리, 로컬 인터럽트 지정 엔트리는 무시
            case MP_ENTRY_TYPE_BUS:
            case MP_ENTRY_TYPE_IO_INTERRUPT_ASSIGNMENT:
            case MP_ENTRY_TYPE_LOCAL_INTERRUPT_ASSIGNMENT:
                qwEntryAddress += 8;
                break;


                /// I/O APIC 엔트리이면, ISA 버스에 관련된 엔트리인지 확인
            case MP_ENTRY_TYPE_IO_APIC:
                pstIOAPICEntry = (IO_APIC_ENTRY *) qwEntryAddress;

                if (pstIOAPICEntry->bIOAPICID == pstIOAssignmentEntry->bDestinationIOAPICID) {
                    return pstIOAPICEntry;
                }
                qwEntryAddress += sizeof(IO_INTERRUPT_ASSIGNMENT_ENTRY);
                break;


            default:
                break;
        }
    }

    return NIL;
}