//
// Created by jewoo on 2023-01-05.
//

#include "BTree.h"

NODE *CreateNode() {
    NODE *pNode = malloc(sizeof(NODE));
    memset(pNode, 0, sizeof(NODE)); // 메모리의 내용(값)을 원하는 크기만큼 특정 값으로 세팅할 수 있는 함수
    return pNode;
}

OVERFLOW_NODE *CreateOverflowNode() {
    OVERFLOW_NODE *pNode = malloc(sizeof(OVERFLOW_NODE));
    memset(pNode, 0, sizeof(OVERFLOW_NODE));
    return pNode;
}

STACK_NODE *CreateStackNode() {
    STACK_NODE *pStackNode = malloc(sizeof(STACK_NODE));
    memset(pStackNode, 0, sizeof(STACK_NODE));
    return pStackNode;
}

STACK_NODE *PushIntoStack(STACK_NODE *pStackTop, NODE *pNode) {
    STACK_NODE *pNewTop;
    if (pStackTop == NULL || pNode == NULL) {
        return NULL;
    }

    if (pStackTop->pCurr == NULL) {
        /* top이 비어 있는 경우 */
        pStackTop->pCurr = pNode;
        pStackTop->pPrev = NULL;
        return pStackTop;
    } else {
        /* 새 top을 생성 */
        pNewTop = CreateStackNode();
        pNewTop->pPrev = (void *) pStackTop;
        pNewTop->pCurr = pNode;
        return pNewTop;
    }
}

STACK_NODE *PopFromStack(STACK_NODE *pStackTop) {
    STACK_NODE *pPrevTop;
    NODE *pNode;

    if (pStackTop == NULL) {
        return NULL;
    }
    if (pStackTop->pCurr == NULL) {
        return NULL;
    } else if (pStackTop->pPrev == NULL) {
        /* 현재 top임 */
        pNode = pStackTop->pCurr;
        pStackTop->pCurr = NULL;
        return pStackTop;
    } else {
        /* pCurr, pPrev이 모두 존재 */
        pNode = pStackTop->pCurr;
        pPrevTop = pStackTop;
        pStackTop = (STACK_NODE *) pStackTop->pPrev;
        /* 이전 top 삭제 */
        free((void *) pPrevTop);
        return pStackTop;
    }
}

NODE *PeepStackTop(STACK_NODE *pStackTop) {
    /* STACK의 top를 peep */
    if (pStackTop == NULL) {
        return NULL;
    }
    return pStackTop->pCurr;
}

int CheckElementExist(NODE *pNode, int key) {
    /* 노드에 key가 있는지 검사 */
    int i;
    int bExist; /* 키가 존재하는지 여부 flag */
    if (pNode == NULL) {
        return 0;
    }

    bExist = 0;

    for (i = 0; i < DEGREE - 1; ++i) {
        if (key == pNode->nElm[i]) {
            bExist = 1;
            break;
        }
    }
    return bExist;
}

int GetElementCount(NODE *pNode) {
    /* 노드의 원소 수 반환 */
    int i;
    int count;
    if (pNode == NULL) { return 0; }
    count = 0;
    for (i = 0; i < DEGREE - 1; ++i) { if (pNode->nElm[i] > 0) { count++; } else { break; }}
    return count;
}

void PrintNodeElement(NODE *pNode) {
    /* 노드의 키값 출력 */
    int i;
    NODE *pChild;
    /* post order로 출력 */
    pChild = pNode->pChild[0];

    if (pChild == NULL) {
        /* leaf */
        fprintf(pOutputStream, "[ ");

        for (i = 0; i < DEGREE - 1; ++i) {
            if (pNode->nElm[i] > 0) {
                fprintf(pOutputStream, "%d", pNode->nElm[i]);
            }
        }

        fprintf(pOutputStream, "] ");
        return;
    }
    PrintNodeElement(pNode->pChild[0]);
    /* leftmst child 출력: node내의 키와 다른 child 출력 */
    for (i = 1; i < DEGREE; ++i) {
        if (pNode->pChild[i] == NULL) { return; }
        PrintNodeElement(pNode->pChild[i]);
        fprintf(pOutputStream, "%d ", pNode->nElm[i - 1]);
    }
}

int makeTreeMain(int argc, char *argv[]) {

    int i, j;
    int nPos;                       /* 삽입 위치를 저장하기 위한 임시 변수 */
    int nKey;                       /* 삽입 및 삭제글 해줄 키 값 */
    int bFinished;                  /* 삽입, 삭제가 일어났는지의 flag */
    int bFound;                     /* B-tree에서 해당 키가 발견되었는지의 flag */
    int bOverflow;                  /* 해당 node에서의 오버플로가 발생했는지의 flag */
    int nCount;                     /* 해당 node에서의 key 수 */
    int nTemp, nTemp2, nTemp3;      /* 임시 변수 */
    NODE *pRoot;                    /* 루트 node */
    NODE *pCurr;                    /* 현재 작업 수행 중인 node */
    NODE *pChild;                   /* 현재 node의 child node */
    NODE *pNewNode;                 /* 생성할 새 node */
    NODE *pInKey;                   /* in-key의 pointer */
    STACK_NODE *pStackTop;          /* stack의 top pointer */
    OVERFLOW_NODE *pOverflow;       /* 오버플로의 경우를 처리할 임시 노드 */
    int nInput[MAX_ELEMENTS];       /* 파일 접근을 줄이기 위해 배열에 저장*/
    int nElementCnt;

    /* 초기화 */
    nKey = 0;
    bFinished = 0;
    bFound = 0;
    bOverflow = 0;
    nCount = 0;
    for (i = 0; i < MAX_ELEMENTS; ++i) {
        nInput[i] = 0;
    }
    nElementCnt = 0;
    /* 공백 B-TREE 생성 */
    pRoot = CreateNode();
    pRoot->bRoot = 1;
    pCurr = pRoot;

    /* input.txt 파일 열기 */
    if ((pInputStream = fopen("input.txt", "r")) == NULL) {
        /* input 파일을 열수 없음 */
        printf("input.txt file cannot be opened!\n");
        return -1;
    }

    /* output.txt 파일 열기 */
    if ((pOutputStream = fopen("output.txt", "w")) == NULL) {
        /* output 파일을 열수 없음 */
        printf("output.txt file cannot be opened!\n");
        return -1;
    }

    /* b-tree로 삽입 */
    printf("[B-tree] insertion Process started\n");
    pStackTop = NULL;

    while (1) {
        nKey = 0;
        if (fscanf(pInputStream, "%d", &nKey) == EOF) { break; }

        /* key 값을 배열에 저장 */
        nInput[nElementCnt] = nKey;
        nElementCnt++;

        pCurr = pRoot;
        /* stack 초기화 */
        while (pStackTop && pStackTop->pCurr) {
            pStackTop = PopFromStack(pStackTop);
        }

        pStackTop = CreateStackNode();

        pChild = NULL;
        printf("Key (%d) inserted \n", nKey);
        fprintf(pOutputStream, "Key (%d) inserted \n", nKey);

        do {
            nCount = GetElementCount(pCurr);
            if (nCount == 0) {
                /* root node는 초기에 키값이 0개일 수 있음 */
                pChild = pCurr->pChild[0];
            } else if (CheckElementExist(pCurr, nKey) == 1) {
                bFound = 1;
            } else if (nKey < pCurr->nElm[0]) {
                pChild = pCurr->pChild[0];
            } else if (nKey > pCurr->nElm[(nCount - 1 > 0) ? nCount - 1 : 0]) {
                pChild = pCurr->pChild[nCount];
            } else {
                /* 현재 node의 key의 중간에 위치해야 함 */
                for (i = 0; i < nCount - 1; ++i) {
                    if (nKey > pCurr->nElm[i] && nKey < pCurr->nElm[i + 1]) {
                        pChild = pCurr->pChild[i + 1];
                        break;
                    }
                }
            }
            if (pChild != NULL) {
                pStackTop = PushIntoStack(pStackTop, pCurr);
                pCurr = pChild;
            }
        } while (!bFound && pChild != NULL);
        if (bFound == 1) {
            printf("[Insertion] Key (%d) alrady exists in B-tree\n", nKey);
        } else {
            /* key를 B-tree에 삽입 만약 pInkey가 존재하면 In-key와 같이 취급 -즉, 1개의 원소 소유 */
            pChild = NULL;
            pInKey = NULL;
            bFinished = 0;
            do {
                nCount = GetElementCount(pCurr);
                if (pInKey) { nKey = pInKey->nElm[0]; }
                /* 만원이 아닌 경우 */
                if (nCount < DEGREE - 1) {
                    /* 해당 위치에 바로 삽입 */
                    if (nCount == 0) {
                        nPos = 0;
                    } else if (nKey < pCurr->nElm[0]) {
                        nPos = 0;
                    } else if (nKey > pCurr->nElm[(nCount - 1 > 0) ? nCount - 1 : 0]) {
                        nPos = nCount;
                    } else {
                        for (i = 0; i < nCount - 1; ++i) {
                            if (nKey > pCurr->nElm[i] && nKey < pCurr->nElm[i + 1]) {
                                nPos = i + 1;
                                break;
                            }
                        }
                    }
                    /* 기존 key들을 shift */
                    for (i = DEGREE - 2; i > nPos; --i) {
                        pCurr->nElm[i] = pCurr->nElm[i - 1];
                        pCurr->pChild[i + 1] = pCurr->pChild[i];
                    }

                    /* 삽입 nKey가 삽입되는 경우는 리프 뿐임 */
                    pCurr->nElm[nPos] = nKey;
                    pCurr->pChild[nPos + 1] = NULL;
                    if (pInKey) {
                        pCurr->pChild[nPos] = pInKey->pChild[0];
                        pCurr->pChild[nPos + 1] = pInKey->pChild[1];
                    }
                    bFinished = 1;
                } else {
                    /* OVERFLOW_NODE에 복사 */
                    pOverflow = CreateOverflowNode();
                    for (i = 0; i < DEGREE - 1; ++i) {
                        pOverflow->nElm[i] = pCurr->nElm[i];
                        pOverflow->pChild[i] = pCurr->pChild[i];
                    }
                    pOverflow->pChild[DEGREE - 1] = pCurr->pChild[DEGREE - 1];
                    /* nKey 삽입 */

                }
            }
        }
    }
}