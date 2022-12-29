//
// Created by jewoo on 2022-12-23.
//

/* 컴파일러(구문 분석과 코드 생성)의 메인 루틴 */
#include "getSource.h"
#include "table.h"
#include "codegen.h"

#define MINERROR 3 /* 오류가 이것보다 적으면 실행 */
#define FIRSTADDR 2 /* 각 블록의 첫 변수 주소 */

static Token token; /* 다음 토큰을 넣어 둘 변수 */

static void block(int pIndex); /* 블록 컴파일 */

static void constDecl(); /* 상수 선언 컴파일 */

static void varDecl(); /* 변수 선언 컴파일 */

static void funcDecl(); /* 함수 선언 컴파일 */

static void statement(); /* 문장 컴파일 */

static void expression(); /* 식 컴파일 */

static void term(); /* 식의 항 컴파일 */

static void factor(); /* 식의 인자 컴파일 */

static void condition(); /* 조건식 컴파일 */

static int isStBeginKey(Token t); /* 토큰 t는 문장 맨 앞의 키인가? */

int compile() {
    int i;
    printf("start compilation\n");
    initSource(); /* getSource 초기 설정 */
    token = nextToken(); /* 첫 토큰 */
    blockBegin(FIRSTADDR); /* 이후 선언은 새로운 블록 */
    block(0); /* 0은 더미 (메인 블록의 함수 이름은 없음) */
    finalSource();
    i = errorN();/* 오류 메시지의 개수 */
    if (i != 0) {
        printf("%d errors\n", i);
    }
    listCode(); /* 목적 코드 출력 (필요한 경우) */
    return (i < MINERROR); /* 오류 메시지의 개수가 적은지 확인 */
}

void block(int pIndex) { /* pIndex는 해당 블록 함수 이름의 인덱스 */
    int backP;
    backP = genCodeV(jmp, 0); /* 내부 함수를 점프하는 명령, 이후에 백 패치 */
    while (1) { /* 선언부 컴파일을 반복 */
        switch (token.kind) {
            case Const: /* 함수 선언부 컴파일 */
                token = nextToken();
                constDecl();
                continue;
            case Var:   /* 함수 선언부 컴파일 */
                token = nextToken();
                varDecl();
                continue;
            case Func: /* 함수 선언부 컴파일 */
                token = nextToken();
                funcDecl();
                continue;
            default: /* 이외에는 선언부 종료 */
                break;
        }
        break;
    }
    backPatch(backP); /* 내부 함수를 점프하는 명령으로 패치 */
    changeV(pIndex, nextCode()); /* 그 함수의 시작 주소를 수정 */
    genCodeV(ict, frameL()); /* 그 블록의 실행 때에 필요한 기억 영역을 잡는 명령 */

    statement(); /* 그 블록의 메인 문장 */
    genCodeR(); /* 리턴 명령 */
    blockEnd(); /* 블록이 끝났다는 것을 table에 전달 */
}

void constDecl() { /* 상수 선언 컴파일 */
    Token temp;
    while (1) {
        if (token.kind == Id) {
            setIdKind(constId);
            temp = token;
            token = checkGet(nextToken(), Equal);
            if (token.kind == Num) {
                enterTconst(temp.u.id, token.u.value);
            } else {
                errorType("number");
            }
            token = nextToken();
        } else {
            errorMissingId();
        }
        if (token.kind == Comma) {
            if (token.kind == Id) {
                errorInsert(Comma);
                continue;
            } else {
                break;
            }
        }
        token = nextToken();
    }
    token = checkGet(token, Semicolon); /* 마지막은 ';' */
}

void varDecl() { /* 변수 선언 컴파일 */
    while (1) {
        if (token.kind == Id) {
            setIdKind(varId); /* 출력을 위한 정보 설정 */
            enterTvar(token.u.id); /* 변수 이름을 테이블에, 주소는 테이블이 결정 */
            token = nextToken();
        } else {
            errorMissingId();
        }
        if (token.kind != Comma) {
            if (token.kind == Id) {
                errorInsert(Comma);
                continue;
            } else {
                break;
            }
        }
        token = nextToken();
    }
    token = checkGet(token, Semicolon); /* 마지막은 ';' */
}

void funcDecl() { /* 함수 선언 컴파일 */
    int fIndex;
    if (token.kind == Id) {
        setIdKind(funcId);
        fIndex = enterTfunc(token.u.id, nextCode());
        /* 함수 이름을 테이블에 등록 */
        /* 목적 주소는 일단 다음 코드의 nextCode()로 */
        token = checkGet(nextToken(), Lparen);
        blockBegin(FIRSTADDR); /* 매개변수 이름의 레벨을 함수 블록과 같음 */
        while (1) {
            if (token.kind == Id) {
                setIdKind(parId);
                enterTpar(token.u.id);
                token = nextToken();
            } else {
                break;
            }
            if (token.kind == Comma) {
                if (token.kind == Id) {

                    errorInsert(Comma);
                    continue;
                } else {
                    break;
                }
            }
            token = nextToken();
        }
        token = checkGet(token, Rparen); /* 마지막은 ')' */
        endpar(); /* 매개변수부가 끝났다는 것을 테이블에 전달 */
        if (token.kind == Semicolon) {
            errorDelete();
            token = nextToken();
        }
        block(fIndex); /* 블록 컴파일, 함수 이름의 인덱스를 전달 */
        token = checkGet(token, Semicolon); /* 마지막은 ';' */
    } else {
        errorMissingId(); /* 함수 이름이 아님 */
    }
}

void statement() { /* 문장 컴파일 */
    int tIndex;
    KindT k;
    int backP, backP2; /* 백패치 전용 */

    while (1) {
        switch (token.kind) {
            case Id:
                tIndex = searchT(token.u.id, varId);
                setIdKind(k = kindT(tIndex));
                if (k != varId && k != parId) {
                    errorType("var/par");
                }

                token = checkGet(nextToken(), Assign);
                expression();
                genCodeT(sto, tIndex); /* 좌변으로의 할당 명령 */
                return;

            case If:
                token = nextToken();
                condition();
                token = checkGet(token, Then);
                backP = genCodeV(jpc, 0);
                statement();
                backPatch(backP);
                return;

            case Ret:
                token = nextToken();
                expression();
                genCodeR();
                return;

            case Begin:
                token = nextToken();
                while (1) {
                    statement();
                    while (1) {
                        if (token.kind == Semicolon) {
                            /*  */
                            token = nextToken();
                            break;
                        }
                        if (token.kind == End) {
                            token = nextToken();
                            return;
                        }
                        if (isStBeginKey(token)) {

                            errorInsert(Semicolon);
                            break;
                        }
                        errorDelete();
                        token = nextToken();
                    }
                }

            case While:
                token = nextToken();
                backP2 = nextCode();
                condition();
                token = checkGet(token, Do);
                backP = genCodeV(jpc, 0);

                statement();
                genCodeV(jmp, backP2);

                backPatch(backP);
                return;

            case Write:
                token = nextToken();
                expression();
                genCodeO(wrt);
                return;

            case WriteLn:
                token = nextToken();
                genCodeO(wrl);
                return;

            case End:
            case Semicolon:
                return;

            default:
                errorDelete();
                token = nextToken();
                continue;
        }
    }
}

int isStBeginKey(Token t) {  /* 토큰 t는 문장 맨 앞의 키인가? */
    switch (t.kind) {
        case If:
        case Begin:
        case Ret:
        case While:
        case WriteLn:
        case Write:
            return 1;
        default:
            return 0;
    }
}

void expression() { /* 식 컴파일 */
    KeyId k;
    k = token.kind;

    if (k == Plus || k == Minus) {
        token = nextToken();
        term();
        if (k == Minus) {
            genCodeO(neg);
        }
    } else {
        term();
    }

    k = token.kind;
    while (k == Plus || k == Minus) {
        token = nextToken();
        term();
        if (k == Minus) {
            genCodeO(neg);
        } else {
            genCodeO(add);
        }
        k = token.kind;
    }
}

void term() { /* 식의 항 컴파일 */
    KeyId k;
    factor();
    k = token.kind;
    while (k == Mult || k == Div) {
        token = nextToken();
        factor();
        if (k == Mult) {
            genCodeO(mul);
        } else {
            genCodeO(div);
        }
        k = token.kind;
    }
}

void factor() { /* 식의 인자 컴파일 */
    int tIndex, i;
    KindT k;
    if (token.kind == Id) {
        tIndex = searchT(token.u.id, varId);
        setIdKind(k = kindT(tIndex));
        switch (k) {
            case varId:
            case parId:
                genCodeT(lod, tIndex);
                token = nextToken();
                break;

            case constId:
                genCodeV(lit, val(tIndex));
                token = nextToken();
                break;

            case funcId:
                token = nextToken();
                if (token.kind == Lparen) {
                    i = 0; /* i */
                    token = nextToken();
                    if (token.kind == Rparen) {
                        for (;;) {
                            expression();
                            i++;
                            if (token.kind == Comma) {
                                /*  */
                                token = nextToken();
                                continue;
                            }
                            token = checkGet(token, Rparen);
                            break;
                        }
                    } else {
                        token = nextToken();
                    }
                    if (pars(tIndex) != i) {
                        errorMessage("\\#par");
                        /* pars(tIndex)는 임시 매개변수의 개수 */
                    }
                } else {
                    errorInsert(Lparen);
                    errorInsert(Rparen);
                }
                genCodeT(cal, tIndex); /* call 명령 */
                break;
        }
    } else if (token.kind == Num) { /* 상수 */
        genCodeV(lit, token.u.value);
        token = nextToken();
    } else if (token.kind == Lparen) {
        token = nextToken();
        expression();
        token = checkGet(token, Rparen);
    }
    switch (token.kind) {
        case Id:
        case Num:
        case Lparen:
            errorMissingOp();
            factor();
        default:
            return;
    }
}

void condition() {                              /* 조건식 컴파일 */
    KeyId k;
    if (token.kind == Odd) {
        token = nextToken();
        expression();
        genCodeO(odd);
    } else {
        expression();
        k = token.kind;
        switch (k) {
            case Equal:
            case Lss:
            case Gtr:
            case NotEq:
            case LssEq:
            case GtrEq:
                break;
            default:
                errorType("rel-op");
                break;
        }

        token = nextToken();
        expression();
        switch (k) {
            case Equal:
                genCodeO(eq);
                break;
            case Lss:
                genCodeO(ls);
                break;
            case Gtr:
                genCodeO(gr);
                break;
            case NotEq:
                genCodeO(neq);
                break;
            case LssEq:
                genCodeO(lseq);
                break;
            case GtrEq:
                genCodeO(greq);
                break;
            default:
                errorType("rel-op");
                break;
        }
    }
}