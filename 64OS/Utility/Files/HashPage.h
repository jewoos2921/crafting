//
// Created by jewoo on 2023-01-22.
//
//

// 해시 페이지를 관리하는 함수를 정의한 파일
#ifndef CRAFTING_HASHPAGE_H
#define CRAFTING_HASHPAGE_H


#include "BaseHeader.h"

typedef int Key;
typedef char Value[12];
typedef struct sHashPage HashPage;
typedef struct sHashRecord HashRecord;
typedef struct sHashHeader HashHeader;

struct sHashRecord {
    Key key;
    Value value;
};

struct sHashPage {
    int iPageNo;                     /// 이 페이지의 번호
    int iNumOfRecords;               /// 이 페이지에 들어 있는 레코드 수
    int iPrevPageNo;                 /// 이전 페이지 번호
    int iNextPageNo;                 /// 다음 오버플로 페이지 번호
    HashRecord *hpRecArray;         /// 레코드에 대한 배열
};

struct sHashHeader {
    int iMaxAddress;
};

void HInitHashHeader(int iMaxAddress);

void HInitHasHPage(HashPage *Page, int iPageNo);

void hLoadHashHeaderPage(void);

void hSaveHashHeaderPage(void);

BOOL hReadHashPage(int iPageNo, HashPage *Page);

BOOL hWriteHashPage(int iPageNo, HashPage *Page);

int hNewHashPage(void);

void hFreeHashPage(int iVictimHashPageNo);


#endif //CRAFTING_HASHPAGE_H
