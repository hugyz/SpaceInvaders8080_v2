#include "cpu.h"
#include "utils.h"
#include "memory.h"

#include <stdlib.h>

CPU* cpu_create(void) {
    CPU* cpu = (CPU*)malloc(sizeof(CPU));
    if (cpu) {
        cpu_reset(cpu);
    }
    return cpu;
}

void cpu_destroy(CPU* cpu) {
    free(cpu);
}

void cpu_reset(CPU* cpu) {
    if (cpu) {
        cpu->a = 0;
        cpu->b = cpu->c = 0;
        cpu->d = cpu->e = 0;
        cpu->h = cpu->l = 0;
        cpu->sp = 0;
        cpu->pc = 0;
        cpu->flags.z = cpu->flags.s = cpu->flags.p = cpu->flags.cy = cpu->flags.ac = 0;
        cpu->interrupt_enable = 0;
        cpu->cycles = 0;
    }
}

void cpu_run(CPU* cpu) {
    while (NULL) {
        uint8_t opcode = read_memory(cpu.pc);
        cpu->pc++;
        execute_instruction(opcode);
        handle_interrupts();
        update_timers();
    }
}

void cpu_step(CPU* cpu) {
    uint8_t opcode = read_memory(memory, cpu->pc++);
    uint8_t cycles = cpu_execute_instruction(cpu, memory);
    cpu->cycles += cycles;
    // Handle interrupts, timing, etc.
}

uint8_t cpu_execute_instruction(CPU* cpu) {
    // Implement instruction execution here
    // This function should handle all opcodes and return the number of cycles taken
    return 0; // Placeholder
}

