//
// Created by jewoo on 2023-01-05.
//

#ifndef CRAFTING_CONSOLESHELL_H
#define CRAFTING_CONSOLESHELL_H

#include "Types.h"

#define CONSOLE_SHELL_MAX_COMMAND_BUFFER_COUNT  300
#define CONSOLE_SHELL_PROMPT_MESSAGE            "MINT64>"

//
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
void kHelp(const char *pcPrameterBuffer);

void kCls(const char *pcPrameterBuffer);

void kShowTotalRAMSize(const char *pcPrameterBuffer);

void kStrintToDecimalHexTest(const char *pcPrameterBuffer);

void kShutdown(const char *pcPrameterBuffer);

#endif //CRAFTING_CONSOLESHELL_H
