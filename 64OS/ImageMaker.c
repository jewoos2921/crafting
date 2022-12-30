//
// Created by jewoo on 2022-12-29.
//

/* 디스크 이미지를 만들어줌 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define BYTESOFSECTOR 512

/* 함수 선언 */
int AdjustInSectorSize(int iFd, int iSourceSize);

void WriteKernelInformation(int iTargetFd, int iKernelSectorCount);

int CopyFile(int iSourceFd, int iTargetFd);


// main 함수
int main(int argc, char *argv[]) {
    int iSourceFd;
    int iTargetFd;
    int iBootLoaderSize;
    int iKernel32SectorCount;
    int iSouceSize;

    // 커맨드 라인 온션 검사
    if (argc < 3) {

    }

}

// 현재 위치부터 512 바이트 배수 위치까지 맞추어 0x00으로 채움
int AdjustInSectorSize(int iFd, int iSourceSize) {
    int i;
    int iAdjustSizeToSector;
    int iSectorCount;
    char cCh;

    iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
    cCh = 0x00;

    if (iAdjustSizeToSector != 0) {
        iAdjustSizeToSector = 512 - iAdjustSizeToSector;
        printf("[INFO] File size [%lu] and fill [%u[ byte\n", iSourceSize,
               iAdjustSizeToSector);
        for (i = 0; i < iAdjustSizeToSector; ++i) {
            write(iFd, &cCh, 1);
        }
    } else {
        printf("[INFO] File size is aligned 512 byte\n");
    }

    // 섹터 수를 되돌려줌
    iSectorCount = (iSourceSize + iAdjustSizeToSector) / BYTESOFSECTOR;
    return iSectorCount;
}

// 부트 로더에 커널에 대한 정보를 삽입
void WriteKernelInformation(int iTargetFd, int iKernelSectorCount) {

}

// 소스파일 iSourceFd의 내용을 목표파일 iTargetFd에 복사하고 그 크기를 되돌려줌
int CopyFile(int iSourceFd, int iTargetFd) {

}

