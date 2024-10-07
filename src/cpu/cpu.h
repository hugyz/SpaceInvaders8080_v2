#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "memory.h" // Include this if you need to access memory functions

// Flags
typedef struct {
    uint8_t Z : 1; // Zero flag
    uint8_t S : 1; // Sign flag
    uint8_t P : 1; // Parity flag
    uint8_t CY : 1; // Carry flag
    uint8_t AC : 1; // Auxiliary carry flag
    uint8_t PAD : 3; // Unused bits
} Flags;

typedef struct {
    // Registers
    uint8_t A;    // Accumulator
    uint8_t B, C; // BC register pair
    uint8_t D, E; // DE register pair
    uint8_t H, L; // HL register pair
    uint16_t SP;  // Stack pointer
    uint16_t PC;  // Program counter

    Flags *flags;

    // Other CPU state
    uint8_t interrupt_enable;
    uint32_t cycles; // Cycle counter
} CPU;

// Function prototypes


void emulate_8080(CPU *cpu);
uint8_t cpu_execute_instruction(CPU* cpu);

CPU* cpu_create(void);
void cpu_destroy(CPU* cpu);
void cpu_reset(CPU* cpu);

#endif