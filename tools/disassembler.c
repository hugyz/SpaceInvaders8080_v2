#include "disassembler.h"
#include "instructions.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
/**
 * Disassembles single instruction
 *
 * @param code  The code
 * @param pc    The program counter
 * @return      Position of the next program counter PC
 */
unsigned disassemble_instruction(unsigned char *buffer, size_t buffer_size, unsigned pc) {
    if(pc > SIZE_MAX) error("program counter surpasses max value of the size_t");
    if((size_t)pc > buffer_size) error("program counter out of bounds");

    unsigned char *opcode = &buffer[pc];
    if (*opcode > buffer_size) error("invalid opcode");
    Instruction instruction = disassembler_instruction_table[*opcode];
    if (pc + instruction.bytes > buffer_size) error("invalid instruction");

    if (!instruction.mnemonic) error("invalid instruction name");
    size_t length = strlen(instruction.mnemonic) + 1;  // +1 for null terminator
    char* duplicate = (char*)malloc(length * sizeof(char));
    if (!duplicate) error("duplicate pointer assignment error");
    strcpy(duplicate, instruction.mnemonic);
    
    switch(instruction.bytes) {
        case 1:
            execute_instruction(duplicate, instruction.bytes);
            printf("%s\n", instruction.mnemonic);
            break;
        case 2:
            execute_instruction(duplicate, instruction.bytes);
            printf("%-12s0x%02x\n", instruction.mnemonic, opcode[1]);
            break;
        case 3:
            execute_instruction(duplicate, instruction.bytes);
            printf("%-12s0x%02x%02x\n", instruction.mnemonic, opcode[2], opcode[1]);
            break;
        default:
            error("invalid instructions.bytes");
    }
    return instruction.bytes;
}