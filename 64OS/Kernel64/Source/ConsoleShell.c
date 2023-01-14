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
#include "Task.h"
#include "Synchronization.h"


// 커맨드 테이블 정의
SHELL_COMMAND_ENTRY gs_vstCommnadTable[] = {
        {"help",           "Show Help",                                                   kHelp},
        {"cls",            "Clear Screen",                                                kCls},
        {"totalram",       "Show Total RAM Size",                                         kShowTotalRAMSize},
        {"strtod",         "String to Decial/Hex Convert",                                kStringToDecimalHexTest},
        {"shutdown",       "Shutdown And Reboot OS",                                      kShutdown},

        {"settimer",       "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", kSetTimer},
        {"wait",           "Wait ms Using PIT, ex)wait 100(ms)",                          kWaitUsingPIT},
        {"rdtsc",          "Read Time Stamp Counter",                                     kReadTimeStampCounter},
        {"cpuspeed",       "Measure Processor Speed",                                     kMeasureProcessorSpeed},
        {"date",           "Show Date And Time",                                          kShowDateAndTime},
        {"createtask",     "Create Task",                                                 kCreateTestTask},
        {"changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)",
                                                                                          kChangeTaskPriority},
        {"tasklist",       "Show Task List",                                              kShowTaskList},
        {"killtask",       "End Task, ex)killtask 1(ID) or 0xffffffff(All Task)",
                                                                                          kKillTask},
        {"cpuload",        "Show Processor Load",                                         kCPULoad},
        {"testmutex",      "Test Mutex Function",                                         kTestMutex},

};

//======================================================================================================================
// 실제 셸을 구성하는 코드
//======================================================================================================================
// 셸의 메인 루프
void kStartConsoleShell(void) {
    char vcCommandBuffer[CONSOLE_SHELL_MAX_COMMAND_BUFFER_COUNT];
    int iCommandBufferIndex = 0;
    BYTE bKey;
    int iCursorX, iCursorY;
    // 프롬프트 출력
    kPrintf(CONSOLE_SHELL_PROMPT_MESSAGE);

    while (1) {
        // 키가 수신될 때까지 대기
        bKey = kGetCh();
        // Backspace 키 처리
        if (bKey == KEY_BACKSPACE) {
            if (iCommandBufferIndex > 0) {


                // 현재 커서 위치를 얻어서 한 문자 앞으로 이동한 다음 공백을 출력하고
                // 커맨드 버퍼에서 마지막 문자 삭제
                kGetCursor(&iCursorX, &iCursorY);
                kPrintStringXY(iCursorX - 1, iCursorY, " ");
                kSetCursor(iCursorX - 1, iCursorY);
                iCommandBufferIndex--;
            }
        }
            // 엔터 키 처리
        else if (bKey == KEY_ENTER) {
            kPrintf("\n");
            if (iCommandBufferIndex > 0) {
                // 커맨드 버퍼에 있는 명령을 실행
                vcCommandBuffer[iCommandBufferIndex] = '\0';
                kExecuteCommand(vcCommandBuffer);
            }
            // 프롬프트 출력 및 커맨드 버퍼 초기화
            kPrintf("%s", CONSOLE_SHELL_PROMPT_MESSAGE);
            kMemSet(vcCommandBuffer, '\0', CONSOLE_SHELL_MAX_COMMAND_BUFFER_COUNT);
            iCommandBufferIndex = 0;
        } else if ((bKey == KEY_LSHIFT) || (bKey == KEY_RSHIFT) || (bKey == KEY_CAPSLOCK) || (bKey == KEY_NUMLOCK) ||
                   (bKey == KEY_SCROLL_LOCK)) { ;
        } else {
            // TAB은 공백으로
            if (bKey == KEY_TAB) {
                bKey = ' ';
            }

            // 버퍼에 공간이 남아 있을 때만 가능
            if (iCommandBufferIndex < CONSOLE_SHELL_MAX_COMMAND_BUFFER_COUNT) {
                vcCommandBuffer[iCommandBufferIndex++] = bKey;
                kPrintf("%c", bKey);
            }
        }
    }
}


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

// 커맨드 버퍼에 있는 커맨드를 비교하여 해당 커맨드를 처리
void kExecuteCommand(const char *pcCommandBuffer) {
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;

    // 공백으로 구분된 커맨드를 추출
    iCommandBufferLength = kStrLen(pcCommandBuffer);
    for (iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++) {
        if (pcCommandBuffer[iSpaceIndex] == ' ') { break; }
    }

    // 커맨드 테이블을 검사해서 같은 이름의 커맨드가 있는지 확인
    iCount = sizeof(gs_vstCommnadTable) / sizeof(SHELL_COMMAND_ENTRY);
    for (i = 0; i < iCount; i++) {
        iCommandLength = kStrLen(gs_vstCommnadTable[i].pcCommand);
        // 커맨드의 길이와 내용이 완전히 일치하는지 검사
        if ((iCommandLength == iSpaceIndex) && (kMemCmp(gs_vstCommnadTable[i].pcCommand, pcCommandBuffer,
                                                        iSpaceIndex) == 0)) {
            gs_vstCommnadTable[i].pfFunction(pcCommandBuffer + iSpaceIndex + 1);
            break;
        }
    }
    // 리스트에서 찾을 수 없다면 에러 출력
    if (i >= iCount) {
        kPrintf("'%s' is not found.\n", pcCommandBuffer);
    }
}

// 파라미터 자료구조를 초기화
void kInitializeParameter(PARAMETER_LIST *pstList, const char *pcParameter) {
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen(pcParameter);
    pstList->iCurrentPosition = 0;
}

// 공백으로 구분된 파라미터의 내용과 길이를 반환
int kGetNextParameter(PARAMETER_LIST *pstList, char *pcParameter) {
    int i;
    int iLength;

    // 더 이상 파리미터가 없으면 나감
    if (pstList->iLength <= pstList->iCurrentPosition) { return 0; }

    // 버퍼의 길이만큼 이동하면서 공백을 검색
    for (i = pstList->iCurrentPosition; i < pstList->iLength; i++) {
        if (pstList->pcBuffer[i] == ' ') { break; }
    }

    // 파라미터를 복사하고 길이를 반환
    kMemCpy(pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i);
    iLength = i - pstList->iCurrentPosition;
    pcParameter[iLength] = '\0';

    // 파라미터의 위치 업데이트
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}

//======================================================================================================================
// 커맨드를 처리하는 함수
//======================================================================================================================
// 셸 도움말을 출력
static void kHelp(const char *pcPrameterBuffer) {
    int i;
    int iCount;
    int iCursorX;
    int iCursorY;
    int iLength;
    int iMaxCommandLength = 0;

    kPrintf("============================================================================\n");
    kPrintf("                                 MINT64 Shell Help                            ");
    kPrintf("============================================================================\n");

    iCount = sizeof(gs_vstCommnadTable) / sizeof(SHELL_COMMAND_ENTRY);

    // 가장 긴 커맨드의 길이를 계산
    for (i = 0; i < iCount; ++i) {
        iLength = kStrLen(gs_vstCommnadTable[i].pcCommand);
        if (iLength > iMaxCommandLength) {
            iMaxCommandLength = iLength;
        }
    }

    // 도움말 출력
    for (i = 0; i < iCount; ++i) {
        kPrintf("%s", gs_vstCommnadTable[i].pcCommand);
        kGetCursor(&iCursorX, &iCursorY);
        kSetCursor(iMaxCommandLength, iCursorY);
        kPrintf(" - %s\n", gs_vstCommnadTable[i].pcHelp);
    }
}

// 화면을 지움
static void kCls(const char *pcPrameterBuffer) {
    // 맨 윗줄은 디버깅 용으로 사용하므로 화면을 지훈 후 라인 1로 커서 이동
    kClearScrren();
    kSetCursor(0, 1);
}

// 총 메모리 크기를 출력
static void kShowTotalRAMSize(const char *pcPrameterBuffer) {
    kPrintf("Total RAM Size = %d MB\n", kGetTotalRAMSize());
}

// 문자열로 된 숫저룰 슛자로 변환하여 화면에 출력
static void kStringToDecimalHexTest(const char *pcPrameterBuffer) {
    char vcParameter[100];
    int iLength;
    PARAMETER_LIST stList;
    int iCount = 0;
    long lValue;

    // 파라미터 초기화
    kInitializeParameter(&stList, pcPrameterBuffer);

    while (1) {
        // 다음 파라미터를 구함,
        // 파라미터의 길이가 0이면 파아미터가 없는 것으므로 종료
        iLength = kGetNextParameter(&stList, vcParameter);
        if (iLength == 0) {
            break;
        }

        //파라미터에 대한 정보를 출력하고 16진수인지 10진수인지 판단하여 변환한 후 결과를 printf 로 출력
        kPrintf("Param %d = '%s' Length = %d,  ", iCount + 1, vcParameter, iLength);

        // 0x로 시작하면 16진수 , 그외에는 10진수로 판단
        if (kMemCmp(vcParameter, "0x", 2) == 0) {
            lValue = kAToI(vcParameter + 2, 16);
            kPrintf("HEX Value = %q\n", lValue);
        } else {
            lValue = kAToI(vcParameter, 10);
            kPrintf("Decimal Value = %d\n", lValue);
        }
        iCount++;
    }
}

// PC를 재시작
static void kShutdown(const char *pcPrameterBuffer) {
    kPrintf("System Shutdown Start...\n");

    // 키보드 컨트롤러를 통해 PC를 재시작
    kPrintf("Press Any Key To Rebbot PC...");
    kGetCh();
    kReboot();
}

// PIT 컨트롤러를 직접 사용하여 ms 동안 대기
static void kWaitUsingPIT(const char *pcParameterBuffer) {
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
static void kReadTimeStampCounter(const char *pcParameterBuffer) {
    QWORD qwTSC;
    qwTSC = kReadTSC();
    kPrintf("Time Stamp Counter = %q\n", qwTSC);
}

// 프로세서의 속도 측정
static void kMeasureProcessorSpeed(const char *pcParameterBuffer) {
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
static void kShowDateAndTime(const char *pcParameterBuffer) {
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


// 화면 테두리를 돌면서 문자를 출력
static void kTestTask1(void) {
    BYTE bData;
    int i = 0;
    int iX = 0;
    int iY = 0;
    int iMargin, j;

    CHARACTER *pstScreen = (CHARACTER *) CONSOLE_VIDEO_MEMORY_ADDRESS;
    TCB *pstRunningTask;

    // 자신의 ID를 얻어서 화면 오프셋으로 사용
    pstRunningTask = kGetRunningTask();
    iMargin = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) % 10;


    // 화면 네 귀퉁이를 돌면서 문자 출력
    for (j = 0; j < 20000; j++) {
        switch (i) {
            case 0:
                iX++;
                if (iX >= (CONSOLE_WIDTH - iMargin)) {
                    i = 1;
                }
                break;

            case 1:
                iY++;
                if (iY >= (CONSOLE_HEIGHT - iMargin)) {
                    i = 2;
                }
                break;

            case 2:
                iX--;
                if (iX < iMargin) {
                    i = 3;
                }
                break;

            case 3:
                iY--;
                if (iY < iMargin) {
                    i = 0;
                }
                break;
        }

        // 문자 및 색깔 지정
        pstScreen[iY * CONSOLE_WIDTH + iX].bCharactor = bData;
        pstScreen[iY * CONSOLE_WIDTH + iX].bAttribute = bData & 0x0F;
        bData++;
        // 다른 태스크로 전환
//        kSchedule();
    }

    kExitTask();
}

// 자신의 ID를 참고하여 특정 위치에 회전하는 바람개비를 출력
static void kTestTask2(void) {
    int i = 0;
    int iOffset;
    CHARACTER *pstScreen = (CHARACTER *) CONSOLE_VIDEO_MEMORY_ADDRESS;
    TCB *pstRunningTask;
    char vcData[4] = {'-', '\\', '|', '/'};

    // 자신의 ID를 얻어서 화면 오프셋으로 사용
    pstRunningTask = kGetRunningTask();
    iOffset = (pstRunningTask->stLink.qwID & 0xFFFFFFFF) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - (iOffset % (CONSOLE_WIDTH * CONSOLE_HEIGHT));

    while (1) {
        // 회전하는 바람개비 표시
        pstScreen[iOffset].bCharactor = vcData[i % 4];
        // 색깔 지정
        pstScreen[iOffset].bAttribute = (iOffset % 15) + 1;
        i++;

        // 다른 태스크로 전환
//        kSchedule();
    }
}

// 테스크를 생성해서 멀티 스태킹 수행
static void kCreateTestTask(const char *pcParameterBuffer) {

    PARAMETER_LIST stList;
    char vcType[30];
    char vcCount[30];
    int i;

    // 파라미터를 출력
    kInitializeParameter(&stList, pcParameterBuffer);
    kGetNextParameter(&stList, vcType);
    kGetNextParameter(&stList, vcCount);

    switch (kAToI(vcType, 10)) {

        case 1:
            for (i = 0; i < kAToI(vcCount, 10); i++) {
                if (kCreateTask(TASK_FLAGS_LOW, (QWORD) kTestTask1) == NIL) {
                    break;
                }
            }
            kPrintf("Task1 %d Crated\n", i);
            break;

        case 2:
        default:
            for (i = 0; i < kAToI(vcCount, 10); i++) {
                if (kCreateTask(TASK_FLAGS_LOW, (QWORD) kTestTask2) == NIL) {
                    break;
                }
            }
            kPrintf("Task2 %d Crated\n", i);
            break;
    }
}

// 태스크의 우선순위를 변경
static void kChangeTaskPriority(const char *pcParameterBuffer) {
    PARAMETER_LIST stList;
    char vcID[30];
    char vcPriority[30];
    QWORD qwID;
    BYTE bPirority;


    // 파라미터 추출
    kInitializeParameter(&stList, pcParameterBuffer);
    kGetNextParameter(&stList, vcID);
    kGetNextParameter(&stList, vcPriority);

    // 태스크의 우선순위를 변경
    if (kMemCmp(vcID, "0x", 2) == 0) {
        qwID = kAToI(vcID + 2, 16);
    } else {
        qwID = kAToI(vcID, 10);
    }

    bPirority = kAToI(vcPriority, 10);

    kPrintf("Change Task Priority ID [0x%q] Priority [%d] ", qwID, bPirority);

    if (kChangePriority(qwID, bPirority) == TRUE) {
        kPrintf("Success\n");
    } else {
        kPrintf("Fail\n");
    }
}

// 현재 생성된 모든 태스크의 정보를 출력
static void kShowTaskList(const char *pcParameterBuffer) {
    int i;
    TCB *pstTCB;
    int iCount = 0;

    kPrintf("============= Task Total Count [%d] ==================\n", kGetTaskCount());

    for (i = 0; i < TASK_MAX_COUNT; i++) {
        // TCB를 구해서 TCB가 사용중이면 ID를 출력
        pstTCB = kGetTCBInTCBPool(i);
        if ((pstTCB->stLink.qwID >> 32) != 0) {
            if ((iCount != 0) && ((iCount % 10) == 0)) {
                kPrintf("Press any key to continue...('q' is exit) : ");
                if (kGetCh() == 'q') {
                    kPrintf("\n");
                    break;
                }
                kPrintf("\n");
            }
            kPrintf("[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q]\n", 1 + iCount++,
                    pstTCB->stLink.qwID, GET_PRIORITY(pstTCB->qwFlags), pstTCB->qwFlags);

        }
    }
}

// 태스크 종료
static void kKillTask(const char *pcParameterBuffer) {
    PARAMETER_LIST stList;
    char vcID[30];
    QWORD qwID;
    TCB *pstTCB;
    int i;

    // 파라미터를 추출
    kInitializeParameter(&stList, pcParameterBuffer);
    kGetNextParameter(&stList, vcID);

    // 태스크를 종료
    if (kMemCmp(vcID, "0x", 2) == 0) {
        qwID = kAToI(vcID + 2, 16);
    } else {
        qwID = kAToI(vcID, 10);
    }

    // 특정 ID만 종료하는 경우
    if (qwID != 0xFFFFFFFF) {
        kPrintf("Kill Task ID [0x%q] ", qwID);
        if (kEndTask(qwID) == TRUE) {
            kPrintf("Success\n");
        } else {
            kPrintf("Fail\n");
        }
    } else {
        // 콘솔 셀과 유휴 태스크를 제외하고 모든 태스크 종료
        for (i = 2; i < TASK_MAX_COUNT; i++) {
            pstTCB = kGetTCBInTCBPool(i);
            qwID = pstTCB->stLink.qwID;
            if ((qwID >> 32) != 0) {
                kPrintf("Kill Task ID [0x%q] ", qwID);
                if (kEndTask(qwID) == TRUE) {
                    kPrintf("Success\n");
                } else {
                    kPrintf("Fail\n");
                }
            }
        }
    }


}

// 프로세서의 사용률 표시
static void kCPULoad(const char *pcParameterBuffer) {
    kPrintf("Processor Load: %d%%\n", kGetProcessorLoad());
}

// 뮤텍스 테스트용 뮤텍스와 변수
static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;

// 뮤텍스를 테스트하는 태스크
static void kPrintNumberTast(void) {
    int i;
    int j;
    QWORD qwTickCount;

    // 50ms 정도 대기하여 콘솔 셸이 출력하는 메시지와 겹치지 않도록 함
    qwTickCount = kGetTickCount();
    while ((kGetTickCount() - qwTickCount) < 50) {
        kSchedule();
    }

    for (i = 0; i < 5; i++) {
        kLock(&(gs_stMutex));
        kPrintf("Task ID [0x%Q] Value[%d]\n",
                kGetRunningTask()->stLink.qwID, gs_qwAdder);

        gs_qwAdder += 1;
        kUnlock(&(gs_stMutex));

        // 프로세서 소모를 늘리려고 추가
        for (j = 0; j < 30000; j++);
    }

    // 모든 태스크가 종료할때 까지 1초(100ms) 정도 대기
    qwTickCount = kGetTickCount();
    while ((kGetTickCount() - qwTickCount) < 1000) {
        kSchedule();
    }
    // 태스크 종료
    kExitTask();
}

// 뮤텍스를 테스트하는 태스크 생성
static void kTestMutex(const char *pcParameterBuffer) {
    int i;
    gs_qwAdder = 1;

    kInitializeMutex(&gs_stMutex);

    for (i = 0; i < 3; i++) {
        // 뮤텍스를 테스트하는 3개 생성
        kCreateTask(TASK_FLAGS_LOW, (QWORD) kPrintNumberTast);
    }

    kPrintf("Wait Util %d Task End...\n", i);
    kGetCh();
}
