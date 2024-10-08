#include "cpu.h"

#include "memory.h"
#include "update_flags.h"

#include "utils.h"
#include "input.h"
#include "output.h"

#include <stdlib.h>

//memory: stored in mem.c
//i/o: stored in i/o respectively

CPU* cpu_init(void) {
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    if (!cpu) error("cpu init failed");

    cpu->flags = (Flags*)malloc(sizeof(Flags));
    if (!cpu->flags) {
        free(cpu);
        error("flags init failed");
    }
    cpu->flags->Z = cpu->flags->S = cpu->flags->P = cpu->flags->CY = cpu->flags->AC = cpu->flags->PAD = 0;

    cpu->callback = (Callbacks*)malloc(sizeof(Callbacks));
    if (!cpu->callback) {
        free(cpu->flags);
        free(cpu);
        error("callback init failed");
    }
    cpu->callback->in = cpu->callback->out = NULL;

    cpu->A = 0;
    cpu->B = cpu->C = 0;
    cpu->D = cpu->E = 0;
    cpu->H = cpu->L = 0;
    cpu->SP = 0;
    cpu->PC = 0;

    cpu->num_steps = 0;
    cpu->interrupts_enabled = 0;
    cpu->cycles = 0;

    memory_init();
    load_rom_into_mem();

    input_init();
    output_init();

    return cpu;
}

void cpu_free(CPU* cpu) {
    if (cpu && cpu->flags && cpu->callback) {
        free(cpu->flags);
        cpu->flags = NULL;
        free(cpu->callback);
        cpu->callback = NULL; 
        free(cpu);
        cpu = NULL;
    }
    else error("no instance of cpu/flags/callback when freeing");
}

void cpu_reset(CPU* cpu) {
    if (cpu && cpu->flags && cpu->callback) {
        cpu->A = 0;
        cpu->B = cpu->C = 0;
        cpu->D = cpu->E = 0;
        cpu->H = cpu->L = 0;
        cpu->SP = 0;
        cpu->PC = 0;

        cpu->flags->Z = 0;
        cpu->flags->S = 0;
        cpu->flags->P = 0;
        cpu->flags->CY = 0;
        cpu->flags->AC = 0;
        cpu->flags->PAD = 0;

        cpu->callback->in = NULL;
        cpu->callback->out = NULL;

        cpu->num_steps = 0;
        cpu->interrupts_enabled = 0;
        cpu->cycles = 0;
    }
    else error("no instance of cpu/flags/callback when resetting");
}

void emulate_8080(CPU *cpu) {/*
    while (1) {
        // Fetch, decode, and execute the next instruction
        execute_instruction(cpu);
        
        // Handle user inputs (e.g., movement, shooting)
        handle_input(cpu);
        
        // Update the display to reflect any changes in the game
        render_display(cpu);
        
        // Optionally: Handle sound and timing
        // sound_emulation(cpu);
        // sleep_for_frame();
    }
    */
}

uint16_t read_opcode_data_word(CPU *cpu) {
    uint16_t value = ((uint16_t)(cpu->PC+2) << 8) | (uint16_t)(cpu->PC+1);
    return value;
} 


void setCallbackIn(CPU *cpu, CallbackIn callback) {
    if(cpu && cpu->callback) cpu->callback->in = callback;
    else error("cpu/callback does not exist");
}
void setCallbackOut(CPU *cpu, CallbackOut callback) {
    if(cpu && cpu->callback) cpu->callback->out = callback;
    else error("cpu/callback does not exist");
}


void interrupt(CPU *cpu, int interrupt_num) {
    if (!cpu->interrupts_enabled) {
        return;
    }

    // Push PC to stack
    uint8_t pclo = (uint8_t)(cpu->PC & 0xff);
    uint8_t pchi = (uint8_t)((cpu->PC >> 8) & 0xff);
    write_memory(cpu->SP - 1, pchi);
    write_memory(cpu->SP - 2, pclo);
    cpu->SP -= 2;

    // Jump to interrupt vector
    cpu->PC = 8 * interrupt_num;
}



uint64_t getNumSteps(const CPU* cpu){
    return cpu->num_steps;
}


uint8_t cpu_execute_instruction(CPU* cpu) {
    
    char *opcode = read_memory(cpu->PC);
    uint16_t opcode_size = 1;           //default bytes taken by instruction

    switch (*opcode) {
        case 0x00: {         //NOP
            break;
        }
        case 0x01: {         //LXI B, D16
            cpu->C = read_memory(cpu->PC + 1);
            cpu->B = read_memory(cpu->PC + 2);
            opcode_size = 3;
            break;
        }
        case 0x02: {         //STAX B
            uint16_t address = (cpu->B << 8) | (cpu->C);
            write_memory(address, cpu->A);
            break;
        }
        case 0x03: {         //INX B
            uint16_t value = ((cpu->B << 8) | cpu->C) + 1; 
            cpu->C = value;
            cpu->B = value >> 8;
            break;
        }
        case 0x04: {         //INR B
            cpu->B += 1;
            update_byte_SZAP(cpu, cpu->B);
            break;
        }
        case 0x05: {         //DCR B
            cpu->B -= 1;
            update_byte_SZAP(cpu, cpu->B);
            break;
        }
        case 0x06: {         //MVI B, D8      
            cpu->B = read_memory(cpu->PC + 1);
            opcode_size = 2;
            break;
        }
        case 0x07: {         //RLC
            cpu->flags->CY = (cpu->A >> 7) & 1;
            cpu->A = (cpu->A << 1) | cpu->flags->CY;
            break;
        }
        case 0x08: {         //*NOP
            break;
        }
        case 0x09: {         //DAD B (Add BC to HL)
            uint16_t hl = (cpu->H << 8) + cpu->L;
            uint16_t bc = (cpu->B << 8) + cpu->C;
            uint32_t value = hl + bc;
            update_word_CY(cpu, value);
            cpu->H = (value >> 8) & 0xff;
            cpu->L = value & 0xff;
            break;
        }
        case 0x0A: {         //LDAX B (Load A from address in BC)
            uint16_t address = (cpu->B << 8) + cpu->C;
            cpu->A = read_memory(address);
            break;
        }
        case 0x0B: {         //DCX B
            uint16_t bc = (cpu->B << 8) | cpu->C;
            bc -= 1;
            cpu->B = (bc >> 8) & 0xff;
            cpu->C = bc & 0xff;
            break;
        }
        case 0x0C: {         //INR C
            cpu->C += 1;
            update_byte_SZAP(cpu, cpu->C);
            break;
        }
        case 0x0D: {         //DCR C
            cpu->C -= 1;
            update_byte_SZAP(cpu, cpu->C);
            break;
        }
        case 0x0E: {         //MVI C, D8
            cpu->C = read_memory(cpu->PC + 1);
            opcode_size = 2;
            break;
        }
        case 0x0F: {         //RRC
            cpu->flags->CY = cpu->A & 0x01;
            cpu->A = (cpu->A >> 1) | (cpu->flags->CY << 7);
            break;
        }
        case 0x11: {         //LXI D, D16
            cpu->E = read_memory(cpu->PC + 1);
            cpu->D = read_memory(cpu->PC + 2);
            opcode_size = 3;
            break;
        }
        case 0x12: {         //STAX D
            uint16_t address = (cpu->D << 8) | cpu->E;
            write_memory(address, cpu->A);
            break;
        }
        case 0x13: {         //INX D
            uint16_t value = (cpu->D << 8) | cpu->E;
            value += 1;
            cpu->E = value & 0xff;
            cpu->D = (value >> 8) & 0xff;
            break;
        }
        case 0x14: {         //INR D
            cpu->D += 1;
            update_byte_SZAP(cpu, cpu->D);
            break;
        }
        case 0x15: {         //DCR D
            cpu->D -= 1;
            update_byte_SZAP(cpu, cpu->D);
            break;
        }
        case 0x16: {         //MVI D, D8
            cpu->D = read_memory(cpu->PC + 1);
            opcode_size = 2;
            break;
        }
        case 0x17: {         //RAL
            uint8_t bit7 = (cpu->A >> 7) & 1;
            uint8_t bit0 = cpu->flags->CY;
            cpu->A = (cpu->A << 1) | bit0;
            cpu->flags->CY = bit7;
            break;
        }
        case 0x18: {         //*NOP
            break;
        }
        case 0x19: {         //DAD D
            uint32_t hl = (cpu->H << 8) | cpu->L;
            uint32_t de = (cpu->D << 8) | cpu->E;
            uint32_t result = hl + de;
            update_word_CY(cpu, result);
            cpu->H = (result >> 8) & 0xff;
            cpu->L = result & 0xff;
            break;
        }
        case 0x1A: {         //LDAX D
            uint16_t address = (cpu->D << 8) | cpu->E;
            cpu->A = read_memory(address);
            break;
        }
        case 0x1B: {         //DCX D
            uint16_t de = (cpu->D << 8) | cpu->E;
            de -= 1;
            cpu->D = (de >> 8) & 0xff;
            cpu->E = de & 0xff;
            break;
        }
        case 0x1C: {         //INR E
            cpu->E += 1;
            update_byte_SZAP(cpu, cpu->E);
            break;
        }
        case 0x1D: {         //DCR E
            cpu->E -= 1;
            update_byte_SZAP(cpu, cpu->E);
            break;
        }
        case 0x1E: {         //MVI E, D8
            cpu->E = read_memory(cpu->PC + 1);
            opcode_size = 2;
            break;
        }
        case 0x1F: {         //RAR
            uint8_t bit0 = cpu->A & 1;
            uint8_t bit7 = cpu->flags->CY;
            cpu->A = (cpu->A >> 1) | (bit7 << 7);
            cpu->flags->CY = bit0;
            break;
        }
        case 0x21: {         //LXI H, D16
            cpu->H = read_memory(cpu->PC + 2);
            cpu->L = read_memory(cpu->PC + 1);
            opcode_size = 3;
            break;
        }
        case 0x22: {         //SHLD
            uint16_t address = read_opcode_data_word(cpu);
            write_memory(address, cpu->L);
            write_memory(address + 1, cpu->H);
            opcode_size = 3;
            break;
        }
        case 0x23: {         //INX H
            uint16_t value = (cpu->H << 8) | cpu->L;
            value += 1;
            cpu->H = (value >> 8) & 0xff;
            cpu->L = value & 0xff;
            break;
        }
        case 0x24: {         //INR H
            cpu->H += 1;
            update_byte_SZAP(cpu, cpu->H);
            break;
        }
        case 0x25: {         //DCR H
            cpu->H -= 1;
            update_byte_SZAP(cpu, cpu->H);
            break;
        }
        case 0x26: {         //MVI H, D8
            cpu->H = read_memory(cpu->PC + 1);
            opcode_size = 2;
            break;
        }
        case 0x27: {         //DAA
            if ((cpu->A & 0x0f) > 9)
                cpu->A += 6;
            if (cpu->flags->CY || ((cpu->A & 0xf0) > 0x90)) {
                cpu->A += 0x60;
                cpu->flags->CY = 1;
            }
            update_byte_SZAP(cpu, cpu->A);
            break;
        }
        case 0x29: {         //DAD H
            uint32_t hl = (cpu->H << 8) | cpu->L;
            uint32_t result = hl + hl;
            update_word_CY(cpu, result);
            cpu->H = (result >> 8) & 0xff;
            cpu->L = result & 0xff;
            break;
        }
        case 0x2A: {         //LHLD
            uint16_t address = read_opcode_data_word(cpu);
            cpu->L = read_memory(address);
            cpu->H = read_memory(address + 1);
            opcode_size = 3;
            break;
        }
        case 0x2B: {         //DCX H
            uint16_t value = (cpu->H << 8) | cpu->L;
            value -= 1;
            cpu->H = (value >> 8) & 0xff;
            cpu->L = value & 0xff;
            break;
        }
        case 0x2C: {         //INR L
            cpu->L += 1;
            update_byte_SZAP(cpu, cpu->L);
            break;
        }
        case 0x2D: {         //DCR L
            cpu->L -= 1;
            update_byte_SZAP(cpu, cpu->L);
            break;
        }
        case 0x2E: {         //MVI L, D8
            cpu->L = read_memory(cpu->PC + 1);
            opcode_size = 2;
            break;
        }
        case 0x2F: {         //CMA
            cpu->A = ~cpu->A;
            break;
        }
        case 0x31: {         //LXI SP, D16
            cpu->SP = read_opcode_data_word(cpu);
            opcode_size = 3;
            break;
        }
        case 0x32: {         //STA adr
            uint16_t address = read_opcode_data_word(cpu);
            write_memory(address, cpu->A);
            opcode_size = 3;
            break;
        }
        case 0x33: {         //INX SP
            cpu->SP += 1;
            break;
        }
        case 0x34: {         //INR M
            uint16_t address = (cpu->H << 8) | cpu->L;
            uint16_t value = read_memory(address);
            value += 1;
            update_byte_SZAP(cpu, value);
            write_memory(address, value & 0xff);
            break;
        }
        case 0x35: {         //DCR M
            uint16_t address = (cpu->H << 8) | cpu->L;
            uint16_t value = read_memory(address);
            value -= 1;
            update_byte_SZAP(cpu, value);
            write_memory(address, value & 0xff);
            break;
        }
        case 0x36: {         //MVI M, D8
            uint16_t value = read_memory(cpu->PC + 1);
            uint16_t address = (cpu->H << 8) | cpu->L;
            write_memory(address, value);
            opcode_size = 2;
            break;
        }
        case 0x37: {         //STC
            cpu->flags->CY = 1;
            break;
        }
        case 0x39: {         //DAD SP
            uint32_t result = ((cpu->H << 8) | cpu->L) + cpu->SP;
            update_word_CY(cpu, result);
            cpu->H = (result >> 8) & 0xff;
            cpu->L = result & 0xff;
            break;
        }
        case 0x3A: {         //LDA word
            uint16_t address = read_opcode_data_word(cpu);
            cpu->A = read_memory(address);
            opcode_size = 3;
            break;
        }
        case 0x3B: {         //DCX SP
            cpu->SP -= 1;
            break;
        }
        case 0x3C: {         //INR A
            cpu->A += 1;
            update_byte_SZAP(cpu, cpu->A);
            break;
        }
        case 0x3D: {         //DCR A
            cpu->A -= 1;
            update_byte_SZAP(cpu, cpu->A);
            break;
        }
        case 0x3E: {         //MVI A, byte
            cpu->A = read_memory(cpu->PC + 1);
            opcode_size = 2;
            break;
        }
        case 0x3F: {         //CMC
            cpu->flags->CY = 1 - cpu->flags->CY;
            break;
        }
        case 0x41: {         //MOV B,C
            cpu->B = cpu->C;
            break;
        }
        case 0x42: {         //MOV B,D
            cpu->B = cpu->D;
            break;
        }
        case 0x43: {         //MOV B,E
            cpu->B = cpu->E;
            break;
        }
        case 0x44: {         //MOV B,H
            cpu->B = cpu->H;
            break;
        }
        case 0x45: {         //MOV B,L
            cpu->B = cpu->L;
            break;
        }
        case 0x46: {         //MOV B,M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->B = read_memory(address);
            break;
        }
        case 0x47: {         //MOV B,A
            cpu->B = cpu->A;
            break;
        }
        case 0x48: {         //MOV C,B
            cpu->C = cpu->B;
            break;
        }
        case 0x4A: {         //MOV C,D
            cpu->C = cpu->D;
            break;
        }
        case 0x4B: {         //MOV C,E
            cpu->C = cpu->E;
            break;
        }
        case 0x4C: {         //MOV C,H
            cpu->C = cpu->H;
            break;
        }
        case 0x4D: {         //MOV C,L
            cpu->C = cpu->L;
            break;
        }
        case 0x4E: {         //MOV C,M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->C = read_memory(address);
            break;
        }
        case 0x4F: {         //MOV C,A
            cpu->C = cpu->A;
            break;
        }
        case 0x50: {         //MOV D,B
            cpu->D = cpu->B;
            break;
        }
        case 0x51: {         //MOV D,C
            cpu->D = cpu->C;
            break;
        }
        case 0x53: {         //MOV D,E
            cpu->D = cpu->E;
            break;
        }
        case 0x54: {         //MOV D,H
            cpu->D = cpu->H;
            break;
        }
        case 0x55: {         //MOV D,L
            cpu->D = cpu->L;
            break;
        }
        case 0x56: {         //MOV D,M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->D = read_memory(address);
            break;
        }
        case 0x57: {         //MOV D,A
            cpu->D = cpu->A;
            break;
        }
        case 0x58: {         //MOV E,B
            cpu->E = cpu->B;
            break;
        }
        case 0x59: {         //MOV E,C
            cpu->E = cpu->C;
            break;
        }
        case 0x5A: {         //MOV E,D
            cpu->E = cpu->D;
            break;
        }
        case 0x5C: {         //MOV E,H
            cpu->E = cpu->H;
            break;
        }
        case 0x5D: {         //MOV E,L
            cpu->E = cpu->L;
            break;
        }
        case 0x5E: {         //MOV E,M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->E = read_memory(address);
            break;
        }
        case 0x5F: {         //MOV E,A
            cpu->E = cpu->A;
            break;
        }
        case 0x60: {         //MOV H,B
            cpu->H = cpu->B;
            break;
        }
        case 0x61: {         //MOV H,C
            cpu->H = cpu->C;
            break;
        }
        case 0x62: {         //MOV H,D
            cpu->H = cpu->D;
            break;
        }
        case 0x63: {         //MOV H,E
            cpu->H = cpu->E;
            break;
        }
        case 0x65: {         //MOV H,L
            cpu->H = cpu->L;
            break;
        }
        case 0x66: {         //MOV H,M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->H = read_memory(address);
            break;
        }
        case 0x67: {         //MOV H,A
            cpu->H = cpu->A;
            break;
        }
        case 0x68: {         //MOV L,B
            cpu->L = cpu->B;
            break;
        }
        case 0x69: {         //MOV L,C
            cpu->L = cpu->C;
            break;
        }
        case 0x6A: {         //MOV L,D
            cpu->L = cpu->D;
            break;
        }
        case 0x6B: {         //MOV L,E
            cpu->L = cpu->E;
            break;
        }
        case 0x6C: {         //MOV L,H
            cpu->L = cpu->H;
            break;
        }
        case 0x6E: {         //MOV L,M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->L = read_memory(address);
            break;
        }
        case 0x6F: {         //MOV L,A
            cpu->L = cpu->A;
            break;
        }
        case 0x70: {         //MOV M,B
            uint16_t address = (cpu->H << 8) | cpu->L;
            write_memory(address, cpu->B);
            break;
        }
        case 0x71: {         //MOV M,C
            uint16_t address = (cpu->H << 8) | cpu->L;
            write_memory(address, cpu->C);
            break;
        }
        case 0x72: {         //MOV M,D
            uint16_t address = (cpu->H << 8) | cpu->L;
            write_memory(address, cpu->D);
            break;
        }
        case 0x73: {         //MOV M,E
            uint16_t address = (cpu->H << 8) | cpu->L;
            write_memory(address, cpu->E);
            break;
        }
        case 0x74: {         //MOV M,H
            uint16_t address = (cpu->H << 8) | cpu->L;
            write_memory(address, cpu->H);
            break;
        }
        case 0x75: {         //MOV M,L
            uint16_t address = (cpu->H << 8) | cpu->L;
            write_memory(address, cpu->L);
            break;
        }
        case 0x77: {         //MOV M,A
            uint16_t address = (cpu->H << 8) | cpu->L;
            write_memory(address, cpu->A);
            break;
        }
        case 0x78: {         //MOV A,B
            cpu->A = cpu->B;
            break;
        }
        case 0x79: {         //MOV A,C
            cpu->A = cpu->C;
            break;
        }
        case 0x7A: {         //MOV A,D
            cpu->A = cpu->D;
            break;
        }
        case 0x7B: {         //MOV A,E
            cpu->A = cpu->E;
            break;
        }
        case 0x7C: {         //MOV A,H
            cpu->A = cpu->H;
            break;
        }
        case 0x7D: {         //MOV A,L
            cpu->A = cpu->L;
            break;
        }
        case 0x7E: {         //MOV A,M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->A = read_memory(address);
            break;
        }
        case 0x80: {         //ADD B
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)cpu->B;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x81: {         //ADD C
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)cpu->C;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x82: {         //ADD D
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)cpu->D;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x83: {         //ADD E
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)cpu->E;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x84: {         //ADD H
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)cpu->H;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x85: {         //ADD L
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)cpu->L;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x86: {         //ADD M
            uint16_t address = (cpu->H << 8) | cpu->L;
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)read_memory(address);
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x87: {         //ADD A
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)cpu->A;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x88: {         //ADC B
            uint16_t value = cpu->A + (uint16_t)cpu->B + cpu->flags->CY;
            update_byte_CY(cpu, value);
            update_byte_SZAP(cpu, value);
            cpu->A = value & 0xff;
            break;
        }
        case 0x89: {         //ADC C
            uint16_t value = cpu->A + (uint16_t)cpu->C + cpu->flags->CY;
            update_byte_CY(cpu, value);
            update_byte_SZAP(cpu, value);
            cpu->A = value & 0xff;
            break;
        }
        case 0x8A: {         //ADC D
            uint16_t value = cpu->A + (uint16_t)cpu->D + cpu->flags->CY;
            update_byte_CY(cpu, value);
            update_byte_SZAP(cpu, value);
            cpu->A = value & 0xff;
            break;
        }
        case 0x8B: {         //ADC E
            uint16_t value = cpu->A + (uint16_t)cpu->E + cpu->flags->CY;
            update_byte_CY(cpu, value);
            update_byte_SZAP(cpu, value);
            cpu->A = value & 0xff;
            break;
        }
        case 0x8C: {         //ADC H
            uint16_t value = cpu->A + (uint16_t)cpu->H + cpu->flags->CY;
            update_byte_CY(cpu, value);
            update_byte_SZAP(cpu, value);
            cpu->A = value & 0xff;
            break;
        }
        case 0x8D: {         //ADC L
            uint16_t value = cpu->A + (uint16_t)cpu->L + cpu->flags->CY;
            update_byte_CY(cpu, value);
            update_byte_SZAP(cpu, value);
            cpu->A = value & 0xff;
            break;
        }
        case 0x8E: {         //ADC M
            uint16_t address = (cpu->H << 8) | cpu->L;
            uint16_t value = cpu->A + (uint16_t)read_memory(address) + cpu->flags->CY;
            update_byte_CY(cpu, value);
            update_byte_SZAP(cpu, value);
            cpu->A = value & 0xff;
            break;
        }
        case 0x8F: {         //ADC A
            uint16_t value = cpu->A + (uint16_t)cpu->A + cpu->flags->CY;
            update_byte_CY(cpu, value);
            update_byte_SZAP(cpu, value);
            cpu->A = value & 0xff;
            break;
        }
        case 0x90: {         //SUB B
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->B;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x91: {         //SUB C
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->C;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x92: {         //SUB D
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->D;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x93: {         //SUB E
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->E;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x94: {         //SUB H
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->H;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x95: {         //SUB L
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->L;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x96: {         //SUB M
            uint16_t address = (cpu->H << 8) | cpu->L;
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)read_memory(address);
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x97: {         //SUB A
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->A;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x98: {         //SBB B
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->B - cpu->flags->CY;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x99: {         //SBB C
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->C - cpu->flags->CY;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x9A: {         //SBB D
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->D - cpu->flags->CY;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x9B: {         //SBB E
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->E - cpu->flags->CY;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x9C: {         //SBB H
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->H - cpu->flags->CY;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x9D: {         //SBB L
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->L - cpu->flags->CY;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x9E: {         //SBB M
            uint16_t address = (cpu->H << 8) | cpu->L;
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)read_memory(address) - cpu->flags->CY;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0x9F: {         //SBB A
            uint16_t answer = (uint16_t)cpu->A - (uint16_t)cpu->A - cpu->flags->CY;
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            break;
        }
        case 0xA0: {         //ANA B
            cpu->A &= cpu->B;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA1: {         //ANA C
            cpu->A &= cpu->C;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA2: {         //ANA D
            cpu->A &= cpu->D;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA3: {         //ANA E
            cpu->A &= cpu->E;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA4: {         //ANA H
            cpu->A &= cpu->H;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA5: {         //ANA L
            cpu->A &= cpu->L;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA6: {         //ANA M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->A &= read_memory(address);
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA7: {         //ANA A
            cpu->A &= cpu->A;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA8: {         //XRA B
            cpu->A ^= cpu->B;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xA9: {         //XRA C
            cpu->A ^= cpu->C;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xAA: {         //XRA D
            cpu->A ^= cpu->D;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xAB: {         //XRA E
            cpu->A ^= cpu->E;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xAC: {         //XRA H
            cpu->A ^= cpu->H;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xAD: {         //XRA L
            cpu->A ^= cpu->L;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xAE: {         //XRA M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->A ^= read_memory(address);
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xAF: {         //XRA A
            cpu->A ^= cpu->A;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB0: {         //ORA B
            cpu->A |= cpu->B;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB1: {         //ORA C
            cpu->A |= cpu->C;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB2: {         //ORA D
            cpu->A |= cpu->D;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB3: {         //ORA E
            cpu->A |= cpu->E;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB4: {         //ORA H
            cpu->A |= cpu->H;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB5: {         //ORA L
            cpu->A |= cpu->L;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB6: {         //ORA M
            uint16_t address = (cpu->H << 8) | cpu->L;
            cpu->A |= read_memory(address);
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB7: {         //ORA A
            cpu->A |= cpu->A;
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            break;
        }
        case 0xB8: {         //CMP B
            uint16_t value = (uint16_t)cpu->A - (uint16_t)cpu->B;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            break;
        }
        case 0xB9: {         //CMP C
            uint16_t value = (uint16_t)cpu->A - (uint16_t)cpu->C;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            break;
        }
        case 0xBA: {         //CMP D
            uint16_t value = (uint16_t)cpu->A - (uint16_t)cpu->D;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            break;
        }
        case 0xBB: {         //CMP E
            uint16_t value = (uint16_t)cpu->A - (uint16_t)cpu->E;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            break;
        }
        case 0xBC: {         //CMP H
            uint16_t value = (uint16_t)cpu->A - (uint16_t)cpu->H;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            break;
        }
        case 0xBD: {         //CMP L
            uint16_t value = (uint16_t)cpu->A - (uint16_t)cpu->L;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            break;
        }
        case 0xBE: {         //CMP M
            uint16_t address = (cpu->H << 8) | cpu->L;
            uint16_t value = (uint16_t)cpu->A - (uint16_t)read_memory(address);
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            break;
        }
        case 0xBF: {         //CMP A
            uint16_t value = (uint16_t)cpu->A - (uint16_t)cpu->A;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            break;
        }
        case 0xC0: {         //RNZ (Return if not zero)
            if (cpu->flags->Z == 0) {
                ret(cpu);
                opcode_size = 0;
            }
            break;
        }
        case 0xC1: {         //POP B (Pop two bytes from stack into BC)
            cpu->C = read_memory(cpu->SP);
            cpu->B = read_memory(cpu->SP + 1);
            cpu->SP += 2;
            break;
        }
        case 0xC2: {         //JNZ adr (Jump if not zero)
            if (cpu->flags->Z == 0) {
                uint16_t address = read_memory(cpu->PC + 1);
                cpu->PC = address;
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xC3: {         //JMP adr (Jump unconditionally)
            uint16_t address = read_memory(cpu->PC + 1);
            cpu->PC = address;
            opcode_size = 0;
            break;
        }
        case 0xC4: {         //CNZ adr (Call if not zero)
            if (cpu->flags->Z == 0) {
                uint16_t address = read_memory(cpu->PC + 1);
                call(cpu, address);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xC5: {         //PUSH B (Push BC onto the stack)
            write_memory(cpu->SP - 2, cpu->C);
            write_memory(cpu->SP - 1, cpu->B);
            cpu->SP -= 2;
            break;
        }
        case 0xC6: {         //ADI byte (Add immediate to A)
            uint16_t answer = (uint16_t)cpu->A + (uint16_t)read_memory(cpu->PC + 1);
            update_byte_SZAP(cpu, answer);
            update_byte_CY(cpu, answer);
            cpu->A = answer & 0xff;
            opcode_size = 2;
            break;
        }
        case 0xC8: {         //RZ (Return if zero)
            if (cpu->flags->Z == 1) {
                ret(cpu);
                opcode_size = 0;
            }
            break;
        }
        case 0xC9: {         //RET (Return from subroutine)
            ret(cpu);
            opcode_size = 0;
            break;
        }
        case 0xCA: {         //JZ adr (Jump if zero)
            if (cpu->flags->Z == 1) {
                uint16_t address = read_memory(cpu->PC + 1);
                cpu->PC = address;
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xCC: {         //CZ adr (Call if zero)
            if (cpu->flags->Z == 1) {
                uint16_t address = read_memory(cpu->PC + 1);
                call(cpu, address);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xCD: {         //CALL adr (Call subroutine)
            uint16_t address = read_memory(cpu->PC + 1);
            call(cpu, address);
            opcode_size = 0;
            break;
        }
        case 0xCE: {         //ACI D8 (Add immediate to A with carry)
            uint16_t value = cpu->A + (uint16_t)read_memory(cpu->PC + 1) + cpu->flags->CY;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            cpu->A = value & 0xff;
            opcode_size = 2;
            break;
        }
        case 0xD0: {         //RNC (Return if no carry)
            if (cpu->flags->CY == 0) {
                ret(cpu);
                opcode_size = 0;
            }
            break;
        }
        case 0xD1: {         //POP D (Pop two bytes from stack into DE)
            cpu->E = read_memory(cpu->SP);
            cpu->D = read_memory(cpu->SP + 1);
            cpu->SP += 2;
            break;
        }
        case 0xD2: {         //JNC adr (Jump if no carry)
            if (cpu->flags->CY == 0) {
                uint16_t address = read_memory(cpu->PC + 1);
                cpu->PC = address;
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xD3: {         //OUT D8 (Output A to port)
            /*uint8_t port = read_memory(cpu->PC + 1);
            if (cpu->callback->out) {
                cpu->callback->out(port, cpu->A);
            }
            opcode_size = 2;
            break;*/
        }
        case 0xD4: {         //CNC adr (Call if no carry)
            if (cpu->flags->CY == 0) {
                uint16_t address = read_memory(cpu->PC + 1);
                call(cpu, address);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xD5: {         //PUSH D (Push DE onto the stack)
            write_memory(cpu->SP - 2, cpu->E);
            write_memory(cpu->SP - 1, cpu->D);
            cpu->SP -= 2;
            break;
        }
        case 0xD6: {         //SUI D8 (Subtract immediate from A)
            uint8_t data = read_memory(cpu->PC + 1);
            uint16_t value = (uint16_t)cpu->A - (uint16_t)data;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            cpu->A = value & 0xff;
            opcode_size = 2;
            break;
        }
        case 0xD8: {         //RC (Return if carry)
            if (cpu->flags->CY == 1) {
                ret(cpu);
                opcode_size = 0;
            }
            break;
        }
        case 0xDA: {         //JC adr (Jump if carry)
            if (cpu->flags->CY == 1) {
                uint16_t address = read_memory(cpu->PC + 1);
                cpu->PC = address;
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xDB: {         //IN D8 (Input to A from port)
            /*uint8_t port = read_memory(cpu->PC + 1);
            if (cpu->callback->in) {
                cpu->A = cpu->callback->in(port);
            }
            opcode_size = 2;
            break;*/
        }
        case 0xDC: {         //CC adr (Call if carry)
            if (cpu->flags->CY == 1) {
                uint16_t address = read_memory(cpu->PC + 1);
                call(cpu, address);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xDE: {         //SBI D8 (Subtract immediate from A with borrow)
            uint8_t data = read_memory(cpu->PC + 1);
            uint16_t value = (uint16_t)cpu->A - (uint16_t)data - cpu->flags->CY;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            cpu->A = value & 0xff;
            opcode_size = 2;
            break;
        }
        case 0xE0: {         //RPO (Return if parity odd)
            if (cpu->flags->P == 0) {
                ret(cpu);
                opcode_size = 0;
            }
            break;
        }
        case 0xE1: {         //POP H (Pop two bytes from stack into HL)
            cpu->L = read_memory(cpu->SP);
            cpu->H = read_memory(cpu->SP + 1);
            cpu->SP += 2;
            break;
        }
        case 0xE2: {         //JPO (Jump if parity odd)
            if (cpu->flags->P == 0) {
                uint16_t address = read_memory(cpu->PC + 1);
                cpu->PC = address;
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xE3: {         //XTHL (Exchange HL with top of stack)
            uint8_t l = cpu->L;
            uint8_t h = cpu->H;
            cpu->L = read_memory(cpu->SP);
            cpu->H = read_memory(cpu->SP + 1);
            write_memory(cpu->SP, l);
            write_memory(cpu->SP + 1, h);
            break;
        }
        case 0xE4: {         //CPO adr (Call if parity odd)
            if (cpu->flags->P == 0) {
                uint16_t address = read_memory(cpu->PC + 1);
                call(cpu, address);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xE5: {         //PUSH H (Push HL onto the stack)
            write_memory(cpu->SP - 2, cpu->L);
            write_memory(cpu->SP - 1, cpu->H);
            cpu->SP -= 2;
            break;
        }
        case 0xE6: {         //ANI D8 (AND immediate with A)
            uint16_t value = cpu->A & read_memory(cpu->PC + 1);
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            cpu->A = value & 0xff;
            opcode_size = 2;
            break;
        }
        case 0xE8: {         //RPE (Return if parity even)
            if (cpu->flags->P == 1) {
                ret(cpu);
                opcode_size = 0;
            }
            break;
        }
        case 0xE9: {         //PCHL (Load PC with HL)
            cpu->PC = (cpu->H << 8) | cpu->L;
            opcode_size = 0;
            break;
        }
        case 0xEA: {         //JPE adr (Jump if parity even)
            if (cpu->flags->P == 1) {
                uint16_t address = read_memory(cpu->PC + 1);
                cpu->PC = address;
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xEB: {         //XCHG (Exchange DE with HL)
            util_swap(cpu->H, cpu->D);
            util_swap(cpu->L, cpu->E);
            break;
        }
        case 0xEC: {         //CPE adr (Call if parity even)
            if (cpu->flags->P == 1) {
                uint16_t address = read_memory(cpu->PC + 1);
                call(cpu, address);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xEE: {         //XRI (XOR immediate with A)
            cpu->A ^= read_memory(cpu->PC + 1);
            update_byte_SZAP(cpu, cpu->A);
            update_byte_CY(cpu, cpu->A);
            opcode_size = 2;
            break;
        }
        case 0xF0: {         //RP (Return if positive)
            if (cpu->flags->S == 0) {
                ret(cpu);
                opcode_size = 0;
            }
            break;
        }
        case 0xF1: {         //POP PSW (Pop two bytes from stack into flags and A)
            uint8_t flags = read_memory(cpu->SP);       // Read the flags from the stack
            *(uint8_t*)(cpu->flags) = flags;            // Store the flags into the CPU flags structure
            cpu->A = read_memory(cpu->SP + 1);          // Read the accumulator (A) from the stack
            cpu->SP += 2;                               // Adjust the stack pointer
            break;
        }
        case 0xF2: {         //JP (Jump if positive)
            if (cpu->flags->S == 0) {
                uint16_t address = read_memory(cpu->PC + 1);
                cpu->PC = address;
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xF4: {         //CP (Call if positive)
            if (cpu->flags->S == 0) {
                uint16_t address = read_memory(cpu->PC + 1);
                call(cpu, address);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xF5: {         //PUSH PSW (Push flags and A onto stack)
            uint8_t psw = *((uint8_t*)(cpu->flags));  // Cast the flags structure to a single byte
            write_memory(cpu->SP - 2, psw);           // Push the flags onto the stack
            write_memory(cpu->SP - 1, cpu->A);        // Push the accumulator (A) onto the stack
            cpu->SP -= 2;                             // Adjust the stack pointer
            break;
        }
        case 0xF6: {         //ORI D8 (OR immediate with A)
            uint8_t data = read_memory(cpu->PC + 1);
            uint8_t value = cpu->A | data;
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            cpu->A = value;
            opcode_size = 2;
            break;
        }
        case 0xF8: {         //RM (Return if minus)
            if (cpu->flags->S == 1) {
                ret(cpu);
                opcode_size = 0;
            }
            break;
        }
        case 0xF9: {         //SPHL (Load SP with HL)
            cpu->SP = (cpu->H << 8) | cpu->L;
            break;
        }
        case 0xFA: {         //JM (Jump if minus)
            if (cpu->flags->S == 1) {
                uint16_t address = read_memory(cpu->PC + 1);
                cpu->PC = address;
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xFB: {         //EI (Enable interrupts)
            cpu->interrupts_enabled = 1;
            break;
        }
        case 0xFC: {         //CM (Call if minus)
            if (cpu->flags->S == 1) {
                uint16_t address = read_memory(cpu->PC + 1);
                call(cpu, address);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            break;
        }
        case 0xFE: {         //CPI D8 (Compare A with immediate)
            uint16_t value = (uint16_t)cpu->A - (uint16_t)read_memory(cpu->PC + 1);
            update_byte_SZAP(cpu, value);
            update_byte_CY(cpu, value);
            opcode_size = 2;
            break;
        }
        default: {
            printf("Unknown opcode: 0x%02X at address 0x%04X\n", opcode, cpu->PC);
            error("");
            break;
        }
    }
    cpu->PC += opcode_size;

}

void ret(CPU *cpu) {
	uint8_t pclo = read_memory(cpu->SP);
	uint8_t pchi = read_memory(cpu->SP + 1);
	cpu->PC = ((uint16_t)pchi << 8) | (uint16_t)pclo;
    cpu->SP += 2;
}