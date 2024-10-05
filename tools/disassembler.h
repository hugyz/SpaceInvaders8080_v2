#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdlib.h>

unsigned disassemble_instruction(unsigned char *buffer, size_t buffer_size, unsigned pc);

#endif