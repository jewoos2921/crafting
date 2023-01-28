//
// Created by jewoo on 2023-01-20.
//
// 파일시스템


#include "FileSystem.h"
#include "HardDisk.h"
#include "DynamicMemory.h"
#include "Utility.h"
#include "Task.h"
#include "Utility.h"


/// 파일 시스템 자료구조
static FILESYSTEM_MANAGER gs_stFileSystemManager;

/// 파일 시스템 임시 버퍼
static BYTE gs_vbTempBuffer[FILE_SYSTEM_SECTOR_SPER_CLUSTER * 512];

/// 하드 디스크 제어에 관련된 함수 포인터 선언
fReadHDDInformation gs_pfReadHDDInformation = NIL;
fReadHDDSector gs_pfReadHDDSector = NIL;
fWrtieHDDSector gs_pfWrtieHDDSector = NIL;

/// 파일 시스템을 초기화
BOOL kInitializeFileSystem(void) {
    /// 자료구조 초기화와 동기화 객체 초기화
    kMemSet(&gs_stFileSystemManager, 0, sizeof(gs_stFileSystemManager));
    kInitializeMutex(&(gs_stFileSystemManager.stMutex));

    /// 하드 디스크를 초기화
    if (kInitializeHDD() == TRUE) {
        /// 초기화가 성공하면 함수 포인터를 하드 디스크용 함수로 설정
        gs_pfReadHDDInformation = kReadHDDInformation;
        gs_pfReadHDDSector = kReadHDDSector;
        gs_pfWrtieHDDSector = kWriteHDDSector;
    } else {
        return FALSE;
    }

    /// 파일 시스템을 연결
    if (kMount() == FALSE) {
        return FALSE;
    }

    /// 핸들등 위한 공간을 할당
    gs_stFileSystemManager.pstHandlePool =
            (FILE *) kAllocateMemory(FILE_SYSTEM_HANDLE_MAX_COUNT * sizeof(FILE));

    /// 메모리 할당이 실패하면 하드 디스크가 인식되지 않은 것으로 설정
    if (gs_stFileSystemManager.pstHandlePool == NIL) {
        gs_stFileSystemManager.bMounted = FALSE;
        return FALSE;
    }

    /// 핸들 풀을 모두 0으로 설정하여 초기화
    kMemSet(gs_stFileSystemManager.pstHandlePool, 0,
            FILE_SYSTEM_HANDLE_MAX_COUNT * sizeof(FILE));

    return TRUE;
}

//=========================================================================================
/// 저수준 함수(Low Level Function)
//=========================================================================================
/// 하드 디스크의 MBR을 읽어서 MINT 파일 시스템인지 확인
///     MINT 파일 시스템이라면 파일 시스템에 관련된 각종 정보를 읽어서 자료구조에 삽입
BOOL kMount(void) {
    MBR *pstMBR;

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// MBR을 읽음
    if (gs_pfReadHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    /// 시그니처를 확인하여 같다면 자료구조에 각 영역에 대한 정보를 삽입
    pstMBR = (MBR *) gs_vbTempBuffer;
    if (pstMBR->dwSignature != FILE_SYSTEM_SIGNATURE) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    /// 파일 시스템 인식 성공
    gs_stFileSystemManager.bMounted = TRUE;

    /// 각 영역의 시작 LBA 어드레스와 섹터 수를 계산
    gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
    gs_stFileSystemManager.dwClusterLinkAreaStartAddress = pstMBR->dwReservedSectorCount + 1;
    gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
    gs_stFileSystemManager.dwDataAreaStartAddress =
            pstMBR->dwReservedSectorCount + pstMBR->dwClusterLinkSectorCount + 1;
    gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return TRUE;
}

/// 하드 디스크에 파일 시스템을 생성
BOOL kFormat(void) {
    HDD_INFORMATION *pstHDD;
    MBR *pstMBR;
    DWORD dwTotalSectorCount;
    DWORD dwRemainSectorCount;
    DWORD dwMaxClusterCount;
    DWORD dwClusterCount;
    DWORD dwClusterLinkSectorCount;
    DWORD i;

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    //=========================================================================================
    ///     하드 디스크 정보를 읽어서 메타 영역의 크기와 클러스터의 개수를 계산
    //=========================================================================================
    /// 하드 디스크 정보를 읽어서 하드 디스크의 총 섹터 수를 구함
    pstHDD = (HDD_INFORMATION *) gs_vbTempBuffer;
    if (gs_pfReadHDDInformation(TRUE, TRUE, pstHDD) == FALSE) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }
    dwTotalSectorCount = pstHDD->dwTotalSectors;

    /// 전체 섹터 수를 4KB, 즉 클러스터 크기로 나누어 최대 클러스터 수를 계산
    dwMaxClusterCount = dwTotalSectorCount / FILE_SYSTEM_SECTOR_SPER_CLUSTER;

    /// 최대 클러스터의 수에 맞춰 클러스터 링크 테이블의 섹터 수를 계산
    /// 링크 데이터는 4바이트이므로, 한 섹터에는 128개가 들어감. 따라서 총 개수를
    /// 128로 나눈 후 올림하여 클러스터 링크의 섹터 수를 구함
    dwClusterLinkSectorCount = (dwMaxClusterCount + 127) / 128;

    /// 예약된 영역은 현재 사용하지 않으므로, 디스크 전체 영역에서 MBR 영역과 클러스터
    /// 링크 테이블 영역의 크기를 뺀 나머지가 실제 데이터 영역이 됨
    /// 해당 영역을 클러스터 크기로 나누어 실제 클러스터의 개수를 구함
    dwRemainSectorCount = dwTotalSectorCount - dwClusterLinkSectorCount - 1;
    dwClusterCount = dwRemainSectorCount / FILE_SYSTEM_SECTOR_SPER_CLUSTER;

    /// 실제 사용 가능한 클러스터 수에 맞춰 다시 한 번 계산
    dwClusterLinkSectorCount = (dwClusterCount + 127) / 128;

    //=========================================================================================
    /// 계산된 정보를 MBR에 덮어 쓰고, 루트 디렉터리 영역까지 모두 0으로 초기화하여 파일 시스템을 생성
    //=========================================================================================
    /// MBR 영역 읽기
    if (gs_pfReadHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }

    /// 파티션 정보와 파일 시스템 정보 설정
    pstMBR = (MBR *) gs_vbTempBuffer;
    kMemSet(pstMBR->vstPartition, 0, sizeof(pstMBR->vstPartition));
    pstMBR->dwSignature = FILE_SYSTEM_SIGNATURE;
    pstMBR->dwReservedSectorCount = 0;
    pstMBR->dwClusterLinkSectorCount = dwClusterLinkSectorCount;
    pstMBR->dwTotalClusterCount = dwClusterCount;

    /// MBR 영역에 1섹터를 씀
    if (gs_pfWrtieHDDSector(TRUE, TRUE, 0, 1, gs_vbTempBuffer) == FALSE) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return FALSE;
    }
    /// MBR 이후로부터 루트 디렉터리까지 모두 0으로 초기화
    kMemSet(gs_vbTempBuffer, 0, 512);
    for (i = 0; i < (dwClusterLinkSectorCount + FILE_SYSTEM_SECTOR_SPER_CLUSTER); i++) {
        /// 루트 디렉터리(클러스터 0)는 이미 파일 시스템이 사용하고 있으므로, 할당된 것으로 표시
        if (i == 0) {
            ((DWORD *) (gs_vbTempBuffer))[0] = FILE_SYSTEM_LAST_CLUSTER;
        } else {
            ((DWORD *) (gs_vbTempBuffer))[0] = FILE_SYSTEM_FREE_CLUSTER;
        };

        /// 1섹터씩 씀
        if (gs_pfWrtieHDDSector(TRUE, TRUE, i + 1, 1, gs_vbTempBuffer) == FALSE) {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return FALSE;
        }
    }

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return TRUE;
}

/// 파일 시스템에 연결된 하드 디스크의 정보를 반환
BOOL kGetHDDInformation(HDD_INFORMATION *pstInformation) {
    BOOL bResult;

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    bResult = gs_pfReadHDDInformation(TRUE, TRUE, pstInformation);

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return bResult;
}

/// 클러스터 링크 테이블 내의 오프셋에 한 섹터를 읽음
static BOOL kReadClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer) {
    /// 클러스터 링크 테이블 영역의 시작 어드레스를 더함
    return gs_pfReadHDDSector(TRUE, TRUE, dwOffset +
                                          gs_stFileSystemManager.dwClusterLinkAreaStartAddress,
                              1, pbBuffer);
}

/// 클러스터 링크 테이블 내의 오프셋에 한 섹터를 씀
static BOOL kWrtieClusterLinkTable(DWORD dwOffset, BYTE *pbBuffer) {
    /// 클러스터 링크 테이블 영역의 시작 어드레스를 더함
    return gs_pfWrtieHDDSector(TRUE, TRUE, dwOffset +
                                           gs_stFileSystemManager.dwClusterLinkAreaStartAddress,
                               1, pbBuffer);
}

/// 데이터 영역의 오프셋에서 한 클러스터를 읽음
static BOOL kReadCluster(DWORD dwOffset, BYTE *pbBuffer) {
    /// 테이블 영역의 시작 어드레스를 더함
    return gs_pfReadHDDSector(TRUE, TRUE, (dwOffset *
                                           FILE_SYSTEM_SECTOR_SPER_CLUSTER) +
                                          gs_stFileSystemManager.dwDataAreaStartAddress,
                              FILE_SYSTEM_SECTOR_SPER_CLUSTER, pbBuffer);
}

/// 데이터 영역의 오프셋에서 한 클러스터를 씀
static BOOL kWrtieCluster(DWORD dwOffset, BYTE *pbBuffer) {
    /// 클러스터 링크 테이블 영역의 시작 어드레스를 더함
    return gs_pfWrtieHDDSector(TRUE, TRUE, (dwOffset *
                                            FILE_SYSTEM_SECTOR_SPER_CLUSTER) +
                                           gs_stFileSystemManager.dwDataAreaStartAddress,
                               FILE_SYSTEM_SECTOR_SPER_CLUSTER, pbBuffer);
}


/// 클러스터 링크 테이블 영역에서 빈 클러스터를 검색함
static DWORD kFindFreeCluster(void) {
    DWORD dwLinkCountInSector;
    DWORD dwLastSectorOffset, dwCurrentSectorOffset;
    DWORD i, j;

    /// 파일 시스템을 인식하지 못했으면 실패
    if (gs_stFileSystemManager.bMounted == FALSE) {
        return FILE_SYSTEM_LAST_CLUSTER;
    }

    /// 마지막으로 클러스터를 할당한 클러스터 링크 테이블의 오프셋을 가져옴
    dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;

    /// 마지막으로 할당한 위치부터 루프를 돌면서 빈 클러스터를 검색
    for (i = 0; i < gs_stFileSystemManager.dwClusterLinkAreaSize; i++) {
        /// 클러스터 링크 테이블의 마지막 섹터이면 전체 섹터만큼 도는 것이 아니라
        /// 남은 클러스터의 수만큼 루프를 돌아야 함
        if ((dwLastSectorOffset + i) == (gs_stFileSystemManager.dwClusterLinkAreaSize - 1)) {
            dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128;
        } else {
            dwLinkCountInSector = 128;
        }

        /// 이번에 읽어야 할 클러스터 링크 테이블의 섹터 오프셋을 구해서 읽음
        dwCurrentSectorOffset = (dwLastSectorOffset + i) % gs_stFileSystemManager.dwClusterLinkAreaSize;
        if (kReadClusterLinkTable(dwCurrentSectorOffset, gs_vbTempBuffer) == FALSE) {
            return FILE_SYSTEM_LAST_CLUSTER;
        }

        /// 섹터 내에서 루프를 돌면서 빈 클러스터를 검색
        for (j = 0; j < dwLinkCountInSector; j++) {
            if (((DWORD *) gs_vbTempBuffer)[i] == FILE_SYSTEM_FREE_CLUSTER) {
                break;
            }
        }

        /// 찾았다면 클러스터 인덱스를 반환
        if (j != dwLinkCountInSector) {
            /// 마지막으로 클러스터를 할당한 클러스터 링크 내의 섹터 오프섹을 저장
            gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset = dwCurrentSectorOffset;

            /// 현재 클러스터 링크 테이블의 오프셋을 감안하여 클러스터 인덱스를 계산
            return (dwCurrentSectorOffset * 128) + j;
        }
    }

    return FILE_SYSTEM_LAST_CLUSTER;
}

/// 클러스터 링크 테이블에 값을 설정
static BOOL kSetClusterLinkData(DWORD dwClusterIndex, DWORD dwData) {
    DWORD dwSectorOffset;

    /// 파일 시스템을 인식하지 못앴으면 실패
    if (gs_stFileSystemManager.bMounted == FALSE) {
        return FALSE;
    }

    /// 한 섹터에 128개의 클러스터 링크가 들어가므로 128로 나누면 섹터 오프셋을 구할 수 있음
    dwSectorOffset = dwClusterIndex / 128;

    /// 해당 섹터를 읽어서 링크 정보를 설정한 후 다시 저장
    if (kReadClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE) {
        return FALSE;
    }

    ((DWORD *) gs_vbTempBuffer)[dwClusterIndex % 128] = dwData;

    if (kWrtieClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE) {
        return FALSE;
    }

    return TRUE;
}

/// 클러스터 링크 테이블에 값을 반환
static BOOL kGetClusterLinkData(DWORD dwClusterIndex, DWORD *pdwData) {
    DWORD dwSectorOffset;

    /// 파일 시스템을 인식하지 못앴으면 실패
    if (gs_stFileSystemManager.bMounted == FALSE) {
        return FALSE;
    }

    /// 한 섹터에 128개의 클러스터 링크가 들어가므로 128로 나누면 섹터 오프셋을 구할 수 있음
    dwSectorOffset = dwClusterIndex / 128;

    if (dwSectorOffset > gs_stFileSystemManager.dwClusterLinkAreaSize) {
        return FALSE;
    }

    /// 해당 섹터를 읽어서 링크 정보를 설정한 후 다시 저장
    if (kReadClusterLinkTable(dwSectorOffset, gs_vbTempBuffer) == FALSE) {
        return FALSE;
    }

    *pdwData = ((DWORD *) gs_vbTempBuffer)[dwClusterIndex % 128];
    return TRUE;
}


/// 루트 디렉터리에서 빈 디렉터리 엔트리를 반환
static int kFindFreeDirectoryEntry(void) {

    DIRECTORY_ENTRY *pstEntry;
    int i;

    /// 파일 시스템을 인식하지 못했으면 실패
    if (gs_stFileSystemManager.bMounted == FALSE) {
        return -1;
    }

    /// 루트 디렉터리를 읽음
    if (kReadCluster(0, gs_vbTempBuffer) == FALSE) {
        return -1;
    }
    /// 루트 디렉터리 안에서 루프를 돌면서 빈 엔트리, 즉 시작 클러스터 번호가 0인 엔트리를 검색
    pstEntry = (DIRECTORY_ENTRY *) gs_vbTempBuffer;
    for (i = 0; i < FILE_SYSTEM_MAX_DIRECTORY_ENTRY_COUNT; i++) {
        if (pstEntry[i].dwStartClusterIndex == 0) {
            return i;
        }
    }
    return -1;

}

/// 루트 디렉터리의 해당 인덱스에 디렉터리 엔트리를 설정
static BOOL kSetDirectoryEntryData(int iIndex, DIRECTORY_ENTRY *pstEntry) {
    DIRECTORY_ENTRY *pstRootEntry;

    /// 파일 시스템을 인식하지 못했거나 인덱스가 올바르지 않으면 실패
    if ((gs_stFileSystemManager.bMounted == FALSE) ||
        (iIndex < 0) || (iIndex >= FILE_SYSTEM_MAX_DIRECTORY_ENTRY_COUNT)) {
        return FALSE;
    }

    /// 루트 디렉터리를 읽음
    if (kReadCluster(0, gs_vbTempBuffer) == FALSE) {
        return FALSE;
    }


    /// 루트 디렉터리에 있는 해당 데이터를 갱신
    pstRootEntry = (DIRECTORY_ENTRY *) gs_vbTempBuffer;
    kMemCpy(pstRootEntry + iIndex, pstEntry, sizeof(DIRECTORY_ENTRY));

    /// 루트 디렉터리에 씀
    if (kWrtieCluster(0, gs_vbTempBuffer) == FALSE) {
        return FALSE;
    }

    return TRUE;
}

/// 루트 디렉터리의 해당 인덱스에 디렉터리 엔트리를 반환
static BOOL kGetDirectoryEntryData(int iIndex, DIRECTORY_ENTRY *pstEntry) {
    DIRECTORY_ENTRY *pstRootEntry;

    /// 파일 시스템을 인식하지 못했거나 인덱스가 올바르지 않으면 실패
    if ((gs_stFileSystemManager.bMounted == FALSE) ||
        (iIndex < 0) || (iIndex >= FILE_SYSTEM_MAX_DIRECTORY_ENTRY_COUNT)) {
        return FALSE;
    }

    /// 루트 디렉터리를 읽음
    if (kReadCluster(0, gs_vbTempBuffer) == FALSE) {
        return FALSE;
    }

    /// 루트 디렉터리에 있는 해당 데이터를 갱신
    pstRootEntry = (DIRECTORY_ENTRY *) gs_vbTempBuffer;
    kMemCpy(pstEntry, pstRootEntry + iIndex, sizeof(DIRECTORY_ENTRY));
    return TRUE;
}

/// 루트 디렉터리에서 파일 이름이 일치하는 엔트리를 찾아서 인덱스를 반환
static int kFindDirectoryEntry(const char *pcFileName, DIRECTORY_ENTRY *pstEntry) {
    DIRECTORY_ENTRY *pstRootEntry;
    int i;
    int iLength;

    /// 파일 시스템을 인식하지 못앴으면 실패
    if (gs_stFileSystemManager.bMounted == FALSE) {
        return -1;
    }

    /// 루트 디렉터리를 읽음
    if (kReadCluster(0, gs_vbTempBuffer) == FALSE) {
        return -1;
    }

    iLength = kStrLen(pcFileName);
    /// 루트 디렉터리 안에서 루프를 돌면서 파일 이름이 일치하는지 엔트리를 반환
    pstRootEntry = (DIRECTORY_ENTRY *) gs_vbTempBuffer;
    for (i = 0; i < FILE_SYSTEM_MAX_DIRECTORY_ENTRY_COUNT; i++) {
        if (kMemCmp(pstRootEntry[i].vcFileName, pcFileName, iLength) == 0) {
            kMemCpy(pstRootEntry, pstRootEntry + i, sizeof(DIRECTORY_ENTRY));
            return i;
        }
    }

    return -1;
}

/// 파일 시스템의 정보를 반환
void kGetFileSystemInformation(FILESYSTEM_MANAGER *pstManager) {
    kMemCpy(pstManager, &gs_stFileSystemManager, sizeof(gs_stFileSystemManager));
}


//=========================================================================================
/// 고수준 함수(High Level Function)
//=========================================================================================
/// 비어 있는 핸들을 할당
static void *kAllocateFileDirectoryHandle(void) {
    int i;
    FILE *pstFile;

    /// 핸들 풀을 모두 검색하여 비어 있는 핸들을 반환
    pstFile = gs_stFileSystemManager.pstHandlePool;
    for (i = 0; i < FILE_SYSTEM_HANDLE_MAX_COUNT; i++) {
        /// 비어 있으면 반환
        if (pstFile->bType == FILE_SYSTEM_TYPE_FREE) {
            pstFile->bType = FILE_SYSTEM_TYPE_FILE;
            return pstFile;
        }

        /// 다음으로 이동
        pstFile++;
    }
    return NIL;
}

/// 사용한 핸들을 반환
static void kFreeFileDirectoryHandle(FILE *pstFile) {
    /// 전체 영역을 초기화
    kMemSet(pstFile, 0, sizeof(FILE));

    /// 비어 있느 타입으로 설정
    pstFile->bType = FILE_SYSTEM_TYPE_FREE;
}

/// 파일을 생성
static BOOL kCreateFile(const char *pcFileName, DIRECTORY_ENTRY *pstEntry,
                        int *piDirectoryEntryIndex) {
    DWORD dwCluster;

    /// 빈 클러스터를 찾아서 할당된 것으로 설정
    dwCluster = kFindFreeCluster();
    if ((dwCluster == FILE_SYSTEM_LAST_CLUSTER) ||
        (kSetClusterLinkData(dwCluster, FILE_SYSTEM_LAST_CLUSTER) == FALSE)) {
        return FALSE;
    }

    /// 빈 디렉터리 엔트리를 검색
    *piDirectoryEntryIndex = kFindFreeDirectoryEntry();
    if (*piDirectoryEntryIndex == -1) {
        /// 실패하면 할당받은 클러스터를 반환해야 함
        kSetClusterLinkData(dwCluster, FILE_SYSTEM_FREE_CLUSTER);
        return FALSE;
    }

    /// 디렉터리 엔트리를 설정
    kMemCpy(pstEntry->vcFileName, pcFileName, kStrLen(pcFileName) + 1);
    pstEntry->dwStartClusterIndex = dwCluster;
    pstEntry->dwFileSize = 0;

    /// 디렉터리 엔트리를 등록
    if (kSetDirectoryEntryData(*piDirectoryEntryIndex, pstEntry) == FALSE) {
        /// 실패하면 할당받은 클러스터를 반환해야 함
        kSetClusterLinkData(dwCluster, FILE_SYSTEM_FREE_CLUSTER);
        return FALSE;
    }

    return TRUE;
}

/// 파라미터로 넘어온 클러스터부터 파일의 끝까지 연결된 클러스터를 모두 반환
static BOOL kFreeClusterUntilEnd(DWORD dwClusterIndex) {
    DWORD dwCurrentClusterIndex;
    DWORD dwNextClusterIndex;

    /// 클러스터 인덱스 초기화
    dwCurrentClusterIndex = dwClusterIndex;

    while (dwCurrentClusterIndex != FILE_SYSTEM_LAST_CLUSTER) {
        /// 다음 클러스터의 인덱스를 가져옴
        if (kGetClusterLinkData(dwCurrentClusterIndex, &dwNextClusterIndex) == FALSE) {
            return FALSE;
        }

        /// 현재 클러스터를 빈 것으로 설정하여 해제
        if (kSetClusterLinkData(dwCurrentClusterIndex, FILE_SYSTEM_FREE_CLUSTER) == FALSE) {
            return FALSE;
        }

        /// 현재 클러스터 인덱스를 다음 클러스터의 인덱스로 바꿈
    }

    return TRUE;
}

/// 파일을 생성하거나 염
FILE *kOpenFile(const char *pcFileName, const char *pcMode) {
    DIRECTORY_ENTRY stEntry;
    int iDirectoryEntryOffset;
    int iFileNameLength;
    DWORD dwSecondCluster;

    FILE *pstFile;

    /// 파일 이름 검사
    iFileNameLength = kStrLen(pcFileName);
    if ((iFileNameLength > (sizeof(stEntry.vcFileName) - 1)) || (iFileNameLength == 0)) {
        return NIL;
    }

    /// 동기화
    kLock(&(gs_stFileSystemManager.stMutex));

    //=============================================================================================
    /// 파일이 먼저 존재하는지 확인하고, 없다면 옵션을 보고 파일을 생성
    //=============================================================================================
    iDirectoryEntryOffset = kFindDirectoryEntry(pcFileName, &stEntry);
    if (iDirectoryEntryOffset == -1) {

        /// 파일이 없다면 읽기(r, r+)옵션은 실패
        if (pcMode[0] == 'r') {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NIL;
        }
        /// 나머지 옵션은 파일을 생성
        if (kCreateFile(pcFileName, &stEntry, &iDirectoryEntryOffset) == FALSE) {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NIL;
        }
    }
        //=============================================================================================
        /// 파일의 내용을 비워야 하는 옵션이면 파일에 연결된 클러스터를 모주 제거하고 파일 크기를 0으로 설정
        //=============================================================================================
    else if (pcMode[0] == 'w') {
        /// 시작 클러스터의 다음 클러스터를 찾음
        if (kGetClusterLinkData(stEntry.dwStartClusterIndex, &dwSecondCluster)) {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NIL;
        }

        /// 시작 클러스터를 마지막 클러스터로 만듦
        if (kSetClusterLinkData(stEntry.dwStartClusterIndex, FILE_SYSTEM_LAST_CLUSTER) == FALSE) {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NIL;
        }

        /// 다음 클러스터부터 마지막 클러스터까지 모두 해젠
        if (kFreeClusterUntilEnd(dwSecondCluster) == FALSE) {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NIL;
        }

        /// 파일의 내용이 모두 비워졌으므로 크기를 0으로 설정
        stEntry.dwFileSize = 0;
        if (kSetDirectoryEntryData(iDirectoryEntryOffset, &stEntry) == FALSE) {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return NIL;
        }
    }

    //=============================================================================================
    /// 파일 핸들을 핟당 받아 데이터를 설정한 후 반환
    //=============================================================================================

    /// 파일 핸들을 핟당 받아 데이터를 설정
    pstFile = kAllocateFileDirectoryHandle();
    if (pstFile == NIL) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return NIL;
    }

    /// 파일 핸들에 파일 정보를 설정
    pstFile->bType = FILE_SYSTEM_TYPE_FILE;
    pstFile->stFileHandle.iDirectoryEntryOffset = iDirectoryEntryOffset;
    pstFile->stFileHandle.dwFileSize = stEntry.dwFileSize;

    pstFile->stFileHandle.dwStartClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwCurrentClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwPreviouClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwCurrentOffset = 0;


    /// 만약 추가 옵션(a)이 설정되어 있으면 파일의 끝으로 이동
    if (pcMode[0] == 'a') {
        kSeekFile(pstFile, 0, FILE_SYSTEM_SEEK_END);
    }

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return pstFile;
}

/// 파일을 읽어 버퍼로 복사
DWORD kReadFile(void *pvBuffer, DWORD dwSize, DWORD dwCount, FILE *pstFile) {
    DWORD dwTotalCount;
    DWORD dwReadCount;
    DWORD dwOffsetInCluster;
    DWORD dwCopySize;

    FILE_HANDLE *pstFileHandle;
    DWORD dwNextClusterIndex;

    /// 핸들 타입이 파일이 아니면 실패
    if ((pstFile == NIL) || (pstFile->bType != FILE_SYSTEM_TYPE_FILE)) {
        return 0;
    }
    pstFileHandle = &(pstFile->stFileHandle);

    /// 파일의 끝이거나 마지막 클러스터면 종료
    if ((pstFileHandle->dwCurrentOffset == pstFileHandle->dwFileSize) ||
        (pstFileHandle->dwCurrentClusterIndex == FILE_SYSTEM_LAST_CLUSTER)) {
        return 0;
    }

    /// 파일 끝과 비교해서 실제로 읽을 수 있는 값을 계산
    dwTotalCount = MIN(dwSize * dwCount, pstFileHandle->dwFileSize - pstFileHandle->dwCurrentOffset);

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// 계산된 값만큼 다 읽을 때까지 반복
    dwReadCount = 0;
    while (dwReadCount != dwTotalCount) {
        //=============================================================================================
        /// 클러스터를 읽어서 버퍼에 복사
        //=============================================================================================
        /// 현재 클래스터를 읽음
        if (kReadCluster(pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer) == FALSE) {
            break;
        }

        /// 클러스터 내에서 파일 포인터가 존재하는 오프셋을 계산
        dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILE_SYSTEM_CLUSTER_SIZE;

        /// 여러 클러스터에 걸쳐져 있다면 현재 클러스너에서 남은 만큼 읽고 다음 클러스터로 이동
        dwCopySize = MIN(FILE_SYSTEM_CLUSTER_SIZE - dwOffsetInCluster,
                         dwTotalCount - dwReadCount);
        kMemCpy((char *) pvBuffer + dwReadCount,
                gs_vbTempBuffer + dwOffsetInCluster, dwCopySize);

        /// 읽은 바이트 수와 파일 포인터의 위치를 갱신
        dwReadCount += dwCopySize;
        pstFileHandle->dwCurrentOffset += dwCopySize;

        //=============================================================================================
        /// 현재 클러스터를 끝까지 다 읽었으면 다음 클러스터로 이동
        //=============================================================================================
        if ((pstFileHandle->dwCurrentOffset % FILE_SYSTEM_CLUSTER_SIZE) == 0) {
            /// 현재 클러스터의 링크 데이터를 찾아 다음 클러스터를 얻음
            if (kGetClusterLinkData(pstFileHandle->dwCurrentClusterIndex,
                                    &dwNextClusterIndex) == FALSE) {
                break;
            }

            /// 클러스터 정보를 갱신
            pstFileHandle->dwPreviouClusterIndex = pstFileHandle->dwCurrentClusterIndex;
            pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        }
    }

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));

    /// 읽은 바이트 수를 반환
    return dwReadCount;
}

/// 루트 디렉터리에서 디렉터리 엔트리 값을 갱신
static BOOL kUpdateDirectoryEntry(FILE_HANDLE *pstFileHandle) {
    DIRECTORY_ENTRY stEntry;

    /// 디렉터리 엔트리 검색
    if ((pstFileHandle == NIL) || (kGetDirectoryEntryData(pstFileHandle->iDirectoryEntryOffset, &stEntry) == FALSE)) {
        return FALSE;
    }


    /// 파일 크기와 시작 클러스터를 변경
    stEntry.dwFileSize = pstFileHandle->dwFileSize;
    stEntry.dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;

    /// 변경된 디렉터리 엔트리를 설정
    if (kSetDirectoryEntryData(pstFileHandle->iDirectoryEntryOffset, &stEntry) == FALSE) {
        return FALSE;
    }

    return TRUE;
}


/// 버퍼의 데이터를 파일에 씀
DWORD kWriteFile(void *pvBuffer, DWORD dwSize, DWORD dwCount, FILE *pstFile) {
    DWORD dwWriteCount;
    DWORD dwTotalCount;
    DWORD dwOffsetInCluster;
    DWORD dwCopySize;
    DWORD dwAllocatedClusterIndex;
    DWORD dwNextClusterIndex;

    FILE_HANDLE *pstFileHandle;
    /// 핸들 타입이 파일이 아니면 실패
    if ((pstFile == NIL) || (pstFile->bType != FILE_SYSTEM_TYPE_FILE)) {
        return 0;
    }
    pstFileHandle = &(pstFile->stFileHandle);

    /// 총 바이트 수
    dwTotalCount = dwSize * dwCount;

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// 다 쓸 때까지 반복
    dwWriteCount = 0;
    while (dwWriteCount != dwTotalCount) {
        //=============================================================================================
        /// 현재 클러스터를 파일의 끝이면 클러스터를 할당하여 연결
        //=============================================================================================
        if (pstFileHandle->dwCurrentClusterIndex == FILE_SYSTEM_LAST_CLUSTER) {
            /// 빈 클러스터 검색
            dwAllocatedClusterIndex = kFindFreeCluster();
            if (dwAllocatedClusterIndex == FILE_SYSTEM_LAST_CLUSTER) {
                break;
            }

            /// 검색된 클러스터를 마지막 클러스터로 설정
            if (kSetClusterLinkData(dwAllocatedClusterIndex, FILE_SYSTEM_LAST_CLUSTER) == FALSE) {
                break;
            }

            ///  파일의 마지막 클러스터레 할당된 클러스터를 연결
            if (kSetClusterLinkData(pstFileHandle->dwPreviouClusterIndex,
                                    dwAllocatedClusterIndex) == FALSE) {
                /// 실패하면 할당된 클러스터를 해제
                kSetClusterLinkData(dwAllocatedClusterIndex, FILE_SYSTEM_FREE_CLUSTER);
                break;
            }

            /// 현재 클러스터를 할당된 것으로 변경
            pstFileHandle->dwCurrentClusterIndex = dwAllocatedClusterIndex;
            /// 새로 할당받았으니 임시 클러스터 버퍼를 0으로 채움
            kMemSet(gs_vbTempBuffer, 0, FILE_SYSTEM_LAST_CLUSTER);

        }

            //=============================================================================================
            /// 한 클러스터를 채우지 못하면 클러스터를 읽어서 임시 클러스터 버퍼로 복사
            //=============================================================================================
        else if (((pstFileHandle->dwCurrentOffset % FILE_SYSTEM_CLUSTER_SIZE) != 0) ||
                 ((dwTotalCount - dwWriteCount) < FILE_SYSTEM_CLUSTER_SIZE)) {

            /// 전체 클러스터를 덮어쓰는 경우가 아니면 부분만 덮어써야 하므로 현재 클러스터를 읽음
            if (kReadCluster(pstFileHandle->dwCurrentClusterIndex,
                             gs_vbTempBuffer) == FALSE) {
                break;
            }
        }

        /// 클러스터 내에서 파일 포인터가 존재하는 오프셋을 계산
        dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILE_SYSTEM_CLUSTER_SIZE;

        /// 여러 클러스터에 걸쳐져 있다면 현재 클러스터에서 남은 만큼 쓰고 다음 클러스터로 이동
        dwCopySize = MIN(FILE_SYSTEM_CLUSTER_SIZE - dwOffsetInCluster,
                         dwTotalCount - dwWriteCount);

        kMemCpy(gs_vbTempBuffer + dwOffsetInCluster, (char *) pvBuffer + dwWriteCount, dwCopySize);

        /// 임시 버퍼에 삽입된 값을 디스크에 씀
        if (kWrtieCluster(pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer) == FALSE) {
            break;
        }

        /// 쓴 바이트 수와 파일 포인터의 위치를 갱신
        dwWriteCount += dwCopySize;
        pstFileHandle->dwCurrentOffset += dwCopySize;

        //=============================================================================================
        /// 현재 클러스터를 끝까지 다 썼으면 다음 클러스터로 이동
        //=============================================================================================
        if ((pstFileHandle->dwCurrentOffset % FILE_SYSTEM_CLUSTER_SIZE) == 0) {
            /// 현재 클러스터의 링크 데이터로 다음 클러스터를 얻음
            if (kGetClusterLinkData(pstFileHandle->dwCurrentClusterIndex, &dwNextClusterIndex) == FALSE) {
                break;
            }

            /// 클러스터 정보를 갱신
            pstFileHandle->dwPreviouClusterIndex = pstFileHandle->dwCurrentClusterIndex;
            pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        }
    }

    //=============================================================================================
    /// 파일 크기가 변했다면 루트 디렉터리에 있는 디렉터리 엔트리 정보를 갱신
    //=============================================================================================
    if (pstFileHandle->dwFileSize < pstFileHandle->dwCurrentOffset) {
        pstFileHandle->dwFileSize = pstFileHandle->dwCurrentOffset;
        kUpdateDirectoryEntry(pstFileHandle);
    }

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return dwWriteCount;
}

/// 파일을 count만큼 0으로 채움
BOOL kWriteZero(FILE *pstFile, DWORD dwCount) {
    BYTE *pbBuffer;
    DWORD dwRemainCount;
    DWORD dwWriteCount;

    /// 핸들이 NIL이면 실패
    if (pstFile == NIL) {
        return FALSE;
    }

    /// 0으로 채움
    kMemSet(pbBuffer, 0, FILE_SYSTEM_CLUSTER_SIZE);
    dwRemainCount = dwCount;

    // 클러스터 단위로 반복해서 쓰기 수랭
    while (dwRemainCount != 0) {
        dwWriteCount = MIN(dwRemainCount, FILE_SYSTEM_CLUSTER_SIZE);
        if (kWriteFile(pbBuffer, 1, dwWriteCount, pstFile) != dwWriteCount) {
            kFreeMemory(pbBuffer);
            return FALSE;
        }
        dwRemainCount -= dwWriteCount;
    }

    kFreeMemory(pbBuffer);
    return TRUE;
}

/// 파일 포인터의 위치를 이동
int kSeekFile(FILE *pstFile, int iOffset, int iOrigin) {
    DWORD dwRealOffset;
    DWORD dwClusterOffsetToMove;
    DWORD dwCurrentClusterOffset;
    DWORD dwLastClusterOffset;
    DWORD dwMoveCount;
    DWORD i;
    DWORD dwStartClusterIndex;
    DWORD dwPreviousClusterIndex;
    DWORD dwCurrentClusterIndex;

    FILE_HANDLE *pstFileHandle;

    /// 핸들 타입이 파일이 아니면 실패
    if ((pstFile == NIL) || (pstFile->bType != FILE_SYSTEM_TYPE_FILE)) {
        return 0;
    }
    pstFileHandle = &(pstFile->stFileHandle);

    //=============================================================================================
    /// Origin & Offset을 조합해서 파일 시작을 기준으로 파일 포인터를 옮겨야 할 위치 계산
    //=============================================================================================
    /// 옵션에 따라서 실제 위치를 계산
    /// 음수면 파일의 시작 반향으로 이동하며, 양수면 파일 의 끝 방향으로 이동
    switch (iOrigin) {

        /// 파일의 시작을 기준으로 이동
        case FILE_SYSTEM_SEEK_SET:
            if (iOffset <= 0) {
                dwRealOffset = 0;
            } else {
                dwRealOffset = iOffset;
            }
            break;

            /// 현재 위치를 기준으로 이동
        case FILE_SYSTEM_SEEK_CUR:
            /// 이동할 오프셋이 음수이고, 현재 파일 포인터의 값보다 크면
            /// 더 이상 갈 수 없으므로 파일의 처음으로 이동

            if ((iOffset < 0) && (pstFileHandle->dwCurrentOffset <= (DWORD) -iOffset)) {
                dwRealOffset = 0;
            } else {
                dwRealOffset = pstFileHandle->dwCurrentOffset + iOffset;
            }
            break;

            /// 파일의 끝부분을 기준으로 이동
        case FILE_SYSTEM_SEEK_END:
            if ((iOffset < 0) && (pstFileHandle->dwFileSize <= (DWORD) -iOffset)) {
                dwRealOffset = 0;
            } else {
                dwRealOffset = pstFileHandle->dwFileSize + iOffset;
            }
            break;
    }

    //=============================================================================================
    /// 파일을 구성하는 클러스터의 개수와 현재 파일 포인터의 위치를 고려하여
    /// 옮겨질 파일 포인터가 위치한는 클러스터까지 클러스터 링크를 탐색
    //=============================================================================================
    /// 파일의 마지막 클러스터의 오프셋
    dwLastClusterOffset = pstFileHandle->dwFileSize / FILE_SYSTEM_CLUSTER_SIZE;
    /// 파일 포인터가 옮겨질 위치의 클러스터 오프셋
    dwClusterOffsetToMove = dwRealOffset / FILE_SYSTEM_CLUSTER_SIZE;
    /// 현재 파일 포인턱 있는 클러스터 오프셋
    dwCurrentClusterOffset = pstFileHandle->dwCurrentOffset / FILE_SYSTEM_CLUSTER_SIZE;

    /// 이동하는 클러스터의 위치가 파일의 마지막 클러스터 오프셋을 넘어서면
    /// 현재 클러스터에서 마지막 파일까지 이동한 후 Write 함수를 이용해서 공백으로 나머지를 채움
    if (dwLastClusterOffset < dwClusterOffsetToMove) {
        dwMoveCount = dwLastClusterOffset - dwCurrentClusterOffset;
        dwStartClusterIndex = pstFileHandle->dwCurrentClusterIndex;
    }
        /// 이동하는 클러스터의 위치가 현재 클러스터와 같거나 다음에 위치해 있다면
        /// 현재 클러스터를 기준으로 차이만큼 이동하면 된다.
    else if (dwCurrentClusterOffset <= dwClusterOffsetToMove) {
        dwMoveCount = dwClusterOffsetToMove - dwCurrentClusterOffset;
        dwStartClusterIndex = pstFileHandle->dwCurrentClusterIndex;
    }
        /// 이동하는 클러스터의 위치가 현재 클러스터 이전에 있다면, 첫 번째 클러스터부터 이동하면서 검색
    else {
        dwMoveCount = dwClusterOffsetToMove;
        dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;
    }

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// 클러스터를 이동
    dwCurrentClusterIndex = dwStartClusterIndex;
    for (i = 0; i < dwMoveCount; i++) {
        /// 값을 보관
        dwPreviousClusterIndex = dwCurrentClusterIndex;

        /// 다음 클러스터의 인덱스를 읽음
        if (kGetClusterLinkData(dwPreviousClusterIndex, &dwCurrentClusterIndex) == FALSE) {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return -1;
        }
    }

    /// 클러스터를 이동했으면 클러스터 정보를 갱신
    if (dwMoveCount > 0) {
        pstFileHandle->dwPreviouClusterIndex = dwPreviousClusterIndex;
        pstFileHandle->dwCurrentClusterIndex = dwCurrentClusterIndex;
    }
        /// 첫 번째 클러스터로 이동하는 경우는 핸들의 클러스터 값을 시작 클러스터로 설정
    else if (dwStartClusterIndex == pstFileHandle->dwStartClusterIndex) {
        pstFileHandle->dwPreviouClusterIndex = pstFileHandle->dwStartClusterIndex;
        pstFileHandle->dwCurrentClusterIndex = pstFileHandle->dwStartClusterIndex;
    }

    //=============================================================================================
    /// 파일 포인터를 갱신하고 파일 오프셋이 파일의 크기를 넘었다면 나머지 부분을 0으로 채워서 파일의 크기를 늘림
    //=============================================================================================
    /// 실제 파일의 크기를 넘어서 파일 포인터가 이동했다면, 파일 끝에서부터 남은 크기만큼 0으로 채움
    if (dwLastClusterOffset < dwClusterOffsetToMove) {
        pstFileHandle->dwCurrentOffset = pstFileHandle->dwFileSize;
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));

        /// 남은 크기만큼 0으로 채움
        if (kWriteZero(pstFile, dwRealOffset - pstFileHandle->dwFileSize) == FALSE) {
            return 0;
        }
    }
    pstFileHandle->dwCurrentOffset = dwRealOffset;

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return 0;
}

/// 파일을 닫음
int kCloseFile(FILE *pstFile) {
    /// 핸들 타입이 파일이 아니면 실패
    if ((pstFile == NIL) || (pstFile->bType != FILE_SYSTEM_TYPE_FILE)) {
        return -1;
    }

    /// 핸들을 반환
    kFreeFileDirectoryHandle(pstFile);
    return 0;
}

/// 핸들 풀을 검사하며 파일이 열려 있는지를 확인
BOOL kIsFileOpended(const DIRECTORY_ENTRY *pstEntry) {
    int i;
    FILE *pstFile;

    /// 핸들 풀의 시작 어드레스부터 끝까지 열린 파일만 검색
    pstFile = gs_stFileSystemManager.pstHandlePool;
    for (i = 0; i < FILE_SYSTEM_HANDLE_MAX_COUNT; i++) {
        /// 파일 타입 중에서 시작 클러스터가 일치하면 반환
        if ((pstFile[i].bType == FILE_SYSTEM_TYPE_FILE) &&
            (pstFile[i].stFileHandle.dwStartClusterIndex == pstEntry->dwStartClusterIndex)) {
            return TRUE;
        }
    }
    return FALSE;
}

/// 파일을 삭제
int kRemoveFile(const char *pcFileName) {
    DIRECTORY_ENTRY stEntry;
    int iDirectoryEntryOffset;
    int iFileNameLength;

    /// 파일 이름 검사
    iFileNameLength = kStrLen(pcFileName);
    if ((iFileNameLength > (sizeof(stEntry.vcFileName) - 1)) || (iFileNameLength == 0)) {
        return NIL;
    }

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// 파일이 존재하는지 확인
    iDirectoryEntryOffset = kFindDirectoryEntry(pcFileName, &stEntry);
    if (iDirectoryEntryOffset == -1) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    /// 다른 태스크에서 해당 파일을 열고 있는지 핸들 풀을 검색하여 확인 파일이 열려 있으면 삭제할 수 없음
    if (kIsFileOpended(&stEntry) == TRUE) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    /// 파일을 구성하는 클러스터를 모두 해제
    if (kFreeClusterUntilEnd(stEntry.dwStartClusterIndex) == FALSE) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }

    /// 디렉터리 엔트리를 빈 것으로 설정
    kMemSet(&stEntry, 0, sizeof(stEntry));
    if (kSetDirectoryEntryData(iDirectoryEntryOffset, &stEntry) == FALSE) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return -1;
    }
    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return 0;

}

/// 디렉터리를 엶
DIR *kOpenDirectory(const char *pcDirectoryName) {
    DIR *pstDirectory;
    DIRECTORY_ENTRY *pstDirectoryBuffer;

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// 루트 디렉터리밖에 없으므로 디렉터리 이름은 무시하고 핸들만 할당받아서 반환
    pstDirectory = kAllocateFileDirectoryHandle();
    if (pstDirectory == NIL) {
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return NIL;
    }

    /// 루트 디렉터리를 저장할 버퍼를 할당
    pstDirectoryBuffer = (DIRECTORY_ENTRY *) kAllocateMemory(FILE_SYSTEM_CLUSTER_SIZE);
//    if (pstDirectory == NIL) {
    if (pstDirectoryBuffer == NIL) {
        /// 실패하면 핸들을 반환해야 함
        kFreeFileDirectoryHandle(pstDirectory);
        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return NIL;
    }

    if (kReadCluster(0, (BYTE *) pstDirectoryBuffer) == FALSE) {
        /// 실패하면 핸들과 메모리를 반환해야 함
        kFreeFileDirectoryHandle(pstDirectory);
        kFreeMemory(pstDirectoryBuffer);

        /// 동기화 처리
        kUnlock(&(gs_stFileSystemManager.stMutex));
        return NIL;
    }

    /// 디렉터리 타입으로 설정하고 현재 디렉터리 엔트리 오프셋을 초기화
    pstDirectory->bType = FILE_SYSTEM_TYPE_DIRECTORY;
    pstDirectory->stDirectoryHandle.iCurrentOffset = 0;
    pstDirectory->stDirectoryHandle.pstDirectoryBuffer = pstDirectoryBuffer;

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return pstDirectory;
}

/// 디렉터리 엔트리를 반환하고 다음으로 이동
struct kDirectoryEntryStruct *kReadDirectory(DIR *pstDirectory) {
    DIRECTORY_HANDLE *pstDirectoryHandle;
    DIRECTORY_ENTRY *pstEntry;

    /// 핸들 타입이 디렉터리가 아니면 실패
    if ((pstDirectory == NIL) || (pstDirectory->bType != FILE_SYSTEM_TYPE_DIRECTORY)) {
        return NIL;
    }
    pstDirectoryHandle = &(pstDirectory->stDirectoryHandle);

    /// 오프셋 범위가 클러스터에 존재하는 최댓값을 넘어서면 실패
    if ((pstDirectoryHandle->iCurrentOffset < 0) ||
        (pstDirectoryHandle->iCurrentOffset >= FILE_SYSTEM_MAX_DIRECTORY_ENTRY_COUNT)) {
        return NIL;
    }

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// 루트 디렉터리에 있는 최대 디렉터리 엔트리의 개수만큼 검색
    pstEntry = pstDirectoryHandle->pstDirectoryBuffer;
    while (pstDirectoryHandle->iCurrentOffset < FILE_SYSTEM_MAX_DIRECTORY_ENTRY_COUNT) {
        /// 파일이 존재하면 해당 디렉터리 엔트리를 반환
        if (pstEntry[pstDirectoryHandle->iCurrentOffset].dwStartClusterIndex != 0) {
            /// 동기화 처리
            kUnlock(&(gs_stFileSystemManager.stMutex));
            return &(pstEntry[pstDirectoryHandle->iCurrentOffset++]);
        }
        pstDirectoryHandle->iCurrentOffset++;
    }

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
    return NIL;
}

/// 디렉터리 포인터를 디렉터리의 처음으로 이동
void kRewindDirectory(DIR *pstDirectory) {
    DIRECTORY_HANDLE *pstDirectoryHandle;

    /// 핸들 타입이 디렉터리가 아니면 실패
    if ((pstDirectory == NIL) || (pstDirectory->bType != FILE_SYSTEM_TYPE_DIRECTORY)) {
        return;
    }
    pstDirectoryHandle = &(pstDirectory->stDirectoryHandle);

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// 디렉터리 엔트리의 포인터만 0으로 바꿔줌
    pstDirectoryHandle->iCurrentOffset = 0;

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));
}


/// 열린 디렉터리를 닫음
int kCloseDirectory(DIR *pstDirectory) {
    DIRECTORY_HANDLE *pstDirectoryHandle;

    /// 핸들 타입이 디렉터리가 아니면 실패
    if ((pstDirectory == NIL) || (pstDirectory->bType != FILE_SYSTEM_TYPE_DIRECTORY)) {
        return -1;
    }

    pstDirectoryHandle = &(pstDirectory->stDirectoryHandle);

    /// 동기화 처리
    kLock(&(gs_stFileSystemManager.stMutex));

    /// 루트 디렉터리의 버퍼를 해제하고 핸들을 반환
    kFreeMemory(pstDirectoryHandle->pstDirectoryBuffer);

    kFreeFileDirectoryHandle(pstDirectory);

    /// 동기화 처리
    kUnlock(&(gs_stFileSystemManager.stMutex));

    return 0;
}