//
// Created by jewoo on 2023-01-16.
//
// 동작 메모리 관리 헤더

#ifndef CRAFTING_DYNAMICMEMORY_H
#define CRAFTING_DYNAMICMEMORY_H
/*
 * 버디 블록 알고리즘의 원리와 내부 단편화
 *
 * 1. 상위 메모리 블록은 1/2 크기의 하위 메모리 블록 2개로 분할될 수 있음
 * 2. 연속된 어드레스의 하위 메모리 블록 2개는 상위 메모리 블록 1개로 결합될 수 있음
 * 3. 메모리 할당을 요청 할 때 요청된 크기와 근접한 크기의 메모리 블록 검색
 *  A. 근접된 크기의 메모리 블록이 존재하지 않을 경우 상위 메모리 블록에서 메모리 할당
 *  B. 상위 메모리 블록은 하위 메모리 블록 2개로 구성되므로 블록 하나는 할당하고 나머지는 하위 블록 리스트에 삽입
 *  C. 상위 블록에서도 찾을 수 없는 경우, 한 단계 더 위로 이동하여 상위 메모리의 블록 리스트에서 검색함
 *  D. 만족하는 블록을 찾을 때까지 위의 A, B, C의 과정을 반복
 * 4. 메모리 해제요청 시 해제한 블록과 연결된 어드레스를 갖는 블록을 검색하여 결합한 후 상위 블록 리스트에 삽입
 *  A. 결합된 상위 블록 주변에 연속된 어드레스를 갖는 블록이 있으면 결합하여 다시 상위 블록 리스트에 삽입
 *  B. 더 이상 만족하는 블록이 없을 때까지 A의 과정을 반복
 *
 * */

#include "Types.h"

// 동적 메모리 영역의 시작 어드레스, 1MB 단위로 결정
#define DYNAMIC_MEMORY_START_ADDRES         ((TASK_STACK_POOL_ADDRESS + (TASK_STACK_SIZE * TASK_MAX_COUNT) + 0xfffff) & 0xfffffffffff00000)

// 버디 블록의 최소 크기, 1KB
#define DYNAMIC_MEMORY_MIN_SIZE             (1 * 1024)

// 비트맵의 플래그
#define DYNAMIC_MEMORY_EXIST                0x01
#define DYNAMIC_MEMORY_EMPTY                0x00

// 구조체
// 비트맵을 관리하는 자료구조
typedef struct kBitmapStruct {
    BYTE *pbBitmap;
    QWORD qwExistBitCount;
} BITMAP;

// 버디 블록을 관리하는 자료구조
typedef struct kDynamicMemoryManagerStruct {
    // 블록 리스트의 총 개수와 가장 크기가 가장 작은 블록의 개수, 할당된 메모리 크기
    int iMaxLevelCount;
    int iBlockCountOfSmallestBlock;
    QWORD qwUsedSize;

    // 블록 풀의 시작 어드레스와 마지막 어드레스
    QWORD qwStartAddress;
    QWORD qwEndAddress;

    // 할당된 메모리가 속한 불록 리스트의 인덱스를 저장하는 영역과 비트맵 자료구조의 어드레스
    BYTE *pbAllocatedBlockListIndex;
    BITMAP *pstBitmapOfLevel;
} DYNAMIC_MEMORY;

// 함수
void kInitializeDynamicMemory(void);

void *kAllocateMemory(QWORD qwSize);

BOOL kFreeMemory(void *pvAddress);

void kGetDynamicMemoryInformation(QWORD *pqwDynamicMemoryStartAddress,
                                  QWORD *pqwDynamicMemoryTotalSize,
                                  QWORD *pqwMetaDataSize,
                                  QWORD *pqwUsedMemorySize);

DYNAMIC_MEMORY *kGetDynamicMemoryManager(void);

static QWORD kCalculateDynamicMemorySize(void);

static int kCalculateMetaBlockCount(QWORD qwDynamicRAMSize);

static int kAllocationBuddyBlock(QWORD qwAlignedSize);

static QWORD kGetBuddyBlockSize(QWORD qwSize);

static int kGetBlockListIndexOfMatchSize(QWORD qwAlignedSize);

static int kFindFreeBlockInBitmap(int iBlockListIndex);

static void kSetFlagInBitmap(int iBlockListIndex, int iOffset, BYTE bFlag);

static BOOL kFreeBudyBlockint(int iBlockListIndex, int iBlockOffset);

static BYTE kGetFlagInBitmap(int iBlockListIndex, int iOffset);

#endif //CRAFTING_DYNAMICMEMORY_H
