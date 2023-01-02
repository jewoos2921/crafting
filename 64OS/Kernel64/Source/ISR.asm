; IDT 테이블에 등록할 어셈블리어 핸들러 코드가 정의
[BITS 64]

SECTION .text

; 외부에서 정의된 함수를 쓸 수 있도록
extern kCommonExceptionHanlder, kCommonInterruptHandler, kKeyboardHandler
; C 언어에서 호출할 수 있도록 이름을 노출함
; 예외 처리를 위한 ISR
global