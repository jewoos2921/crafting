//
// Created by jewoo on 2022-12-29.
//
// C 코드 엔트리 포인트 파일
#include "Types.h"
#include "Page.h"

// 보호 모드 커널의 C언어 엔트리 포인트
#include "ModeSwitch.h"

void kPrintString(int iX, int iY, const char *pcString);

BOOL kInitializeKernel64Area(void);

BOOL kIsMemoryEnough(void);

void kCopyKernel64ImageTo2Mbyte(void);

/// Bootstrap Processor 여부가 저장된 어드레스, 부트 로더 영역의 앞쪽에 위치
#define BOOTSTRAP_PROCESSOR_FLAG_ADDRESS            0x7C09

int main() {

    DWORD i;
    DWORD dwEAX, dwEBX, dwECX, dwEDX;
    char vcVendorString[13] = {0,};

    /// Application Processor이면 아래의 코드를 생략하고 바로 64비트 모드로 전환
    if (*((BYTE *) BOOTSTRAP_PROCESSOR_FLAG_ADDRESS) == 0) {
        kSwitchAndExecute64bitKernel();
        while (1);
    }
    //==============================================================================
    /// BSP만 수행하는 코드
    //==============================================================================

    kPrintString(0, 3, "C Language Kernel Started~!!!");

    // 최소 메모리 크기를 만족하는 지 검사
    kPrintString(0, 4, "Minimum Memory Size Check....................[      ]");
    if (kIsMemoryEnough() == FALSE) {
        kPrintString(45, 4, "Fail");
        kPrintString(0, 5, "Not Enough Memory~!! "
                           "MINTI64 OS Requires Over 64MB Memory~!!");

        while (1);
    } else {
        kPrintString(45, 4, "Pass");
    }


    // IA-32e 모드의 커널 영역을 초기화
    kPrintString(0, 5, "IA-32e Kernel Area Intialize.............[   ]");
    if (kInitializeKernel64Area() == FALSE) {
        kPrintString(45, 5, "Fail");
        kPrintString(0, 6, "Kernel Area Initialization Fail~!!");
        while (1);
    }
    kPrintString(45, 5, "Pass");

    // IA-32e 모드 커널을 위한 페이지 테이블 생성
    kPrintString(0, 6, "IA-32e Page Tables Intialize..........[    ]");
    kInitializePageTables();
    kPrintString(45, 6, "Pass");

    // 프로세스 제조사 정보 읽기
    kReadCPUID(0x00, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    *(DWORD *) vcVendorString = dwEBX;
    *((DWORD *) vcVendorString + 1) = dwEDX;
    *((DWORD *) vcVendorString + 2) = dwECX;

    kPrintString(0, 7, "Processor vendor string ......... [          ]");
    kPrintString(45, 7, vcVendorString);

    // 64 비트 지원 유무 확인
    kReadCPUID(0x80000001, &dwEAX, &dwEBX, &dwECX, &dwEDX);
    kPrintString(0, 8, "64bit Mode Support Check .............[         ]");

    if (dwEDX & (1 << 29)) {
        kPrintString(45, 8, "Pass");
    } else {
        kPrintString(45, 8, "Fail");
        kPrintString(0, 9, "This processor does not support 64bit model~!!");
        while (1);
    }

    // IA-32e 모드 커널을 0x200000(2MBbyte) 어드레스로 이동
    kPrintString(0, 9, "Copy IA-32e Kernel to 2M Address...........[    ]");
    kCopyKernel64ImageTo2Mbyte();
    kPrintString(45, 9, "Pass");


    // IA-32e 모드로 전환
    kPrintString(0, 10, "Switch to IA-32e Mode Success~!!");
    kSwitchAndExecute64bitKernel();

    while (1);
}

// 문자열 출력 함수
// 문자열을 X, Y 위치에 출력
void kPrintString(int iX, int iY, const char *pcString) {
    CHARACTER *pstScreen = (CHARACTER *) 0xB8000;

    // X, Y 좌표를 이용해서 문자열을 출력할 어드레스르 계산
    pstScreen += (iY * 80) + iX;

    // NULL이 나올 때까지 문자열 출력
    for (int i = 0; pcString[i] != 0; ++i) {
        pstScreen[i].bCharactor = pcString[i];
    }
}

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
