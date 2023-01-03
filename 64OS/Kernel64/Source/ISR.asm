; IDT 테이블에 등록할 어셈블리어 핸들러 코드가 정의
[BITS 64]

SECTION .text

; 외부에서 정의된 함수를 쓸 수 있도록
extern kCommonExceptionHanlder, kCommonInterruptHandler, kKeyboardHandler
; C 언어에서 호출할 수 있도록 이름을 노출함
; 예외 처리를 위한 ISR
global kISRDivideError, kISRDebug, kISRNMI, kISRBreakPoint, kISROverflow
global kISRBoundRangeExceeded, kISRInvalidOpCode, kISRDeviceNotAvailable, kISRDoubleFault, kISRCoprocessorSegmentOverrun
global kISRInvalidTSS, kISRSegmentNotPresent, kISRStackSegmentFault, kISRGeneralProtection
global kISRPageFault, kISR15, kISRFPUError, kISRAlignmentCheck, kISRMachineCheck
global kISRSIMDError, kISRETCException

; 인터럽트 처리용 ISR
global kISRTimer, kISRKeyboard, kISRSlavePIC, kISRSerial2, kISRSerial1
global kISRParallel2, kISRFloopy, kISRParallel1, kISRRTC,  kISRReserved
global kISRNotUsed1, kISRNotUsed2, kISRMouse, kISRCoprocessor
global kISRHDD1, kISRHDD2, kISRETCInterrupt

; 컨텍스트를 저장하고 셀렉터를 교체하는 매크로
%macro KSAVECONTEXT 0           ; 파라미터를 전달받지 않은 KSAVECONTEXT 매크로 정의
                                ; RBP 레지스터부터 GS 세그먼트 셀렉터 까지 모두 스택에 삽입
        push rbp
        mov rbp, rsp
        push rax
        push rbx
        push rcx
        push rdx
        push rdi
        push rsi
        push r8
        push r9
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15

        mov ax, ds          ; AX 레지스터에 커널 데이터 세그먼트 디스크립터 저장
        push rax            ; DS
        mov ax, es
        push rax
        push fs
        push gs

        ; 세그먼트 셀렉터 교체
        mov ax, 0x10    ; AX 레지스터에 커널 데이터 세그먼트 디스크립터 저장
        mov ds, ax      ; DS 세그먼트 셀렉터부터 FS 세그먼트 셀렉터까지 모두 커널 데이터 세그먼트로 교체
        mov es, ax
        mov gs, ax
        mov fs, ax
%endmacro

; 컨텍스트를 복원하는 매크로
%macro KLOADCONTEXT 0   ; 파라미터를 전달받지 않는 KLOADCONTEXT 매크로 정의
                        ; GS 세그먼트 부터 RBP 레지스터까지 모두 스택에서 꺼내 복원

       pop gs
       pop fs
       pop rax
       mov es, ax       ; ES 세그먼트 셀렉터부터 DS 세그먼트 셀렉터는 스택에 직접 꺼내 복원할 수 없으므로 RAX 레지스터에 저장한 뒤에 복원
       pop rax
       mov ds, ax


       pop r15
       pop r14
       pop r13
       pop r12
       pop r11
       pop r10
       pop r9
       pop r8
       pop rsi
       pop rdi
       pop rdx
       pop rcx
       pop rbx
       pop rax
       pop rbp
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 예외 핸들러
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; #0, Divide Error ISR
kISRDivideError:
    KSAVECONTEXT ; 콘택스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체

    ; 핸들러에 예외 번호를 호출하고 핸들러 호출
    mov rdi, 0
    call kCommonExceptionHanlder

    KLOADCONTEXT ; 콘텍스트를 복원
    iretq         ; 인터럽트를 처리하고 이전에 수행하던 코드로 복원


; #1 Debug ISR
kISRDebug:
    KSAVECONTEXT        ; 콘택스트를 저장한 뒤 셀렉터를 커널 데이터 디스크립터로 교체

    ; 핸들러에 예외 번호를 호출하고 핸들러 호출
    mov rdi, 1
    call kCommonExceptionHanlder

    KLOADCONTEXT ; 콘텍스트를 복원
    iretq        ; 인터럽트를 처리하고 이전에 수행하던 코드로 복원


