//
// Created by jewoo on 2022-12-29.
//
/*
 * 데이터 모델링 아키텍처
 * 데이터를 정확하고 용이하게 사용할수 있도록 정돈
 * 현재와 미래를 보는 소프트웨어 기술 | 과거와 현재를 보는 데이터 기술
 * 많은 사람이 달리는 것만 생각하고 뒤에 남을 데이터는 생각하지 않는다.
 * 얻고자 하는것, 가치가 무엇이냐?? 원하는 방향
 * 데이터를 사용하느것은 일하는 방식과 패러다임 까지 바꾸는 것
 * 데이터와 관련된 여러일은 동시에
 * 새로 생긴 분야에 대해 이해
 * 머신러닝 알고리즘은 불확실성과 확률이 존재
 * */
// C 코드 엔트리 포인트 파일
#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "PIT.h"
#include "DynamicMemory.h"
#include "HardDisk.h"
#include "SerialPort.h"
#include "FileSystem.h"
#include "MultiProcessor.h"

#include "Page.h"


// 보호 모드 커널의 C언어 엔트리 포인트
#include "ModeSwitch.h"
#include "AssemblyUtility.h"
#include "Utility.h"
#include "LocalAPIC.h"


void kPrintString(int iX, int iY, const char *pcString);

BOOL kInitializeKernel64Area(void);

BOOL kIsMemoryEnough(void);

void kCopyKernel64ImageTo2Mbyte(void);

/// Application Processor를 위한 Main함수
void MainForApplicationProcessor(void);

/// Bootstrap Processor 용 C 언어 커널 엔트리 포인트
///             아래 함수는 C 언어 커널의 시작 부분
int main() {
    int iCursorX, iCursorY;

    /// 부트로더에 있는 BSP 플래그를 읽어서 Application Processor이면
    /// 해당 코어용 초기화 함수로 이동
    if (*((BYTE *) BOOTSTRAP_PROCESSOR_FLAG_ADDRESS) == 0) {
        MainForApplicationProcessor();
    }

    /// 콘솔을 먼저 초기화한 후 다음 작업을 수행
    kInitializeConsole(0, 10);
    kPrintf("Switch to IA-32e Mode Success~!!\n");
    kPrintf("IA-32e C Language Kernel Start.............[Pass]\n");
    kPrintf("Initialize Console...................[Pass]\n");


    /// 부팅 상황을 화면에 출력
    kGetCursor(&iCursorX, &iCursorY);
    kPrintf("GDT Initialize And Switch For IA-32e Mode...[   ]");
    kInitializeGDTTableAndTSS();
    kLoadGDTR(GDTR_STAR_ADDRESS);
    kSetCursor(45, iCursorY++);
    kPrintf("Pass\n");

    kPrintf("TSS Segment Load.........................[     ]");
    kLoadTR(GDT_TSS_SEGMENT);
    kSetCursor(45, iCursorY++);
    kPrintf("Pass\n");

    kPrintf("IDT Initialize..........................[      ]");
    kInitializeIDTTables();
    kLoadGDTR(IDTR_START_ADDRESS);
    kSetCursor(45, iCursorY++);
    kPrintf("Pass\n");

    kPrintf("Total RAM Size Check.....................[     ]");
    kCheckTotalRAMSize();
    kSetCursor(45, iCursorY++);
    kPrintf("Pass] ,Size= %d MB\n", kGetTotalRAMSize());

    kPrintf("TCB Pool And Scheduler Initialzie.......[Pass]\n");
    iCursorY++;
    kInitializeScheduler();

    /// 동적 메모리 초기화
    kPrintf("Dynamic Memory Initialzie................[Pass]\n");
    iCursorY++;
    kInitializeDynamicMemory();


    /// 1ms당 한 번씩 인터럽트가 발생하도록 설정
    kInitializePIT(MSTOCOUNT(1), 1);

    kPrintf("Keyboard Activate And Queue Initialize......[    ]");

    /// 키보드를 활성화
    if (kInitializeKeyboard() == TRUE) {
        kSetCursor(45, iCursorY++);
        kPrintf("Pass\n");
        kChangeKeyboardLED(FALSE, FALSE, FALSE);
    } else {
        kSetCursor(45, iCursorY++);
        kPrintf("Fail\n");
        while (1);
    }

    kPrintf("PIC Controller And Interrupt Initialize....[   ]");
    /// PIC 컨트롤러 초기화 및 모든 인터럽트 활성화
    kInitializePIC();
    kMaskPICInterrupt(0);
    kEnableInterrupt();
    kSetCursor(45, iCursorY++);
    kPrintf("Pass\n");


    /// 하드 디스크를 초기화
    kPrintf("HDD Initialize...................[     ]");
    if (kInitializeHDD() == TRUE) {
        kSetCursor(45, iCursorY++);
        kPrintf("Pass\n");
    } else {
        kSetCursor(45, iCursorY++);
        kPrintf("Fail\n");
    }

    /// 파일 시스템을 초기화
    kPrintf("File System Initialize...................[     ]");
    if (kInitializeFileSystem() == TRUE) {
        kSetCursor(45, iCursorY++);
        kPrintf("Pass\n");
    } else {
        kSetCursor(45, iCursorY++);
        kPrintf("Fail\n");
    }

    kPrintf("Serial Port Initialize...................[Pass]\n");
    iCursorY++;
    kInitializeSerialPort();

    /// 유휴 태스크를 수행하고 셸을 시작
    kCreateTask(TASK_FLAGS_LOWEST | TASK_FLAGS_IDLE | TASK_FLAGS_SYSTEM | TASK_FLAGS_THREAD,
                0, 0, (QWORD) kIdleTask);

    kStartConsoleShell();
}

/// Application Processor용 C 언어 커널 엔트리 포인트
///         대부분의 자료구조는 Bootstrap Processor가 생성해 놓았으므로 코어에 설정하는 작업만 함
///             대칭 I/O 모드를 지원하도록 수정
void MainForApplicationProcessor(void) {
    QWORD qwTickCount;

    /// GDT 테이블을 설정
    kLoadGDTR(GDTR_STAR_ADDRESS);

    /// TSS 디스크립터 설정. TSS 세그먼트와 디스크립터를  Application Processor의 수만큼
    /// 생성했으므로, APIC ID를 이용하여 TSS 디스크립터를 할당
    kLoadTR(GDT_TSS_SEGMENT + (kGetAPICID() * sizeof(GDTENTRY16)));

    /// IDT 테이블을 설정
    kLoadIDTR(IDTR_START_ADDRESS);

    /// 현재 코어의 로컬 APIC를 활성화
    kEnableGlobalLocalAPIC(); // 모든 코어 또는 프로세서의 로컬 APIC를 활성화는 BSP가 이미 했으므로, 개별 로컬 APIC 활성화 처리만 수행

    /// 모든 인터럽트를 수신할 수 있도록 태스크 우선순위 레지스터를 0으로 설정
    kSetTaskPriority(0);


    /// 로컬 APIC의 로컬 벡터 테이블을 초기화
    kInitializeLocalVectorTable();

    /// 인터럽트를 활성화
    kEnableInterrupt();

    /// 1초마다 한 번씩 메시지를 출력
    qwTickCount = kGetTickCount();
    while (1) {
        if (kGetTickCount() - qwTickCount > 1000) {
            qwTickCount - kGetTickCount();

            /// 대칭 I/O 모드 테스크를 위해 Application Processor가 시작한 후 한 번만 출력
//            kPrintf("Application Processor[APIC ID: %d] Is Activated\n",
//                    kGetAPICID());
        }
    }
}


//
//
//int main() {
//
//    char vcTemp[2] = {0,};
//    BYTE bTemp;
//    DWORD i;
//    KEYDATA stData;
//
//    // IA-32e 모드로 전환
//    kPrintString(0, 10, "Switch to IA-32e Mode Success~!!");
//    kPrintString(0, 11, "IA-32e C Language Kernel Start.............[Pass]");
//
//    kPrintString(0, 12, "GDT Initialize And Switch For IA-32e Mode...[   ]");
//    kInitializeGDTTableAndTSS();
//    kLoadGDTR(GDTR_STAR_ADDRESS);
//    kPrintString(45, 12, "Pass");
//
//    kPrintString(0, 13, "TSS Segment Load........................[    ]");
//    kLoadTR(GDT_TSS_SEGMENT);
//    kPrintString(45, 13, "Pass");
//
//
//    kPrintString(0, 14, "IDT Initialize..........................[      ]");
//    kInitializeIDTTables();
//    kLoadGDTR(IDTR_START_ADDRESS);
//    kPrintString(45, 14, "Pass");
//
//    kPrintString(0, 15, "Keyboard Activate And Queue Initialize.......[      ]");
//
//
//    // 키보드를 활성화
//    if (kInitializeKeyboard() == TRUE) {
//        kPrintString(45, 15, "Pass");
//        kChangeKeyboardLED(FALSE, FALSE, FALSE);
//    } else {
//        kPrintString(45, 15, "Fail");
//        while (1);
//    }
//
//    kPrintString(0, 16, "PIC Controller And Interrupt Initialize....[   ]");
//    // PIC 컨트롤러 초기화 및 모든 인터럽트 활성화
//    kInitializePIC();
//    kMaskPICInterrupt(0);
//    kEnableInterrupt();
//    kPrintString(45, 16, "Pass");
//
//    while (1) {
//
//        // 키가 큐에 데이터가 있으면 키를 처리
//        if (kGetKeyFromKeyQeueue(&stData) == TRUE) {
//            // 키가 눌러졌으면 키의 ASCII 코드 값을 화면에 출력
//            if (stData.bFlags & KEY_FLAGS_DOWN) {
//
//                // 키 데이터의 ASCII 코드 값을 저장
//                vcTemp[0] = stData.bASCIICode;
//                kPrintString(i++, 17, vcTemp);
//                // 0이 입력되면 변수를 0으로 나누어 Divide Error 예외(벡터 0번)를 발생시킴
//                if (vcTemp[0] == '0') {
//                    // 아래 코드를 수행하면 Divide Error 예외가 발생하여
//                    // 커널의 임시 핸들러가 수행됨
//                    bTemp = bTemp / 0;
//                }
//            }
//        }
//    }


//    kSwitchAndExecute64bitKernel();
//
//    while (1);
//}



//
//int main() {
//
//    DWORD i;
//    DWORD dwEAX, dwEBX, dwECX, dwEDX;
//
//    char vcVendorString[13] = {0,};
//
//    kPrintString(0, 3, "C Language Kernel Started~!!!");
//
//
//    // 최소 메모리 크기를 만족하는 지 검사
//    kPrintString(0, 4, "Minimum Memory Size Check....................[      ]");
//    if (kIsMemoryEnough() == FALSE) {
//        kPrintString(45, 4, "Fail");
//        kPrintString(0, 5, "Not Enough Memory~!! "
//                           "MINTI64 OS Requires Over 64MB Memory~!!");
//
//        while (1);
//    } else {
//        kPrintString(45, 4, "Pass");
//    }
//
//
//    // IA-32e 모드의 커널 영역을 초기화
//    kPrintString(0, 5, "IA-32e Kernel Area Intialize.............[   ]");
//    if (kInitializeKernel64Area() == FALSE) {
//        kPrintString(45, 5, "Fail");
//        kPrintString(0, 6, "Kernel Area Initialization Fail~!!");
//        while (1);
//    }
//    kPrintString(45, 5, "Pass");
//
//    // IA-32e 모드 커널을 위한 페이지 테이블 생성
//    kPrintString(0, 6, "IA-32e Page Tables Intialize..........[    ]");
//    kInitializePageTables();
//    kPrintString(45, 6, "Pass");
//
//    // 프로세스 제조사 정보 읽기
//    kReadCPUID(0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX);
//    *(DWORD *) vcVendorString = dwEBX;
//    *((DWORD *) vcVendorString + 1) = dwEDX;
//    *((DWORD *) vcVendorString + 2) = dwECX;
//
//    kPrintString(0, 7, "Processor vendor string ......... [          ]");
//    kPrintString(45, 7, vcVendorString);
//
//    // 64 비트 지원 유무 확인
//    kReadCPUID(0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX);
//    kPrintString(0, 8, "64bit Mode Support Check .............[         ]");
//
//    if (dwEDX & (1 << 29)) {
//        kPrintString(45, 8, "Pass");
//    } else {
//        kPrintString(45, 8, "Fail");
//        kPrintString(0, 9, "This processor does not support 64bit model~!!");
//        while (1);
//    }
//
//    // IA-32e 모드 커널을 0x200000(2MBbyte) 어드레스로 이동
//    kPrintString(0, 9, "Copy IA-32e Kernel to 2M Address...........[    ]");
//    kCopyKernel64ImageTo2Mbyte();
//    kPrintString(45, 9, "Pass");
//
//
//    char vcTemp[2] = {0,};
//    BYTE bFlags;
//    BYTE bTemp;
//
//    // IA-32e 모드로 전환
//    kPrintString(0, 10, "Switch to IA-32e Mode Success~!!");
//    kPrintString(0, 11, "IA-32e C Language Kernel Start.............[Pass]");
//    kPrintString(0, 12, "Keyboard Activate .............[    ]");
//
//
//    // 키보드를 활성화
//    if (kActivateKeyboard() == TRUE) {
//        kPrintString(45, 12, "Pass");
//        kChangeKeyboardLED(FALSE, FALSE, FALSE);
//    } else {
//        kPrintString(45, 12, "Fail");
//        while (1);
//    }
//
//    while (1) {
//        // 출력 버퍼(포트 0x60)가 차 있으면 스캔 코드를 읽을 수 있음
//        if (kIsOutputBufferFull() == TRUE) {
//            // 출력 버퍼(포트 0x60)에서 스캔 코드를 읽어서 저장
//            bTemp = kGetKeyboardScanCode();
//
//            // 스캔 코드를 ASCII 코드로 변환하는 함수를 호출하여 ASCII 코드와
//            // 눌림 또는 떨어짐 정보를 반환
//            if (kConvertScanCodeToASCIICode(bTemp, &(vcTemp[0]), &bFlags) == TRUE) {
//                // 키가 눌러졌으면 키의 ASCII 코드 값을 화면에 출력
//                if (bFlags & KEY_FLAGS_DOWN) {
//                    kPrintString(i++, 13, vcTemp);
//                }
//            }
//        }
//    }
//
//
//   kSwitchAndExecute64bitKernel();
//
//    while (1);
//}



/* IA-32e 모드 커널이 위치할 공간을 0으로 초기화하는 코드 */
BOOL kInitializeKernel64Area(void) {
    DWORD *pdwCurrentAddress;

    // 초기화를 시작할 어드레스인 0x100000(1MB)을 설정
    pdwCurrentAddress = (DWORD *) 0x100000;

    // 마지막 어드레스인 0x600000(6MB)까지 루프를 돌면서 4바이트씩 0으로 채움
    while ((DWORD) pdwCurrentAddress < 0x600000) {
        *pdwCurrentAddress = 0x00;

        // 0으로 저장한 후 다시 읽었을 때 0이 나오지 않으면 해당 어드레스를 사용하는데
        // 문제가 생긴 것이므로 더이상 진행하지 않고 종료
        if (*pdwCurrentAddress != 0) {
            return FALSE;
        }

        // 다음 어드레스로 이동
        pdwCurrentAddress++;
    }
    return TRUE;
}

/* MINT64 OS를 실행하기에 충분한 메모리를 가지고 있는지 체크 */
BOOL kIsMemoryEnough(void) {
    DWORD *pdwCurrentAddress;

    // 0x100000(1MB)부터 검사 시작
    pdwCurrentAddress = (DWORD *) 0x100000;

    // 0x4000000(64MB)까지 루프를 돌면서 확인
    while ((DWORD) pdwCurrentAddress < 0x4000000) {
        *pdwCurrentAddress = 0x12345678;

        // 0x12345678으로 저장한 후 다시 읽었을 때 0x12345678이 나오지 않으면 해당 어드레스를 사용하는데
        // 문제가 생긴 것이므로 더이상 진행하지 않고 종료
        if (*pdwCurrentAddress != 0x12345678) {
            return FALSE;
        }

        // 1MB씩 이동하면서 확인
        pdwCurrentAddress += (0x100000 / 4);
    }

    return TRUE;
}

// IA-32e 모드 커널을 0x200000(2Mbyte) 어드레스에 복사
void kCopyKernel64ImageTo2Mbyte(void) {
    WORD wKernel32SectorCount, wTotalKernelSectorCount;
    DWORD *pdwSourceAddress;
    DWORD *pdwDestinationAddress;

    // 0x7C05에 총 커널 섹터 수, 0x7C07에 보호 모드 커널 수가 들어 있음
    wTotalKernelSectorCount = *((WORD *) 0x7C05);
    wKernel32SectorCount = *((WORD *) 0x7C07);

    pdwSourceAddress = (DWORD *) (0x10000 + (wKernel32SectorCount * 512));
    pdwDestinationAddress = (DWORD *) 0x200000;

    // IA-32E 모드 커널 섹터 크기만큼 복사
    for (int i = 0; i < 512 * (wTotalKernelSectorCount - wKernel32SectorCount) / 4; ++i) {
        *pdwDestinationAddress = *pdwSourceAddress;
        pdwDestinationAddress++;
        pdwSourceAddress++;
    }
}
