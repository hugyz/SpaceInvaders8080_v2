#ifndef SOUND_H
#define SOUND_H

#define NUM_SOUND_EFFECTS 4  // Update this based on the number of sound effects

void audio_init();
void audio_free();
void play_sound(int index);

#endif
