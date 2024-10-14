#include "output.h"
#include <SDL.h>           // For SDL2
#include <SDL_mixer.h>      // For SDL2_mixer

#include <stdio.h>

static uint8_t *output_ports;
static uint16_t shift_register = 0;  // 16-bit shift register
static uint8_t shift_offset = 0;     // 3-bit shift amount

Mix_Chunk* sound_ufo = NULL;
Mix_Chunk* sound_shot = NULL;
Mix_Chunk* sound_player_die = NULL;
Mix_Chunk* sound_invader_die = NULL;

void output_init() {
    output_ports = (uint8_t *)malloc(NUM_OUTPUT_PORTS);
    memset(output_ports, 0, NUM_OUTPUT_PORTS);

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        printf("SDL Mixer initialization failed: %s\n", Mix_GetError());
        return;
    }

    sound_ufo = Mix_LoadWAV("sounds/ufo.wav");
    sound_shot = Mix_LoadWAV("sounds/shot.wav");
    sound_player_die = Mix_LoadWAV("sounds/player_die.wav");
    sound_invader_die = Mix_LoadWAV("sounds/invader_die.wav");
}

void free_output_ports() {
    free(output_ports);
    output_ports = NULL;

    Mix_FreeChunk(sound_ufo);
    Mix_FreeChunk(sound_shot);
    Mix_FreeChunk(sound_player_die);
    Mix_FreeChunk(sound_invader_die);
    Mix_CloseAudio();
}

uint8_t output_read(uint8_t port) {
    return output_ports[port];
}

void output_write(uint8_t port, uint8_t value) {
    output_ports[port] = value;

    switch (port) {
        case 2:
            shift_offset = value & 0x07;  // 3-bit shift offset
            break;
        case 4:
            shift_register = (shift_register >> 8) | (value << 8);
            break;
        case 3:
            handle_sound_effects(value);
            break;
        case 5:
            handle_sound_effects(value);
            break;
        case 6:
            // Watchdog reset logic (if needed)
            break;
        default:
            break;
    }
}

uint8_t output_read_shift_register() {
    return shift_register >> (8 - shift_offset);
}

void handle_sound_effects(uint8_t value) {
    if (value & 0x01) Mix_PlayChannel(-1, sound_ufo, 0);
    if (value & 0x02) Mix_PlayChannel(-1, sound_shot, 0);
    if (value & 0x04) Mix_PlayChannel(-1, sound_player_die, 0);
    if (value & 0x08) Mix_PlayChannel(-1, sound_invader_die, 0);
}
