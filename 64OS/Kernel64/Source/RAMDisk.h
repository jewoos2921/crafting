//
// Created by jewoo on 2023-01-29.
//

#ifndef CRAFTING_RAMDISK_H
#define CRAFTING_RAMDISK_H

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"

// 매크로

/// 램 디스크의 총 섹터 수. 8MB 크기로 생성
#define RDD_TOTAL_SECTOR_COUNT          (8 * 1024 * 1024 / 512)

/// 구조체
/// 1바이트로 정력
#pragma pack(push, 1)

/// 램 디스크의 자료구조를 저장하는 구조체
typedef struct kManagerStruct {
    /// 램 디스크용으로 할당받은 메모리 영역
    BYTE *pbBuffer;
    /// 총 섹터 수
    DWORD dwTotalSectorCount;
    /// 동기화 객체
    MUTEX stMutex;
} RDD_MANAGER;

#pragma pack(pop)

// 함수
BOOL kInitializeRDD(DWORD dwTotalSectorCount);

BOOL kReadRDDInformation(BOOL bPrimary, BOOL bMaster,
                         HDD_INFORMATION *pstHDDInformation);

int kReadRDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
                   char *pcBuffer);

int kWriteRDDSector(BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
                    char *pcBuffer);

#endif //CRAFTING_RAMDISK_H
