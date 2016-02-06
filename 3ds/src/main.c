#include <3ds.h>
#include <stdio.h>
#include "../su/libsu.h"
#include <brahma.h>

#warning "This is a concept only! Everything is wrong!"
// The following only works if IO is already in the MMU...
#define REG_NTRCARDROMCNT (*(vu32*)(0x10164004 + 0xEB00000))

// #shitcode time

bool exit(void) {
    hidScanInput();
    gfxFlushBuffers();
    gfxSwapBuffers();

    if(hidKeysDown() & KEY_START) return true;
    return false;
}

void cleanup(void) {
    gfxExit();
        // sdmcExit();
}

s32 overflow_buffer(void) {
    REG_NTRCARDROMCNT = 0x883F1FFF | (6 << 24); // TODO: Less magic?
    return 0;
}

s32 priv_brahma_stuff(void) {
    __asm__ volatile ("cpsid aif"); // Copied from Brahma2
    u32 *save = (u32 *)(g_expdata.va_fcram_base + 0x3FFFE00);
    save[0] = topFramebufferInfo.framebuf0_vaddr;
    save[1] = topFramebufferInfo.framebuf1_vaddr;
    save[2] = bottomFramebufferInfo.framebuf0_vaddr;

    // Working around a GCC bug to translate the va address to pa...
    save[0] += 0xC000000;  // (pa FCRAM address - va FCRAM address)
    save[1] += 0xC000000;
    save[2] += 0xC000000;

    map_arm9_payload();
    return 0;
}

int main(int argc, char **argv) {
    gfxinitDefault();
        // sdmcInit();
    consoleInit(GFX_TOP, NULL);
    printf("ntrcardhax assistant\nDon't get your hopes up.\n\n");
    if(suInit()) {
        printf("Privilege Escalation failed!\nPress START to exit.\n");
        while (aptMainLoop() && !exit()) ;
        cleanup();
        return -1;
    }

    bool error = false;
    printf("Setting up payload area...");
    brahma_init(); // TODO: Actually check for error codes.
    load_arm9_payload_offset("/payload.bin", 0, 0);
    setup_exploit_data();
    svcBackdoor(priv_brahma_stuff);

    printf("Insert cartridge now, or press START to exit.\n\n");
    // Wait for ARM9 to begin reading the cartridge...
    while (~REG_NTRCARDROMCNT & (1 << 31)) {
        if (exit()) {
            cleanup();
            return 0;
        }
    }

    svcBackdoor(overflow_buffer);
    printf("Profit?\nPress START to exit.\n");
    while (aptMainLoop() && !exit()) ;
    cleanup(); return 0;
}
