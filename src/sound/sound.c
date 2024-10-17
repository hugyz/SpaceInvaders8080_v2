#include "sound.h"
#include <SDL_mixer.h>
#include <stdint.h>
#include <stdio.h>

static Mix_Chunk* sound_effects[NUM_SOUND_EFFECTS]; // Array for sound effects

void audio_init() {
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        printf("SDL Mixer initialization failed: %s\n", Mix_GetError());
        return;
    }

    // Load sound effects
    sound_effects[0] = Mix_LoadWAV("sounds/ufo.wav");
    sound_effects[1] = Mix_LoadWAV("sounds/shot.wav");
    sound_effects[2] = Mix_LoadWAV("sounds/player_die.wav");
    sound_effects[3] = Mix_LoadWAV("sounds/invader_die.wav");
    
    // Ensure all sound effects are loaded properly
    for (int i = 0; i < NUM_SOUND_EFFECTS; i++) {
        if (sound_effects[i] == NULL) {
            printf("Failed to load sound %d: %s\n", i, Mix_GetError());
        }
    }
}

void audio_free() {
    for (int i = 0; i < NUM_SOUND_EFFECTS; i++) {
        Mix_FreeChunk(sound_effects[i]);
    }
    Mix_CloseAudio();
}

void play_sound(int index) {
    if (index >= 0 && index < NUM_SOUND_EFFECTS) {
        Mix_PlayChannel(-1, sound_effects[index], 0);  // Play sound on any free channel
    } else {
        printf("Invalid sound index: %d\n", index);
    }
}
