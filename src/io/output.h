#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdlib.h>
#include <stdint.h>

#define NUM_OUTPUT_PORTS 256  // Assume 256 ports

void output_init();
void free_output_ports();
uint8_t output_read(uint8_t port);
void output_write(uint8_t port, uint8_t value);

#endif 