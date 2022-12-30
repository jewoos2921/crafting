//
// Created by jewoo on 2022-12-29.
//

// 보호 모드 커널용 공용 헤더 파일
#ifndef CRAFTING_TYPES_H
#define CRAFTING_TYPES_H


#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int
#define QWORD unsigned long
#define BOOL unsigned short

#define TRUE 1
#define FALSE 0
#define NIL 0

#pragma pack (push, 1)
// 비디오 모드 중 텍스트 모드 화면을 구성하는 자료구조
typedef struct kCharactorStruct {
    BYTE bCharactor;
    BOOL bAttribute;
} CHARACTER;

#pragma pack (pop)

#endif //CRAFTING_TYPES_H
