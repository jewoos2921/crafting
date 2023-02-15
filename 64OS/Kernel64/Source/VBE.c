//
// Created by jewoo on 2023-02-15.
//

#include "VBE.h"

static VBEMODE_INFOBLOCK *gs_pstVBEModeBlockInfo = (VBEMODE_INFOBLOCK *) VBE_MODE_INFO_BLOCK_ADDRESS;


inline VBEMODE_INFOBLOCK *kGetVBEModeInfoBlock(void) {
    return gs_pstVBEModeBlockInfo;
}