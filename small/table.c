//
// Created by jewoo on 2022-12-23.
//

/* 기호 테이블의 처리, 블록 레벨 관리 */


#include <string.h>
#include "table.h"
#include "getSource.h"

#define MAXTABLE 100                                    /* 이름 테이블의 최대 길이 */
#define MAXNAME 31                                      /* 이름의 최대 길이 */
#define MAXLEVEL 5                                      /* 블록의 최대 깊이 */

typedef struct tableE {         /* 이름 테이블 엔트리의 형태 */
    KindT kind;                 /* 이름의 종류 */
    char name[MAXNAME];         /* 이름 철자 */
    union {
        int value;              /* 상수의 경우: 값 */
        struct {
            RelAddr raddr;      /* 함수의 경우: 앞부분의 주소 */
            int pars;           /* 함수의 경우: 매개변수 수 */
        } f;
        RelAddr raddr;          /* 변수, 매개변수의 경우: 주소 */
    } u;
} TableE;

static TableE nameTable[MAXTABLE]; /* 이름 테이블 */
static int tIndex = 0; /* 이름 테이블의 인덱스 */
static int level = -1; /* 현재 블록 레벨 */
static int indexes[MAXLEVEL]; /* index[i]에는 블록 레벨 i의 마지막 인덱스 */
static int addr[MAXLEVEL];  /* addr[i]에는 블록 레벨 i의 마지막 변수의 주소 */
static int localAddr; /* 현재 블록 마지막 변수의 주소 */
static int tfIndex;

/* 이름 종류 츌력을 위한 함수 */
static char *kindName(KindT k) {
    switch (k) {
        case varId:
            return "var";
        case parId:
            return "par";
        case funcId:
            return "func";
        case constId:
            return "const";
    }
}

void blockBegin(int firstAddr) {                            /* 블록시작(첫 변수의 주소)으로 호출 */
    if (level == -1) {
        localAddr = firstAddr;
        tIndex = 0;
        level++;
        return;
    }
    if (level == MAXLEVEL - 1) {
        errorF("too many nested blocks");
    }
    indexes[level] = tIndex; /* 지금까지 블록 정보 저장 */
    addr[level] = localAddr;
    localAddr = firstAddr; /* 새로운 블록이 첫 변수의 위치 */
    level++; /* 새로운 블록 레벨 */
}

void blockEnd() {                                           /* 블록 종료 때 호출 */
    level--;
    tIndex = indexes[level];                                /* 바로 밖 블록의 정보 복구 */
    localAddr = addr[level];
}

int bLevel() {                                               /* 현재 블록 레벨 리턴 */
    return level;
}

int fPars() {                                                /* 현재 블록 함수의 매개변수 이름 등록 */
    return nameTable[indexes[level - 1]].u.f.pars;
}

static void enterT(char *id) { /* 이름 테이블에 이름 등록 */
    if (tIndex++ < MAXTABLE) {
        strcpy(nameTable[tIndex].name, id);
    } else {
        errorF("too many names");
    }
}

int enterTfunc(char *id, int v) {                            /* 이름 테이블에 함수 이름과 맨 앞 주소 등록 */
    enterT(id);
    nameTable[tIndex].kind = funcId;
    nameTable[tIndex].u.f.raddr.level = level;
    nameTable[tIndex].u.f.raddr.addr = v;               /* 함수 맨 앞부분의 주소 */
    nameTable[tIndex].u.f.pars = 0;                     /* 매개변수 수의 초깃값 */
    tfIndex = tIndex;
    return tIndex;
}

int enterTvar(char *id) {                                    /* 이름 테이블에 변수 이름 등록 */
    enterT(id);
    nameTable[tIndex].kind = parId;
    nameTable[tIndex].u.raddr.level = level;
    nameTable[tfIndex].u.f.pars++; /* 함수 매개변수 수 세기 */
    return tIndex;
}

int enterTpar(char *id) {                                    /* 이름 테이블에 매개변수 이름 등록 */
    enterT(id);
    nameTable[tIndex].kind = varId;
    nameTable[tIndex].u.raddr.level = level;
    nameTable[tIndex].u.raddr.addr = localAddr++;
    return tIndex;
}

int enterTconst(char *id, int v) {                           /* 이름 테이블에 상수 이름과 값 등록 */
    enterT(id);
    nameTable[tIndex].kind = constId;
    nameTable[tIndex].u.value = v;
    return tIndex;
}

void endpar() {                  /* 매개변수 선언부의 마지막에서 호출됨 */
    int i;
    int pars = nameTable[tfIndex].u.f.pars;
    if (pars == 0) { return; }
    for (i = 1; i <= pars; i++) {
        /* 각 매개변수의 주소 구하기 */
        nameTable[tfIndex].u.raddr.addr = i - 1 - pars;
    }
}

void changeV(int ti, int newVal) { /* 이름_테이블 [ti]의 값(함수 맨 앞부분의 주소)을 변경 */
    nameTable[ti].u.f.raddr.addr = newVal;
}

int searchT(char *id, KindT k) { /* 이름 id의 이름 테이블 위치 리턴, 없는 경우 오류 발생 */
    int i;
    i = tIndex;
    strcpy(nameTable[0].name, id); /* 센티널 생성 */
    while (strcmp(id, nameTable[i].name) != 0) {
        i--;
    }
    if (i) { /* 이름이 있을 때 */
        return i;
    } else { /* 이름이 없을 때 */
        errorType("undef");
        if (k == varId) {
            return enterTvar(id); /* 변수라면 일단 등록 */
        }
        return 0;
    }
}

KindT kindT(int i) {  /* 이름_테이블 [ti]의 종류 리턴 */
    return nameTable[i].kind;
}

RelAddr relAddr(int ti) { /* 이름_테이블 [ti]의 주소 리턴 */
    return nameTable[ti].u.raddr;
}

int val(int ti) { /* 이름_테이블 [ti]의 값 리턴 */
    return nameTable[ti].u.value;
}

int pars(int ti) { /* 이름_테이블 [ti]의 함수 매개변수 수 리턴 */
    return nameTable[ti].u.f.pars;
}

int frameL() { /* 블록 실행 때에 필요한 메모리 용량 확인 */
    return localAddr;
}