/*
 * vdi_inline.h - Definitions of VDI inline functions
 *
 * Copyright 2024-2025 The EmuTOS development team.
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef _VDI_INLINE_H
#define _VDI_INLINE_H

#if CONF_WITH_VDI_16BIT
static __inline__ UWORD *get_start_addr16(const WORD x, const WORD y)
{
    return (UWORD *)(v_bas_ad + (x * sizeof(WORD)) + muls(y, v_lin_wr));
}
#endif

#ifdef MACHINE_IE
#include "../bios/ie_machine.h"

static __inline__ IE_PIXEL *get_start_addr_ie(const WORD x, const WORD y)
{
    LONG addr = (LONG)v_bas_ad + muls(y, v_lin_wr) + ((LONG)x * IE_BPP);
    return (IE_PIXEL *)addr;
}

/*
 * ie_pixel - the screen value for a VDI colour index. In CLUT8 the framebuffer
 * stores the index directly (the IE hardware palette maps it to RGBA); in
 * RGBA32 it stores the packed colour from the software palette.
 */
static __inline__ IE_PIXEL ie_pixel(WORD color)
{
#if CONF_IE_CLUT8
    return (IE_PIXEL)(UBYTE)color;
#else
    return (IE_PIXEL)ie_vdi_palette[color];
#endif
}
#endif

#endif                          /* _VDI_INLINE_H */
