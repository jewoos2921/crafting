//
// Created by jewoo on 2022-12-29.
//
// C 코드 엔트리 포인트 파일
#include "Types.h"

void kPrintString(int iX, int iY, const char *pcString);

//int main() {
//    kPrintString(0, 3, "C Language Kernel Started~!!!");
//
//    while (1);
//}

// 문자열 출력 함수
void kPrintString(int iX, int iY, const char *pcString) {
    CHARACTER *pstScreen = (CHARACTER *) 0xB8000;

    pstScreen += (iY * 80) + iX;
    for (int i = 0; pcString[i] != 0; ++i) {
        pstScreen[i].bCharactor = pcString[i];
    }
}