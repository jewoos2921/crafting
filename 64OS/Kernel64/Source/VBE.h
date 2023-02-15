//
// Created by jewoo on 2023-02-15.
//

#ifndef CRAFTING_VBE_H
#define CRAFTING_VBE_H

#include "Types.h"

// 매크로
/// 모드 정보 블록이 저장된 어드레스
#define VBE_MODE_INFO_BLOCK_ADDRESS             0x7E00

/// 그래픽 모드로 시작하는 플래그가 저장된 어드레스
#define VBE_START_GRAPHIC_MODE_FLAG_ADDRESS     0x7C0A

// 구조체
// 1바이트로 정렬
#pragma pack(push, 1)

/// VBE에서 정의한 모드 정보 블록(ModeInfoBlock) 자료구조, 256바이트
typedef struct kVBEInfoBlockStruct {

    //=======================================================================================================
    /// 모든 VBE 버전에 공통인 부분
    //=======================================================================================================

    WORD wModeAttribute;        /// 모드의 속성
    BYTE bWinAAttribute;        /// 윈도우 A의 속성
    BYTE bWinBAttribute;        /// 윈도우 B의 속성
    WORD wWinGranulity;         /// 윈도우의 가중치
    WORD wWinSize;              /// 윈도우의 크기
    WORD wWinASegment;          /// 윈도우 A가 시작하는 세그먼트 어드레스
    WORD wWinBSegment;          /// 윈도우 B가 시작하는 세그먼트 어드레스
    DWORD dwWinFuncPtr;         /// 윈도우 관련 함수의 포인터
    WORD wBytesPerScanLine;     /// 화면 스캔 라인당 바이트 수

    //=======================================================================================================
    /// VBE 버전 1.2 이상 공통인 부분
    //=======================================================================================================

    WORD wXResolution;          /// X축 픽셀 수 또는 문자 수
    WORD wYResolution;          /// Y축 픽셀 수 또는 문자 수
    BYTE bXCharSize;            /// 한 문자의 X축 픽셀 수
    BYTE bYCharSize;            /// 한 문자의 Y축 픽셀 수
    BYTE bNumberOfPlane;        /// 메모리 플레인 수
    BYTE bBitsPerPixel;         /// 한 픽셀을 구성하는 비트 수
    BYTE bNumberOfBanks;        /// 뱅크 수
    BYTE bMemoryMode;           /// 비디오 메모리 구성
    BYTE bBankSize;             /// 뱅크의 크기(KB)
    BYTE bNumberOfImagePages;   /// 이미지 페이지 개수
    BYTE bReserved;             /// 페이지 기능을 위해 예약된 영역

    /// 다이렉트 컬러(Direct Color)에 관련된 필드
    BYTE bRedMaskSize;              /// 빨간색 필드가 차지하는 크기
    BYTE bRedFieldPosition;         /// 빨간색 필드의 위치
    BYTE bGreenMaskSize;            /// 초록색 필드가 차지하는 크기
    BYTE bGreenFieldPosition;       /// 초록색 필드의 위치
    BYTE bBlueMaskSize;             /// 파란색 필드가 차지하는 크기
    BYTE bBlueFieldPosition;        /// 파란색 필드의 위치
    BYTE bReservedMaskSize;         /// 예약된 필드가 차지하는 크기
    BYTE bReservedFieldPosition;    /// 예약된 필드의 위치

    BYTE bDirectColorModeInfo;         /// 다이렉트 컬러 모드의 정보

    //=======================================================================================================
    /// VBE 버전 2.0 이상 공통인 부분
    //=======================================================================================================
    DWORD dwPhysicalBasePointer;     /// 선형 프레임 버퍼 메모리의 시작 어드레스
    DWORD dwReserved1;               /// 예약된 필드
    DWORD dwReserved2;               ///

    //=======================================================================================================
    /// VBE 버전 3.0 이상 공통인 부분
    //=======================================================================================================
    WORD wLinearBytesPerScanLine;         /// 선형 프레임 버퍼 모드의
                                          /// 화면 스캔 라인당 바이트 수
    BYTE bBankNumberOfImagePages;         /// 뱅크 모인일 때 이미지 페이지 수
    BYTE bLinearNumberOfImagePages;       /// 선형 프레임 버퍼 모드일 때 이미지 페이지 수

    /// 선형 프레임 버퍼 모드일때 다이렉트 컬러에 관련된 필드
    BYTE bLinearRedMaskSize;              /// 빨간색 필드가 차지하는 크기
    BYTE bLinearRedFieldPosition;         /// 빨간색 필드의 위치
    BYTE bLinearGreenMaskSize;            /// 초록색 필드가 차지하는 크기
    BYTE bLinearGreenFieldPosition;       /// 초록색 필드의 위치
    BYTE bLinearBlueMaskSize;             /// 파란색 필드가 차지하는 크기
    BYTE bLinearBlueFieldPosition;        /// 파란색 필드의 위치
    BYTE bLinearReservedMaskSize;         /// 예약된 필드가 차지하는 크기
    BYTE bLinearReservedFieldPosition;    /// 예약된 필드의 위치
    DWORD dwMaxPixelClock;                /// 픽셀 클록의 최댓값(Hz)

    BYTE vbReserved[189];                 /// 나머지 영역
} VBEMODE_INFOBLOCK;

#pragma pack(pop)

// 함수
VBEMODE_INFOBLOCK *kGetVBEModeInfoBlock(void);

#endif //CRAFTING_VBE_H
