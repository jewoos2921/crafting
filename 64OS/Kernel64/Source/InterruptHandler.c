//
// Created by jewoo on 2023-01-02.
//
// 인터럽트 핸들러
#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"


void kPrintString(int iX, int iY, const char *pcString);

void kCommonExceptionHanlder(int iVectorNumber, QWORD qwErrorCode) {
    char vcBuffer[3] = {0,};

    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[0] = '0' + iVectorNumber / 10;
    vcBuffer[1] = '0' + iVectorNumber % 10;

    kPrintString(0, 0, "=====================================");
    kPrintString(0, 1, "        Exception Occur~!!           ");
    kPrintString(0, 2, "                Vector:              ");
    kPrintString(27, 2, vcBuffer);
    kPrintString(0, 3, "=====================================");

    while (1);
}

void kCommonInterruptHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT: , ]";
    static int g_iCommonInterruptCount = 0;

    //=========================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    // 발생한 횟수 출력
    vcBuffer[8] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
    kPrintString(70, 0, vcBuffer);
    //=========================================================

    // EOI 전송
    kSendEOIToPIC(iVectorNumber - PIC_IRQ_START_VECTOR);
}

// 키보드 인터럽트의 핸들러
void kKeyboardHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT: , ]";
    static int g_iKeyBoardInterruptCount = 0;
    BYTE bTemp;

    //=========================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    // 발생한 횟수 출력
    vcBuffer[8] = '0' + g_iKeyBoardInterruptCount;
    g_iKeyBoardInterruptCount = (g_iKeyBoardInterruptCount + 1) % 10;

    kPrintString(0, 0, vcBuffer);
    //=========================================================

    // 키보드 컨트롤러에 데이터를 읽어서 ASCII로 변환하여 큐에 삽입
    if (kIsOutputBufferFull() == TRUE) {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue(bTemp);
    }

    // EOI 전송
    kSendEOIToPIC(iVectorNumber - PIC_IRQ_START_VECTOR);
}

// 타이머 인터럽트의 핸들러
void kTimerHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT: , ]";
    static int g_iTimerInterruptCount = 0;

    //==============================================================================
    // 인터럽트가 발생했을음 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    // 발생한 횟수 출력
    vcBuffer[8] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = (g_iTimerInterruptCount + 1) % 10;
    kPrintString(70, 0, vcBuffer);
    //==============================================================================

    // EOI 전송
    kSendEOIToPIC(iVectorNumber - PIC_IRQ_START_VECTOR);

    // 타이머 발생 횟수를 증가
    g_qwTickCount++;

    // 태스크가 사용한 프로세서의 시간을 줄임
    kDecreaseProcessorTime();
    // 프로세서가 사용할 수 있는 시간을 다 썼다면 태스크 전환을 수행
    if (kIsProcessorTimeExpired() == TRUE) {
        kScheduleInterrupt();
    }
}