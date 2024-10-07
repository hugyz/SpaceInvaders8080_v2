#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdint.h>

void error_stub(void);
void error(const char *message);
uint8_t parity(uint16_t value);

#endif