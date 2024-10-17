#include "update_flags.h"
#include "cpu.h"
#include "utils.h"
#include <stdint.h>

void reset_flags(CPU *cpu) {
    cpu->flags->Z = 0;
    cpu->flags->S = 0;
    cpu->flags->P = 0;
    cpu->flags->CY = 0;
    cpu->flags->AC = 0;
    cpu->flags->PAD = 1;  // Bit 1 is always set in 8080
}

void update_SZP(CPU *cpu, uint8_t value) {
    cpu->flags->Z = (value == 0);
    cpu->flags->S = ((value & 0x80) == 0x80);
    cpu->flags->P = parity(value);
}

void update_AC(CPU *cpu, uint8_t before, uint8_t after) {
    cpu->flags->AC = ((before & 0xF) + (after & 0xF)) > 0xF;
}

void update_CY_8bit(CPU *cpu, uint16_t result) {
    cpu->flags->CY = (result > 0xFF);
}

void update_CY_16bit(CPU *cpu, uint32_t result) {
    cpu->flags->CY = (result > 0xFFFF);
}