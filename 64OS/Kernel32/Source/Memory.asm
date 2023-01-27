; 1024 섹터 크기의 이미지를 메모리로 복사
TOTALSECTORCOUNT: dw 1024   ; 부트로더를 제외한 MINT64 OS 이미지 크기
                            ; 최대 1152 섹터(0x90000byte)까지 가능
SECTORNUMBER:       db 0x02     ; OS 이미지가 시작하는 섹터 번호를 저장하는 영역
HEADNUMBER:     db 0x00         ; OS 이미지가 시작하는 헤드 번호를 저장하는 영역
TRACKNUMBER:        db 0x00     ; OS 이미지가 시작하는 트랙 번호를 저장하는 영역

       ;
       mov si, 0x1000

       mov es, si
       mov bx, 0x0000




       mov di, word[TOTALSECTORCOUNT];

READDATA:
    ;
    cmp di, 0

    je READEND
    sub di, 0x1

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS Read Instruction 호출
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov ah, 0x02
    mov al, 0x1
    mov ch,

