#ifndef CPU_H
#define CPU_H

#include <stdint.h>  // Include this for fixed-width integer types

typedef uint8_t (*CallbackIn)(uint8_t port);
typedef void (*CallbackOut)(uint8_t port, uint8_t value);

// Define the Callbacks struct after declaring the function pointer types
typedef struct {
    CallbackIn in;  // Function pointer for handling IN opcode
    CallbackOut out;  // Function pointer for handling OUT opcode
} Callbacks;

// Define Flags struct
typedef struct {
    uint8_t Z : 1;  // Zero flag
    uint8_t S : 1;  // Sign flag
    uint8_t P : 1;  // Parity flag
    uint8_t CY : 1;  // Carry flag
    uint8_t AC : 1;  // Auxiliary carry flag
    uint8_t PAD : 3;  // Unused bits
} Flags;

// Define the CPU struct
typedef struct {
    // Registers
    uint8_t A;
    uint8_t B, C;
    uint8_t D, E;
    uint8_t H, L;
    uint16_t SP;
    uint16_t PC;

    Flags *flags;
    Callbacks *callback;

    uint64_t num_steps;
    uint8_t interrupts_enabled;
    uint32_t cycles;
} CPU; 

void setCallbackIn(CPU *cpu, CallbackIn callback);
void setCallbackOut(CPU *cpu, CallbackOut callback);
uint8_t cpu_execute_instruction(CPU* cpu);
CPU* cpu_init(void);
void cpu_free(CPU* cpu);
void cpu_reset(CPU* cpu);
void ret(CPU *cpu);
void interrupt(CPU *cpu, int interrupt_num);
int get_num_steps(CPU *cpu);
uint16_t makeWord(uint8_t hi, uint8_t lo);

#endif
