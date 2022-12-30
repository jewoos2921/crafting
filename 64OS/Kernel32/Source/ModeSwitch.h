//
// Created by jewoo on 2022-12-30.
//

#ifndef CRAFTING_MODESWITCH_H
#define CRAFTING_MODESWITCH_H

#include "Types.h"

void kReadCPUID(DWORD dwEAX, DWORD *pdwEAX, DWORD *pdwEBX, DWORD *pdwECX,
                DWORD *pdwEDX);

void kSwitchAndExecute64bitKernel(void);

#endif //CRAFTING_MODESWITCH_H
