//
// Created by jewoo on 2023-01-03.
//

#include "Queue.h"
#include "Utility.h"

void kInitializeQueue(QUEUE *pstQueue, void *pvQueueBuffer, int iMAxDataCount,
                      int iDataSize) {
    // 큐의 최대 개수와 크기와 버퍼 어드레스를 저장
    pstQueue->iMaxDataCount = iMAxDataCount;
    pstQueue->iDataSize = iDataSize;
    pstQueue->pvQueueArray = pvQueueBuffer;

    // 큐의 삽입 위치와 제거 위치를 초기화하고 마지막으로 수행된 명령을 제거로 설정하여 큐를 빈 상태로 만듦
    pstQueue->iPutIndex = 0;
    pstQueue->iGetIndex = 0;
    pstQueue->bLastOperationPut = FALSE;
}

// 큐가 가득 찼는지 여부를 반환
BOOL kIsQueueFull(const QUEUE *pstQueue) {
    // 큐의 삽입 인덱스와 제거 인덱스가 같고 마지막으로 수행된 명령이 삽입이면
    // 큐가 가득찼으므로 삽입이 불가
    if ((pstQueue->iGetIndex == pstQueue->iPutIndex) &&
        (pstQueue->bLastOperationPut == TRUE)) {
        return TRUE;
    }
    return FALSE;
}

// 큐가 비었는지 여부를 반환
BOOL kIsQueueEmpty(const QUEUE *pstQueue) {
    // 큐의 삽입 인덱스와 제거 인덱스가 같고 마지막으로 수행된 명령이 제거이면
    // 큐가 비었으므로 제거 불가
    if ((pstQueue->iGetIndex == pstQueue->iPutIndex) &&
        (pstQueue->bLastOperationPut == FALSE)) {
        return TRUE;
    }
    return FALSE;
}

// 큐에 데이터를 삽입
BOOL kPutQueue(QUEUE *pstQueue, const void *pvData) {

    // 큐가 가득 찼으면 삽입할 수 없음
    if (kIsQueueFull(pstQueue) == TRUE) {
        return FALSE;
    }

    // 삽입 인덱스가 가리키는 위치에서 데이터의 크기만큼을 복사
    kMemCpy((char *) pstQueue->pvQueueArray + (pstQueue->iDataSize * pstQueue->iPutIndex),
            pvData, pstQueue->iDataSize);

    // 삽입 인덱스를 변경하고 삽입 동작을 수행했음을 기록
    pstQueue->iPutIndex = (pstQueue->iPutIndex + 1) % pstQueue->iMaxDataCount;
    pstQueue->bLastOperationPut = TRUE;
    return TRUE;
}

// 큐에 데이터를 제거
BOOL kGetQueue(QUEUE *pstQueue, void *pvData) {
    // 큐가 비었면 제거 할 수 없음
    if (kIsQueueEmpty(pstQueue) == TRUE) {
        return FALSE;
    }

    // 제거 인덱스가 가리키는 위치에서 데이터의 크기만큼을 복사
    kMemCpy(pvData, (char *) pstQueue->pvQueueArray + (pstQueue->iDataSize * pstQueue->iPutIndex),
            pstQueue->iDataSize);

    // 제거 인덱스를 변경하고 제거 동작을 수행했음을 기록
    pstQueue->iGetIndex = (pstQueue->iGetIndex + 1) % pstQueue->iMaxDataCount;
    pstQueue->bLastOperationPut = FALSE;
    return TRUE;
}
