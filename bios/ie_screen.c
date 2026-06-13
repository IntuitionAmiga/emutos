#include "emutos.h"
#include "ie_machine.h"
#include "ie_blitter.h"

/*
 * IE screen BIOS helpers. The framebuffer format is selected by CONF_IE_CLUT8
 * (see ie_machine.h): CLUT8 stores 1-byte palette indices at 1920x1080, RGBA32
 * stores 32-bit pixels at 640x480. IE_PIXEL/IE_BPP abstract the element type.
 *
 * Colour arguments to these BIOS entry points are mode-dependent: in CLUT8 the
 * low byte is a palette index, in RGBA32 it is a packed ie_rgba() value.
 */

static inline volatile IE_PIXEL *ie_vram_ptr(ULONG byte_off)
{
    return (volatile IE_PIXEL *)(IE_VRAM_BASE + byte_off);
}

/* Blitter fill colour for a mode-dependent colour argument. */
static inline ULONG ie_screen_fillcolor(unsigned long c)
{
#if CONF_IE_CLUT8
    return (ULONG)(c & 0xFF);       /* palette index, never byte-swapped */
#else
    return ie_blt_color((ULONG)c);  /* RGBA32: reverse for the blitter */
#endif
}

void ie_screen_init(void) {
    UWORD i;

    IE_MMIO32(IE_VIDEO_MODE) = IE_VIDEO_MODE_NATIVE;
#if CONF_IE_CLUT8
    /* Indexed framebuffer: select CLUT8 mode and point it at our VRAM, then
     * seed a minimal palette so the early console is legible before the VDI
     * loads the full palette via set_color_ie(). */
    IE_MMIO32(IE_VIDEO_COLOR_MODE) = 1;
    IE_MMIO32(IE_VIDEO_FB_BASE) = IE_VRAM_BASE;
    IE_MMIO32(IE_VIDEO_PAL_ENTRY(0)) = 0x00000000UL;   /* black */
    IE_MMIO32(IE_VIDEO_PAL_ENTRY(1)) = 0x00FFFFFFUL;   /* white */
    IE_MMIO32(IE_VIDEO_PAL_ENTRY(255)) = 0x00FFFF00UL; /* boot-marker yellow */
#endif
    IE_MMIO32(IE_VIDEO_CTRL) = 1; /* enable video */
    ie_clear(0, 0, 0);
    /* Boot marker: visible without any VDI/VT52 path involvement. */
    for (i = 0; i < 64; i++) {
        ie_putpixel(i, i, ie_rgba(255, 255, 0, 255));
    }
}

void ie_putpixel(unsigned short x, unsigned short y, unsigned long rgba) {
    ULONG row_off;
    ULONG byte_off;

    if (x >= IE_VRAM_WIDTH || y >= IE_VRAM_HEIGHT) {
        return;
    }
    row_off = (ULONG)y * IE_VRAM_STRIDE;
    byte_off = row_off + (ULONG)x * IE_BPP;
#if CONF_IE_CLUT8
    *ie_vram_ptr(byte_off) = (IE_PIXEL)(rgba & 0xFF);   /* palette index */
#else
    *ie_vram_ptr(byte_off) = (IE_PIXEL)rgba;
#endif
}

void ie_clear(unsigned char r, unsigned char g, unsigned char b) {
    /* Whole-screen solid fill via the hardware blitter. ie_clear() is only
     * called with black; in CLUT8 that is palette index 0. */
#if CONF_IE_CLUT8
    (void)r; (void)g; (void)b;
    ie_blt_fill_ex(IE_VRAM_BASE, IE_VRAM_WIDTH, IE_VRAM_HEIGHT,
                   IE_VRAM_STRIDE, 0UL, IE_BLT_PXFLAGS(IE_BLT_DM_COPY));
#else
    ie_blt_fill_ex(IE_VRAM_BASE, IE_VRAM_WIDTH, IE_VRAM_HEIGHT,
                   IE_VRAM_STRIDE, ie_screen_fillcolor(ie_rgba(r, g, b, 0xFF)),
                   IE_BLT_PXFLAGS(IE_BLT_DM_COPY));
#endif
}

void ie_fill_rect(unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned long rgba) {
    ULONG x0;
    ULONG y0;
    ULONG w0;
    ULONG h0;
    ULONG dst;

    if (x >= IE_VRAM_WIDTH || y >= IE_VRAM_HEIGHT || w == 0 || h == 0) {
        return;
    }
    x0 = (ULONG)x;
    y0 = (ULONG)y;
    w0 = (ULONG)w;
    h0 = (ULONG)h;
    if ((x0 + w0) > (ULONG)IE_VRAM_WIDTH) {
        w0 = (ULONG)IE_VRAM_WIDTH - x0;
    }
    if ((y0 + h0) > (ULONG)IE_VRAM_HEIGHT) {
        h0 = (ULONG)IE_VRAM_HEIGHT - y0;
    }
    dst = IE_VRAM_BASE + y0 * IE_VRAM_STRIDE + x0 * IE_BPP;
    ie_blt_fill_ex(dst, (UWORD)w0, (UWORD)h0, (UWORD)IE_VRAM_STRIDE,
                   ie_screen_fillcolor(rgba), IE_BLT_PXFLAGS(IE_BLT_DM_COPY));
}

void ie_scroll_up_pixels(unsigned short lines, unsigned long bg_rgba) {
    ULONG lines32;
    ULONG swapped32;

    if (lines == 0) {
        return;
    }
    lines32 = (ULONG)lines;
    /* Defensive recovery: on some paths a 16-bit line count can arrive byte-swapped
     * (e.g. 0x1000 instead of 0x0010). If the swapped value is sane, use it.
     */
    if (lines32 >= (ULONG)IE_VRAM_HEIGHT) {
        swapped32 = (ULONG)((UWORD)((lines >> 8) | (lines << 8)));
        if (swapped32 > 0 && swapped32 < (ULONG)IE_VRAM_HEIGHT) {
            lines32 = swapped32;
        } else {
            lines32 = 16UL;
        }
    }
    if (lines32 >= (ULONG)IE_VRAM_HEIGHT) {
        lines32 = (ULONG)IE_VRAM_HEIGHT - 1UL;
    }

    /*
     * Hardware scroll: move the visible content up by lines32 rows with a
     * single blitter block copy (RAM-to-RAM, handles the overlap), then clear
     * the exposed band at the bottom. Strides are byte-based (IE_VRAM_STRIDE),
     * so this is format-agnostic.
     */
    {
        ULONG move_rows = (ULONG)IE_VRAM_HEIGHT - lines32;
        ULONG move_bytes = move_rows * IE_VRAM_STRIDE;
        ULONG src = IE_VRAM_BASE + lines32 * IE_VRAM_STRIDE;
        ULONG bottom = IE_VRAM_BASE + move_rows * IE_VRAM_STRIDE;

        ie_blt_memcopy(src, IE_VRAM_BASE, move_bytes);
        ie_blt_fill_ex(bottom, IE_VRAM_WIDTH, (UWORD)lines32, (UWORD)IE_VRAM_STRIDE,
                       ie_screen_fillcolor(bg_rgba), IE_BLT_PXFLAGS(IE_BLT_DM_COPY));
    }
}
