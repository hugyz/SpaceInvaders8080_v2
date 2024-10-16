#ifndef UPDATE_FLAGS_H
#define UPDATE_FLAGS_H

#include "cpu.h"

#include <stdint.h>

void reset_flags(CPU *cpu, uint16_t value);

void update_byte_SZAP(CPU *cpu, uint8_t value);
void update_half_word_SZAP(CPU *cpu, uint16_t value);
void update_word_SZAP(CPU *cpu, uint32_t value);

void update_byte_CY(CPU *cpu, uint8_t value);
void update_half_word_CY(CPU *cpu, uint16_t value);
void update_word_CY(CPU *cpu, uint32_t value);




#endif