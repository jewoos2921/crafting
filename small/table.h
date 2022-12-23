//
// Created by jewoo on 2022-12-23.
//

#pragma once

/* 식별자의 종류 */
typedef enum kindT {
    varId, funcId, parId, constId
} KindT;

/* 변수, 매개변수, 함수 주소의 형태 */
typedef struct relAddr {
    int level;
    int addr;
} RelAddr;

void blockBegin(int firstAddr);                             /* 블록 시작 때 호출 (맨 앞부분의) */

void blockEnd();                                            /* 블록 종료 때 호출 */

int bLevel();                                               /* 현재 블록 레벨 리턴 */

int fPars();                                                /* 현재 블록 함수의 매개변수 이름 등록 */

int enterTfunc(char *id, int v);                            /* 이름 테이블에 함수 이름과 맨 앞 주소 등록 */

int enterTvar(char *id);                                    /* 이름 테이블에 변수 이름 등록 */

int enterTpar(char *id);                                    /* 이름 테이블에 매개변수 이름 등록 */

int enterTconst(char *id, int v);                           /* 이름 테이블에 상수 이름과 값 등록 */

void endpar();                                              /* 매개변수 선언부의 마지막에서 호출됨 */

void changeV(int ti, int newVal);                           /* 이름_테이블 [ti]의 값(함수 맨 앞부분의 주소)을 변경 */

int searchT(char *id, KindT k);                             /* 이름 id의 이름 테이블 위치 리턴, 없는 경우 오류 발생 */

KindT kindT(int i);                                         /* 이름_테이블 [ti]의 종류 리턴 */

RelAddr relAddr(int ti);                                    /* 이름_테이블 [ti]의 주소 리턴 */

int val(int ti);                                            /* 이름_테이블 [ti]의 값 리턴 */

int pars(int ti);                                           /* 이름_테이블 [ti]의 함수 매개변수 수 리턴 */

int frameL();                                               /* 블록 실행 때에 필요한 메모리 용량 확인 */
