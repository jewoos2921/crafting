//
// Created by jewoo on 2022-12-31.
//
// 어셈블리어 유틸리티 함수 헤더 파일
//=========================================================================================================
///                                 리얼 모드의 주요 인터럽트 벡터 테이블
//============================================================================================================
/// 테이블 인덱스  |                      용도               |                          설명
//============================================================================================================
/// 0x00        |       CPU Exeption                      |         Divide by Zero
/// 0x01        |       CPU Exeption                      |         Single step for Debugging
/// 0x02        |       CPU Exeption                      |         Non-maskable interrupt
/// 0x03        |       CPU Exeption                      |         Breakpoint instruction
/// 0x04        |       CPU Exeption                      |         Overflow trap
/// 0x05        |       Bios Service                      |         Print Screen
/// 0x06        |       CPU Exeption                      |         Invalid opcode
/// 0x07        |       CPU Exeption                      |         Coprocessor not available
/// 0x08        |       IRQ0 Interrupt                    |         Timer
/// 0x09        |       IRQ1 Interrupt                    |         Keyboard
/// 0x0A        |       IRQ2 Interrupt                    |         Slave interrupt
/// 0x0B        |       IRQ3 Interrupt                    |         COM2 port
/// 0x0C        |       IRQ4 Interrupt                    |         COM1 port
/// 0x0D        |       IRQ5 Interrupt                    |         Printer Port2
/// 0x0E        |       IRQ6 Interrupt                    |         Floppy Disk
/// 0x0F        |       IRQ7 Interrupt                    |         Printer Port1
/// 0x10        |       Bios Service                      |         Video control service
/// 0x13        |       Bios Service                      |         Disk I/O Service
/// 0x70        |       IRQ8 Interrupt                    |         Real time clock
/// 0x74        |       IRQ12 Interrupt                   |         Mouse
/// 0x75        |       IRQ13 Interrupt                   |         Math coprocessor error
/// 0x76        |       IRQ14 Interrupt                   |         Hard Disk





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

void kHlt(void);

BOOL kTestAndSet(volatile BYTE *pbDestination, BYTE bCompare, BYTE bSource);

void kInitializeFPU(void);

void kSaveFPUContext(void *pVFPUContext);

void kLoadFPUContext(void *pVFPUContext);

void kSetTS(void);

void kClearTS(void);

WORD kInPortWord(WORD wPort);

void kOutPortWord(WORD wPort, WORD wData);

void kEnableGlobalLocalAPIC(void);

void kPause(void);

#endif //CRAFTING_ASSEMBLYUTILITY_H
