#ifndef INPUT_H
#define INPUT_H

#define NUM_INPUT_PORTS 4  // 4 Input ports used in Space Invaders

#include <stdint.h>
#include <stdlib.h>

void input_init();
void free_input_ports();
uint8_t input_read(uint8_t port);
void input_write(uint8_t port, uint8_t value);
void input_update(uint8_t *state);

// This function emulates the machine's input handling
uint8_t machine_in(uint8_t state, uint8_t port);

#endif  // INPUT_H
