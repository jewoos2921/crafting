//
// Created by jewoo on 2022-12-23.
//

// 전체 메인 루틴

#include <stdio.h>
#include "getSource.h"

int main() {
    char fileName[30]; // 소스 프로그램 파일 이름
    printf("enter source file name\n");
    scanf("%s", fileName);
    if (!openSource(fileName)) { /* 소스 프로그램 파일 열기 */
        return 0; /* 오픈 실패시 리턴 */
    }

    if (compile()) {    /* 컴파일 진행 */
        execute(); /* 오류가 없다면 즉시 실행 */
    }
    closeSource(); /* 소스 프로그램 파일 닫기 */

}