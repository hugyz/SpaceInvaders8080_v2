#include "cpu.h"

#include "memory.h"
#include "update_flags.h"

#include "utils.h"
#include "input.h"
#include "output.h"

#include <stdlib.h>
#include <stdio.h>

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

    cpu->A = 0;
    cpu->B = cpu->C = 0;
    cpu->D = cpu->E = 0;
    cpu->H = cpu->L = 0;
    cpu->SP = 0;
    cpu->PC = 0;

    cpu->num_steps = 0;
    cpu->interrupts_enabled = 0;
    cpu->cycles = 0;

    return cpu;
}

void cpu_free(CPU* cpu) {
    if (cpu && cpu->flags) {
        free(cpu->flags);
        cpu->flags = NULL;
        free(cpu);
        cpu = NULL;

        audio_free();
    }
    else error("no instance of cpu/flags when freeing");
}

void cpu_reset(CPU* cpu) {
    if (cpu && cpu->flags) {
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

        cpu->num_steps = 0;
        cpu->interrupts_enabled = 0;
        cpu->cycles = 0;
    }
    else error("no instance of cpu/flags when resetting");
}

void interrupt(CPU *cpu, int interrupt_num) {
    if (!cpu->interrupts_enabled) {
        error("Interrupts have been disabled");
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

void ret(CPU *cpu) {
	uint8_t pclo = read_memory(cpu->SP);
	uint8_t pchi = read_memory(cpu->SP + 1);
	cpu->PC = ((uint16_t)pchi << 8) | (uint16_t)pclo;
    cpu->SP += 2;
}

uint16_t read_opcode_data_word(CPU *cpu) {
		uint16_t value = ((uint16_t)read_memory(cpu->PC + 2) << 8) | ((uint16_t)read_memory(cpu->PC + 1));
		return value;
}

uint16_t make_word(uint8_t hi, uint8_t lo) {
		return (((uint16_t)hi) << 8) | ((uint16_t)lo);
}

void call(CPU *cpu, uint16_t address, uint16_t return_address) {
	uint8_t rethi = (uint8_t)((return_address >> 8) & 0xff);
	uint8_t retlo = (uint8_t)(return_address & 0xff);
	write_memory(cpu->SP - 1, rethi);
	write_memory(cpu->SP - 2, retlo);
	cpu->SP = cpu->SP - 2;
	cpu->PC = address;
}

void print_status(CPU *cpu) {
    printf("Regs: A:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X\n",
           cpu->A, cpu->B, cpu->C, cpu->D, cpu->E, cpu->H, cpu->L, cpu->SP, cpu->PC);
    printf("Flags: Z:%d S:%d P:%d CY:%d AC:%d PAD:%d\n",
           cpu->flags->Z, cpu->flags->S, cpu->flags->P, cpu->flags->CY, cpu->flags->AC, cpu->flags->PAD);
    printf("Steps:%lu Interrupts:%d Cycles:%lu\n",
           cpu->num_steps, cpu->interrupts_enabled, cpu->cycles);
    
    // Print first 15 bytes of VRAM
    printf("VRAM: ");
    for (int i = 0; i < 15; i++) {
        printf("%02X ", read_memory(0x2400 + i));
    }
    printf("\n");
}


uint16_t cpu_execute_instruction(CPU* cpu) {
    
    uint8_t opcode = read_memory(cpu->PC);  // Fixing the data type here
    uint16_t opcode_size = 1;  // Default bytes taken by instruction
    uint16_t cycle = 0;

    print_status(cpu);

    switch (opcode) {
        case 0x00: {  // NOP
            cycle += 4;
            break;
        }
        case 0x01: {  // LXI B, D16
            cpu->C = read_memory(cpu->PC + 1);
            cpu->B = read_memory(cpu->PC + 2);
            opcode_size = 3;
            cycle += 10;
            break;
        }
        case 0x02: {  // STAX B
            uint16_t address = make_word(cpu->B, cpu->C);
            write_memory(address, cpu->A);
            cycle += 7;
            break;
        }
        case 0x03: {  // INX B
            uint16_t value = make_word(cpu->B, cpu->C) + 1;
            cpu->B = value >> 8;
            cpu->C = value & 0xFF;
            cycle += 5;
            break;
        }
        case 0x04: {  // INR B
            cpu->B++;
            update_byte_SZAP(cpu, cpu->B);
            cycle += 5;
            break;
        }
        case 0x05: {  // DCR B
            cpu->B--;
            update_byte_SZAP(cpu, cpu->B);
            cycle += 5;
            break;
        }
        case 0x06: {  // MVI B, D8
            cpu->B = read_memory(cpu->PC + 1);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0x07: {  // RLC
            cpu->flags->CY = (cpu->A >> 7) & 1;
            cpu->A = (cpu->A << 1) | cpu->flags->CY;
            cycle += 4;
            break;
        }
        case 0x08: {  // *NOP
            cycle += 4;
            break;
        }
        case 0x09: {  // DAD B
            uint32_t result = make_word(cpu->H, cpu->L) + make_word(cpu->B, cpu->C);
            update_word_CY(cpu, result);
            cpu->H = result >> 8;
            cpu->L = result & 0xFF;
            cycle += 10;
            break;
        }
        case 0x0A: {  // LDAX B
            cpu->A = read_memory(make_word(cpu->B, cpu->C));
            cycle += 7;
            break;
        }
        case 0x0B: {  // DCX B
            uint16_t value = make_word(cpu->B, cpu->C) - 1;
            cpu->B = value >> 8;
            cpu->C = value & 0xFF;
            cycle += 5;
            break;
        }
        case 0x0C: {  // INR C
            cpu->C++;
            update_byte_SZAP(cpu, cpu->C);
            cycle += 5;
            break;
        }
        case 0x0D: {  // DCR C
            cpu->C--;
            update_byte_SZAP(cpu, cpu->C);
            cycle += 5;
            break;
        }
        case 0x0E: {  // MVI C, D8
            cpu->C = read_memory(cpu->PC + 1);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0x0F: {  // RRC
            cpu->flags->CY = cpu->A & 1;
            cpu->A = (cpu->A >> 1) | (cpu->flags->CY << 7);
            cycle += 4;
            break;
        }
        case 0x10: {  // *NOP
            cycle += 4;
            break;
        }
        case 0x11: {  // LXI D, D16
            cpu->E = read_memory(cpu->PC + 1);
            cpu->D = read_memory(cpu->PC + 2);
            opcode_size = 3;
            cycle += 10;
            break;
        }
        case 0x12: {  // STAX D
            write_memory(make_word(cpu->D, cpu->E), cpu->A);
            cycle += 7;
            break;
        }
        case 0x13: {  // INX D
            uint16_t value = make_word(cpu->D, cpu->E) + 1;
            cpu->D = value >> 8;
            cpu->E = value & 0xFF;
            cycle += 5;
            break;
        }
        case 0x14: {  // INR D
            cpu->D++;
            update_byte_SZAP(cpu, cpu->D);
            cycle += 5;
            break;
        }
        case 0x15: {  // DCR D
            cpu->D--;
            update_byte_SZAP(cpu, cpu->D);
            cycle += 5;
            break;
        }
        case 0x16: {  // MVI D, D8
            cpu->D = read_memory(cpu->PC + 1);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0x17: {  // RAL
            uint8_t bit7 = (cpu->A >> 7) & 1;
            cpu->A = (cpu->A << 1) | cpu->flags->CY;
            cpu->flags->CY = bit7;
            cycle += 4;
            break;
        }
        case 0x18: {  // *NOP
            cycle += 4;
            break;
        }
        case 0x19: {  // DAD D
            uint32_t result = make_word(cpu->H, cpu->L) + make_word(cpu->D, cpu->E);
            update_word_CY(cpu, result);
            cpu->H = result >> 8;
            cpu->L = result & 0xFF;
            cycle += 10;
            break;
        }
        case 0x1A: {  // LDAX D
            cpu->A = read_memory(make_word(cpu->D, cpu->E));
            cycle += 7;
            break;
        }
        case 0x1B: {  // DCX D
            uint16_t value = make_word(cpu->D, cpu->E) - 1;
            cpu->D = value >> 8;
            cpu->E = value & 0xFF;
            cycle += 5;
            break;
        }
        case 0x1C: {  // INR E
            cpu->E++;
            update_byte_SZAP(cpu, cpu->E);
            cycle += 5;
            break;
        }
        case 0x1D: {  // DCR E
            cpu->E--;
            update_byte_SZAP(cpu, cpu->E);
            cycle += 5;
            break;
        }
        case 0x1E: {  // MVI E, D8
            cpu->E = read_memory(cpu->PC + 1);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0x1F: {  // RAR
            uint8_t bit0 = cpu->A & 1;
            cpu->A = (cpu->A >> 1) | (cpu->flags->CY << 7);
            cpu->flags->CY = bit0;
            cycle += 4;
            break;
        }
        case 0x20: {  // *NOP
            cycle += 4;
            break;
        }
        case 0x21: {  // LXI H, D16
            cpu->L = read_memory(cpu->PC + 1);
            cpu->H = read_memory(cpu->PC + 2);
            opcode_size = 3;
            cycle += 10;
            break;
        }
        case 0x22: {  // SHLD
            uint16_t address = read_opcode_data_word(cpu);
            write_memory(address, cpu->L);
            write_memory(address + 1, cpu->H);
            opcode_size = 3;
            cycle += 16;
            break;
        }
        case 0x23: {  // INX H
            uint16_t value = make_word(cpu->H, cpu->L) + 1;
            cpu->H = value >> 8;
            cpu->L = value & 0xFF;
            cycle += 5;
            break;
        }
        case 0x24: {  // INR H
            cpu->H++;
            update_byte_SZAP(cpu, cpu->H);
            cycle += 5;
            break;
        }
        case 0x25: {  // DCR H
            cpu->H--;
            update_byte_SZAP(cpu, cpu->H);
            cycle += 5;
            break;
        }
        case 0x26: {  // MVI H, D8
            cpu->H = read_memory(cpu->PC + 1);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0x27: {  // DAA
            if ((cpu->A & 0x0F) > 9) cpu->A += 6;
            if ((cpu->A & 0xF0) > 0x90 || cpu->flags->CY) cpu->A += 0x60;
            update_byte_SZAP(cpu, cpu->A);
            cycle += 4;
            break;
        }
        case 0x28: {  // *NOP
            cycle += 4;
            break;
        }
        case 0x29: {  // DAD H
            uint32_t result = make_word(cpu->H, cpu->L) + make_word(cpu->H, cpu->L);
            update_word_CY(cpu, result);
            cpu->H = result >> 8;
            cpu->L = result & 0xFF;
            cycle += 10;
            break;
        }
        case 0x2A: {  // LHLD
            uint16_t address = read_opcode_data_word(cpu);
            cpu->L = read_memory(address);
            cpu->H = read_memory(address + 1);
            opcode_size = 3;
            cycle += 16;
            break;
        }
        case 0x2B: {  // DCX H
            uint16_t value = make_word(cpu->H, cpu->L) - 1;
            cpu->H = value >> 8;
            cpu->L = value & 0xFF;
            cycle += 5;
            break;
        }
        case 0x2C: {  // INR L
            cpu->L++;
            update_byte_SZAP(cpu, cpu->L);
            cycle += 5;
            break;
        }
        case 0x2D: {  // DCR L
            cpu->L--;
            update_byte_SZAP(cpu, cpu->L);
            cycle += 5;
            break;
        }
        case 0x2E: {  // MVI L, D8
            cpu->L = read_memory(cpu->PC + 1);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0x2F: {  // CMA
            cpu->A = ~cpu->A;
            cycle += 4;
            break;
        }
        case 0x30: {  // *NOP
            cycle += 4;
            break;
        }
        case 0x31: {  // LXI SP, D16
            cpu->SP = read_opcode_data_word(cpu);
            opcode_size = 3;
            cycle += 10;
            break;
        }
        case 0x32: {  // STA adr
            write_memory(read_opcode_data_word(cpu), cpu->A);
            opcode_size = 3;
            cycle += 13;
            break;
        }
        case 0x33: {  // INX SP
            cpu->SP++;
            cycle += 5;
            break;
        }
        case 0x34: {  // INR M
            uint16_t address = make_word(cpu->H, cpu->L);
            uint8_t value = read_memory(address) + 1;
            update_byte_SZAP(cpu, value);
            write_memory(address, value);
            cycle += 10;
            break;
        }
        case 0x35: {  // DCR M
            uint16_t address = make_word(cpu->H, cpu->L);
            uint8_t value = read_memory(address) - 1;
            update_byte_SZAP(cpu, value);
            write_memory(address, value);
            cycle += 10;
            break;
        }
        case 0x36: {  // MVI M, D8
            write_memory(make_word(cpu->H, cpu->L), read_memory(cpu->PC + 1));
            opcode_size = 2;
            cycle += 10;
            break;
        }
        case 0x37: {  // STC
            cpu->flags->CY = 1;
            cycle += 4;
            break;
        }
        case 0x38: {  // *NOP
            cycle += 4;
            break;
        }
        case 0x39: {  // DAD SP
            uint32_t result = make_word(cpu->H, cpu->L) + cpu->SP;
            update_word_CY(cpu, result);
            cpu->H = result >> 8;
            cpu->L = result & 0xFF;
            cycle += 10;
            break;
        }
        case 0x3A: {  // LDA adr
            cpu->A = read_memory(read_opcode_data_word(cpu));
            opcode_size = 3;
            cycle += 13;
            break;
        }
        case 0x3B: {  // DCX SP
            cpu->SP--;
            cycle += 5;
            break;
        }
        case 0x3C: {  // INR A
            cpu->A++;
            update_byte_SZAP(cpu, cpu->A);
            cycle += 5;
            break;
        }
        case 0x3D: {  // DCR A
            cpu->A--;
            update_byte_SZAP(cpu, cpu->A);
            cycle += 5;
            break;
        }
        case 0x3E: {  // MVI A, D8
            cpu->A = read_memory(cpu->PC + 1);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0x3F: {  // CMC
            cpu->flags->CY = !cpu->flags->CY;
            cycle += 4;
            break;
        }
        case 0x40: {  // MOV B, B
            cycle += 5;
            break;
        }
        case 0x41: {  // MOV B, C
            cpu->B = cpu->C;
            cycle += 5;
            break;
        }
        case 0x42: {  // MOV B, D
            cpu->B = cpu->D;
            cycle += 5;
            break;
        }
        case 0x43: {  // MOV B, E
            cpu->B = cpu->E;
            cycle += 5;
            break;
        }
        case 0x44: {  // MOV B, H
            cpu->B = cpu->H;
            cycle += 5;
            break;
        }
        case 0x45: {  // MOV B, L
            cpu->B = cpu->L;
            cycle += 5;
            break;
        }
        case 0x46: {  // MOV B, M
            cpu->B = read_memory(make_word(cpu->H, cpu->L));
            cycle += 7;
            break;
        }
        case 0x47: {  // MOV B, A
            cpu->B = cpu->A;
            cycle += 5;
            break;
        }
        case 0x48: {  // MOV C, B
            cpu->C = cpu->B;
            cycle += 5;
            break;
        }
        case 0x49: {  // MOV C, C
            cycle += 5;
            break;
        }
        case 0x4A: {  // MOV C, D
            cpu->C = cpu->D;
            cycle += 5;
            break;
        }
        case 0x4B: {  // MOV C, E
            cpu->C = cpu->E;
            cycle += 5;
            break;
        }
        case 0x4C: {  // MOV C, H
            cpu->C = cpu->H;
            cycle += 5;
            break;
        }
        case 0x4D: {  // MOV C, L
            cpu->C = cpu->L;
            cycle += 5;
            break;
        }
        case 0x4E: {  // MOV C, M
            cpu->C = read_memory(make_word(cpu->H, cpu->L));
            cycle += 7;
            break;
        }
        case 0x4F: {  // MOV C, A
            cpu->C = cpu->A;
            cycle += 5;
            break;
        }
        case 0x50: {  // MOV D, B
            cpu->D = cpu->B;
            cycle += 5;
            break;
        }
        case 0x51: {  // MOV D, C
            cpu->D = cpu->C;
            cycle += 5;
            break;
        }
        case 0x52: {  // MOV D, D
            cycle += 5;
            break;
        }
        case 0x53: {  // MOV D, E
            cpu->D = cpu->E;
            cycle += 5;
            break;
        }
        case 0x54: {  // MOV D, H
            cpu->D = cpu->H;
            cycle += 5;
            break;
        }
        case 0x55: {  // MOV D, L
            cpu->D = cpu->L;
            cycle += 5;
            break;
        }
        case 0x56: {  // MOV D, M
            cpu->D = read_memory(make_word(cpu->H, cpu->L));
            cycle += 7;
            break;
        }
        case 0x57: {  // MOV D, A
            cpu->D = cpu->A;
            cycle += 5;
            break;
        }
        case 0x58: {  // MOV E, B
            cpu->E = cpu->B;
            cycle += 5;
            break;
        }
        case 0x59: {  // MOV E, C
            cpu->E = cpu->C;
            cycle += 5;
            break;
        }
        case 0x5A: {  // MOV E, D
            cpu->E = cpu->D;
            cycle += 5;
            break;
        }
        case 0x5B: {  // MOV E, E
            cycle += 5;
            break;
        }
        case 0x5C: {  // MOV E, H
            cpu->E = cpu->H;
            cycle += 5;
            break;
        }
        case 0x5D: {  // MOV E, L
            cpu->E = cpu->L;
            cycle += 5;
            break;
        }
        case 0x5E: {  // MOV E, M
            cpu->E = read_memory(make_word(cpu->H, cpu->L));
            cycle += 7;
            break;
        }
        case 0x5F: {  // MOV E, A
            cpu->E = cpu->A;
            cycle += 5;
            break;
        }
        case 0x60: {  // MOV H, B
            cpu->H = cpu->B;
            cycle += 5;
            break;
        }
        case 0x61: {  // MOV H, C
            cpu->H = cpu->C;
            cycle += 5;
            break;
        }
        case 0x62: {  // MOV H, D
            cpu->H = cpu->D;
            cycle += 5;
            break;
        }
        case 0x63: {  // MOV H, E
            cpu->H = cpu->E;
            cycle += 5;
            break;
        }
        case 0x64: {  // MOV H, H
            cycle += 5;
            break;
        }
        case 0x65: {  // MOV H, L
            cpu->H = cpu->L;
            cycle += 5;
            break;
        }
        case 0x66: {  // MOV H, M
            cpu->H = read_memory(make_word(cpu->H, cpu->L));
            cycle += 7;
            break;
        }
        case 0x67: {  // MOV H, A
            cpu->H = cpu->A;
            cycle += 5;
            break;
        }
        case 0x68: {  // MOV L, B
            cpu->L = cpu->B;
            cycle += 5;
            break;
        }
        case 0x69: {  // MOV L, C
            cpu->L = cpu->C;
            cycle += 5;
            break;
        }
        case 0x6A: {  // MOV L, D
            cpu->L = cpu->D;
            cycle += 5;
            break;
        }
        case 0x6B: {  // MOV L, E
            cpu->L = cpu->E;
            cycle += 5;
            break;
        }
        case 0x6C: {  // MOV L, H
            cpu->L = cpu->H;
            cycle += 5;
            break;
        }
        case 0x6D: {  // MOV L, L
            cycle += 5;
            break;
        }
        case 0x6E: {  // MOV L, M
            cpu->L = read_memory(make_word(cpu->H, cpu->L));
            cycle += 7;
            break;
        }
        case 0x6F: {  // MOV L, A
            cpu->L = cpu->A;
            cycle += 5;
            break;
        }
        case 0x70: {  // MOV M, B
            write_memory(make_word(cpu->H, cpu->L), cpu->B);
            cycle += 7;
            break;
        }
        case 0x71: {  // MOV M, C
            write_memory(make_word(cpu->H, cpu->L), cpu->C);
            cycle += 7;
            break;
        }
        case 0x72: {  // MOV M, D
            write_memory(make_word(cpu->H, cpu->L), cpu->D);
            cycle += 7;
            break;
        }
        case 0x73: {  // MOV M, E
            write_memory(make_word(cpu->H, cpu->L), cpu->E);
            cycle += 7;
            break;
        }
        case 0x74: {  // MOV M, H
            write_memory(make_word(cpu->H, cpu->L), cpu->H);
            cycle += 7;
            break;
        }
        case 0x75: {  // MOV M, L
            write_memory(make_word(cpu->H, cpu->L), cpu->L);
            cycle += 7;
            break;
        }
        case 0x76: {  // HLT (Halt)
            cycle += 7;
            break;
        }
        case 0x77: {  // MOV M, A
            write_memory(make_word(cpu->H, cpu->L), cpu->A);
            cycle += 7;
            break;
        }
        case 0x78: {  // MOV A, B
            cpu->A = cpu->B;
            cycle += 5;
            break;
        }
        case 0x79: {  // MOV A, C
            cpu->A = cpu->C;
            cycle += 5;
            break;
        }
        case 0x7A: {  // MOV A, D
            cpu->A = cpu->D;
            cycle += 5;
            break;
        }
        case 0x7B: {  // MOV A, E
            cpu->A = cpu->E;
            cycle += 5;
            break;
        }
        case 0x7C: {  // MOV A, H
            cpu->A = cpu->H;
            cycle += 5;
            break;
        }
        case 0x7D: {  // MOV A, L
            cpu->A = cpu->L;
            cycle += 5;
            break;
        }
        case 0x7E: {  // MOV A, M
            cpu->A = read_memory(make_word(cpu->H, cpu->L));
            cycle += 7;
            break;
        }
        case 0x7F: {  // MOV A, A
            cycle += 5;
            break;
        }
        case 0x80: {  // ADD B
            uint16_t result = cpu->A + cpu->B;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x81: {  // ADD C
            uint16_t result = cpu->A + cpu->C;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x82: {  // ADD D
            uint16_t result = cpu->A + cpu->D;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x83: {  // ADD E
            uint16_t result = cpu->A + cpu->E;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x84: {  // ADD H
            uint16_t result = cpu->A + cpu->H;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x85: {  // ADD L
            uint16_t result = cpu->A + cpu->L;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x86: {  // ADD M
            uint8_t value = read_memory(make_word(cpu->H, cpu->L));
            uint16_t result = cpu->A + value;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 7;
            break;
        }
        case 0x87: {  // ADD A
            uint16_t result = cpu->A + cpu->A;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x88: {  // ADC B
            uint16_t result = cpu->A + cpu->B + cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x89: {  // ADC C
            uint16_t result = cpu->A + cpu->C + cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x8A: {  // ADC D
            uint16_t result = cpu->A + cpu->D + cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x8B: {  // ADC E
            uint16_t result = cpu->A + cpu->E + cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x8C: {  // ADC H
            uint16_t result = cpu->A + cpu->H + cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x8D: {  // ADC L
            uint16_t result = cpu->A + cpu->L + cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x8E: {  // ADC M
            uint8_t value = read_memory(make_word(cpu->H, cpu->L));
            uint16_t result = cpu->A + value + cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 7;
            break;
        }
        case 0x8F: {  // ADC A
            uint16_t result = cpu->A + cpu->A + cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x90: {  // SUB B
            uint16_t result = cpu->A - cpu->B;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x91: {  // SUB C
            uint16_t result = cpu->A - cpu->C;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x92: {  // SUB D
            uint16_t result = cpu->A - cpu->D;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x93: {  // SUB E
            uint16_t result = cpu->A - cpu->E;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x94: {  // SUB H
            uint16_t result = cpu->A - cpu->H;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x95: {  // SUB L
            uint16_t result = cpu->A - cpu->L;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x96: {  // SUB M
            uint8_t value = read_memory(make_word(cpu->H, cpu->L));
            uint16_t result = cpu->A - value;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 7;
            break;
        }
        case 0x97: {  // SUB A
            uint16_t result = cpu->A - cpu->A;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x98: {  // SBB B
            uint16_t result = cpu->A - cpu->B - cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x99: {  // SBB C
            uint16_t result = cpu->A - cpu->C - cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x9A: {  // SBB D
            uint16_t result = cpu->A - cpu->D - cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x9B: {  // SBB E
            uint16_t result = cpu->A - cpu->E - cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x9C: {  // SBB H
            uint16_t result = cpu->A - cpu->H - cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x9D: {  // SBB L
            uint16_t result = cpu->A - cpu->L - cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0x9E: {  // SBB M
            uint8_t value = read_memory(make_word(cpu->H, cpu->L));
            uint16_t result = cpu->A - value - cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 7;
            break;
        }
        case 0x9F: {  // SBB A
            uint16_t result = cpu->A - cpu->A - cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            cycle += 4;
            break;
        }
        case 0xA0: {  // ANA B
            cpu->A &= cpu->B;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xA1: {  // ANA C
            cpu->A &= cpu->C;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xA2: {  // ANA D
            cpu->A &= cpu->D;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xA3: {  // ANA E
            cpu->A &= cpu->E;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xA4: {  // ANA H
            cpu->A &= cpu->H;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xA5: {  // ANA L
            cpu->A &= cpu->L;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xA6: {  // ANA M
            uint8_t value = read_memory(make_word(cpu->H, cpu->L));
            cpu->A &= value;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 7;
            break;
        }
        case 0xA7: {  // ANA A
            cpu->A &= cpu->A;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xA8: {  // XRA B
            cpu->A ^= cpu->B;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xA9: {  // XRA C
            cpu->A ^= cpu->C;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xAA: {  // XRA D
            cpu->A ^= cpu->D;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xAB: {  // XRA E
            cpu->A ^= cpu->E;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xAC: {  // XRA H
            cpu->A ^= cpu->H;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xAD: {  // XRA L
            cpu->A ^= cpu->L;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xAE: {  // XRA M
            uint8_t value = read_memory(make_word(cpu->H, cpu->L));
            cpu->A ^= value;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 7;
            break;
        }
        case 0xAF: {  // XRA A
            cpu->A ^= cpu->A;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xB0: {  // ORA B
            cpu->A |= cpu->B;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xB1: {  // ORA C
            cpu->A |= cpu->C;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xB2: {  // ORA D
            cpu->A |= cpu->D;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xB3: {  // ORA E
            cpu->A |= cpu->E;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xB4: {  // ORA H
            cpu->A |= cpu->H;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xB5: {  // ORA L
            cpu->A |= cpu->L;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xB6: {  // ORA M
            uint8_t value = read_memory(make_word(cpu->H, cpu->L));
            cpu->A |= value;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 7;
            break;
        }
        case 0xB7: {  // ORA A
            cpu->A |= cpu->A;
            update_byte_SZAP(cpu, cpu->A);
            cpu->flags->CY = 0;
            cycle += 4;
            break;
        }
        case 0xB8: {  // CMP B
            uint16_t result = cpu->A - cpu->B;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cycle += 4;
            break;
        }
        case 0xB9: {  // CMP C
            uint16_t result = cpu->A - cpu->C;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cycle += 4;
            break;
        }
        case 0xBA: {  // CMP D
            uint16_t result = cpu->A - cpu->D;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cycle += 4;
            break;
        }
        case 0xBB: {  // CMP E
            uint16_t result = cpu->A - cpu->E;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cycle += 4;
            break;
        }
        case 0xBC: {  // CMP H
            uint16_t result = cpu->A - cpu->H;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cycle += 4;
            break;
        }
        case 0xBD: {  // CMP L
            uint16_t result = cpu->A - cpu->L;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cycle += 4;
            break;
        }
        case 0xBE: {  // CMP M
            uint8_t value = read_memory(make_word(cpu->H, cpu->L));
            uint16_t result = cpu->A - value;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cycle += 7;
            break;
        }
        case 0xBF: {  // CMP A
            uint16_t result = cpu->A - cpu->A;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cycle += 4;
            break;
        }
        case 0xC0: {  // RNZ
            if (cpu->flags->Z == 0) {
                ret(cpu);
                opcode_size = 0;
            }
            cycle += 5;
            break;
        }
        case 0xC1: {  // POP B
            cpu->C = read_memory(cpu->SP);
            cpu->B = read_memory(cpu->SP + 1);
            cpu->SP += 2;
            cycle += 10;
            break;
        }
        case 0xC2: {  // JNZ addr
            if (cpu->flags->Z == 0) {
                cpu->PC = read_opcode_data_word(cpu);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 10;
            break;
        }
        case 0xC3: {  // JMP addr
            cpu->PC = read_opcode_data_word(cpu);
            opcode_size = 0;
            cycle += 10;
            break;
        }
        case 0xC4: {  // CNZ addr
            if (cpu->flags->Z == 0) {
                call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 11;
            break;
        }
        case 0xC5: {  // PUSH B
            write_memory(cpu->SP - 1, cpu->B);
            write_memory(cpu->SP - 2, cpu->C);
            cpu->SP -= 2;
            cycle += 11;
            break;
        }
        case 0xC6: {  // ADI D8
            uint16_t result = cpu->A + read_memory(cpu->PC + 1);
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = (uint8_t)(result & 0xFF);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0xC7: {  // RST 0
            cycle += 11;
            break;
        }
        case 0xC8: {  // RZ
            if (cpu->flags->Z == 1) {
                ret(cpu);
                opcode_size = 0;
            }
            cycle += 5;
            break;
        }
        case 0xC9: {  // RET
            ret(cpu);
            opcode_size = 0;
            cycle += 10;
            break;
        }
        case 0xCA: {  // JZ addr
            if (cpu->flags->Z == 1) {
                cpu->PC = read_opcode_data_word(cpu);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 10;
            break;
        }
        case 0xCB: {  // *JMP addr (duplicate)
            cycle += 10;
            break;
        }
        case 0xCC: {  // CZ addr
            if (cpu->flags->Z == 1) {
                call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 11;
            break;
        }
        case 0xCD: {  // CALL addr
            call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
            opcode_size = 0;
            cycle += 17;
            break;
        }
        case 0xCE: {  // ACI D8
            uint16_t result = cpu->A + (uint16_t)read_memory(cpu->PC + 1) + (uint16_t)cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0xCF: {  // RST 1
            cycle += 11;
            break;
        }
        case 0xD0: {  // RNC
            if (cpu->flags->CY == 0) {
                ret(cpu);
                opcode_size = 0;
            }
            cycle += 5;
            break;
        }
        case 0xD1: {  // POP D
            cpu->E = read_memory(cpu->SP);
            cpu->D = read_memory(cpu->SP + 1);
            cpu->SP += 2;
            cycle += 10;
            break;
        }
        case 0xD2: {  // JNC addr
            if (cpu->flags->CY == 0) {
                cpu->PC = read_opcode_data_word(cpu);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 10;
            break;
        }
        case 0xD3: {  // OUT D8
            uint8_t port = read_memory(cpu->PC + 1);
            machine_out(cpu, port, cpu->A);
            opcode_size = 2;
            cycle += 10;
            break;
        }
        case 0xD4: {  // CNC addr
            if (cpu->flags->CY == 0) {
                call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 11;
            break;
        }
        case 0xD5: {  // PUSH D
            write_memory(cpu->SP - 1, cpu->D);
            write_memory(cpu->SP - 2, cpu->E);
            cpu->SP -= 2;
            cycle += 11;
            break;
        }
        case 0xD6: {  // SUI D8
            uint16_t result = (uint16_t)cpu->A - (uint16_t)read_memory(cpu->PC + 1);
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = result & 0xFF;
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0xD7: {  // RST 2
            cycle += 11;
            break;
        }
        case 0xD8: {  // RC
            if (cpu->flags->CY == 1) {
                ret(cpu);
                opcode_size = 0;
            }
            cycle += 5;
            break;
        }
        case 0xD9: {  // *RET (duplicate)
            cycle += 10;
            break;
        }
        case 0xDA: {  // JC addr
            if (cpu->flags->CY == 1) {
                cpu->PC = read_opcode_data_word(cpu);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 10;
            break;
        }
        case 0xDB: {  // IN D8 input
            uint8_t port = read_memory(cpu->PC + 1);
            cpu->A = machine_in(port);
            opcode_size = 2;
            cycle += 10;
            break;
        }
        case 0xDC: {  // CC addr
            if (cpu->flags->CY == 1) {
                call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 11;
            break;
        }
        case 0xDD: {  // *CALL a16 (duplicate)
            cycle += 17;
            break;
        }
        case 0xDE: {  // SBI D8
            uint16_t result = (uint16_t)cpu->A - (uint16_t)read_memory(cpu->PC + 1) - (uint16_t)cpu->flags->CY;
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = (uint8_t)(result & 0xFF);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0xDF: {  // RST 3
            cycle += 11;
            break;
        }
        case 0xE0: {  // RPO
            if (cpu->flags->P == 0) {
                ret(cpu);
                opcode_size = 0;
            }
            cycle += 5;
            break;
        }
        case 0xE1: {  // POP H
            cpu->L = read_memory(cpu->SP);
            cpu->H = read_memory(cpu->SP + 1);
            cpu->SP += 2;
            cycle += 10;
            break;
        }
        case 0xE2: {  // JPO addr
            if (cpu->flags->P == 0) {
                cpu->PC = read_opcode_data_word(cpu);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 10;
            break;
        }
        case 0xE3: {  // XTHL
            uint8_t l = cpu->L;
            cpu->L = read_memory(cpu->SP);
            write_memory(cpu->SP, l);
            uint8_t h = cpu->H;
            cpu->H = read_memory(cpu->SP + 1);
            write_memory(cpu->SP + 1, h);
            cycle += 18;
            break;
        }
        case 0xE4: {  // CPO addr
            if (cpu->flags->P == 0) {
                call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 11;
            break;
        }
        case 0xE5: {  // PUSH H
            write_memory(cpu->SP - 1, cpu->H);
            write_memory(cpu->SP - 2, cpu->L);
            cpu->SP -= 2;
            cycle += 11;
            break;
        }
        case 0xE6: {  // ANI D8
            uint16_t result = (uint16_t)(cpu->A) & (uint16_t)read_memory(cpu->PC + 1);
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            cpu->A = (uint8_t)(result & 0xff);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0xE7: {  // RST 4
            cycle += 11;
            break;
        }
        case 0xE8: {  // RPE
            if (cpu->flags->P == 1) {
                ret(cpu);
                opcode_size = 0;
            }
            cycle += 5;
            break;
        }
        case 0xE9: {  // PCHL
            cpu->PC = make_word(cpu->H, cpu->L);
            opcode_size = 0;
            cycle += 5;
            break;
        }
        case 0xEA: {  // JPE addr
            if (cpu->flags->P == 1) {
                cpu->PC = read_opcode_data_word(cpu);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 10;
            break;
        }
        case 0xEB: {  // XCHG
            uint8_t temp = cpu->H;
            cpu->H = cpu->D;
            cpu->D = temp;
            temp = cpu->L;
            cpu->L = cpu->E;
            cpu->E = temp;
            cycle += 5;
            break;
        }
        case 0xEC: {  // CPE addr
            if (cpu->flags->P == 1) {
                call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 11;
            break;
        }
        case 0xED: {  // *CALL a16 (duplicate)
            cycle += 17;
            break;
        }
        case 0xEE: {  // XRI D8
            cpu->A ^= read_memory(cpu->PC + 1);
            update_byte_CY(cpu, cpu->A);
            update_byte_SZAP(cpu, cpu->A);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0xEF: {  // RST 5
            cycle += 11;
            break;
        }
        case 0xF0: {  // RP
            if (cpu->flags->S == 0) {
                ret(cpu);
                opcode_size = 0;
            }
            cycle += 5;
            break;
        }
        case 0xF1: {  // POP PSW
            uint8_t flags = read_memory(cpu->SP);
            cpu->flags = *(uint8_t*)(&flags);
            cpu->A = read_memory(cpu->SP + 1);
            cpu->SP += 2;
            cycle += 10;
            break;
        }
        case 0xF2: {  // JP addr
            if (cpu->flags->S == 0) {
                cpu->PC = read_opcode_data_word(cpu);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 10;
            break;
        }
        case 0xF3: {  // DI
            cpu->interrupts_enabled = 0;
            cycle += 4;
            break;
        }
        case 0xF4: {  // CP addr
            if (cpu->flags->S == 0) {
                call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 11;
            break;
        }
        case 0xF5: {  // PUSH PSW
            write_memory(cpu->SP - 1, cpu->A);
            write_memory(cpu->SP - 2, *(uint8_t*)(&cpu->flags));
            cpu->SP -= 2;
            cycle += 11;
            break;
        }
        case 0xF6: {  // ORI D8
            cpu->A |= read_memory(cpu->PC + 1);
            update_byte_CY(cpu, cpu->A);
            update_byte_SZAP(cpu, cpu->A);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0xF7: {  // RST 6
            cycle += 11;
            break;
        }
        case 0xF8: {  // RM
            if (cpu->flags->S == 1) {
                ret(cpu);
                opcode_size = 0;
            }
            cycle += 5;
            break;
        }
        case 0xF9: {  // SPHL
            cpu->SP = make_word(cpu->H, cpu->L);
            cycle += 5;
            break;
        }
        case 0xFA: {  // JM addr
            if (cpu->flags->S == 1) {
                cpu->PC = read_opcode_data_word(cpu);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 10;
            break;
        }
        case 0xFB: {  // EI
            cpu->interrupts_enabled = 1;
            error("there");
            cycle += 4;
            break;
        }
        case 0xFC: {  // CM addr
            if (cpu->flags->S == 1) {
                call(cpu, read_opcode_data_word(cpu), cpu->PC + 3);
                opcode_size = 0;
            } else {
                opcode_size = 3;
            }
            cycle += 11;
            break;
        }
        case 0xFD: {  // *CALL a16 (duplicate)
            cycle += 17;
            break;
        }
        case 0xFE: {  // CPI D8
            uint16_t result = (uint16_t)cpu->A - (uint16_t)read_memory(cpu->PC + 1);
            update_half_word_SZAP(cpu, result);
            update_half_word_CY(cpu, result);
            opcode_size = 2;
            cycle += 7;
            break;
        }
        case 0xFF: {  // RST 7
            cycle += 11;
            break;
        }
        default: {
            printf("Unknown opcode: 0x%02X\n", opcode);
            break;
        }
    }
    cpu->PC += opcode_size;
    return cycle;
}