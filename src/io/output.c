#include "output.h"

static uint8_t *output_ports;  // Static array of I/O ports

void output_init() {
    output_ports = (char *)malloc(NUM_OUTPUT_PORTS);
    memset(output_ports, 0, NUM_OUTPUT_PORTS);  // Initialize memory to zero
}

void free_output_ports() {
    free(output_ports);
    output_ports = NULL;    
}

uint8_t output_read(uint8_t port) {
    return output_ports[port];
}

void output_write(uint8_t port, uint8_t value) {
    output_ports[port] = value;
}

