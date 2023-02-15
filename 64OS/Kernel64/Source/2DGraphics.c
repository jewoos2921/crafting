//
// Created by jewoo on 2023-02-15.
//

#include "2DGraphics.h"
#include "VBE.h"
#include "Font.h"
#include "Utility.h"

/// 점 그리기
inline void kDrawPixel(int iX, int iY, COLOR stColor) {

}

/// 직선 그리기
void kDrawLine(int iX1, int iY1, int iX2, int iY2, COLOR stColor) {

}

/// 사각형 그리기
void kDrawRect(int iX1, int iY1, int iX2, int iY2, COLOR stColor, BOOL bFill) {

}

/// 원 그리기
void kDrawCircle(int iX, int iY, int iRadius, COLOR stColr, BOOL bFill) {

}

/// 문자 출력
void kDrawText(int iX, int iY, COLOR stColor, COLOR stBackgroundColor,
               const char *pcString, int iLength) {
    int iCurrentX, iCurrentY;
    int i, j, k;
    BYTE bBitmask;
    int iBitmaskStartIndex;
    VBEMO
}