#include <3ds.h>
#include <stdio.h>
#include "../su/libsu.h"

#warning "This is a concept only! I probably did this wrong!"

#define REG_NTRCARDROMCNT (*(vu32*)0x10164004)

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
    printf("Insert cartridge now, or press START to exit.\n\n");
    // Wait for ARM9 to begin reading the cartridge...
    while (~REG_NTRCARDROMCNT & (1 << 31)) {
        if (exit()) {
            cleanup();
            return 0;
        }
    }
    REG_NTRCARDROMCNT = 0x883F1FFF | (6 << 24);
    printf("Profit?\nPress START to exit.\n");
    while (aptMainLoop() && !exit()) ;
    cleanup(); return 0;
}
