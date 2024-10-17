#ifndef UPDATE_FLAGS_H
#define UPDATE_FLAGS_H

#include "cpu.h"
#include <stdint.h>

void reset_flags(CPU *cpu);
void update_SZP(CPU *cpu, uint8_t value);
void update_AC(CPU *cpu, uint8_t before, uint8_t after);
void update_CY_8bit(CPU *cpu, uint16_t result);
void update_CY_16bit(CPU *cpu, uint32_t result);

#endif // UPDATE_FLAGS_H