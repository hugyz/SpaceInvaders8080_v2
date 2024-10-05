Function of CPU

Fetch, decode, execute: 

    This is the heart of the CPU emulation. You'll fetch an opcode from memory, decode it to determine what operation to perform, and then execute it by modifying the CPU state (registers, flags, memory, etc.).


Opcode handling: 

    Each opcode will have an associated function or handler that knows how to perform the operation. For example, 0x00 is a no-op (NOP), so it would just increment the program counter.


Interrupt handling: 
    
    Space Invaders uses interrupts, so your CPU emulation will also need to handle them.


The CPU struct contains all necessary registers, flags, and state information.
cpu_create() allocates memory for a new CPU instance.
cpu_destroy() frees the allocated memory.
cpu_reset() initializes the CPU to a known state.
cpu_step() executes one instruction cycle.
cpu_execute_instruction() is where you'll implement the logic for each opcode