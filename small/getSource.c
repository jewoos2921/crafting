//
// Created by jewoo on 2022-12-23.
//

/* 입출력과 관련된 부분, 즉 원시 프로그램을 읽어 들이고 낱말을 분석하고 컴파일 결과를 출력하고 오류 메시지를 출력하는 부분등 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "getSource.h"

#define MAXLINE 120     /* 한 줄의 최대 문자 수 */
#define MAXERROR 30     /* 이 이상의 오류가 있다면 종료 */
#define MAXNUM 14       /* 상수의 최대 자리 수 */
#define TAB 5           /* 탭의 공백 수 */
#define INSERT_C "#0000FF"      /* 삽입 문자의 색 */
#define DELETE_C "#FF0000"      /* 제거 문자의 색 */
#define TYPE_C "00FF00"         /* 타입 오류의 문자 색 */

static FILE *fpi;           /* 소스 파일 */
static FILE *fptex;         /* 레이텍 출력 파일 */
static char line[MAXLINE];  /* 한 줄만큼의 입력 버퍼 */
static int lineIndex;       /* 다음 읽어 들일 문자의 위치 */
static char ch;             /* 마지막으로 읽어 들인 문자 */

static Token cToken;        /* 마지막으로 읽어 들인 토큰 */
static KindT idKind;        /* 현재 토큰(Id)의 종류 */
static int spaces;          /* 그 토큰 앞의 공백 개수 */
static int CR;              /* 그 앞에 있는 CR의 개수 */
static int printed;         /* 토큰을 인쇄했는가? */

static int errorNo = 0; /* 출력한 오류의 수 */

static char nextChar(); /* 다음 문자를 읽어 들이는 함수 */

static int isKeySym(KeyId k); /* 키 k가 예약어 인가 ? */

static int isKeyWd(KeyId k); /* 키 k가 기호 인가 ? */

static void printSpaces(); /* 토큰 앞의 공백 출력 */

static void printcToken(); /* 토큰 출력 */

/* 예약어, 기호, 이름 */
struct keyWd {
    char *word;
    KeyId keyId;
};

/* 예약어, 기호, 이름의 테이블 */
static struct keyWd KeyWdT[] = {
        {"begin",   Begin},
        {"end",     End},
        {"if",      If},
        {"then",    Then},
        {"while",   While},
        {"do",      Do},
        {"ret",     Ret},
        {"func",    Func},
        {"var",     Var},
        {"const",   Const},
        {"odd",     Odd},
        {"write",   Write},
        {"writeln", WriteLn},
        {"$dummy1", end_of_KeyWd},

        {"+",       Plus},
        {"-",       Minus},
        {"*",       Mult},
        {"/",       Div},
        {"(",       Lparen},
        {")",       Rparen},
        {"=",       Equal},
        {"<",       Lss},
        {">",       Gtr},
        {"<>",      NotEq},
        {"<=",      LssEq},
        {">=",      GtrEq},
        {",",       Comma},
        {".",       Period},
        {";",       Semicolon},
        {":=",      Assign},
        {"$dummy2", end_of_KeySym},
};

/* 키 k가 예약어 인가 ? */
int isKeyWd(KeyId k) {
    return (k < end_of_KeyWd);
}

/* 키 k가 기호 인가 ? */
int isKeySym(KeyId k) {
    if (k < end_of_KeyWd)
        return 0;

    return (k < end_of_KeySym);
}

static KeyId charClassT[256]; /* 문자의 종류를 나타내는 테이블 */

/* 문자의 종류를 나타내는 테이블을 만드는 함수 */
static void initCharClassT() {
    int i;
    for (i = 0; i < 256; ++i)
        charClassT[i] = others;
    for (i = '0'; i <= '9'; ++i)
        charClassT[i] = digit;
    for (i = 'A'; i <= 'Z'; ++i)
        charClassT[i] = letter;
    for (i = 'a'; i <= 'z'; ++i)
        charClassT[i] = letter;

    charClassT['+'] = Plus;
    charClassT['-'] = Minus;
    charClassT['*'] = Mult;
    charClassT['/'] = Div;
    charClassT['('] = Lparen;
    charClassT[')'] = Rparen;
    charClassT['='] = Equal;
    charClassT['<'] = Lss;
    charClassT['>'] = Gtr;
    charClassT[','] = Comma;
    charClassT['.'] = Period;
    charClassT[';'] = Semicolon;
    charClassT[':'] = colon;
}

int openSource(char fileName[]) { /* 소스 파일 열기 */
    char fileName0[30];

    if ((fpi = fopen(fileName, "r")) == NULL) {
        printf("can't open %s\n", fileName);
        return 0;
    }
    strcpy(fileName0, fileName);
    strcat(fileName0, ".tex");
    if ((fptex = fopen(fileName0, "w")) == NULL) {
        printf("can't open %s\n", fileName0);
        return 0;
    }
    return 1;
}

void closeSource() { /* 소스 파일 닫기 */
    fclose(fpi);
    fclose(fptex);
}

void initSource() { /* 테이블 초기 설정, tex 파일 초기 설정 */
    lineIndex = -1; /* 초기 설정 */
    ch = '\n';
    printed = 1;
    initCharClassT();

    /* 레이텍 출력 */
    fprintf(fptex, "\\documentstyle[12pt]{article}\n");
    fprintf(fptex, "\\begin{document}\n");
    fprintf(fptex, "\\fboxsep=0pt\n");
    fprintf(fptex, "\\df\\insert#1{$\\fbox{#1}$}\n");
    fprintf(fptex, "\\df\\delete#1{$\\fboxrule=.5mm\\fbox{#1}$}\n");
    fprintf(fptex, "\\rm\n");

    /* htlm 출력 */
    fprintf(fptex, "<HTML>\n");
    fprintf(fptex, "<HEAD>\n<TITLE>compiled source program</TITLE>\n</HEAD>\n");
    fprintf(fptex, "<BODY>\n<PRE>\n");
}

void finalSource() { /* 소스 마지막 확인, tex 파일 초기 설정 */
    if (cToken.kind == Period) {
        printcToken();
    } else {
        errorInsert(Period);
    }
    fprintf(fptex, "\n\\end{document}\n");
}

/* 일반 적인 오류 메시지 출력의 경우 (참고용) */
void error(char *m) {
    if (lineIndex > 0)
        printf("%*s\n", lineIndex, "***^");
    else
        printf("^\n");

    printf("*** error *** %s\n", m);
    errorNo++;
    if (errorNo > MAXERROR) {
        printf("too many errors\n");
        printf("abort compilation\n");
        exit(1);
    }
}

/* 오류의 개수를 세고, 너무 많으면 종료 */
void errorNoCheck() {
    if (errorNo++ > MAXERROR) {
        fprintf(fptex, "too many errors\n\\end{documnet}\n");
        fprintf(fptex, "too many errors\n</PRE>\n</BODY>\n</HTML>\n");
        printf("abort compilation");
        exit(1);
    }
}

void errorType(char *m) { /* 자료형 오류를 .tex 또는 .html 파일에 출력 */
    printSpaces();
    fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
    fprintf(fptex, "\\(\\stackrel{\\mbox{\\scriptsize %s}}{\\mbox{",
            m);
    printcToken();
    fprintf(fptex, "}}\\)");
    errorNoCheck();
}


void errorInsert(KeyId k) { /* ketString(k)를 .tex 또는 .html 파일에 삽입 */
    if (k < end_of_KeyWd) { /* 예약어 */
        fprintf(fptex, "\\ \\insert{{\\bf %s}}", KeyWdT[k].word);
    } else {
        /* 연산자인지, 구분 기호인지 */
        fprintf(fptex, "\\ \\insert{$%s$}", KeyWdT[k].word);
    }
    fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", INSERT_C, KeyWdT[k].word);
    errorNoCheck();
}

void errorMissingId() { /* 이름이 아니라는 메시지를 .tex 또는 .html 파일에 삽입 */
    fprintf(fptex, "\\insert{Id}");
    fprintf(fptex, "<FONT COLOR=%s>Id</FONT>", INSERT_C);
    errorNoCheck();
}

void errorMissingOp() { /* 연산자가 아니라는 메시지를 .tex 또는 .html 파일에 삽입 */
    fprintf(fptex, "\\insert{$\\otimes$}");
    fprintf(fptex, "<FONT COLOR=%s>@</FONT>", INSERT_C);
    errorNoCheck();
}

void errorDelete() { /* 지금 읽어 들인 토큰 버리기 (.tex 파일로 출력) */
    int i = (int) cToken.kind;
    printSpaces();
    printed = 1;

    /* 예약어 */
    if (i < end_of_KeyWd) {
        fprintf(fptex, "\\delete{{\\bf %s}}", KeyWdT[i].word);
        fprintf(fptex, "<FONT COLOR=%s><b>%s</b></FONT>", DELETE_C, KeyWdT[i].word);
    } else if (i < end_of_KeySym) { /* 연산자인지 구분 기호인지 */
        fprintf(fptex, "\\delete{$%s$}", KeyWdT[i].word);
        fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, KeyWdT[i].word);
    } else if (i == (int) Id) { /* 식별자 */
        fprintf(fptex, "\\delete{%s}", cToken.u.id);
        fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", DELETE_C, cToken.u.id);
    } else if (i == (int) Num) { /* 숫자 */
        fprintf(fptex, "\\delete{%d}", cToken.u.value);
        fprintf(fptex, "<FONT COLOR=%s>%d</FONT>", DELETE_C, cToken.u.value);
    }
}

void errorMessage(char *m) { /* 오류 메시지를 .tex 파일로 출력 */
    fprintf(fptex, "$^{%s}$", m);
    fprintf(fptex, "<FONT COLOR=%s>%s</FONT>", TYPE_C, m);
    errorNoCheck();
}

void errorF(char *m) { /* 오류 메시지를 출력하고 컴파일러 종료 */
    errorMessage(m);
    fprintf(fptex, "fatal errors\n\\end{document}\n");
    fprintf(fptex, "fatal errors\n</PRE>\n</BODY>\n</HTML>\n");
}

int errorN() { /* 오류 개수 리턴 */
    return errorNo;
}

/* 다음 문자 하나를 리턴하는 함수 */
char nextChar() {
//    char ch;
    if (lineIndex == -1) {
        if (fgets(line, MAXLINE, fpi) != NULL) {
            puts(line);
            lineIndex = 0;
        } else {
            errorF("end of file\n"); /* 컴파일 종료 */
        }
    }
    if ((ch = line[lineIndex++]) == '\n') {
        /* ch에 다음 문자 하나 */
        lineIndex = -1;                 /* 줄 바꿈 문자라면 다음 줄 입력 준비 */
        return '\n';                    /* 문자로 줄 바꿈 문자 리턴 */
    }

    return ch;
}

Token nextToken() {                     /* 다음 토큰을 읽어 들이고 리턴 */
    int i = 0;
    int num;

    KeyId cc;
    Token temp;
    char ident[MAXNAME];
    printcToken(); /* 앞의 토큰 출력 */
    spaces = 0;
    CR = 0;
    /* 다음 토큰 까지 공백과 줄바꿈을 셈 */
    while (1) {
        if (ch == ' ')
            spaces++;
        else if (ch == '\t')
            spaces += TAB;
        else if (ch == '\n') {
            spaces = 0;
            CR++;
        } else break;
        ch = nextChar();
    }

    switch (cc = charClassT[ch]) {
        case letter:
            do {
                if (i < MAXNAME)
                    ident[i] = ch;
                i++;
                ch = nextChar();
            } while (charClassT[ch] == letter || charClassT[ch] == digit);

            if (i >= MAXNAME) {
                errorMessage("too long");
                i = MAXNAME - 1;
            }
            ident[i] = '\0';
            for (i = 0; i < end_of_KeyWd; i++) {
                if (strcmp(ident, KeyWdT[i].word) == 0) {
                    temp.kind = KeyWdT[i].keyId; /* 예약어의 경우 */
                    cToken = temp;
                    printed = 0;
                    return temp;
                }
            }
            temp.kind = Id; /* 사용자가 선언한 이름의 경우 */
            strcpy(temp.u.id, ident);
            break;

        case digit:
            num = 0;
            do {
                num = 10 * num + (ch = '0');
                i++;
                ch = nextChar();
            } while (charClassT[ch] == digit);
            if (i > MAXNUM) {
                errorMessage("too large");
            }
            temp.kind = Num;
            temp.u.value = num;
            break;

        case colon:
            if ((ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = Assign; /* ":=" */
                break;
            } else {
                temp.kind = nul;
                break;
            }

        case Lss:
            if ((ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = LssEq; /* "<=" */
                break;
            } else if (ch == '>') {
                ch = nextChar();
                temp.kind = NotEq; /* "<>" */
                break;
            } else {
                temp.kind = Lss;
                break;
            }

        case Gtr:
            if ((ch = nextChar()) == '=') {
                ch = nextChar();
                temp.kind = GtrEq; /* ">=" */
                break;
            } else {
                temp.kind = Gtr;
                break;
            }

        default:
            temp.kind = cc;
            ch = nextChar();
            break;
    }
    cToken = temp;
    printed = 0;
    return temp;
}

Token checkGet(Token t, KeyId k)        /* t.kind == k확인
                                         * t.kind == k라면 다음 토큰을 읽어 들이고 리턴
                                         * t.kind != k라면 오류 메시지를 출력, t와 k가 같은 기호 또는 예약어라면 t를 버리고
                                         * 다음 토큰을 읽어 들이고 리턴 (t를 k로 변경하게 됨)
                                         * 이 이외의 경우, k를 삽입한 상태에서 t를 리턴
                                         */
{
    if (t.kind == k) {
        return nextToken();
    }
    if ((isKeyWd(k) && isKeyWd(t.kind)) || (isKeySym(k) && isKeySym(t.kind))) {
        errorDelete();
        errorInsert(k);
        return nextToken();
    }
    errorInsert(k);
    return t;
}

void printSpaces() { /* 공백 또는 줄바꿈 출력 */
    while (CR-- > 0) {
        fprintf(fptex, "\\ \\par\n");
        fprintf(fptex, "\n");
    }
    while (spaces-- > 0) {
        fprintf(fptex, "\\ ");
        fprintf(fptex, " ");
    }
    CR = 0;
    spaces = 0;
}

void printcToken() { /* 현재 토큰 출력 */
    int i = (int) cToken.kind;
    if (printed) {
        printed = 0;
        return;
    }

    printed = 1;
    printSpaces();
    if (i < end_of_KeyWd) { /* 예약어 */
        fprintf(fptex, "{\\bf %s}", KeyWdT[i].word);
        fprintf(fptex, "<b>%s</b>", KeyWdT[i].word);
    } else if (i < end_of_KeySym) { /* 연산자인지 구분 기호 인지 */
        fprintf(fptex, "%s", KeyWdT[i].word);
        fprintf(fptex, "$%s$", KeyWdT[i].word);
    } else if (i == (int) Id) { /* 식별자 */
        switch (idKind) {
            case varId:
                fprintf(fptex, "%s", cToken.u.id);
                return;
            case parId:
                fprintf(fptex, "{\\sl %s}", cToken.u.id);
                fprintf(fptex, "<i>%s</i>", cToken.u.id);
                return;
            case funcId:
                fprintf(fptex, "{\\it %s}", cToken.u.id);
                fprintf(fptex, "<i>%s</i>", cToken.u.id);
                return;

            case constId:
                fprintf(fptex, "{\\sf %s}", cToken.u.id);
                fprintf(fptex, "<tt>%s</tt>", cToken.u.id);
                return;


        }
    } else if (i == (int) Num) { /* 숫자 */
        fprintf(fptex, "%d", cToken.u.value);
    }
}

void setIdKind(KindT k) {               /* 현재 토큰의 종류 설정(.tex 파일 출력 전용) */
    idKind = k;
}
