#include "input.h"
#include "output.h"
#include <SDL.h>           // For SDL2

#include <stdio.h>

static uint8_t button_state;
static uint8_t input_ports[NUM_INPUT_PORTS];  // Static array for input ports

uint8_t input_read(uint8_t port) {
    return input_ports[port];
}

void input_write(uint8_t port, uint8_t value) {
    input_ports[port] = value;
}

void input_update(uint8_t * state) {

    if(state[SDL_SCANCODE_EQUALS])
                exit(0);

    // Port 0: DIP4, Fire, Left, Right
    input_ports[0] = 0x0E;  // Bits 1, 2, 3 are always 1
    if (state[SDL_SCANCODE_SPACE]) input_ports[0] |= 0x10;  // Fire
    if (state[SDL_SCANCODE_LEFT])  input_ports[0] |= 0x20;  // Move left
    if (state[SDL_SCANCODE_RIGHT]) input_ports[0] |= 0x40;  // Move right
    printf("Port 0mofo: %02X\n", input_ports[0]);
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
    input_ports[2] |= (state[SDL_SCANCODE_F]) ? update_button_state() : 0x00;  // DIP3 for ships
    input_ports[2] |= (state[SDL_SCANCODE_T]) ? 0x04 : 0x00;  // Tilt
    input_ports[2] |= (state[SDL_SCANCODE_W]) ? 0x10 : 0x00;  // P2 Fire
    if (state[SDL_SCANCODE_A]) input_ports[2] |= 0x20;        // P2 Left
    if (state[SDL_SCANCODE_D]) input_ports[2] |= 0x40;        // P2 Right
    printf("Port 2: %02X\n", input_ports[2]);
}

uint8_t update_button_state() {
    button_state++;
    if (button_state > 3) button_state = 0;
    return button_state;
}

void reset_ports() {
    input_ports[0] = input_ports[1] = input_ports[2] = 0;
}

uint8_t machine_in(uint8_t port) {
    uint8_t a;    
    switch(port) {
        case 0: {
            a = input_ports[0];
        }
        case 1: {
            a = input_ports[1];
        }
        case 2: {
            a = input_ports[2];
        }
        case 3: {    
               a = read_shift_register();
               break;
           }
        default: {
            error("not a port #");
            break;
        } 
       }    
       return a;  
}
