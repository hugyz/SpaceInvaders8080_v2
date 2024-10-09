#include "input.h"
#include "cpu.h"

static uint8_t * input_ports;  // Static array of I/O ports

void input_init() {
    input_ports = (char *) malloc(NUM_INPUT_PORTS);
    memset(input_ports, 0, NUM_INPUT_PORTS);  // Initialize memory to zero
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

uint8_t machine_in(uint8_t state, uint8_t port){
    
}