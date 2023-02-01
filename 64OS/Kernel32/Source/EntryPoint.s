; 보호 모드 커널의 엔트리 포인트
; 엔트리 포인트는 외부에서 해당 모듈울 실행 할때 실행을 시작하는 지점
; 보호 모드 커널의 가장 앞부분에 위치하는 코드로 보호 모드 전환과 초기화를 수행하여 이후에 위치하는 코드를 위한 환경을 제공
[ORG 0x00]  ; 코드 시작 주소를 0x00으로 설정
[BITS 16]   ; 이하의 코드는 16 비트로 설정

SECTION .text ; text 섹션 정의

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x1000  ; 보호 모드 엔트리 포인트의 시작 어드레스(0x10000)를
                    ; 세그먼트 레지스터 값으로 변환


    mov ds, ax ; DS 세그먼트 레지스터에 설정
    mov es, ax ; ES 세그먼트 레지스터에 설정


    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;  Application Processor이면 아래의 과정을 모두 뛰어넘어서 보호 모드 커널로 이동
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ax, 0x0000                      ; Application Processor 플래그를 확인하려고
    mov es, ax                          ; ES 세그먼트 레지스터의 시작 어드레스를 0으로 설정

    cmp byte [es: 0x7C09],, 0x00        ; 플래그가 0이면  Application Processor이므로 
    je .APPLICATIONPROCESSORSTARTPOINT  ; Application Processor용 코드로 이동

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;  Bootstrap Processor만 실행
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; A20 게이트를 활성화
    ; BIOS를 이용한 전환이 실패했을 때 시스템 컨트롤 포트로 전환 시도
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS 서비스를 사용해서 A20 게이트를 활성화
    mov ax, 0x2401      ; A20 게이트 활성화 서비스 설정

    int 0x15 ; BIOS 인터럽트 서비스 호출

    jc .A20GATEERROR    ; A20 게이트 활성화가 성공했는지 확인
    jmp .A20GATESUCCESS

.A20GATEERROR:
    ; 에러 발생시, 시스템 컨트롤 포트로 전환
    in al, 0x92     ; 시스템 컨트롤 포트(0x92)에서 1바이트를 읽어 AL 레지스터에 저장
    or al, 0x02     ; 읽은 값에 A20 게이트 비트(비트 1)를 1로 설정
    and al, 0xFE    ; 시스템 리셋 방지를 위해 0xFE와 AND 연산하여 비트 0을 0으로 설정
    out 0x92, al    ; 시스텐 컨트롤 포트(0x92)에 변경된 값을 1바이트 설정


    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Bootstrap Processor와 Application Processor가 공통으로 수행하는 부분
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.A20GATESUCCESS:
.APPLICATIONPROCESSORSTARTPOINT:
    cli             ; 인터럽트가 발생하지 못하도록 설정
    lgdt [GDTR]     ; GDTR 자료구조를 프로세스에서 설정하여 GDT 테이블을 로드

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 보호 모드로 진입
    ; Disable Pagong, Disable Cache, Internal FPU, Disable Align Check.
    ; Enable ProtectedMode
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, 0x4000003B ; PG=0, CD=1, NW=0, AM=0, WP=0, NE=1, TS=1, EM=0, MP=1, PE=1
    mov cr0, eax        ; CR0 컨트롤 레지스터에 위에서 저장한 플래그를 설정하여 보호모드로 전환

    ; 커널 코드 세그먼트를 0x00을 기준으로 하는 것으로 교체하여 EIP의 값을 0x00을 기준으로 재설정
    ; CS 세그먼트 셀렉터: EIP
    jmp dword 0x08: (PROTECTEDMODE - $$ + 0x10000 )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; 보호 모드로 진입
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]           ; 이하의 코드는 32비트 코드로 설정
PROTECTEDMODE:
    mov ax, 0x20    ;  보호 모드 커널용 데이터 세그먼트 디스크립터를 AX 레지스터에 저장
    mov ds, ax      ; DS 세그먼트 셀렉터에 설정
    mov es, ax      ; ES 세그먼트 셀렉터에 설정
    mov fs, ax      ; FS 세그먼트 셀렉터에 설정
    mov gs, ax      ; GS 세그먼트 셀렉터에 설정

    ; 스택을 0x00000000~0x0000FFFF
    mov ss, ax
    mov esp, 0xFFFE     ; ESP 레지스터의 어드레스를 OxFFFE로 설정
    mov ebp, 0xFFFE     ; EBP 레지스터의 어드레스를 OxFFFE로 설정

    ; Application Processor이면 아래의 과정을 모두 뛰어넘어서 C 언어 커널 엔트리 포인터로 이동
    cmp byte [0x7C09], 0x00
    je .APPLICATIONPROCESSORSTARTPOINT

    ; 화면에 보호 모드로 전환되었다는 메시지를 찍는다.
    push (SWITCHSUCCESSMESSAGE - $$ + 0x10000)
    push 2
    push 0
    call PRINTMESSAGE
    add esp, 12

.APPLICATIONPROCESSORSTARTPOINT:
    jmp dword 0x18: 0x10200 ; C언어 커널이 존재하는 0x10200 어드레스를 이용해서 C언어 커널 수행

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 함수 코드 영역
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 데이터 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 아래 데이터를 8바이트에 맞춰 정렬하기 위해 추가
align 8, db 0

; GDTR 의 끝을 8바이트로 정렬하기 위해 추가
dw 0x0000
; GDTR 자료구조 정의
GDTR:
    dw GDTEND - GDT -1          ; 아래에 위치하는 GDT 테이블의 전체 크기
    dd (GDT - $$ + 0x10000)     ; 아래에 위치하는 GDT 테이블의 시작 어드레스

; GDT 테이블 정의
GDT:
    ; 널(NULL) 디스크립터, 반드시 0으로 초기화해야 함
    NULLDescriptor:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x00
    db 0x00
    db 0x00

; IA-32e 모드 커널용 코드 세그먼트 디스크립터
IA_32eCODEDESCRIPTOR:
    dw 0xFFFF   ; Limit [15:0]
    dw 0x0000   ; Base [15:0]
    db 0x00     ; Base [23:16]
    db 0x9A     ; P=1, DPL=0, Code Segment, Execute/Read
    db 0xAF     ; G=1, D=0, L=1, Limit[19:16]
    db 0x00     ; Base [31:24]

; IA-32e 모드 커널용 데이터 세그먼트 디스크립터
IA_32eDATADESCRIPTOR:
     dw 0xFFFF   ; Limit [15:0]
     dw 0x0000   ; Base [15:0]
     db 0x00     ; Base [23:16]
     db 0x92     ; P=1, DPL=0, Data Segment, Read/Write
     db 0xAF     ; G=1, D=0, L=1, Limit[19:16]
     db 0x00     ; Base [31:24]

; 보호 모드 커널용 코드 세그먼트 디스크립터
CODEDESCRIPTOR:
     dw 0xFFFF   ; Limit [15:0]
     dw 0x0000   ; Base [15:0]
     db 0x00     ; Base [23:16]
     db 0x92     ; P=1, DPL=0, Code Segment, Execute/Write
     db 0xCF     ; G=1, D=1, L=0, Limit[19:16]
     db 0x00     ; Base [31:24]

; 보호 모드 커널용 데이터 세그먼트 디스크립터
CODEDESCRIPTOR:
     dw 0xFFFF   ; Limit [15:0]
     dw 0x0000   ; Base [15:0]
     db 0x00     ; Base [23:16]
     db 0x92     ; P=1, DPL=0, Data Segment, Read/Write
     db 0xCF     ; G=1, D=1, L=0, Limit[19:16]
     db 0x00     ; Base [31:24]

GDTEND:

; 보호 모드로 전환되었다는 메시지
SWITCHSYCCESSMESSAGE: db `Switch to Protected Mode Success~!!`, 0

times 512 - ($ - $$) db 0x00 ; 512 바이트를 맞추기 위해 남은 부분을 0으로 채움

[BITS 64] ; 이하의 코드는 64 비트 코드로 설정

SECTION .text ; text 섹션을 정의

; 외부에서 정의된 함수를 쓸 수 있도록 선언함 (Import)
extern Main

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x10    ; IA-32e 모드 커널용 데이터 세그먼트 디스크립터를 AX 레지스터에 저장
    mov ds, ax      ; DS 세그먼트 셀렉터에 설정
    mov es, ax      ; ES 세그먼트 셀렉터에 설정
    mov fs, ax      ; FS 세그먼트 셀렉터에 설정
    mov gs, ax      ; GS 세그먼트 셀렉터에 설정

    ; 스택을 0x600000~0x6FFFFF 영역에 1MB 크기로 생성
    mov ss, ax      ; SS 세그먼트 셀렉터에 설정

    mov rsp, 0x6FFFF8 ; RSP 레지스터의 어드레스를 0x6FFFF8로 설정
    mov rbp, 0x6FFFF8 ; RBP 레지스터의 어드레스를 0x6FFFF8로 설정

    call Main       ; C 언어 엔트리 포인트 함수 호출

    jmp $

