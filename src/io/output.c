#include "output.h"
#include "sound.h"  // Include sound for handling sound effects
#include "cpu.h"
#include <SDL.h>
#include <stdio.h>

static uint8_t output_ports[NUM_OUTPUT_PORTS];
static uint16_t shift_register = 0;  // 16-bit shift register
static uint8_t shift_offset = 0;     // 3-bit shift amount

// Read from the shift register
uint8_t read_shift_register() {
    return (uint8_t)((shift_register >> (shift_offset + 8)) & 0xFF);
}

// Process output based on the specified port and value
void machine_out(CPU *cpu, uint8_t port, uint8_t value) {
    switch (port) {
        case 2:
            shift_offset = value & 0x7; // Set shift amount based on bits 0-2
            break;
        case 4:
            shift_register = (uint16_t)(value << 8) | (shift_register >> 8); // Update shift register
            break;
        case 3:
        case 5:
            play_sound(value);  // Play sound based on value
            break;
        case 6:
            cpu_reset(cpu);  // Ensure this function is properly implemented in CPU code
            break;
        default:
            output_ports[port] = value;   // Write value to output port
            break;
    }
}

// Optionally provide a way to read from output ports if needed later
uint8_t output_read(uint8_t port) {
    return output_ports[port];
}
