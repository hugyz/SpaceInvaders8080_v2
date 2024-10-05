#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "memory.h" // Include this if you need to access memory functions

typedef struct {
    // Registers
    uint8_t a;    // Accumulator
    uint8_t b, c; // BC register pair
    uint8_t d, e; // DE register pair
    uint8_t h, l; // HL register pair
    uint16_t sp;  // Stack pointer
    uint16_t pc;  // Program counter

    // Flags
    struct {
        uint8_t z : 1; // Zero flag
        uint8_t s : 1; // Sign flag
        uint8_t p : 1; // Parity flag
        uint8_t cy : 1; // Carry flag
        uint8_t ac : 1; // Auxiliary carry flag
        uint8_t pad : 3; // Unused bits
    } flags;

    // Other CPU state
    uint8_t interrupt_enable;
    uint32_t cycles; // Cycle counter
} CPU;

// Function prototypes
CPU* cpu_create(void);
void cpu_destroy(CPU* cpu);
void cpu_run();
void cpu_reset(CPU* cpu);
void cpu_step(CPU* cpu);
uint8_t cpu_execute_instruction(CPU* cpu);

#endif