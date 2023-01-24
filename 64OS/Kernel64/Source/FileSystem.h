//
// Created by jewoo on 2023-01-20.
//
// 파일 시스템 헤더 파일


#ifndef CRAFTING_FILESYSTEM_H
#define CRAFTING_FILESYSTEM_H

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"


/// MINT 파일 시스템 시그너처 (Signature)
#define FILE_SYSTEM_SIGNATURE                       0x7E38CF10

/// 클러스터의 크기(섹터 수), 4KB
#define FILE_SYSTEM_SECTOR_SPER_CLUSTER             8

/// 파일 클러스터의 마지막 표시
#define FILE_SYSTEM_LAST_CLUSTER                    0xFFFFFFFF

/// 빈 클러스터 표시
#define FILE_SYSTEM_FREE_CLUSTER                    0x00

/// 루트 디렉터리에 있는 최대 디렉터리 엔트리의 수
#define FILE_SYSTEM_MAX_DIRECTORY_ENTRY_COUNT       ((FILE_SYSTEM_SECTOR_SPER_CLUSTER * 512) / sizeof(DIRECTORY_ENTRY))

/// 클러스트의 크기(바이트 수)
#define FILE_SYSTEM_CLUSTER_SIZE                    (FILE_SYSTEM_SECTOR_SPER_CLUSTER * 512)

/// 파일 이름의 최대 길이
#define FILE_SYSTEM_MAX_FILE_NAME_LENGTH            24

/// 하드 디스크 제어에 관련된 함수 포인터 타입 정의
typedef BOOL (*fReadHDDInformation)(BOOL bPrimary, BOOL bMaster,
                                    HDD_INFORMATION *pstHDDInformation);

typedef int (*fReadHDDSector)(BOOL bPrimary, BOOL bMaster, DWORD dwLBA,
                              int iSectorCount, char *pcBuffer);

typedef int (*fWrtieHDDSector)(BOOL bPrimary, BOOL bMaster, DWORD dwLBA,
                               int iSectorCount, char *pcBuffer);

/// 구조체
/// 1바이트로 정렬
#pragma pack(push, 1)

/// 파티션 자료 구조
typedef struct kPartitionStruct {
    /// 부팅 가능 플래그, 0x80이면 부팅 가능을 나타내며 0x00은 부팅 불가
    BYTE bBootableFlag;

    /// 파티션의 시작 어드레스, 현재는 거의 사용하지 않으며 아래의 LBA 어드레스를 대신 사용
    BYTE vbStartingCHSAddress[3];

    /// 파티션 타입
    BYTE bPartitionType;

    /// 파티션의 마지막 어드레스. 현재는 거의 사용 안 함
    BYTE vbEndingCHSAddress[3];

    /// 파티션의 시작 어드레스. LBA 어드레스로 나타낸 값
    DWORD dwStartingLBAAddress;

    /// 파티션에 포함된 섹터 수
    DWORD dwSizeInSector;
} PARTITION;

/// MBR 자료구조
typedef struct kMBRStruct {
    /// 부트 로더 코드가 위치하는 영역
    BYTE vbBootCode[430];

    /// 파일 시스템 시그너처, 0x7E38CF10
    DWORD dwSignature;

    /// 예약된 영역의 섹터 수
    DWORD dwReservedSectorCount;

    /// 클러스터 링크 테이블 영역의 섹터 수
    DWORD dwClusterLinkSectorCount;

    /// 클러스터의 전체 개수
    DWORD dwTotalClusterCount;

    /// 파티션 테이블
    PARTITION vstPartition[4];

    /// 부트 로더 시그니처, 0x55, 0xAA
    BYTE vbBootLoaderSignature[2];
} MBR;

/// 디렉터리 엔트리 자료구조
typedef struct kDirectoryEntryStruct {

    /// 파일 이름
    char vcFileName[FILE_SYSTEM_MAX_FILE_NAME_LENGTH];

    /// 파일의 실제 크기
    DWORD dwFileSize;

    /// 파일이 시작하는 클러스트 인덱스
    DWORD dwStartClusterIndex;
} DIRECTORY_ENTRY;

#pragma pack(pop)


typedef struct kFileSystemManagerStruct {
    /// 파일 시스템이 정상적으로 인식되었는지 여부
    BOOL bMounted;

    /// 각 영역의 섹터 수와 시작 LBA 어드레스
    DWORD dwReservedSectorCount;
    DWORD dwClusterLinkAreaStartAddress;
    DWORD dwClusterLinkAreaSize;
    DWORD dwDataAreaStartAddress;

    /// 데이터 영역의 클러스터의 총 개수
    DWORD dwTotalClusterCount;

    /// 마지막으로 클러스터를 할당한 클러스터 링크 테이블의 섹터 오프셋을 저장
    DWORD dwLastAllocatedClusterLinkSectorOffset;

    /// 파일 시스템 동기화 객체
    MUTEX stMutex;
} FILESYSTEM_MANAGER;

BOOL kInitializeFileSystem(void);

BOOL kFormat(void);

BOOL kMount(void);

BOOL kGetHDDInformation(HDD_INFORMATION *pstInformation);

BOOL kReadClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer);

BOOL kWrtieClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer);

BOOL kReadCluster(DWORD dwOffset, BYTE *pbBuffer);

BOOL kWrtieCluster(DWORD dwOffset, BYTE *pbBuffer);

DWORD kFindFreeCluster(void);

BOOL kSetClusterLinkData(DWORD dwClusterIndex, DWORD dwData);

BOOL kGetClusterLinkData(DWORD dwClusterIndex, DWORD *pdwData);

int kFindFreeDirectoryEntry(void);

BOOL kSetDirectoryEntryData(int iIndex, DIRECTORY_ENTRY *pstEntry);

BOOL kGetDirectoryEntryData(int iIndex, DIRECTORY_ENTRY *pstEntry);

int kFindDirectoryEntry(const char *pcFileName, DIRECTORY_ENTRY *pstEntry);

void kGetFileSystemInformation(FILESYSTEM_MANAGER *pstManager);


#endif //CRAFTING_FILESYSTEM_H
