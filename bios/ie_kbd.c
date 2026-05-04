#include "emutos.h"
#include "ikbd.h"
#include "ie_machine.h"

/*
 * ST-compatible make/break scancode MMIO helpers.
 *
 * IE runtimes that drain SCAN_CODE outside the guest must inject those raw
 * scancodes through ie_kbd_enqueue(), so EmuTOS updates modifier state and
 * key repeat through kbd_int(). Target code must not also poll IE_SCAN_CODE
 * from kb_timerc_int(); that MMIO read dequeues from the host event queue and
 * races the runtime scancode pump.
 */
unsigned short ie_kbd_has_code(void) { return IE_MMIO16(IE_SCAN_STATUS); }
unsigned short ie_kbd_code(void) { return IE_MMIO16(IE_SCAN_CODE); }
unsigned short ie_kbd_mods(void) { return IE_MMIO16(IE_SCAN_MODIFIERS); }

void ie_kbd_enqueue(unsigned short scancode)
{
    kbd_int((UBYTE)scancode);
}
