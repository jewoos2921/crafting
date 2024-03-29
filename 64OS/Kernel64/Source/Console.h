//
// Created by jewoo on 2023-01-05.
//

#ifndef CRAFTING_CONSOLE_H
#define CRAFTING_CONSOLE_H

#include "Types.h"

// 비디오 메모리의 속성 값 설정
#define CONSOLE_BACKGROUND_BLACK                    0x00
#define CONSOLE_BACKGROUND_BLUE                     0x10
#define CONSOLE_BACKGROUND_GREEN                    0x20
#define CONSOLE_BACKGROUND_CYAN                     0x30
#define CONSOLE_BACKGROUND_RED                      0x40
#define CONSOLE_BACKGROUND_MAGENTA                  0x50
#define CONSOLE_BACKGROUND_BROWN                    0x60
#define CONSOLE_BACKGROUND_WHITE                    0x70
#define CONSOLE_BACKGROUND_BLINK                    0x80
#define CONSOLE_BACKGROUND_DARKBLACK                0x00
#define CONSOLE_BACKGROUND_DARKBLUE                 0x01
#define CONSOLE_BACKGROUND_DARKGREEN                0x02
#define CONSOLE_BACKGROUND_DARKCYAN                 0x03
#define CONSOLE_BACKGROUND_DARKRED                  0x04
#define CONSOLE_BACKGROUND_DARKMAGENTA              0x05
#define CONSOLE_BACKGROUND_DARKBROWN                0x06
#define CONSOLE_BACKGROUND_DARKWHITE                0x07
#define CONSOLE_BACKGROUND_BRIGHTBLACK              0x08
#define CONSOLE_BACKGROUND_BRIGHTBLUE               0x09
#define CONSOLE_BACKGROUND_BRIGHTGREEN              0x0A
#define CONSOLE_BACKGROUND_BRIGHTCYAN               0x0B
#define CONSOLE_BACKGROUND_BRIGHTRED                0x0C
#define CONSOLE_BACKGROUND_BRIGHTMAGENTA            0x0D
#define CONSOLE_BACKGROUND_BRIGHTBROWN              0x0E
#define CONSOLE_BACKGROUND_BRIGHTWHITE              0x0F

// 기본 문자 색상
#define CONSOLE_DEFAULT_TEXT_COLOR (CONSOLE_BACKGROUND_BLACK | CONSOLE_BACKGROUND_BRIGHTGREEN)

// 콘솔의 너비와 높이, 그리고 비디오 메모리의 시작 어드레스 설정
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT  25
#define CONSOLE_VIDEO_MEMORY_ADDRESS  0xB8000


// 비디오 컨트롤러의 I/O 포트 어드레스와 레지스터
#define VGA_PORT_INDEX                  0x3D4
#define VGA_PORT_DATA                   0x3D5
#define VGA_INDEX_UPPERCURSOR           0x0E
#define VGA_INDEX_LOWERCURSOR           0x0F


// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

// 콘솔에 대한 정보를 저장하는 자료구조
typedef struct kConsoleMangerStruct {
    // 문자와 커서를 출력하는 위치
    int iCurrentPrintOffset;
} CONSOLE_MANNAGER;

#pragma pack(pop)

void kInitializeConsole(int iX, int iY);

void kSetCursor(int iX, int iY);

void kGetCursor(int *piX, int *piY);

void kPrintf(const char *pcFormatString, ...);

int kConsolePrintString(const char *pcBuffer);

void kClearScrren(void);

BYTE kGetCh(void);

void kPrintStringXY(int iX, int iY, const char *pcString);

#endif //CRAFTING_CONSOLE_H
