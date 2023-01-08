//
// Created by jewoo on 2023-01-05.
//

#ifndef CRAFTING_BTREE_H
#define CRAFTING_BTREE_H

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define DEGREE 5 /* B-TREE의 차수 */
#define MAX_ELEMENTS 1000 /* 입력되는 키의 최대 수 */
#define DELETE_COUNT 10 /* 삭제시 빼줄 원소 수 */


/* 글로벌 변수 */
FILE *pInputStream; /* input 파일의 핸들 */
FILE *pOutputStream; /* output 파일의 핸들 */

/* 비트리 노드 구조 */
typedef struct SNode {
    int bRoot;
    int nElm[DEGREE - 1];
    void *pChild[DEGREE];
} NODE;

typedef struct SOverflowNode {
    int bRoot;
    int nElm[DEGREE];
    void *pChild[DEGREE + 1];
} OVERFLOW_NODE;

typedef struct SSTackNode {
    NODE *pCurr;
    void *pPrev;
} STACK_NODE;

NODE *CreateNode();

OVERFLOW_NODE *CreateOverflowNode();

STACK_NODE *CreateStackNode();

STACK_NODE *PushIntoStack(STACK_NODE *pStackTop, NODE *pNode) ;

STACK_NODE *PopFromStack(STACK_NODE *pStackTop);

NODE *PeepStackTop(STACK_NODE *pStackTop);

int CheckElementExist(NODE *pNode, int key);

int GetElementCount(NODE *pNode);

void PrintNodeElement(NODE *pNode);

int makeTreeMain(int argc, char *argv[]);

#endif //CRAFTING_BTREE_H
