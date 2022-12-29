//
// Created by jewoo on 2022-12-23.
//

#pragma once

#include <stdio.h>
#include "table.h"

#define MAXNAME 31                      /* 이름의 최대 길이 */

/* 키와 문자이 종류 */
typedef enum keys {
    Begin, End,                         /* 예약어 이름 */
    If, Then,
    While, Do,
    Ret, Func,
    Var, Const, Odd,
    Write, WriteLn,
    end_of_KeyWd,                       /* 예약어 이름은 여기까지 */
    Plus, Minus,                        /* 연산자와 구분 기호 */
    Mult, Div,
    Lparen, Rparen,
    Equal, Lss, Gtr,
    NotEq, LssEq, GtrEq,
    Comma, Period, Semicolon,
    Assign,
    end_of_KeySym,                      /* 연산자와 구분 기호는 여기까지 */
    Id, Num, nul,                       /* 토큰의 종류 */
    end_of_Token,
    letter, digit, colon, others        /* 여기 까지가 문자의 종류*/
} KeyId;

/* 토큰의 형태 */
typedef struct token {
    KeyId kind;                         /* 토큰의 종류 또는 키의 이름*/
    union {
        char id[MAXNAME];               /* 식별자라면 이름 */
        int value;                      /* 숫자라면 그 값 */
    } u;
} Token;

Token nextToken();                      /* 다음 토큰을 읽어 들이고 리턴 */

Token checkGet(Token t, KeyId k);       /* t.kind == k확인
                                         * t.kind == k라면 다음 토큰을 읽어 들이고 리턴
                                         * t.kind != k라면 오류 메시지를 출력, t와 k가 같은 기호 또는 예약어라면 t를 버리고
                                         * 다음 토큰을 읽어 들이고 리턴 (t를 k로 변경하게 됨)
                                         * 이 이외의 경우, k를 삽입한 상태에서 t를 리턴
                                         * */

int openSource(char fileName[]);        /* 소스 파일 열기 */

void closeSource();                     /* 소스 파일 닫기 */

void initSource();                      /* 테이블 초기 설정, tex 파일 초기 설정 */

void finalSource();                     /* 소스 마지막 확인, tex 파일 초기 설정 */

void errorType(char *m);                /* 자료형 오류를 .tex 파일에 삽입 */

void errorInsert(KeyId k);              /* ketString(k)를 .tex 파일에 삽입 */

void errorMissingId();                  /* 이름이 없다는 메시지를 .tex 파일에 삽입 */

void errorMissingOp();                  /* 연산자가 없다는 메시지를 .tex 파일에 삽입 */

void errorDelete();                     /* 지금 읽어 들인 토큰 버리기 (.tex 파일로 출력) */

void errorMessage(char *m);             /* 오류 메시지를 .tex 파일로 출력 */

void errorF(char *m);                   /* 오류 메시지를 출력하고 컴파일러 종료 */

int errorN();                           /* 오류 개수 리턴 */

void setIdKind(KindT k);                /* 현재 토큰의 종류 설정(.tex 파일 출력 전용) */

int compile();