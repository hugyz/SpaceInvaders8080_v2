#ifndef UPDATE_FLAGS_H
#define UPDATE_FLAGS_H

#include "cpu.h"

#include <stdint.h>

void reset_flags(CPU *cpu, uint16_t value);
void update_SZAP(CPU *cpu, uint16_t value);
void update_CY(CPU *cpu, uint16_t value);
void update_SZAPCY(CPU *cpu, uint16_t value);

#endif