#ifndef OUTPUT_H
#define OUTPUT_H

#include "cpu.h"

#include <stdlib.h>
#include <stdint.h>

#define NUM_OUTPUT_PORTS 5  // 5 output ports used in Space Invaders

void audio_init();
void audio_free();

uint8_t output_read(uint8_t port);
void output_write(uint8_t port, uint8_t value);

uint8_t read_shift_register();
void handle_sound_effects(uint8_t value);

// Function for handling output to the machine (sound, shift registers)
void machine_out(CPU *cpu, uint8_t state, uint8_t port);

#endif  // OUTPUT_H
