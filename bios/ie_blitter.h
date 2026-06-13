/*
 * ie_blitter.h - Intuition Engine hardware blitter driver for EmuTOS.
 *
 * Thin wrappers over the IE blitter MMIO registers (see ie_machine.h). Each
 * routine writes the operation's parameters then triggers BLT_CTRL_START. The
 * IE blitter is synchronous: the operation completes before the start write
 * returns, so no busy-wait is required.
 *
 * All colors are natural big-endian RGBA ULONGs as produced by ie_rgba(); the
 * IE chip runs in big-endian mode for the M68K target and reconciles them with
 * its little-endian framebuffer, so callers must NOT pre-swap.
 *
 * Pixel format is RGBA32 (4 bytes/pixel) unless a *_ex routine is passed a
 * CLUT8 flag. In RGBA32, only COPY/CLEAR/SET draw modes are correct.
 */

#ifndef IE_BLITTER_H
#define IE_BLITTER_H

#include "emutos.h"

/*
 * ie_blt_color - byte-reverse a native big-endian RGBA32 pixel for the
 * blitter's color registers (BLT_COLOR / BLT_FG / BLT_BG).
 *
 * CPU-direct stores land in VRAM big-endian ([R,G,B,A]), but the IE blitter
 * consumes its color registers little-endian (R in the low byte) and stores
 * them little-endian, so a color that must match CPU-rendered pixels has to be
 * reversed to 0xAABBGGRR first. Apply this ONLY for RGBA32; CLUT8 uses an
 * 8-bit index in the low byte and must never be swapped. Mirrors AROS
 * arch/m68k-ie/hidd/iegfx/iegfx_onbitmap.c:ie_blt_color().
 */
static __inline__ ULONG ie_blt_color(ULONG px)
{
    return ((px & 0x000000FFUL) << 24) |
           ((px & 0x0000FF00UL) << 8)  |
           ((px & 0x00FF0000UL) >> 8)  |
           ((px & 0xFF000000UL) >> 24);
}

/*
 * Mode-aware helpers so VDI/screen call sites stay format-agnostic.
 * CLUT8: the screen value is an 8-bit index in the low byte, never swapped,
 * and BLT_FLAGS selects CLUT8 BPP. RGBA32: the value is byte-reversed for the
 * blitter's little-endian colour registers.
 */
#if CONF_IE_CLUT8
#define IE_BLT_PXFLAGS(drawmode) IE_BLT_MAKE_FLAGS(IE_BLT_FLAGS_BPP_CLUT8, (drawmode))
static __inline__ ULONG ie_blt_pxcolor(IE_PIXEL c) { return (ULONG)(UBYTE)c; }
#else
#define IE_BLT_PXFLAGS(drawmode) IE_BLT_MAKE_FLAGS(IE_BLT_FLAGS_BPP_RGBA32, (drawmode))
static __inline__ ULONG ie_blt_pxcolor(IE_PIXEL c) { return ie_blt_color((ULONG)c); }
#endif

/* RGBA32 solid rectangle fill (Copy mode). */
void ie_blt_fill(ULONG dst, UWORD w, UWORD h, UWORD dst_stride, ULONG color);

/* Rectangle fill with an explicit BLT_FLAGS value (BPP + draw mode). */
void ie_blt_fill_ex(ULONG dst, UWORD w, UWORD h, UWORD dst_stride,
                    ULONG color, ULONG flags);

/* RGBA32 rectangular copy (Copy mode). */
void ie_blt_copy(ULONG src, ULONG dst, UWORD w, UWORD h,
                 UWORD src_stride, UWORD dst_stride);

/* Rectangular copy with an explicit BLT_FLAGS value. */
void ie_blt_copy_ex(ULONG src, ULONG dst, UWORD w, UWORD h,
                    UWORD src_stride, UWORD dst_stride, ULONG flags);

/* 1-bit template expanded to fg/bg pixels (text/glyph rendering). Pass
 * IE_BLT_FLAGS_JAM1 in flags for transparent (skip-background) rendering. */
void ie_blt_color_expand(ULONG mask, ULONG dst, UWORD w, UWORD h,
                         UWORD mask_mod, UWORD mask_srcx, UWORD dst_stride,
                         ULONG fg, ULONG bg, ULONG flags);

/* Copy src pixels to dst where the 1-bit mask is set. Pass
 * IE_BLT_FLAGS_MASK_MSB for MSB-first (PLANEPTR) bit order. */
void ie_blt_masked_copy(ULONG src, ULONG dst, UWORD w, UWORD h,
                        UWORD src_stride, UWORD dst_stride,
                        ULONG mask, UWORD mask_mod, UWORD mask_srcx,
                        ULONG flags);

/* Line from (x0,y0) to (x1,y1) into the framebuffer at base dst. flags must be
 * non-zero to select extended-line mode (base + packed endpoints). */
void ie_blt_line(ULONG dst, UWORD dst_stride, WORD x0, WORD y0,
                 WORD x1, WORD y1, ULONG color, ULONG flags);

/* Byte-granular block copy (RAM-to-RAM); handles overlap. Used for scroll. */
void ie_blt_memcopy(ULONG src, ULONG dst, ULONG byte_len);

/* CLUT8 / palette helpers. */
void ie_set_color_mode(UWORD mode);                 /* 0=RGBA32, 1=CLUT8 */
void ie_load_clut(const ULONG *palette, UWORD start, UWORD count);

#endif /* IE_BLITTER_H */
