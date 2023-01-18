//
// Created by jewoo on 2023-01-18.
//
// 확장성 해싱 구현

#pragma once

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define PAGE_SIZE       512                     /* page size */
#define  MAX_D          8                       /* max bitstring */
#define POW(x, y)       (int) pow(x, y)

typedef struct SRecord {
    int iKey; /* key field */
    char vcName[120]; /* name field */
} Record;

typedef struct SBucket {
    int iHeader;/* bucket header */
    int iRecordCnt;/* bucket 내의 record 개수 */
    Record vstRecord[4]; /* bucket 내의 record field */
    char vcPadding[8]; /* page size를 맞추기 위한 padding */
} Bucket;

typedef struct SDirectory {
    int iHeader;
    int miTable[512][3];
    /* directory table - index, bucket number, header value */
} Directory;

typedef struct SBlockManager {
    int iBlockCnt; /* Current Block 개수 */
    int viBlockTable[512]; /* 전체 block의 현재 할당 상황 기록 field */
} BlockManager;

int pow(int x, int y);

int PseudoKeyFunc(int key, int digits);

int Delete(int key);

int Retrieve(int key);

void PrintHash();

int MakeHashMain(int argc, char *argv);