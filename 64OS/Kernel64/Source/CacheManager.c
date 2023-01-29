//
// Created by jewoo on 2023-01-29.
//

#include "CacheManager.h"
#include "FileSystem.h"
#include "DynamicMemory.h"
#include "Utility.h"
#include "Console.h"

/// 파일 시스템 캐시 자료구조
static CACHE_MANAGER gs_stCacheManager;

/// 파일 시스템 캐시를 초기화
BOOL kInitializeCacheManager(void) {
    int i;

    /// 자료구조를 초기화
    kMemSet(&gs_stCacheManager, 0, sizeof(gs_stCacheManager));

    /// 접근시간을 초기화
    gs_stCacheManager.vdwAccessTime[CACHE_CLUSTER_LINK_TABLE_AREA] = 0;
    gs_stCacheManager.vdwAccessTime[CACHE_DATA_AREA] = 0;

    /// 캐시 버퍼의 최댓값 설정
    gs_stCacheManager.vdwMaxCount[CACHE_CLUSTER_LINK_TABLE_AREA] = CACHE_MAX_CLUSTER_LINK_TABLE_AREA_COUNT;
    gs_stCacheManager.vdwMaxCount[CACHE_DATA_AREA] = CACHE_MAX_DATA_AREA_COUNT;

    /// 클러스터 링크 테이블 영역용 메모리 할당, 클러스터 링크 테이블은 512바이트로 관리함
    gs_stCacheManager.vpbBuffer[CACHE_CLUSTER_LINK_TABLE_AREA] = (BYTE *) kAllocateMemory(
            CACHE_MAX_CLUSTER_LINK_TABLE_AREA_COUNT * 512);
    if (gs_stCacheManager.vpbBuffer[CACHE_CLUSTER_LINK_TABLE_AREA] == NIL) {
        return FALSE;
    }

    /// 할당받은 메모리 영역을 나누어서 캐시 버퍼에 등록
    for (i = 0; i < CACHE_MAX_CLUSTER_LINK_TABLE_AREA_COUNT; i++) {
        /// 캐시 버퍼에 메모리 공간 할당
        gs_stCacheManager.vvstCacheBuffer[CACHE_CLUSTER_LINK_TABLE_AREA][i].pbBuffer =
                gs_stCacheManager.vpbBuffer[CACHE_CLUSTER_LINK_TABLE_AREA] + (i * 512);

        /// 태그를 유효하지 않는 것으로 설정하여 빈 것으로 만듦
        gs_stCacheManager.vvstCacheBuffer[CACHE_CLUSTER_LINK_TABLE_AREA][i].dwTag = CACHE_INVALIDING;
    }

    /// 데이터 영역용 메모리 할당, 데이터 영역은 클러스터 단위(4KB)로 관리
    gs_stCacheManager.vpbBuffer[CACHE_DATA_AREA] = (BYTE *) kAllocateMemory(
            CACHE_MAX_DATA_AREA_COUNT * FILE_SYSTEM_CLUSTER_SIZE);
    if (gs_stCacheManager.vpbBuffer[CACHE_DATA_AREA] == NIL) {

        /// 실패하면 이전에 할당받은 메모리를 해제해야 함
        kFreeMemory(gs_stCacheManager.vpbBuffer[CACHE_CLUSTER_LINK_TABLE_AREA]);
        return FALSE;
    }

    /// 할당받은 메모리 영역을 나누어서 캐시 버퍼에 등록
    for (i = 0; i < CACHE_MAX_DATA_AREA_COUNT; i++) {
        /// 캐시 버퍼에 메모리 공간 할당
        gs_stCacheManager.vvstCacheBuffer[CACHE_DATA_AREA][i].pbBuffer =
                gs_stCacheManager.vpbBuffer[CACHE_DATA_AREA] + (i * FILE_SYSTEM_CLUSTER_SIZE);

        /// 태그를 유효하지 않는 것으로 설정하여 빈 것으로 만듦
        gs_stCacheManager.vvstCacheBuffer[CACHE_DATA_AREA][i].dwTag = CACHE_INVALIDING;
    }

    return TRUE;
}

/// 캐시 버퍼에서 빈 것을 찾아서 반환
CACHE_BUFFER *kAllocateCacheBuffer(int iCacheTableIndex) {
    CACHE_BUFFER *pstCacheBuffer;
    int i;

    /// 캐시 테이블의 최대 개수를 넘어서면 실패
    if (iCacheTableIndex > CACHE_MAX_CACHE_TABLE_INDEX) { return FALSE; }

    /// 접근 시간 필드가 최댓값까지 가면 전체적으로 접근 시간을 낮춰줌
    kCutDownAccessTime(iCacheTableIndex);

    /// 최대 개수만큼 검색하여 빈 것을 반환
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    for (i = 0; i < gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++) {
        if (pstCacheBuffer[i].dwTag == CACHE_INVALIDING) {
            /// 임시로 캐시 태그를 설정하여 할당된 것으로 만듦
            pstCacheBuffer[i].dwTag = CACHE_INVALIDING - 1;

            /// 접근 시간을 갱신
            pstCacheBuffer[i].dwAccessTime =
                    gs_stCacheManager.vdwAccessTime[iCacheTableIndex]++;

            return &(pstCacheBuffer[i]);
        }
    }

    return NIL;
}

/// 캐시 버퍼에서 태그가 일치하는 것을 찾아 반환
CACHE_BUFFER *kFindCacheBuffer(int iCacheTableIndex, DWORD dwTag) {
    CACHE_BUFFER *pstCacheBuffer;
    int i;

    /// 캐시 테이블의 최대 개수를 넘어서면 실패
    if (iCacheTableIndex > CACHE_MAX_CACHE_TABLE_INDEX) { return FALSE; }

    /// 접근 시간 필드가 최댓값까지 가면 전체적으로 접근 시간을 낮춰줌
    kCutDownAccessTime(iCacheTableIndex);

    /// 최대 개수만큼 검색하여 빈 것을 반환
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    for (i = 0; i < gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++) {
        if (pstCacheBuffer[i].dwTag == dwTag) {
            /// 접근 시간을 갱신
            pstCacheBuffer[i].dwAccessTime =
                    gs_stCacheManager.vdwAccessTime[iCacheTableIndex]++;

            return &(pstCacheBuffer[i]);
        }
    }
    return NIL;

}

/// 접근한 시간을 전체적으로 낮춤
static void kCutDownAccessTime(int iCacheTableIndex) {
    CACHE_BUFFER stTemp;
    CACHE_BUFFER *pstCacheBuffer;
    BOOL bSorted;
    int i, j;

    /// 캐시 테이블의 최대 개수를 넘어서면 실패
    if (iCacheTableIndex > CACHE_MAX_CACHE_TABLE_INDEX) {
        return;
    }

    /// 접근 시간이 아직 최대치를 넘지 않았다면 접근시간을 줄일 필요 없음
    if (gs_stCacheManager.vdwAccessTime[iCacheTableIndex] < 0xfffffffe) {
        return;
    }

    /// 캐시 버퍼를 오름차순으로 정렬
    /// 버블 정렬 사용
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    for (j = 0; j < gs_stCacheManager.vdwMaxCount[iCacheTableIndex] - 1; j++) {
        /// 기본을 정렬된 것으로 저장
        bSorted = TRUE;
        for (i = 0; i < gs_stCacheManager.vdwMaxCount[iCacheTableIndex] - 1 - j; i++) {
            /// 인접한 두 데이터를 비교하여 접근 시간이 큰 것을 오른쪽(i+1)에 위치
            if (pstCacheBuffer[i].dwAccessTime > pstCacheBuffer[i + 1].dwAccessTime) {
                /// 두 데이터를 교환하므로 정렬되지 않은 것으로 표시
                bSorted = FALSE;

                /// i번째 캐시와 i+1번째 캐시를 교환
                kMemCpy(&stTemp, &(pstCacheBuffer[i]), sizeof(CACHE_BUFFER));
                kMemCpy(&(pstCacheBuffer[i]), &(pstCacheBuffer[i + 1]), sizeof(CACHE_BUFFER));
                kMemCpy(&(pstCacheBuffer[i + 1]), &stTemp, sizeof(CACHE_BUFFER));
            }
        }
        /// 다 정렬되었으면 루프를 빠져나감
        if (bSorted == TRUE) { break; }
    }

    /// 오름차순으로 정렬했으므로 인덱스가 증가할수록 접근 시간 큰(최신) 캐시 버퍼임
    /// 접근 시간을 0부터 순차적으로 설정하여 데이터를 갱신
    for (i = 0; i < gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++) {
        pstCacheBuffer[i].dwAccessTime = i;
    }

    /// 접근 시간을 파일 시스템 캐시 자료구조에 저장해 다음부터는 변경된 값으로 접근 시간을 설정
    gs_stCacheManager.vdwAccessTime[iCacheTableIndex] = i;
}

/// 캐시 버퍼에서 내보낼 것을 찾음
CACHE_BUFFER *kGetVictimInCacheBuffer(int iCacheTableIndex) {

    DWORD dwOldTime;
    CACHE_BUFFER  *pstCacheBuffer;
    int i, iOldIndex;

    /// 캐시 테이블의 최대 개수를 넘어서면 실패
    if (iCacheTableIndex > CACHE_MAX_CACHE_TABLE_INDEX) {
        return FALSE;
    }

    /// 접근 시간을 최대로 해서 접근 시간이 가장 오래된 (값이 작은) 캐시 버퍼를 검색
    iOldIndex = -1;
    dwOldTime = 0xFFFFFFFF;

    /// 캐시 버퍼에서 사용 중이지 않거나 접근한 지 가장 오래된 것을 찾아서 반환
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    for (i = 0; i < gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++) {
        /// 빈 캐시 버퍼가 있으면 빈 것을 반환
        if (pstCacheBuffer[i].dwTag == CACHE_INVALIDING) {
            iOldIndex = i;
            break;
        }

        // 접근 시간이 현재 값보다 오래되었으면 저장해둠
        if (pstCacheBuffer[i].dwAccessTime < dwOldTime) {
            dwOldTime = pstCacheBuffer[i].dwAccessTime;
            iOldIndex = i;
        }
    }

    /// 캐시 버퍼를 찾지 못하면 문제가 발생한 것
    if (iOldIndex == -1) {
        kPrintf("Cache Buffer Find Error\n");

        return NIL;
    }

    /// 선택된 캐시 버퍼의 접근 시간을 갱신
    pstCacheBuffer[iOldIndex].dwAccessTime = gs_stCacheManager.vdwAccessTime[iCacheTableIndex]++;
    return &(pstCacheBuffer[iOldIndex]);
}

/// 캐시 버퍼의 내용을 모두 비움
void kDiscardAllCacheBuffer(int iCacheTableIndex) {
    CACHE_BUFFER *pstCacheBuffer;
    int i;
    /// 캐시 버퍼를 모두 빈 것으로 설정
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    for (i = 0; i < gs_stCacheManager.vdwMaxCount[iCacheTableIndex]; i++) {
        pstCacheBuffer[i].dwTag = CACHE_INVALIDING;
    }

    /// 접근 시간을 초기화
    gs_stCacheManager.vdwAccessTime[iCacheTableIndex] = 0;
}

/// 캐시 버퍼의 포인터와 최대 개수를 반환
BOOL kGetCacheBufferAndCount(int iCacheTableIndex,
                             CACHE_BUFFER **ppstCacheBuffer, int *piMaxCount) {
    /// 캐시 테이블의 최대 개수를 넘어서면 실패
    if (iCacheTableIndex > CACHE_MAX_CACHE_TABLE_INDEX) {
        return FALSE;
    }

    /// 캐시 버퍼의 포인터와 최대 값을 반환
    *ppstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[iCacheTableIndex];
    *piMaxCount = gs_stCacheManager.vdwMaxCount[iCacheTableIndex];

    return TRUE;
}
