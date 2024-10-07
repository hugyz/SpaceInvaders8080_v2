Function of CPU

    uint8_t Z : 1; // Zero flag
    uint8_t S : 1; // Sign flag
    uint8_t P : 1; // Parity flag
    uint8_t CY : 1; // Carry flag
    uint8_t AC : 1; // Auxiliary carry flag
    uint8_t PAD : 3; // Unused bits

    // Registers
    uint8_t A;    // Accumulator
    uint8_t B, C; // BC register pair
    uint8_t D, E; // DE register pair
    uint8_t H, L; // HL register pair
    uint16_t SP;  // Stack pointer
    uint16_t PC;  // Program counter
    uint8_t interrupt_enable;
    uint32_t cycles; // Cycle counter


execute_instruction:

    Fetch: Retrieve the opcode (instruction) from memory at the current program counter (PC).
    Increment PC: Move the program counter to the next instruction in memory.
    Decode: Determine which operation to perform based on the fetched opcode.
    Execute: Carry out the corresponding operation (e.g., arithmetic, logic, I/O, control flow).
    Update PC and Cycle Count: Adjust the program counter (if needed) and update the cycle count.

