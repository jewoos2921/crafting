//
// Created by jewoo on 2022-12-31.
//
// 어셈블리어 유틸리티 함수 헤더 파일

#ifndef CRAFTING_ASSEMBLYUTILITY_H
#define CRAFTING_ASSEMBLYUTILITY_H

#include "Types.h"
#include "Task.h"

// 함수
BYTE kInPortByte(WORD wPort);

void kOutPortByte(WORD wPort, BYTE bData);

void kLoadGDTR(QWORD qwGDTRAddress);

void kLoadTR(WORD wTSSSegmentOffset);

void kLoadIDTR(QWORD qwIDTRAddress);

void kEnableInterrupt(void);

void kDisableInterrupt(void);

QWORD kReadRFLAGS(void);

QWORD kReadTSC(void);

void kSwitchContext(CONTEXT *pstCurrentContext, CONTEXT *pstNextContext);

#endif //CRAFTING_ASSEMBLYUTILITY_H
