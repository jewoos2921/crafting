//
// Created by jewoo on 2022-12-23.
//

/* 목적 코드 생성 서브 루틴과 목적 코드 실행 루틴 */
#include <stdio.h>
#include "codegen.h"
#include "table.h"
#include "getSource.h"

#define MAXCODE 200 /* 목적 코드의 최대 길이 */
#define MAXMEM 2000 /* 실행할 때 스택의 최대 크기 */
#define MAXREG 20 /* 연산 레지스터 스택의 최대 크기 */
#define MAXLEVEL 5 /*블록 최대 길이 */

/* 명령어의 형태 */
typedef struct inst {
    OpCode opCode;
    union {
        RelAddr addr;
        int value;
        Operator optr;
    } u;
} Inst;

static Inst code[MAXCODE]; /* 목적 코드 집합 */
static int cIndex = -1; /* 최종적으로 생성한 명령어의 인덱스 */

static void checkMax(); /* 목적 코드의 인덱스 증가와 확인 */

static void printCode(int i); /* 명령어 출력 */

int nextCode() {   /* 다음 명령어의 주소 리턴 */
    return cIndex + 1;
}

int genCodeV(OpCode op, int v) {         /* 명령어 생성, 주소부에 v */
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.value = v;
    return cIndex;
}

int genCodeT(OpCode op, int ti) {        /* 명령어 생성, 주소는 이름 테이블에서 */
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.addr = relAddr(ti);
    return cIndex;
}

int genCodeO(Operator p) {               /* 명령어 생성, 주소부에 연산 명령 */
    checkMax();
    code[cIndex].opCode = opr;
    code[cIndex].u.optr = p;
    return cIndex;
}

int genCodeR() {                         /* ret 명령어 생성 */
    if (code[cIndex].opCode == ret) {
        return cIndex; /* 바로 앞이 ret이라면 생성하지 않음 */
    }
    checkMax();
    code[cIndex].opCode = ret;
    code[cIndex].u.addr.level = bLevel();
    code[cIndex].u.addr.addr = fPars(); /* 매개변수 수 (실행 스택 해제 목적) */
    return cIndex;
}

void checkMax() { /* 목적 코드의 인덱스 증가와 확인 */
    if (++cIndex < MAXCODE) {
        return;
    }
    errorF("too many code");
}

void backPatch(int i) {                  /* 명령어 패치 */
    code[i].u.value = cIndex + 1;
}

void listCode() {                        /* 목적 코드 출력 */
    int i;
    printf("\ncode\n");
    for (i = 0; i <= cIndex; i++) {
        printf("%3d: ", i);
        printCode(i);
    }
}


void printCode(int i) { /* 명령어 출력 */
    int flag;
    switch (code[i].opCode) {
        case lit:
            printf("lit");
            flag = 1;
            break;
        case opr:
            printf("opr");
            flag = 3;
            break;
        case lod:
            printf("lod");
            flag = 2;
            break;
        case sto:
            printf("sto");
            flag = 2;
            break;
        case cal:
            printf("cal");
            flag = 2;
            break;
        case ret:
            printf("ret");
            flag = 2;
            break;
        case ict:
            printf("ict");
            flag = 1;
            break;
        case jmp:
            printf("jmp");
            flag = 1;
            break;
        case jpc:
            printf("jpc");
            flag = 1;
            break;
    }

    switch (flag) {
        case 1:
            printf(",%d\n", code[i].u.value);
            return;
        case 2:
            printf(",%d", code[i].u.addr.level);
            printf(",%d\n", code[i].u.addr.addr);
            return;

        case 3:
            switch (code[i].u.optr) {

                case neg:
                    printf(",neg\n");
                    return;
                case add:
                    printf(",add\n");
                    return;
                case sub:
                    printf(",sub\n");
                    return;
                case mul:
                    printf(",mul\n");
                    return;
                case div:
                    printf(",div\n");
                    return;
                case odd:
                    printf(",odd\n");
                    return;
                case eq:
                    printf(",eq\n");
                    return;
                case ls:
                    printf(",ls\n");
                    return;
                case gr:
                    printf(",gr\n");
                    return;
                case neq:
                    printf(",neq\n");
                    return;
                case lseq:
                    printf(",lseq\n");
                    return;
                case greq:
                    printf(",greq\n");
                    return;
                case wrt:
                    printf(",wrt\n");
                    return;
                case wrl:
                    printf(",wrl\n");
                    return;
            }
        default:
            printf("error\n");
            return;
    }
}

void execute() {                         /* 목적 코드 실행 */
    int stack[MAXMEM]; /* 실행 스택 */
    int display[MAXLEVEL]; /* 현재 보이는 블록 맨 앞 주소의 디스플레이 */
    int pc, top, lev, temp;
    Inst i; /* 실행할 명령 */

    printf("start execution\n");
    top = 0; /* 스택 탑 */
    pc = 0; /* 명령어 카운터 */
    /* stack[top]은 호출한 쪽에서 일시적으로 사라지는 디스플레이의 퇴피 장소 */
    /* stack[top+1]은 호출되는 곳으로 리턴 주소 */
    stack[0] = 0;
    stack[1] = 0;

    display[0] = 0; /* 메인 블록의 맨 앞부분 주소는 0 */
    do {
        i = code[pc++]; /* 이제 실행할 명령어 */
        switch (i.opCode) {

            case lit:
                stack[top++] = i.u.value;
                break;
            case lod:
                stack[top++] = stack[display[i.u.addr.level] + i.u.addr.addr];
                break;
            case sto:
                stack[display[i.u.addr.level] + i.u.addr.addr] = stack[--top];
                break;
            case cal:
                lev = i.u.addr.level + 1;
                /* i.u.addr.level은 callee 이름 레벨 */
                /* callee 블록의 레벨 lev는 거기에 +1 한것 */
                stack[top] = display[lev]; /* display[lev]로 퇴피 */
                stack[top + 1] = pc;
                display[lev] = top;
                /* 현재 top 이 callee 블록 맨 앞의 주소 */
                pc = i.u.addr.addr;
                break;

            case ret:
                temp = stack[--top];  /* 스택 탑에 있는 것이 리턴 값 */
                top = display[i.u.addr.level]; /* top을 호출한 때의 값으로 복구 */
                display[i.u.addr.level] = stack[top]; /* 이전 디스플레이 복구 */
                pc = stack[top + 1];
                top -= i.u.addr.addr; /* 살인수만큼 탑을 제거 */
                stack[top++] = temp; /* 리턴값을 스택 탑에 */
                break;

            case ict:
                top += i.u.value;
                if (top >= MAXMEM + MAXREG) {
                    errorF("stack overflow");
                }
                break;


            case jmp:
                pc = i.u.value;
                break;

            case jpc:
                if (stack[--top] == 0) {
                    pc = i.u.value;
                }

                break;

            case opr:
                switch (i.u.optr) {
                    case neg:
                        stack[top - 1] = -stack[top - 1];
                        continue;
                    case add:
                        --top;
                        stack[top - 1] += stack[top];
                        continue;
                    case sub:
                        --top;
                        stack[top - 1] -= stack[top];
                        continue;
                    case mul:
                        --top;
                        stack[top - 1] *= stack[top];
                        continue;
                    case div:
                        --top;
                        stack[top - 1] /= stack[top];
                        continue;
                    case odd:
                        stack[top - 1] = stack[top - 1] & 1;
                        continue;
                    case eq:
                        --top;
                        stack[top - 1] = (stack[top - 1] == stack[top]);
                        continue;
                    case ls:
                        --top;
                        stack[top - 1] = (stack[top - 1] < stack[top]);
                        continue;;
                    case gr:
                        --top;
                        stack[top - 1] = (stack[top - 1] > stack[top]);
                        continue;;
                    case neq:
                        --top;
                        stack[top - 1] = (stack[top - 1] != stack[top]);
                        continue;;
                    case lseq:
                        --top;
                        stack[top - 1] = (stack[top - 1] <= stack[top]);
                        continue;;
                    case greq:
                        --top;
                        stack[top - 1] = (stack[top - 1] >= stack[top]);
                        continue;
                    case wrt:
                        printf("%d ", stack[--top]);
                        continue;
                    case wrl:
                        printf("\n");
                        continue;
                }
        }
    } while (pc != 0);
}

