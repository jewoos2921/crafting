//
// Created by jewoo on 2023-01-02.
//

#ifndef CRAFTING_ISR_H
#define CRAFTING_ISR_H

// 함수
// 예외 (Exception) 처리용 ISR.asm
void kISRDivideError(void);

void kISRDebug(void);

void kISRNMI(void);

void kISRBreakPoint(void);

void kISROverflow(void);

void kISRBoundRangeExceeded(void);

void kISRDeviceNotAvailable(void);

void kISRDoubleFault(void);

void kISRCoprocessorSegmentOverrun(void);

void kISRInvalidTSS(void);

void kISRSegmentNotPresent(void);

void kISRStackSegmentFault(void);

void kISRGeneralProtection(void);

void kISRPageFault(void);

void kISR15(void);

void kISRFPUError(void);

void kISRAlignmentCheck(void);

void kISRMachineCheck(void);

void kISRSIMDError(void);

void kISRETCException(void);

// 인터럽트 처리용 ISR
void kISRTimer(void);

void kISRKeyboard(void);

void kISRSlavePIC(void);

void kISRSerial2(void);

void kISRSerial1(void);

void kISRParallel2(void);

void kISRFloopy(void);

void kISRParallel1(void);

void kISRRTC(void);

void kISRReserved(void);

void kISRNotUsed1(void);

void kISRNotUsed2(void);

void kISRMouse(void);

void kISRCoprocessor(void);

void kISRHDD1(void);

void kISRHDD2(void);

void kISRETCInterrupt(void);

#endif //CRAFTING_ISR_H
