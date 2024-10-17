#include "video.h"
#include "memory.h"
#include "cpu.h"
#include <SDL.h>
#include <stdio.h>

void update_texture(SDL_Texture* texture, CPU *cpu) {
    uint32_t* pixels;
    int pitch;

    if (SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch) != 0) {
        printf("Failed to lock texture: %s\n", SDL_GetError());
        return;
    }

    // Iterate over the screen dimensions to update pixel data from VRAM
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int byte_index = VRAM_START + (y * 32) + (x / 8);  // 32 bytes per row
            int bit_index = 7 - (x % 8);  // Which bit in the byte corresponds to the x position
            
            // Add a debug print to verify byte_index and bit_index are correct
            uint8_t byte = read_memory(byte_index);
            if (y == 0 && x < 10) {  // Limit prints for readability
                printf("VRAM Byte: 0x%04X, Value: 0x%02X, BitIndex: %d\n", byte_index, byte, bit_index);
            }

            uint32_t color = (byte & (1 << bit_index)) ? 0xFFFFFFFF : 0xFF000000;  // White or black
            pixels[y * (pitch / sizeof(uint32_t)) + x] = color;
        }
    }

    SDL_UnlockTexture(texture);
}


void sync_to_real_time() {
    static uint32_t last_time = 0;
    uint32_t current_time = SDL_GetTicks();
    uint32_t frame_time = 1000 / FRAMES_PER_SECOND;

    uint32_t elapsed_time = current_time - last_time;

    if (elapsed_time < frame_time) {
        SDL_Delay(frame_time - elapsed_time);
    }

    // Update last_time to the current time after syncing
    last_time = SDL_GetTicks();
}
