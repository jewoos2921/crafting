//
// Created by jewoo on 2022-12-30.
//
// 페이지 소스 파일
// IA-32e 모드 커널용 페이지 테이블을 생성
// 64GB까지 물리 주소를 선형 주소와 1:1로 매핑하는 역할을 담당
// 페이지 테이블의 위치와 순서는
// PML4 테이블 -> 페이지 디렉터리 포인터 테이블 -> 페이지 디렉터리 순이며
// 각각 0x100000~0x101000, 0x101000~0x102000, 0x102000~0x142000의 영역에서 생김

#include "Page.h"

// IA-32e 모드 커널을 위한 페이지 테이블 생성
void kInitializePageTables(void) {
    PML4TENTRY *pstPML4TENTRY;
    PDPTENTRY *pstPDPTENTRY;
    PDENTRY *pstPDENTRY;
    DWORD dwMappingAddress;
    int i;

    // PML4 테이블 생성
    // 첫 번째 엔트리외에 나머지는 모두 0으로 초기화
    pstPML4TENTRY = (PML4TENTRY *) 0x100000;
    kSetPageEntryData(&(pstPML4TENTRY[0]), 0x00, 0x101000, PAGE_FLAGS_DEFAULT, 0);
    for (i = 1; i < PAGE_MAXENTRYCOUNT; ++i) {
        kSetPageEntryData(&(pstPML4TENTRY[i]), 0, 0, 0, 0);
    }

    // 페이지 디렉터리 포인터 테이블 생성
    // 하나의 PDPT로 512GB까지 매핑 가능하므로 하나로 충분함
    // 64개의 엔트리를 설정하여 64GB까지 매핑함
    pstPDPTENTRY = (PDPTENTRY *) 0x101000;
    for (i = 1; i < 64; ++i) {
        kSetPageEntryData(&(pstPDPTENTRY[i]), 0, 0x102000 + (i * PAGE_TABLESIZE),
                          PAGE_FLAGS_DEFAULT, 0);
    }

    for (i = 64; i < PAGE_MAXENTRYCOUNT; ++i) {
        kSetPageEntryData(&(pstPDPTENTRY[i]), 0, 0, 0, 0);
    }

    // 페이지 디렉터리 테이블 생성
    // 하나의 페이지 디렉터리가 1GB까지 매핑 가능
    // 여유있게 64개의 페이지 디렉터리를 생성하여 총 64GB까지 지원
    pstPDENTRY = (PDENTRY *) 0x102000;
    dwMappingAddress = 0;
    for (i = 0; i < PAGE_MAXENTRYCOUNT * 64; ++i) {
        // 32 비트로의 상위 어드레스를 표현할 수 없으므로, MB 단위로 계산한 다음,
        // 최종 결과를 다시 4KB로 나누어 32비트 이상의 어드레스르 계산함
        kSetPageEntryData(&(pstPDENTRY[i]), (i * (PAGE_DEFAULTSIZE >> 20)) >> 12,
                          dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0);
        dwMappingAddress += PAGE_DEFAULTSIZE;
    }

}

// 페이지 엔트리에 기준 주소와 속성 플래그를 설정
void kSetPageEntryData(PTENTRY *pstEntry, unsigned int dwUpperBaseAddress, unsigned int dwLowerBaseAddress,
                       unsigned int dwLowerFlags, unsigned int dwUpperFlags) {
    pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
    pstEntry->dwUpperBaseAddressAndEXB = (dwUpperBaseAddress & 0xFF) | dwUpperFlags;
}
