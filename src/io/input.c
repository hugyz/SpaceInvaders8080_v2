#include "input.h"
#include <SDL.h>           // For SDL2

#include <stdio.h>

static uint8_t *input_ports;  // Static array for input ports

void input_init() {
    input_ports = (uint8_t *)malloc(NUM_INPUT_PORTS);
    memset(input_ports, 0, NUM_INPUT_PORTS);  // Initialize input ports to zero
}

void free_input_ports() {
    free(input_ports);
    input_ports = NULL;
}

uint8_t input_read(uint8_t port) {
    return input_ports[port];
}

void input_write(uint8_t port, uint8_t value) {
    input_ports[port] = value;
}

void input_update(uint8_t * state) {

    // Port 0: DIP4, Fire, Left, Right
    input_ports[0] = 0x0E;  // Bits 1, 2, 3 are always 1
    if (state[SDL_SCANCODE_SPACE]) input_ports[0] |= 0x10;  // Fire
    if (state[SDL_SCANCODE_LEFT])  input_ports[0] |= 0x20;  // Move left
    if (state[SDL_SCANCODE_RIGHT]) input_ports[0] |= 0x40;  // Move right
    printf("Port 0: %02X\n", input_ports[0]);
    if(input_ports[0] == 0x0E) return;

    // Port 1: CREDIT, 2P Start, 1P Start, Fire, Left, Right
    input_ports[1] = 0x08;  // Bit 3 is always 1
    if (state[SDL_SCANCODE_C])    input_ports[1] |= 0x01;  // Credit
    if (state[SDL_SCANCODE_2])    input_ports[1] |= 0x02;  // 2P Start
    if (state[SDL_SCANCODE_1])    input_ports[1] |= 0x04;  // 1P Start
    if (state[SDL_SCANCODE_SPACE]) input_ports[1] |= 0x10;  // Fire
    if (state[SDL_SCANCODE_LEFT])  input_ports[1] |= 0x20;  // Left
    if (state[SDL_SCANCODE_RIGHT]) input_ports[1] |= 0x40;  // Right
    printf("Port 1: %02X\n", input_ports[1]);
    if(input_ports[1] == 0x08) return;

    // Port 2: DIP switches, Player 2 shot, left, right
    input_ports[2] = 0x00;  // Initialize to 0
    input_ports[2] |= (state[SDL_SCANCODE_D]) ? 0x01 : 0x00;  // DIP3 for ships
    input_ports[2] |= (state[SDL_SCANCODE_T]) ? 0x04 : 0x00;  // Tilt
    input_ports[2] |= (state[SDL_SCANCODE_X]) ? 0x10 : 0x00;  // P2 Fire
    if (state[SDL_SCANCODE_Z]) input_ports[2] |= 0x20;        // P2 Left
    if (state[SDL_SCANCODE_X]) input_ports[2] |= 0x40;        // P2 Right
    printf("Port 2: %02X\n", input_ports[2]);
}

void reset_ports() {
    input_ports[0] = input_ports[1] = input_ports[2] = 0;
}

uint8_t machine_in(uint8_t state, uint8_t port) {
    switch (port) {
        case 0:
        case 1:
        case 2:
            return input_read(port);  // Handle input read from port 0, 1, 2
        default:
            return 0;  // Default return value for unhandled ports
    }
}
