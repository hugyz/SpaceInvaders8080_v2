#include "update_flags.h"

#include "cpu.h"
#include "utils.h"

#include <stdint.h>

void reset_flags(CPU *cpu, uint16_t value) {
    cpu->flags->Z = 0;
    cpu->flags->S = 0;
    cpu->flags->P = 0;
    cpu->flags->CY = 0;
    cpu->flags->AC = 0;
    cpu->flags->PAD = 0;
}

void update_byte_SZAP(CPU *cpu, uint16_t value) {
    cpu->flags->Z = (value == 0);
    cpu->flags->S = ((value & 0x80) == 0x80);
	cpu->flags->P = parity(value);
    cpu->flags->AC = ((cpu->A & 0xF) + (value & 0xF)) > 0xF; 
}

void update_byte_CY(CPU *cpu, uint16_t value) {
    cpu->flags->CY = (value > 0xff);
}

void update_word_CY(CPU *cpu, uint16_t value) {
    cpu->flags->CY = (value > 0xffff);
}