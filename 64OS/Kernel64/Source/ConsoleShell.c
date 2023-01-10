//
// Created by jewoo on 2023-01-05.
//

#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"

// 커맨드 테이블 정의
SHELL_COMMAND_ENTRY gs_vstCommnadTable[] = {
        {"help",       "Show Help",                                                   kHelp},
        {"cls",        "Clear Screen",                                                kCls},
        {"totalram",   "Show Total RAM Size",                                         kShowTotalRAMSize},
        {"strtod",     "String to Decial/Hex Convert",                                kStringToDecimalHexTest},
        {"shutdown",   "Shutdown And Reboot OS",                                      kShutdown},

        {"settimer",   "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", kSetTimer},
        {"wait",       "Wait ms Using PIT, ex)wait 100(ms)",                          kWaitUsingPIT},
        {"rdtsc",      "Read Time Stamp Counter",                                     kReadTimeStampCounter},
        {"cpuspeed",   "Measure Processor Speed",                                     kMeasureProcessorSpeed},
        {"date",       "Show Date And Time",                                          kShowDateAndTime},
        {"createtask", "Create Task",                                                 kCreateTestTask},
};


// PIT 컨트롤러의 카운터 0 설정
void kSetTimer(const char *pcParameterBuffer) {
    char vcParamteter[100];
    PARAMETER_LIST stList;
    long lValue;
    BOOL bPeriodic;

    // 파라미터 초기화
    kInitializeParameter(&stList, pcParameterBuffer);

    // milisecond 추출
    if (kGetNextParameter(&stList, vcParamteter) == 0) {
        kPrintf("ex)settimer 10(ms) 1(periodic)\n");
        return;
    }
    lValue = kAToI(vcParamteter, 10);

    // Periodic 추출
    if (kGetNextParameter(&stList, vcParamteter) == 0) {
        kPrintf("ex)settimer 10(ms) 1(periodic)\n");
        return;
    }
    bPeriodic = kAToI(vcParamteter, 10);

    kInitializePIT(MSTOCOUNT(lValue), bPeriodic);
    kPrintf("Time = %d ms, Periodic = %d Change Complete\n", lValue, bPeriodic);
}

// PIT 컨트롤러를 직접 사용하여 ms 동안 대기
void kWaitUsingPIT(const char *pcParameterBuffer) {
    char vcParamteter[100];
    PARAMETER_LIST stList;
    long lMillisecond;
    int i;

    // 파라미터 초기화
    kInitializeParameter(&stList, pcParameterBuffer);
    if (kGetNextParameter(&stList, vcParamteter) == 0) {
        kPrintf("ex)wait 100(ms)\n");
        return;
    }
    lMillisecond = kAToI(pcParameterBuffer, 10);
    kPrintf("%d ms Sleep Start ... \n", lMillisecond);

    // 인터럽트를 비활성화하고 PIT 컨트롤러를 통해 직접 시간을 측정
    kDisableInterrupt();
    for (i = 0; i < lMillisecond / 30; i++) {
        kWaitUsingDirectPIT(MSTOCOUNT(30));
    }

    kWaitUsingDirectPIT(MSTOCOUNT(lMillisecond % 30));
    kEnableInterrupt();
    kPrintf("%d ms Sleep Complete\n", lMillisecond);

    // 타이머 복원
    kInitializePIT(MSTOCOUNT(1), TRUE);
}

// 타임 스탬프 카운터를 읽음
void kReadTimeStampCounter(const char *pcParameterBuffer) {
    QWORD qwTSC;
    qwTSC = kReadTSC();
    kPrintf("Time Stamp Counter = %q\n", qwTSC);
}

// 프로세서의 속도 측정
void kMeasureProcessorSpeed(const char *pcParameterBuffer) {
    int i;
    QWORD qwLastTSC, qwTotalTSC = 0;

    kPrintf("Not Measuring.");

    // 10초 동안 변화한 타임스탬프 카운터를 이용하여 프로세서의 속도를 간접적으로 측정
    kDisableInterrupt();

    for (i = 0; i < 200; i++) {
        qwLastTSC = kReadTSC();
        kWaitUsingDirectPIT(MSTOCOUNT(50));
        qwTotalTSC += kReadTSC() - qwLastTSC;

        kPrintf(".");
    }

    // 타이머 복원
    kInitializePIT(MSTOCOUNT(1), TRUE);
    kEnableInterrupt();

    kPrintf("");
}

// RTC 컨트롤러에 저장된 일자 및 시간 정보를 표시
void kShowDateAndTime(const char *pcParameterBuffer) {
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    // RTC 컨트롤러에서 시간 및 일자를 읽음
    kReadRTCTime(&bHour, &bMinute, &bSecond);
    kReadRTCDate(&wYear, &bMonth, &bDayOfMonth, &bDayOfWeek);

    kPrintf("Date: %d|%d|%d %s, ", wYear, bMonth, bDayOfMonth,
            kConvertDayOfWeekToString(bDayOfWeek));

    kPrintf("Time: %d:%d:%d\n", bHour, bMinute, bSecond);
}

// TCB 자료구조와 스택 정의
static TCB gs_vstTask[2] = {0,};
static QWORD gs_vstStack[1024] = {0,};


// 테스크 전환을 테스트하는 태스크
static void kTestTask(void) {
    int i = 0;
    while (1) {
        // 메시지를 출력하고 키 입력을 대기
        kPrintf("[%d] This message is from kTestTask. Press any key to switch kConsoleShell~!!\n", i++);
        kGetCh();

        // 위에서 키가 입력되면 태스크를 전환
        kSwitchContext(&(gs_vstTask[1].stContext), &(gs_vstTask[0].stContext));
    }
}

// 테스크를 생성해서 멀티 스태킹 수행
void kCreateTestTask(const char *pcParameterBuffer) {
    KEYDATA stData;
    int i = 0;

    // 태스크 설정
    kSetUpTask(&(gs_vstTask[1]), 1, 0, (QWORD) kTestTask, &(gs_vstStack),
               sizeof(gs_vstStack));

    // 'q' 키가 입력되지 않을 때까지 수행
    while (1) {
        kPrintf("[%d] This message is from kConsoleShell. Press any key to switch TestTask~!!\n", i++);
        if (kGetCh() == 'q') { break; }
        // 위에서 키가 입력되면 태스크를 전환
        kSwitchContext(&(gs_vstTask[0].stContext), &(gs_vstTask[1].stContext));
    }
}