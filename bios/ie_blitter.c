/*
 * ie_blitter.c - Intuition Engine hardware blitter driver for EmuTOS.
 *
 * See ie_blitter.h for the contract. Register writes use IE_MMIO32 with full
 * 32-bit ULONG addresses from ie_machine.h, matching the existing ie_screen.c
 * idiom and avoiding the GCC -mshort address-folding hazards fixed in the IE
 * EmuTOS port (commit 3037826).
 */

#include "emutos.h"
#include "ie_machine.h"
#include "ie_blitter.h"

void ie_blt_fill(ULONG dst, UWORD w, UWORD h, UWORD dst_stride, ULONG color)
{
    IE_MMIO32(IE_BLT_OP)         = IE_BLT_OP_FILL;
    IE_MMIO32(IE_BLT_DST)        = dst;
    IE_MMIO32(IE_BLT_WIDTH)      = (ULONG)w;
    IE_MMIO32(IE_BLT_HEIGHT)     = (ULONG)h;
    IE_MMIO32(IE_BLT_DST_STRIDE) = (ULONG)dst_stride;
    IE_MMIO32(IE_BLT_COLOR)      = color;
    IE_MMIO32(IE_BLT_FLAGS)      = 0; /* RGBA32 Copy (legacy default) */
    IE_MMIO32(IE_BLT_CTRL)       = IE_BLT_CTRL_START;
}

void ie_blt_fill_ex(ULONG dst, UWORD w, UWORD h, UWORD dst_stride,
                    ULONG color, ULONG flags)
{
    IE_MMIO32(IE_BLT_OP)         = IE_BLT_OP_FILL;
    IE_MMIO32(IE_BLT_DST)        = dst;
    IE_MMIO32(IE_BLT_WIDTH)      = (ULONG)w;
    IE_MMIO32(IE_BLT_HEIGHT)     = (ULONG)h;
    IE_MMIO32(IE_BLT_DST_STRIDE) = (ULONG)dst_stride;
    IE_MMIO32(IE_BLT_COLOR)      = color;
    IE_MMIO32(IE_BLT_FLAGS)      = flags;
    IE_MMIO32(IE_BLT_CTRL)       = IE_BLT_CTRL_START;
}

void ie_blt_copy(ULONG src, ULONG dst, UWORD w, UWORD h,
                 UWORD src_stride, UWORD dst_stride)
{
    IE_MMIO32(IE_BLT_OP)         = IE_BLT_OP_COPY;
    IE_MMIO32(IE_BLT_SRC)        = src;
    IE_MMIO32(IE_BLT_DST)        = dst;
    IE_MMIO32(IE_BLT_WIDTH)      = (ULONG)w;
    IE_MMIO32(IE_BLT_HEIGHT)     = (ULONG)h;
    IE_MMIO32(IE_BLT_SRC_STRIDE) = (ULONG)src_stride;
    IE_MMIO32(IE_BLT_DST_STRIDE) = (ULONG)dst_stride;
    IE_MMIO32(IE_BLT_FLAGS)      = 0;
    IE_MMIO32(IE_BLT_CTRL)       = IE_BLT_CTRL_START;
}

void ie_blt_copy_ex(ULONG src, ULONG dst, UWORD w, UWORD h,
                    UWORD src_stride, UWORD dst_stride, ULONG flags)
{
    IE_MMIO32(IE_BLT_OP)         = IE_BLT_OP_COPY;
    IE_MMIO32(IE_BLT_SRC)        = src;
    IE_MMIO32(IE_BLT_DST)        = dst;
    IE_MMIO32(IE_BLT_WIDTH)      = (ULONG)w;
    IE_MMIO32(IE_BLT_HEIGHT)     = (ULONG)h;
    IE_MMIO32(IE_BLT_SRC_STRIDE) = (ULONG)src_stride;
    IE_MMIO32(IE_BLT_DST_STRIDE) = (ULONG)dst_stride;
    IE_MMIO32(IE_BLT_FLAGS)      = flags;
    IE_MMIO32(IE_BLT_CTRL)       = IE_BLT_CTRL_START;
}

void ie_blt_color_expand(ULONG mask, ULONG dst, UWORD w, UWORD h,
                         UWORD mask_mod, UWORD mask_srcx, UWORD dst_stride,
                         ULONG fg, ULONG bg, ULONG flags)
{
    IE_MMIO32(IE_BLT_OP)         = IE_BLT_OP_COLOR_EXPAND;
    IE_MMIO32(IE_BLT_MASK)       = mask;
    IE_MMIO32(IE_BLT_DST)        = dst;
    IE_MMIO32(IE_BLT_WIDTH)      = (ULONG)w;
    IE_MMIO32(IE_BLT_HEIGHT)     = (ULONG)h;
    IE_MMIO32(IE_BLT_MASK_MOD)   = (ULONG)mask_mod;
    IE_MMIO32(IE_BLT_MASK_SRCX)  = (ULONG)mask_srcx;
    IE_MMIO32(IE_BLT_DST_STRIDE) = (ULONG)dst_stride;
    IE_MMIO32(IE_BLT_FG)         = fg;
    IE_MMIO32(IE_BLT_BG)         = bg;
    IE_MMIO32(IE_BLT_FLAGS)      = flags;
    IE_MMIO32(IE_BLT_CTRL)       = IE_BLT_CTRL_START;
}

void ie_blt_masked_copy(ULONG src, ULONG dst, UWORD w, UWORD h,
                        UWORD src_stride, UWORD dst_stride,
                        ULONG mask, UWORD mask_mod, UWORD mask_srcx,
                        ULONG flags)
{
    IE_MMIO32(IE_BLT_OP)         = IE_BLT_OP_MASKED;
    IE_MMIO32(IE_BLT_SRC)        = src;
    IE_MMIO32(IE_BLT_DST)        = dst;
    IE_MMIO32(IE_BLT_WIDTH)      = (ULONG)w;
    IE_MMIO32(IE_BLT_HEIGHT)     = (ULONG)h;
    IE_MMIO32(IE_BLT_SRC_STRIDE) = (ULONG)src_stride;
    IE_MMIO32(IE_BLT_DST_STRIDE) = (ULONG)dst_stride;
    IE_MMIO32(IE_BLT_MASK)       = mask;
    IE_MMIO32(IE_BLT_MASK_MOD)   = (ULONG)mask_mod;
    IE_MMIO32(IE_BLT_MASK_SRCX)  = (ULONG)mask_srcx;
    IE_MMIO32(IE_BLT_FLAGS)      = flags;
    IE_MMIO32(IE_BLT_CTRL)       = IE_BLT_CTRL_START;
}

void ie_blt_line(ULONG dst, UWORD dst_stride, WORD x0, WORD y0,
                 WORD x1, WORD y1, ULONG color, ULONG flags)
{
    /* Extended line mode (flags != 0): BLT_DST is the framebuffer base,
     * BLT_SRC packs the start point and BLT_WIDTH the end point as
     * (y << 16) | (x & 0xFFFF). */
    IE_MMIO32(IE_BLT_OP)         = IE_BLT_OP_LINE;
    IE_MMIO32(IE_BLT_SRC)        = ((ULONG)(UWORD)y0 << 16) | (ULONG)(UWORD)x0;
    IE_MMIO32(IE_BLT_WIDTH)      = ((ULONG)(UWORD)y1 << 16) | (ULONG)(UWORD)x1;
    IE_MMIO32(IE_BLT_DST)        = dst;
    IE_MMIO32(IE_BLT_DST_STRIDE) = (ULONG)dst_stride;
    IE_MMIO32(IE_BLT_COLOR)      = color;
    IE_MMIO32(IE_BLT_FLAGS)      = flags;
    IE_MMIO32(IE_BLT_CTRL)       = IE_BLT_CTRL_START;
}

void ie_blt_memcopy(ULONG src, ULONG dst, ULONG byte_len)
{
    if (byte_len == 0)
        return;
    /* BLT_WIDTH carries the byte count; BLT_HEIGHT is 1 by convention. */
    IE_MMIO32(IE_BLT_OP)     = IE_BLT_OP_MEMCOPY;
    IE_MMIO32(IE_BLT_SRC)    = src;
    IE_MMIO32(IE_BLT_DST)    = dst;
    IE_MMIO32(IE_BLT_WIDTH)  = byte_len;
    IE_MMIO32(IE_BLT_HEIGHT) = 1;
    IE_MMIO32(IE_BLT_CTRL)   = IE_BLT_CTRL_START;
}

void ie_set_color_mode(UWORD mode)
{
    IE_MMIO32(IE_VIDEO_COLOR_MODE) = (ULONG)mode;
}

void ie_load_clut(const ULONG *palette, UWORD start, UWORD count)
{
    UWORD i;
    for (i = 0; i < count; i++) {
        IE_MMIO32(IE_VIDEO_PAL_ENTRY(start + i)) = palette[i];
    }
}
