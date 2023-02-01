//
// Created by jewoo on 2023-01-05.
//

#ifndef CRAFTING_CONSOLESHELL_H
#define CRAFTING_CONSOLESHELL_H

#include "Types.h"

#define CONSOLE_SHELL_MAX_COMMAND_BUFFER_COUNT  300
#define CONSOLE_SHELL_PROMPT_MESSAGE            "MINT64>"

// 문자열 포인터를 파라미터로 받는 함수 포인터
typedef void (*CommandFunction)(const char *pcParameter);

// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

// 셸의 커맨드를 저장하는 자료구조
typedef struct kShellCommandEntryStruct {
    // 커맨드 문자열
    char *pcCommand;
    // 커맨드의 도움말
    char *pcHelp;
    // 커맨드를 수행하는 함수의 포인터
    CommandFunction pfFunction;
} SHELL_COMMAND_ENTRY;

// 파라미터를 처리하기 위해 정보를 저장하는 자료 구조
typedef struct kParameterListStruct {
    // 파라미터 버퍼의 어드레스
    const char *pcBuffer;
    // 파라미터의 길이
    int iLength;
    // 현재 처리할 파라미터가 시작하는 위치
    int iCurrentPosition;
} PARAMETER_LIST;

#pragma pack(pop)

// 실제 셸 코드
void kStartConsoleShell(void);

void kExecuteCommand(const char *pcCommandBuffer);

void kInitializeParameter(PARAMETER_LIST *pstList, const char *pcParameter);

int kGetNextParameter(PARAMETER_LIST *pstList, char *pcParameter);

// 커맨드를 처리하는 함수
static void kHelp(const char *pcPrameterBuffer);

static void kCls(const char *pcPrameterBuffer);

static void kShowTotalRAMSize(const char *pcPrameterBuffer);

static void kStringToDecimalHexTest(const char *pcPrameterBuffer);

static void kShutdown(const char *pcPrameterBuffer);

static void kSetTimer(const char *pcParameterBuffer);

static void kWaitUsingPIT(const char *pcParameterBuffer);

static void kReadTimeStampCounter(const char *pcParameterBuffer);

static void kMeasureProcessorSpeed(const char *pcParameterBuffer);

static void kShowDateAndTime(const char *pcParameterBuffer);

static void kCreateTestTask(const char *pcParameterBuffer);

static void kChangeTaskPriority(const char *pcParameterBuffer);

static void kShowTaskList(const char *pcParameterBuffer);

static void kKillTask(const char *pcParameterBuffer);

static void kCPULoad(const char *pcParameterBuffer);

static void kTestMutex(const char *pcParameterBuffer);

static void kCreateThreadTask(const char *pcParameterBuffer);

static void kTestThread(const char *pcParameterBuffer);

static void kShowMatrix(const char *pcParameterBuffer);

static void kTestPIE(const char *pcParameterBuffer);

static void kShowDynamicMemoryInformation(const char *pcParameterBuffer);

static void kTestSequentialAllocation(const char *pcParameterBuffer);

static void kTestRandomAllocation(const char *pcParameterBuffer);

static void kRandomAllocationTask(const char *pcParameterBuffer);


static void kShowHDDInformation(const char *pcParameterBuffer);

static void kReadSector(const char *pcParameterBuffer);

static void kWriteSector(const char *pcParameterBuffer);

static void kMountHDD(const char *pcParameterBuffer);

static void kFormatHDD(const char *pcParameterBuffer);

static void kShowFileSystemInformation(const char *pcParameterBuffer);

static void kCreateFileInRootDirectory(const char *pcParameterBuffer);

static void kDeleteFileInRootDirectory(const char *pcParameterBuffer);

static void kShowRootDirectory(const char *pcParameterBuffer);

static void kWriteDataToFile(const char *pcParameterBuffer);

static void kReadDataToFile(const char *pcParameterBuffer);

static void kTestFileIO(const char *pcParameterBuffer);

static void kFlushCache(const char *pcParameterBuffer);

static void kTestPerformance(const char *pcParameterBuffer);

static void kDownloadFile(const char *pcParameterBuffer);

static void kShowMPConfigurationTable(const char *pcParameterBuffer);

#endif //CRAFTING_CONSOLESHELL_H
