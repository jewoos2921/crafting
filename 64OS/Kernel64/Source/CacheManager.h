//
// Created by jewoo on 2023-01-29.
//

#ifndef CRAFTING_CACHEMANAGER_H
#define CRAFTING_CACHEMANAGER_H

#include "Types.h"


/// 클러스터 링크 테이블 영역의 최대 캐시 버퍼의 개수
#define CACHE_MAX_CLUSTER_LINK_TABLE_AREA_COUNT                                                                         16
#define CACHE_MAX_DATA_AREA_COUNT                                                                                       32
#define CACHE_INVALIDING                                                                                                0xFFFFFFFF

#define CACHE_MAX_CACHE_TABLE_INDEX                                                                                     2
#define CACHE_CLUSTER_LINK_TABLE_AREA                                                                                   0
#define CACHE_DATA_AREA                                                                                                 1

/// 구조체
/// 파일 시스템 캐시를 구성하는 캐시 버퍼의 구조체
typedef struct kCacheBufferStruct {
    /// 캐시와 대응하는 클러스터 링크 테이블 영역이나 데이터 영역의 인덱스
    DWORD dwTag;

    /// 캐시 버퍼에 접근하는 시간
    DWORD dwAccessTime;

    /// 데이터의 내용이 변경되었는지 여부
    BOOL bChanged;

    /// 데이터 버퍼
    BYTE *pbBuffer;
} CACHE_BUFFER;

/// 파일 시스템 캐시를 관리하는 캐시 매니저의 구조체
typedef struct kCacheManagerStruct {
    /// 클러스터 링크 테이블 영역과 데이터 영역의 접근시간 필드
    DWORD vdwAccessTime[CACHE_MAX_CACHE_TABLE_INDEX];

    /// 클러스터 링크 테이블 영역과 데이터 영역의 데이터 버퍼
    BYTE *vpbBuffer[CACHE_MAX_CACHE_TABLE_INDEX];

    /// 클러스터 링크 테이블 영역과 데이터 영역의 캐시 버퍼
    /// 두 값 중에서 큰 값만큼 생성해야 함
    CACHE_BUFFER vvstCacheBuffer[CACHE_MAX_CACHE_TABLE_INDEX][CACHE_MAX_DATA_AREA_COUNT];

    /// 캐시 버퍼의 최댓값을 저장
    DWORD vdwMaxCount[CACHE_MAX_CACHE_TABLE_INDEX];
} CACHE_MANAGER;

// 함수
BOOL kInitializeCacheManager(void);

CACHE_BUFFER *kAllocateCacheBuffer(int iCacheTableIndex);

CACHE_BUFFER *kFindCacheBuffer(int iCacheTableIndex, DWORD dwTag);

CACHE_BUFFER *kGetVictimInCacheBuffer(int iCacheTableIndex);

void kDiscardAllCacheBuffer(int iCacheTableIndex);

BOOL kGetCacheBufferAndCount(int iCacheTableIndex,
                             CACHE_BUFFER **ppstCacheBuffer, int *piMaxCount);

static void kCutDownAccessTime(int iCacheTableIndex);


#endif //CRAFTING_CACHEMANAGER_H
