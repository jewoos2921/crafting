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
#include "AssemblyUtility.h"
#include "HardDisk.h"


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
    /// 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    /// 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    /// 발생한 횟수 출력
    vcBuffer[8] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = (g_iCommonInterruptCount + 1) % 10;
    kPrintString(70, 0, vcBuffer);
    //=========================================================

    /// EOI 전송
    kSendEOIToPIC(iVectorNumber - PIC_IRQ_START_VECTOR);
}

/// 키보드 인터럽트의 핸들러
void kKeyboardHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT: , ]";
    static int g_iKeyBoardInterruptCount = 0;
    BYTE bTemp;

    //=========================================================
    /// 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    /// 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    /// 발생한 횟수 출력
    vcBuffer[8] = '0' + g_iKeyBoardInterruptCount;
    g_iKeyBoardInterruptCount = (g_iKeyBoardInterruptCount + 1) % 10;

    kPrintString(0, 0, vcBuffer);
    //=========================================================

    /// 키보드 컨트롤러에 데이터를 읽어서 ASCII로 변환하여 큐에 삽입
    if (kIsOutputBufferFull() == TRUE) {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue(bTemp);
    }

    /// EOI 전송
    kSendEOIToPIC(iVectorNumber - PIC_IRQ_START_VECTOR);
}

/// 타이머 인터럽트의 핸들러
void kTimerHandler(int iVectorNumber) {
    char vcBuffer[] = "[INT: , ]";
    static int g_iTimerInterruptCount = 0;

    //==============================================================================
    /// 인터럽트가 발생했을음 알리려고 메시지를 출력하는 부분
    /// 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    /// 발생한 횟수 출력
    vcBuffer[8] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = (g_iTimerInterruptCount + 1) % 10;
    kPrintString(70, 0, vcBuffer);
    //==============================================================================

    /// EOI 전송
    kSendEOIToPIC(iVectorNumber - PIC_IRQ_START_VECTOR);

    /// 타이머 발생 횟수를 증가
    g_qwTickCount++;

    /// 태스크가 사용한 프로세서의 시간을 줄임
    kDecreaseProcessorTime();
    /// 프로세서가 사용할 수 있는 시간을 다 썼다면 태스크 전환을 수행
    if (kIsProcessorTimeExpired() == TRUE) {
        kScheduleInterrupt();
    }
}

/// Device Not Available 예외의 핸들러
void kDeviceNotAvailableHandler(int iVectorNumber) {
    TCB *pstFPUTask;
    TCB *pstCurrentTask;
    QWORD qwLastFPUTaskID;

    // =================================================================================
    /// FPU 예외가 발생했음을 알리려고 메시지를 출력하는 부분
    char vcBuffer[] = "[EXC: , ]";
    static int g_iFPUInterruptCount = 0;

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    /// 발생한 횟수 출력
    vcBuffer[8] = '0' + g_iFPUInterruptCount;
    g_iFPUInterruptCount = (g_iFPUInterruptCount + 1) % 10;
    kPrintStringXY(0, 0, vcBuffer);
    // =================================================================================

    /// CR0 컨트롤러 레지스터의 TS 비트를 0으로 설정
    kClearTS();

    /// 이전에 FPU를 사용한 태스크가 있는지 확인해 있다면 FPU의 상태를 태스크에 저장
    qwLastFPUTaskID = kGetLastFPUUsedTaskID();
    pstCurrentTask = kGetRunningTask();

    /// 이전에 FPU를 사용한 것이 자신이면 아무것도 안 함
    if (qwLastFPUTaskID == pstCurrentTask->stLink.qwID) {
        return;
    }   /// FPU를 사용한 태스크가 있으면 FPU 상태를 저장
    else if (qwLastFPUTaskID != TASK_INVALID_ID) {
        pstFPUTask = kGetTCBInTCBPool(GET_TCB_OFF_SET(qwLastFPUTaskID));
        if ((pstFPUTask != NIL) && (pstFPUTask->stLink.qwID == qwLastFPUTaskID)) {
            kSaveFPUContext(pstFPUTask->vqwFPUContext);
        }
    }
    /// 현잿 태스크가 FPU를 사용한 적이 있는지 확인해 FPU를 사용한 적이 없다면 초기화하고,
    /// 사용한 적이 있다면 저장된 FPU 콘텍스트를 복원
    if (pstCurrentTask->bFPUUsed == FALSE) {
        kInitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;
    } else {
        kLoadFPUContext(pstCurrentTask->vqwFPUContext);
    }

    /// FPU를 사용한 태스크 ID를 현재 태스크로 변경
    kSetLastFPUUsedTaskID(pstCurrentTask->stLink.qwID);

}

/// 하드 디스크에서 발생하는 인터럽트의 핸들러
void kHDDHandler(int iVectorNumber) {

    char vcBuffer[] = "[INT: , ]";
    static int g_iHDDInterruptCount = 0;
    BYTE bTemp;

    //========================================================================================
    /// 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    /// 인터럽트 벡터를 화면 왼쪽 위에 두 자리 정수로 출력

    vcBuffer[5] = '0' + iVectorNumber / 10;
    vcBuffer[6] = '0' + iVectorNumber % 10;

    /// 발생한 횟수 출력
    vcBuffer[8] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = (g_iHDDInterruptCount + 1) % 10;
    /// 왼쪽 위에 있는 메시지와 겹치지 않도록 (10, 0)에 출력
    kPrintStringXY(10, 0, vcBuffer);
    //========================================================================================

    /// 첫 번째 PATA 포트의 인터럽트 벡터(IRQ 14) 처리
    if (iVectorNumber - PIC_IRQ_START_VECTOR == 14) {
        /// 첫 번째 PATA 포트의 인터럽트의 발생 여부를 TRUE로 설정
        kSetHDDInterruptFlag(TRUE, TRUE);
    }
        /// 두 번째 PATA 포트의 인터럽트 벡터(IRQ 15) 처리
    else {
        /// 두 번째 PATA 포트의 인터럽트의 발생 여부를 TRUE로 설정
        kSetHDDInterruptFlag(FALSE, TRUE);
    }
    /// EOI 전송
    kSendEOIToPIC(iVectorNumber - PIC_IRQ_START_VECTOR);
}
