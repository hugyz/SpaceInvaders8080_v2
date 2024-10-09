#include "cpu.h"

int main() {
    CPU *cpu = cpu_init();
    
    while (cpu->PC < 0xffff) {
        cpu_execute_instruction(cpu);
    }

    cpu_free(cpu);
    return 0;
}