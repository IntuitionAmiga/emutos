#ifndef IE_MACHINE_H
#define IE_MACHINE_H

/* Intuition Engine MMIO map used by EmuTOS IE target */
#define IE_VIDEO_CTRL      0x000F0000UL
#define IE_VIDEO_MODE      0x000F0004UL
#define IE_VIDEO_STATUS    0x000F0008UL
#define IE_VRAM_BASE       0x00100000UL

/* IE_VIDEO_MODE values */
#define IE_VIDEO_MODE_640x480   0U
#define IE_VIDEO_MODE_1920x1080 6U

/*
 * Screen colour format. CLUT8 (default) is a 1-byte-per-pixel indexed mode at
 * 1920x1080 — 4x less framebuffer bandwidth than RGBA32 and the 16 VDI raster
 * ops operate index-correct. Set CONF_IE_CLUT8 to 0 to fall back to the legacy
 * 32-bit chunky RGBA mode at 640x480.
 */
#ifndef CONF_IE_CLUT8
#define CONF_IE_CLUT8 1
#endif

#if CONF_IE_CLUT8
typedef unsigned char IE_PIXEL;       /* one palette index per pixel */
#define IE_BPP             1
#define IE_VRAM_WIDTH      1920U
#define IE_VRAM_HEIGHT     1080U
#define IE_VRAM_STRIDE     1920UL      /* 1920 * 1 */
#define IE_VRAM_PLANES     8U
#define IE_VIDEO_MODE_NATIVE IE_VIDEO_MODE_1920x1080
#else
typedef unsigned long IE_PIXEL;        /* RGBA32 pixel */
#define IE_BPP             4
#define IE_VRAM_WIDTH      640U
#define IE_VRAM_HEIGHT     480U
#define IE_VRAM_STRIDE     2560UL      /* 640 * 4 */
#define IE_VRAM_PLANES     32U
#define IE_VIDEO_MODE_NATIVE IE_VIDEO_MODE_640x480
#endif

/*
 * VDI dispatch predicate: true when the active screen is the IE framebuffer.
 * Under MACHINE_IE the screen is always the IE chip, so this is simply the
 * line-A plane count matching the IE depth (8 for CLUT8, 32 for RGBA32).
 * Expands at the call site where v_planes is in scope.
 */
#define IE_SCREEN_MODE (v_planes == IE_VRAM_PLANES)

/* CLUT8 palette mode (canonical: IntuitionEngine video_chip.go) */
#define IE_VIDEO_PAL_INDEX   0x000F0078UL  /* Palette write index (0-255) */
#define IE_VIDEO_PAL_DATA    0x000F007CUL  /* Palette data 0x00RRGGBB, auto-increments */
#define IE_VIDEO_COLOR_MODE  0x000F0080UL  /* 0=RGBA32 (direct), 1=CLUT8 (indexed) */
#define IE_VIDEO_FB_BASE     0x000F0084UL  /* Framebuffer base address */
#define IE_VIDEO_PAL_TABLE   0x000F0088UL  /* 256x4-byte direct palette access */
#define IE_VIDEO_PAL_ENTRY(x) (IE_VIDEO_PAL_TABLE + ((ULONG)(x) << 2))

/* Blitter core registers (0xF001C - 0xF0044) */
#define IE_BLT_CTRL        0x000F001CUL  /* Control: bit 0 = start */
#define IE_BLT_OP          0x000F0020UL  /* Operation code (see IE_BLT_OP_*) */
#define IE_BLT_SRC         0x000F0024UL  /* Source address */
#define IE_BLT_DST         0x000F0028UL  /* Destination address */
#define IE_BLT_WIDTH       0x000F002CUL  /* Width in pixels */
#define IE_BLT_HEIGHT      0x000F0030UL  /* Height in pixels */
#define IE_BLT_SRC_STRIDE  0x000F0034UL  /* Source stride in bytes */
#define IE_BLT_DST_STRIDE  0x000F0038UL  /* Destination stride in bytes */
#define IE_BLT_COLOR       0x000F003CUL  /* Fill/line color (RGBA32) */
#define IE_BLT_MASK        0x000F0040UL  /* Mask/template address */
#define IE_BLT_STATUS      0x000F0044UL  /* Status: bit 0 = error, bit 1 = done */

/* Extended blitter registers (0xF0488 - 0xF0498) */
#define IE_BLT_FLAGS       0x000F0488UL  /* BPP, draw mode, JAM1/invert flags */
#define IE_BLT_FG          0x000F048CUL  /* Foreground color for color expand */
#define IE_BLT_BG          0x000F0490UL  /* Background color for color expand */
#define IE_BLT_MASK_MOD    0x000F0494UL  /* Template/mask row stride (bytes) */
#define IE_BLT_MASK_SRCX   0x000F0498UL  /* Start X bit offset in template */

/* Blitter operation codes (IE_BLT_OP) */
#define IE_BLT_OP_COPY         0U
#define IE_BLT_OP_FILL         1U
#define IE_BLT_OP_LINE         2U
#define IE_BLT_OP_MASKED       3U
#define IE_BLT_OP_ALPHA        4U
#define IE_BLT_OP_MODE7        5U
#define IE_BLT_OP_COLOR_EXPAND 6U
#define IE_BLT_OP_SCALE        7U
#define IE_BLT_OP_MEMCOPY      8U

/* Blitter control bits (IE_BLT_CTRL) */
#define IE_BLT_CTRL_START      0x00000001UL
#define IE_BLT_CTRL_BUSY       0x00000002UL
#define IE_BLT_CTRL_IRQ        0x00000004UL

/* BLT_FLAGS bit definitions. NOTE: a BLT_FLAGS value of 0 is the legacy
 * "RGBA32 Copy" default in the IE chip, so RGBA32 Copy must be expressed as
 * flags == 0 (do not OR in a zero draw-mode expecting Clear). */
#define IE_BLT_FLAGS_BPP_RGBA32     0x00U
#define IE_BLT_FLAGS_BPP_CLUT8      0x01U
#define IE_BLT_FLAGS_BPP_MASK       0x03U
#define IE_BLT_FLAGS_DRAWMODE_SHIFT 4U
#define IE_BLT_FLAGS_DRAWMODE_MASK  0xF0U
#define IE_BLT_FLAGS_JAM1           0x00000100UL  /* Template: skip BG pixels */
#define IE_BLT_FLAGS_INVERT_TMPL    0x00000200UL  /* Invert template bits */
#define IE_BLT_FLAGS_INVERT_MODE    0x00000400UL  /* XOR dst for set template bits */
#define IE_BLT_FLAGS_MASK_MSB       0x00000800UL  /* Masked copy: MSB-first sampling */
#define IE_BLT_FLAGS_ALPHA_TMPL     0x00001000UL  /* Alpha copy: src is 8bpp alpha plane */

/* Draw mode codes (16 raster ops). In RGBA32 only COPY/CLEAR/SET operate
 * correctly because the ops act on the full pixel including alpha; the other
 * 13 are index-correct only in CLUT8 mode. */
#define IE_BLT_DM_CLEAR  0x00U
#define IE_BLT_DM_COPY   0x03U
#define IE_BLT_DM_XOR    0x06U
#define IE_BLT_DM_SET    0x0FU

#define IE_BLT_MAKE_FLAGS(bpp, drawmode) \
    (((ULONG)(bpp) & 0x03UL) | (((ULONG)(drawmode) & 0x0FUL) << 4))

/* Copper (0xF000C - 0xF0018) */
#define IE_COPPER_CTRL     0x000F000CUL
#define IE_COPPER_PTR      0x000F0010UL
#define IE_COPPER_STATUS   0x000F0018UL

#define IE_PSG_BASE        0x000F0C00UL

#define IE_MOUSE_X         0x000F0730UL
#define IE_MOUSE_Y         0x000F0734UL
#define IE_MOUSE_BUTTONS   0x000F0738UL
#define IE_MOUSE_STATUS    0x000F073CUL

#define IE_SCAN_CODE       0x000F0740UL
#define IE_SCAN_STATUS     0x000F0744UL
#define IE_SCAN_MODIFIERS  0x000F0748UL

#define IE_TERM_OUT        0x000F0700UL

/* Timer levels delivered by IE runtime */
#define IE_IRQ_TIMER_LEVEL 5U
#define IE_IRQ_VBL_LEVEL   4U

/* Simple MMIO helpers */
#define IE_MMIO8(a)   (*(volatile unsigned char *)(a))
#define IE_MMIO16(a)  (*(volatile unsigned short *)(a))
#define IE_MMIO32(a)  (*(volatile unsigned long *)(a))

void ie_screen_init(void);
void ie_putpixel(unsigned short x, unsigned short y, unsigned long rgba);
void ie_clear(unsigned char r, unsigned char g, unsigned char b);

unsigned short ie_kbd_has_code(void);
unsigned short ie_kbd_code(void);
unsigned short ie_kbd_mods(void);
void ie_kbd_enqueue(unsigned short scancode);

unsigned short ie_mouse_x(void);
unsigned short ie_mouse_y(void);
unsigned short ie_mouse_buttons(void);
unsigned short ie_mouse_changed(void);

void ie_timer_install(void);
void ie_timer_handle_l5(void);
void ie_timer_handle_l4(void);

void ie_psg_write(unsigned int reg, unsigned int value);

void ie_fill_rect(unsigned short x, unsigned short y, unsigned short w, unsigned short h, unsigned long rgba);
void ie_scroll_up_pixels(unsigned short lines, unsigned long bg_rgba);

/* Pack RGBA bytes into a big-endian ULONG for direct VRAM writes */
static inline unsigned long ie_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    return ((unsigned long)r << 24) | ((unsigned long)g << 16) | ((unsigned long)b << 8) | (unsigned long)a;
}

#endif
