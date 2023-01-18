//
// Created by jewoo on 2023-01-18.
//

#include "Hashing.h"

FILE *g_pfInputStream;
FILE *g_pfHashStream;
Directory *g_pstDirectory;
BlockManager g_stBlockManager;

int pow(int x, int y) {
    int r = 1;
    for (; y > 0; y--) {
        r *= x;
    }
    return r;
}

int PseudoKeyFunc(int key, int digits) {
    int i;
    int pseudo_key = 0;
    int bit = 0;
    /* key의 뒤 8자리 bit string에서 digits 만큼의 bit string을 얻기 */
    for (i = MAX_D - 1; i >= MAX_D - digits; --i) {
        bit = (key & POW(2, i)) > 0 ? 1 : 0;
        pseudo_key += POW(2, i - (MAX_D - digits)) * bit; /* 해당 bit를 누적 */
    }
    return pseudo_key;
}

int Insert(int key, char name[120]) {
    int i, j, temp, k;
    int iPseudoKey = 0;
    int iBucketNum = -1;/* 할당된 버킷 번호 */
    Bucket stBucket; /* 디스크의 버킷을 가져올 struct */
    Bucket stNewBucket;
    Record stRecord;

    Directory *pstNewDirectory;
    /* pseudo_key를 얻어 디렉터리 참조 */
    iPseudoKey = PseudoKeyFunc(key, g_pstDirectory->iHeader);
    iBucketNum = g_pstDirectory->miTable[iPseudoKey][1];
    /* 얻어 온 버킷 위치를 찾아서 읽기 */
    fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
    j = fread(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
    if (j == 0) { /* 버킷이 존재하지 않음 */
        printf("Error occurred while reading hash file.\n");
        return -1;
    }
    /* 이미 해당 key가 존재하는지 확인 */
    if (stBucket.iRecordCnt > 0) {
        for (i = 0; i < stBucket.iRecordCnt; ++i) {
            if (stBucket.vstRecord[i].iKey == key) { /* 해당 key 존재 */
                printf("Insert failed. Key (%d) already exists\n", key);
                return 0;
            }
        }
    }

    /* 여유 공간이 있는지 확인 */
    if (stBucket.iRecordCnt < 4) { /* 공간이 있으면 삽입 */
        stRecord.iKey = key;
        memcpy(stRecord.vcName, name, 120);
        stBucket.vstRecord[stBucket.iRecordCnt] = stRecord;
        stBucket.iRecordCnt++;
        /* 바뀐 내용을 디스크에 기록 */
        fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
        fwrite(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
        /* 블록 관리자 갱신 */
        g_stBlockManager.viBlockTable[iBucketNum] = 1;
        printf("(%d, %s) inserted\n", key, name);
    } else { /* 버킷에 여유 공간이 없으면 분할 */
        while (1) {
            /* 디렉터리를 확장해야 하는지 검사 */
            if (g_pstDirectory->iHeader < (stBucket.iHeader + 1)) {
                /* 디렉터리 확장 */
                if (g_pstDirectory->iHeader >= MAX_D) { /* 이미 만원인지 검사 */
                    printf("Error. Reached the maximum entry.\n");
                    return 0;
                }

                /* 새 디렉터리를 만들어 복사 */
                pstNewDirectory = (Directory *) malloc(sizeof(Directory));
                memset(pstNewDirectory, 0, sizeof(Directory));
                pstNewDirectory->iHeader = g_pstDirectory->iHeader + 1;
                for (i = 0; i < 512; ++i) {
                    pstNewDirectory->miTable[i][0] = -1;
                    pstNewDirectory->miTable[i][1] = -1;
                    pstNewDirectory->miTable[i][2] = 0;
                }
                for (i = 0; i < POW(2, pstNewDirectory->iHeader); ++i) {
                    pstNewDirectory->miTable[i][0] = i;
                    pstNewDirectory->miTable[i][1] = g_pstDirectory->miTable[i / 2][i];
                    pstNewDirectory->miTable[i][2] = g_pstDirectory->miTable[i / 2][2];
                }
                free(g_pstDirectory);
                g_pstDirectory = pstNewDirectory;
            }

            /* 오버플로가 발생한 버킷을 분할 */
            /* 새로운 버킷을 생성 */
            memset(&stNewBucket, 0, sizeof(Bucket));
            stNewBucket.iHeader = stBucket.iHeader;
            stNewBucket.iRecordCnt = 0;

            /* 레코드를 두 버킷에 분배 */
            stBucket.iRecordCnt = 0;
            for (i = 0; i < 4; ++i) {
                if (PseudoKeyFunc(stBucket.vstRecord[i].iKey, stBucket.iHeader + 1) % 2 == 0) {
                    /* 첫 밴째 버킷 */
                    stBucket.vstRecord[stBucket.iRecordCnt] = stBucket.vstRecord[i];
                    stBucket.iRecordCnt++;
                } else {
                    /* 두 번째 버킷 */
                    stNewBucket.vstRecord[stNewBucket.iRecordCnt] = stBucket.vstRecord[i];
                    stNewBucket.iRecordCnt++;
                }
            }
            stBucket.iHeader++;
            stNewBucket.iHeader++;
            /* 디스크에 자유 블록이 있는지 확인 */
            temp = -1;
            for (i = 0; i < g_stBlockManager.iBlockCnt; ++i) {
                if (g_stBlockManager.viBlockTable[i] == 0) { /* 자유 블록 */
                    temp = i;
                    break;
                }
            }

            if (temp != -1) { /* 자유 블록이 존재 - 해당 블록에 기록 */
            } else {/* 새로운 블록 할당 */
                temp = g_stBlockManager.iBlockCnt;
                g_stBlockManager.iBlockCnt++;
            }

            /* 해당 자유 블록에 할당 */
            fseek(g_pfHashStream, PAGE_SIZE * temp, SEEK_SET);
            fwrite(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
            g_stBlockManager.viBlockTable[temp] = 1;
            fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
            fwrite(&stBucket, sizeof(Bucket), 1, g_pfHashStream);

            /* 디렉터리의 버킷 인덱스를 변경 */
            j = POW(2, g_pstDirectory->iHeader - stBucket.iHeader + 1);
            k = PseudoKeyFunc(key, stBucket.iHeader - 1) * j;

            /* 디렉터리의 첫 번째 바뀔 부분 인덱스 */
            for (i = 0; i < j / 2, i++, k++;) {
                g_pstDirectory->miTable[k][1] = temp;
                g_pstDirectory->miTable[k][2] = stNewBucket.iHeader;
            }
            for (i = 0; i < j / 2, i++, k++;) {
                g_pstDirectory->miTable[k][1] = temp;
                g_pstDirectory->miTable[k][2] = stNewBucket.iHeader;
            }

            /* 입력 받은 key 삽입 */
            iPseudoKey = PseudoKeyFunc(key, g_pstDirectory->iHeader);
            iBucketNum = g_pstDirectory->miTable[iPseudoKey][1];

            /* 해당 버킷 위치를 찾아 읽기 */
            fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
            j = fread(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
            if (i == 0) { /* 버킷이 존재하지 않음 */
                printf("Error occurred while reading hash file.\n");
                return -1;
            }

            /* 여유 공간이 있으면 삽입 */
            /* 여유 공간이 있는지 확인 */
            if (stBucket.iRecordCnt < 4) { /* 여유 공간이 있으면 삽입 */
                stRecord.iKey = key;
                memcpy(stRecord.vcName, name, 120);

                /* stRecord.vcName = name */
                stBucket.vstRecord[stBucket.iRecordCnt] = stRecord;
                stBucket.iRecordCnt++;
                /* 바뀐 내용을 디스크에 기록 */
                fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
                fwrite(&stBucket, sizeof(Bucket), 1, g_pfHashStream);

                /* block manager update */
                g_stBlockManager.viBlockTable[iBucketNum] = 1;
                printf("(%d, %s) inserted.\n", key, name);
                return 0;
            }
                /* 여유 공간이 없는 경우 계속 루프를 돌며 디렉터리 확장 */
            else {
                printf("Error occurred while inserting.\n");
                return -1;
            }
        }
    }

    return 0;
}

int Delete(int key) {
    int i, j, k, temp;
    int b1, b2, cnt;
    int iPseudoKey = 0; /* 삭제될 키의 Pseudo Key */
    int iBucketNum = -1; /* 버킷 번호 */

    Bucket stBucket;
    Bucket stBuddyBucket;

    Directory *pstNewDirectory;
    /* pseudo key를 얻어 디렉토리를 참조 */
    iPseudoKey = PseudoKeyFunc(key, g_pstDirectory->iHeader);
    iBucketNum = g_pstDirectory->miTable[iPseudoKey][1];

    /* 얻어 온 버킷 번호를 통해 디스크 접근 */
    if (iBucketNum >= 0) {
        /* 해당 버킷의 위치를 찾아 읽기 */
        fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
        j = fread(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
        if (j == 0) { /* 버킷이 존재하지 않음 */
            printf("Error occurred while reading hash file.\n");
            return -1;
        }
        /* 해당 key가 존재하는지 확인 */
        temp = -1;
        if (stBucket.iRecordCnt > 0) {
            for (i = 0; i < stBucket.iRecordCnt; ++i) {
                if (stBucket.vstRecord[i].iKey == key) { /* 해당 key 존재 */
                    temp = i;
                    break;
                }
            }
            if (temp == -1) { /* 해당 key 없음 */
                printf("Delete failed. Key (%d) does not exist\n", key);
                return 0;
            }
        } else { /* 해당 버킷에서 레코드가 없음 */
            printf("Delete failed. Key (%d) does not exist\n", key);
            return 0;
        }

        /* 버킷에서 key 삭제. 버킷 정렬 */
        printf("(%d, %s) deleted\n",
               stBucket.vstRecord[temp].iKey,
               stBucket.vstRecord[temp].vcName);

        for (i = temp; i < stBucket.iRecordCnt - 1; ++i) {
            stBucket.vstRecord[i] = stBucket.vstRecord[i + 1];
        }

        stBucket.iRecordCnt++;

        /* 디스크에 기록 */
        fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
        fwrite(&stBucket, sizeof(Bucket), 1, g_pfHashStream);

        /* 버킷이 공백이거나 합병 가능하면 buddy bucket를 찾아서 합병 */
        while (stBucket.iHeader >= 2) {
            /* buddy bucket 찾기 */
            b1 = PseudoKeyFunc(key, stBucket.iHeader - 1);
            b2 = PseudoKeyFunc(k, stBucket.iHeader);
            cnt = POW(2, g_pstDirectory->iHeader - stBucket.iHeader);
            /* 연관된 리프 수 */
            k = (b2 + (b1 * 2 == b2 ? 1 : -1)) * cnt;
            /* 연관된 리프 중 첫 번째 리프 인덱스 */
            temp = g_pstDirectory->miTable[k][1];
            if (temp < 0) {
                return 0;
            }

            /* 디스크에서 buddy bucket 읽어 오기 */
            memset(&stBuddyBucket, 0, sizeof(Bucket));
            fseek(g_pfHashStream, PAGE_SIZE * temp, SEEK_SET);
            j = fread(&stBuddyBucket, sizeof(Bucket), 1, g_pfHashStream);
            if (j == 0) { /* 버킷이 존재 하지 않음 */
                printf("Error occurred while reading hash file.\n");
                return -1;
            }

            /* 합병해야 하는지 검사 */
            if ((stBucket.iHeader != stBuddyBucket.iHeader) || (stBucket.iRecordCnt + stBuddyBucket.iRecordCnt > 4)) {
                /* 합병이 필요 없음 */
                break;
            }
            /* 합병: buddy bucket에서 버킷으로 레코드 복사 */
            for (i = 0; i < stBuddyBucket.iRecordCnt; ++i) {
                stBucket.vstRecord[stBucket.iRecordCnt] = stBuddyBucket.vstRecord[i];
                stBucket.iRecordCnt++;
            }

            stBucket.iHeader--;
            fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
            fwrite(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
            /* 디렉토리에서 리프 조정 */
            g_pstDirectory->miTable[iPseudoKey][2] = stBucket.iHeader;
            for (i = 0; i < cnt; ++i) { /* buddy 변경 */
                g_pstDirectory->miTable[k + i][1] = iBucketNum;
                g_pstDirectory->miTable[k + i][2] = stBucket.iHeader;
            }

            /* buddy bucket 제거 */
            g_stBlockManager.viBlockTable[temp] = 0; /* 자유 블록으로 설정 */
            memset(&stBuddyBucket, 0, sizeof(Bucket));
            fseek(g_pfHashStream, PAGE_SIZE * temp, SEEK_SET);
            fwrite(&stBuddyBucket, sizeof(Bucket), 1, g_pfHashStream);
            /* 디렉터리가 줄어들 수 있는지 확인 */
            j = 0;
            for (i = 0; i < POW(2, g_pstDirectory->iHeader); ++i) {
                if (g_pstDirectory->miTable[i][2] >= g_pstDirectory->iHeader) {
                    j = 1;
                    break;
                }
            }
            if (j == 0) {
                /* 버킷의 모든 헤더 값이 디렉터리 헤더 값보다 작은 경우 디렉터리 축소 */
                pstNewDirectory = (Directory *) malloc(sizeof(Directory));
                memset(pstNewDirectory, 0, sizeof(Directory));
                pstNewDirectory->iHeader = g_pstDirectory->iHeader - 1;
                for (i = 0; i < 512; ++i) {
                    pstNewDirectory->miTable[i][0] = -1;
                    pstNewDirectory->miTable[i][1] = -1;
                    pstNewDirectory->miTable[i][2] = 0;
                }
                for (i = 0; i < POW(2, pstNewDirectory->iHeader); ++i) {
                    pstNewDirectory->miTable[i][0] = i;
                    pstNewDirectory->miTable[i][1] = g_pstDirectory->miTable[i * 2][1];
                    pstNewDirectory->miTable[i][2] = g_pstDirectory->miTable[i * 2][2];
                }
                free(g_pstDirectory);
                g_pstDirectory = pstNewDirectory;
            }
        }
    } else {
        printf("Error occurred while deleting.\n");
        return -1;
    }
    return 0;
}

int Retrieve(int key) {
    int i, j, temp;
    int iPseudoKey = 0; /* 삭제될 키의 Pseudo Key */
    int iBucketNum = -1; /* 버킷 번호 */
    Bucket stBucket;


    /* pseudo key를 얻어 디렉토리를 참조 */
    iPseudoKey = PseudoKeyFunc(key, g_pstDirectory->iHeader);
    iBucketNum = g_pstDirectory->miTable[iPseudoKey][1];
    /* 얻어 온 버킷 번호를 통해 디스크 접근 */
    if (iBucketNum >= 0) {
        /* 해당 버킷의 위치를 찾아 읽기 */
        fseek(g_pfHashStream, PAGE_SIZE * iBucketNum, SEEK_SET);
        j = fread(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
        if (j == 0) { /* 버킷이 존재하지 않음 */
            printf("Error occurred while reading hash file.\n");
            return -1;
        }


        /* 해당 key가 존재하는지 확인 */
        temp = -1;
        if (stBucket.iRecordCnt > 0) {
            for (i = 0; i < stBucket.iRecordCnt; ++i) {
                if (stBucket.vstRecord[i].iKey == key) { /* 해당 key 존재 */
                    temp = i;
                    printf("(%d %s) found\n",
                           stBucket.vstRecord[temp].iKey,
                           stBucket.vstRecord[temp].vcName);
                    break;
                }
            }
            if (temp == -1) { /* 해당 key 없음 */
                printf("Search failed. Key (%d) was not found\n", key);
                return 0;
            }
        } else { /* 해당 버킷에서 레코드가 없음 */
            printf("Search failed. Key (%d) was not exist\n", key);
            return 0;
        }
    } else {
        printf("Error occurred while searching.\n");
        return -1;
    }
    return 0;
}

// 출력
void PrintHash() {
    int i, j;
    int iPrevBucket = -1;
    Bucket stBucket;
    char vcBitStr[10];

    printf("========================================================================\n");
    printf("Directory (%d)          Hash State \n", g_pstDirectory->iHeader);
    printf("========================================================================\n");
    for (i = 0; i < POW(2, g_pstDirectory->iHeader); ++i) {
        /* bit string 생성 */
        for (j = 0; j < g_pstDirectory->iHeader; ++j) {
            vcBitStr[i] = (g_pstDirectory->miTable[i][0] & POW(2, g_pstDirectory->iHeader - j - 1)) > 0 ? '1' : '0';
        }
        vcBitStr[g_pstDirectory->iHeader] = '\0';
        /* 디렉터리 내용 출력 */
        printf("[%s] [B: %d]", vcBitStr, g_pstDirectory->miTable[i][1]);
        if (g_pstDirectory->miTable[i][1] != iPrevBucket) {
            iPrevBucket = g_pstDirectory->miTable[i][1];
            /* 해당 버킷 내용 출력 */
            fseek(g_pfHashStream, PAGE_SIZE * iPrevBucket, SEEK_SET);
            j = fread(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
            if (j == 0) { /* 버킷이 존재하지 않음 */
                printf("Error occurred while reading hash file.\n");
                return;
            }
            printf("\t[B: %d](%d) ", iPrevBucket, stBucket.iHeader);
            for (j = 0; j < stBucket.iRecordCnt; ++j) {
                printf("(%d, %s) \n",
                       stBucket.vstRecord[i].iKey,
                       stBucket.vstRecord[i].vcName);

            }
        }
        printf("\n");
    }
    printf("\n");
}

int MakeHashMain(int argc, char *argv) {
    int i, iKey;
    char c;
    char vcName[120];
    Bucket stBucket;

    /* 디렉토리 초기화 */
    g_pstDirectory = (Directory *) malloc(sizeof(Directory));
    memset(g_pstDirectory, 0, sizeof(Directory));
    g_pstDirectory->iHeader = 0;

    for (i = 0; i < 512; ++i) {
        g_pstDirectory->miTable[i][0] = -1;
        g_pstDirectory->miTable[i][1] = -1;
        g_pstDirectory->miTable[i][2] = 0;
    }

    /* 디렉토리에 초기 table 생성 */
    g_pstDirectory->iHeader = 2;
    for (i = 0; i < 4; ++i) {
        g_pstDirectory->miTable[i][0] = i;  /* bit string */
        g_pstDirectory->miTable[i][1] = 0;  /* 버킷 번호는 0 */
        g_pstDirectory->miTable[i][2] = 0;
    }

    /* Block Manager 초기화 */
    g_stBlockManager.iBlockCnt = 0;
    for (i = 0; i < 512; ++i) {
        g_stBlockManager.viBlockTable[i] = 0;
    }

    /* File 열기 */
    /* input.txt 파일 열기 */
    if ((g_pfInputStream = fopen("input.txt", "r")) == NULL) {
        printf("input.txt file cannot be opened\n");
        return -1;
    }
    /* hash.txt 파일 열기 */
    if ((g_pfHashStream = fopen("hash.txt", "w+")) == NULL) {
        printf("hash.txt file cannot be opened\n");
        return -1;
    }

    g_stBlockManager.iBlockCnt = 4;
    memset(&stBucket, 0, sizeof(Bucket));
    stBucket.iRecordCnt = 0;
    stBucket.iHeader = 0;
    for (i = 0; i < 4; ++i) {
        fwrite(&stBucket, sizeof(Bucket), 1, g_pfHashStream);
    }
    printf("\n[Extendible Hashing] Started\n\n");
    while (1) {
        if (fscanf(g_pfInputStream, "%c", &c) == EOF) {
            break;
        }
        switch (c) {
            case 'i':
            case 'I':
                if (fscanf(g_pfInputStream, "%d %s", &iKey, vcName) == EOF) {
                    printf("Error occurred input file corrupted.\n");
                    return -1;
                }
                if (Insert(iKey, vcName) == 0) {
                    PrintHash();
                }
                break;

            case 'd':
            case 'D':
                if (fscanf(g_pfInputStream, "%d", &iKey) == EOF) {
                    printf("Error occurred input file corrupted.\n");
                    return -1;
                }
                if (Delete(iKey) == 0) {
                    PrintHash();
                }
                break;
            case 'r':
            case 'R':
                if (fscanf(g_pfInputStream, "%d", &iKey) == EOF) {
                    printf("Error occurred input file corrupted.\n");
                    return -1;
                }
                if (Retrieve(iKey) == 0) {
                    PrintHash();
                }
                break;
            default:
                break;
        }
    }

    fclose(g_pfInputStream);
    fclose(g_pfHashStream);
    printf("\n[Extendible Hashing] Ended\n\n");

    return 0;
}
