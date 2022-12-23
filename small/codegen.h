//
// Created by jewoo on 2022-12-23.
//

#pragma once

/* 명령어 코드 */
typedef enum codes {
    lit, opr, lod, sto, cal, ret, ict, jmp, jpc
} OpCode;

/* 연산 명령 코드 */
typedef enum ops {
    neg, add, sub, mul, div, odd, eq, ls, gr,
    neq, lseq, greq, wrt, wrl
} Operator;

int genCodeV(OpCode op, int v);         /* 명령어 생성, 주소부에 v */

int genCodeT(OpCode op, int ti);        /* 명령어 생성, 주소는 이름 테이블에서 */

int genCodeO(Operator p);               /* 명령어 생성, 주소부에 연산 명령 */

int genCodeR();                         /* ret 명령어 생성 */

void backPatch(int i);                  /* 명령어 패치 */

int nextCode();                         /* 다음 명령어의 주소 리턴 */

void listCode();                        /* 목적 코드 출력 */

void execute();                         /* 목적 코드 실행 */

