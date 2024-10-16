#include "output.h"
#include "cpu.h"
#include <SDL.h>           // For SDL2
#include <SDL_mixer.h>      // For SDL2_mixer

#include <stdio.h>

static uint8_t output_ports[NUM_OUTPUT_PORTS];
static uint16_t shift_register = 0;  // 16-bit shift register
static uint8_t shift_offset = 0;     // 3-bit shift amount

Mix_Chunk* sound_ufo = NULL;
Mix_Chunk* sound_shot = NULL;
Mix_Chunk* sound_player_die = NULL;
Mix_Chunk* sound_invader_die = NULL;

void audio_init() {
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        printf("SDL Mixer initialization failed: %s\n", Mix_GetError());
        return;
    }

    sound_ufo = Mix_LoadWAV("sounds/ufo.wav");
    sound_shot = Mix_LoadWAV("sounds/shot.wav");
    sound_player_die = Mix_LoadWAV("sounds/player_die.wav");
    sound_invader_die = Mix_LoadWAV("sounds/invader_die.wav");
}

void audio_free() {
    Mix_FreeChunk(sound_ufo);
    Mix_FreeChunk(sound_shot);
    Mix_FreeChunk(sound_player_die);
    Mix_FreeChunk(sound_invader_die);
    Mix_CloseAudio();
}

uint8_t output_read(uint8_t port) {
    if(port == 5) error("watchdog implementation");
    return output_ports[port];
}

void output_write(uint8_t port, uint8_t value) {
    if(port == 5) error("watchdog implementation");
    output_ports[port] = value;
}


uint8_t read_shift_register() {
    return (uint8_t)(shift_register >> (shift_offset + 8)) & 0xFF;
}

void handle_sound_effects(uint8_t value) {
    if (value & 0x01) Mix_PlayChannel(-1, sound_ufo, 0);
    if (value & 0x02) Mix_PlayChannel(-1, sound_shot, 0);
    if (value & 0x04) Mix_PlayChannel(-1, sound_player_die, 0);
    if (value & 0x08) Mix_PlayChannel(-1, sound_invader_die, 0);
}

void machine_out(CPU *cpu, uint8_t port, uint8_t value) {
    output_ports[port] = value;

    switch (port) {
        case 2:
            shift_offset = value & 0x7;
            break;
        case 4:
            shift_register = (uint16_t)(value << 8) | (shift_register >> 8);
            break;
        case 3:
            handle_sound_effects(value);
            break;
        case 5:
            handle_sound_effects(value);
            break;
        case 6:
            cpu_reset(cpu);
            break;
        default: {
            error("not a port #");
            break;
        } 
    }
}