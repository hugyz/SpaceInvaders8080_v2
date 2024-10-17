#ifndef VIDEO_H
#define VIDEO_H

#include <SDL.h>
#include "cpu.h"

#define SCREEN_WIDTH 224
#define SCREEN_HEIGHT 256
#define VRAM_START 0x2400
#define FRAMES_PER_SECOND 60

void update_texture(SDL_Texture* texture, CPU *cpu);
void sync_to_real_time();

#endif
