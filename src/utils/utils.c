#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void error_stub(void) {
  fprintf(stderr, "8080-emu: ");
}

void error(const char *message) {
  error_stub();
  fprintf(stderr, "%s\n", message);
  exit(1);
}


