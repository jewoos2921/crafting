//
// Created by jewoo on 2022-12-23.
//

/* 컴파일러(구문 분석과 코드 생성)의 메인 루틴 */
#include "getSource.h"
#include "table.h"
#include "codegen.h"

#define MINERROR 3
#define FIRSTADDR 2

static Token token;

static void block(int pIndex);

static void constDecl();

static void varDecl();
