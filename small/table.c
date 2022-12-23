//
// Created by jewoo on 2022-12-23.
//


#include "table.h"
#include "getSource.h"

#define MAXTABLE 100 /* 이름 테이블의 최대 길이 */
#define MAXNAME 31 /* 이름의 최대 길이 */
#define MAXLEVEL 5 /* 블록의 최대 깊이 */

typedef struct tableE {
    KindT kind;
    char name[MAXNAME];
    union {
        int value;
        struct {
            RelAddr raddr;
            int pars;
        } f;
        RelAddr raddr;
    } u;
} TableE;

static TableE nameTable[MAXTABLE];
static int tIndex = 0;
static int level = -1;
static int index[MAXLEVEL];
static int addr[MAXLEVEL];
static int localAddr;
static int tfIndex;

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

void blockBegin(int firstAddr) {

}


void endpar(); /* 매개변수 선언부의 마지막에서 호출됨 */

void changeV(int ti, int newVal); /* 이름_테이블 [ti]의 값(함수 맨 앞부분의 주소)을 변경 */

int searchT(char *id, KindT k) { /* 이름 id의 이름 테이블 위치 리턴, 없는 경우 오류 발생 */
    int i;
    i = tIndex;
    strcpy()
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