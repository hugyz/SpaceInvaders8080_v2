#include "utils.h"

#include <stdio.h>

void error_stub(void) {
  fprintf(stderr, "8080-emu: ");
}

void error(const char *message) {
  error_stub();
  fprintf(stderr, "%s\n", message);
  exit(1);
}

uint8_t parity(uint16_t value) {
    int count = 0;
    while (value) {
        value &= (value - 1);  // Clear the lowest set bit
        count++;
    }
    return (count % 2 == 0) ? 1 : 0;
}


