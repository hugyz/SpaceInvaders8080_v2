#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdint.h>
#include "cpu.h"

#define NUM_OUTPUT_PORTS 8

uint8_t output_read(uint8_t port);
void machine_out(CPU *cpu, uint8_t port, uint8_t value);

#endif
