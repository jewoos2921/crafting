//
// Created by jewoo on 2023-01-19.
//

#ifndef CRAFTING_HARDDISK_H
#define CRAFTING_HARDDISK_H

#include "Types.h"
#include "Synchronization.h"


/// 매크로
/// 첫 번째 PATA 포트 (Primary PATA Port)와 두 번째  PATA 포트(Secondary PATA Port)의 정보
#define HDD_PORT_PRIMARY_BASE                                                                       0x1F0
#define HDD_PORT_SECONDARY_BASE                                                                     0x170

/// 포트 인덱스에 관련된 매크로
#define HDD_PORT_INDEX_DATA                                                                         0x00
#define HDD_PORT_INDEX_SECTOR_COUNT                                                                 0x02
#define HDD_PORT_INDEX_SECTOR_NUMBER                                                                0x03
#define HDD_PORT_INDEX_CYLINDER_LSB                                                                 0x04
#define HDD_PORT_INDEX_CYLINDER_MSB                                                                 0x05
#define HDD_PORT_INDEX_DRIVE_AND_HEAD                                                               0x06
#define HDD_PORT_INDEX_STATUS                                                                       0x07
#define HDD_PORT_INDEX_COMMAND                                                                      0x07
#define HDD_PORT_INDEX_DIGITAL_OUTPUT                                                               0x206

/// 커맨드 레지스터에 관련된 매크로
#define HDD_COMMAND_READ                                                                            0x20
#define HDD_COMMAND_WRITE                                                                           0x30
#define HDD_COMMAND_IDENTIFY                                                                        0xEC

/// 상태 레지스터에 관련된 매크로
#define HDD_STATUS_ERROR                                                                            0x01
#define HDD_STATUS_INDEX                                                                            0x02
#define HDD_STATUS_CORREXTED_DATA                                                                   0x04
#define HDD_STATUS_DATA_REQUEST                                                                     0x08
#define HDD_STATUS_SEEK_COMPLETE                                                                    0x10
#define HDD_STATUS_WRITE_FAULT                                                                      0x20
#define HDD_STATUS_READY                                                                            0x40
#define HDD_STATUS_BUSY                                                                             0x80

/// 드라이버/헤드 레지스터에 관련된 매크로
#define HDD_DRIVE_AND_HEAD_LBA                                                                      0xE0
#define HDD_DRIVE_AND_HEAD_SLAVE                                                                    0x10

/// 디지털 출력 레지스터에 관련된 매크로
#define HDD_DIGITAL_OUTPUT_RESET                                                                    0x04
#define HDD_DIGITAL_OUTPUT_DISABLE_INTERRUPT                                                        0x01

/// 하드 디스크의 응답을 대기하는 시간(millisecond)
#define HDD_WAIT_TIME                                                                               500

/// 한 번에 HDD에 읽거나 쓸 수 있는 섹터의 수
#define HDD_MAX_BULK_SECTOR_COUNT                                                                   256


/// 구조체
/// 1바이트로 정렬
#pragma pack(push, 1)

typedef struct kHDDInformationStruct {
    /// 설정 값
    WORD wConfiguration;

    /// 실린더 수
    WORD wNumberOfCylinder;
    WORD wReserved1;

    /// 헤드 수
    WORD wNumberOfHead;
    WORD wUniformattedBytesPerTrack;
    WORD wUnifromattedBytesPerSector;

    /// 실린더당 섹터 수
    WORD wNumberOfSectorPerCylinder;
    WORD wInterSectorGap;
    WORD wBytesInPhaseLock;
    WORD wNumberOfVendorUniqueStatusWord;

    /// 하드 디스크의 시리얼 넘버
    WORD vwSerialNumber[10];
    WORD wControllerType;
    WORD wBufferSize;
    WORD wNumberOfECCBytes;
    WORD vwFirmwareRevision[4];

    /// 하드 디스크의 모델 번호
    WORD vwModelNumber[20];
    WORD vwReserved2[13];

    /// 디스크의 총 섹터 수
    DWORD dwTotalSectors;
    WORD vwReserved3[196];
} HDD_INFORMATION;

#pragma pack(pop)


typedef struct kHDDManagerStruct {
    /// HDD 존재 여부와 쓰기를 수행할 수 있는지 여부
    BOOL bHDDDetected;
    BOOL bCanWrite;

    /// 인터럽트 발생 여부와 동기화 객체
    volatile BOOL bPrimaryInterruptOccur;
    /// 메모리 맵으로 제어되는 입출력장치는 volatile
    volatile BOOL bSecondaryInterruptOccur;
    MUTEX stMutex;
    /// HDD 정보
    HDD_INFORMATION stHDDInformation;
} HDD_MANAGER;

BOOL kInitializeHDD(void);

BOOL kReadHDDInformation(BOOL bPRimary, BOOL bMaster,
                         HDD_INFORMATION *pstHDDInformation);


int kReadHDDSector(BOOL bPRimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
                   char *pcBuffer);

int kWriteHDDSector(BOOL bPRimary, BOOL bMaster, DWORD dwLBA, int iSectorCount,
                    char *pcBuffer);

void kSetHDDInterruptFlag(BOOL bPrimary, BOOL bFlag);

static void kSwapByteInWord(WORD *pwData, int iWordCount);

static BYTE kReadHDDStatus(BOOL bPRimary);

static BOOL kIsHDDBusy(BOOL bPRimary);

static BOOL kIsHDDReady(BOOL bPRimary);

static BOOL kWaitForHDDNoBusy(BOOL bPRimary);

static BOOL kWaitForHDDReady(BOOL bPRimary);

static BOOL kWaitForHDDInterrupt(BOOL bPRimary);


#endif //CRAFTING_HARDDISK_H
