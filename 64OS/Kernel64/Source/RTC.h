//
// Created by jewoo on 2023-01-09.
//

#ifndef CRAFTING_RTC_H
#define CRAFTING_RTC_H

#include "Types.h"

// I/O 포트

#define RTC_CMOS_ADDRESS    0x70
#define RTC_CMOS_DATA       0x71

// CMOS 메모리 어드레스
#define RTC_ADDRESS_SECOND  0x00
#define RTC_ADDRESS_MINUTE 0x02
#define RTC_ADDRESS_HOUR 0x04
#define RTC_ADDRESS_DAYOFWEEK 0x06
#define RTC_ADDRESS_DAYOFMONTH 0x07
#define RTC_ADDRESS_MONTH 0x08
#define RTC_ADDRESS_YEAR 0x09

// BCD 포멧을 Binary로 변환하는 매크로
#define RTC_BCD_TO_BINARY(x)            ((((x)>>4) * 10) + ((x) & 0x0F))

void kReadRTCTime(BYTE *pbHour, BYTE *pbMinute, BYTE *pbSecond);

void kReadRTCDate(WORD *pwYear, BYTE *pbMonth, BYTE *pbDayOfMonth,
                  BYTE *pbDayOfWeek);

char *kConvertDayOfWeekToString(BYTE bDayOfWeek);

#endif //CRAFTING_RTC_H
