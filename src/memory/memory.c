#include "memory.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t * memory;

void memory_init(void) {
    memory = (uint8_t *)malloc(MEMORY_SIZE);
    memset(memory, 0, MEMORY_SIZE);  // Initialize memory to zero
}

void memory_free() {
    free(memory);
    memory = NULL;
}

uint8_t read_memory(uint16_t address) {
    if (address >= MEMORY_START && address < MEMORY_END) return memory[address];
    else error("read_memory out of bounds");
}

void write_memory(uint16_t address, uint8_t value) {
    if(address >= ROM_START && address < ROM_END) error("cannot write to rom");
    else memory[address] = value;
}


void load_rom_into_mem(void) {
    const char* rom_file_path = "C:\\Users\\hugoz\\OneDrive\\Desktop\\Projects\\SpaceInvaders8080_v2\\roms\\cpudiag\\cpudiag.bin";
    //const char* rom_file_path = "C:\\Users\\hugoz\\OneDrive\\Desktop\\Projects\\SpaceInvaders8080_v2\roms\\invaders\\invaders";

    FILE *rom_file = fopen(rom_file_path, "rb");
    if(!rom_file) error("cannot open rom_file");
    fseek(rom_file, 0L, SEEK_END);
    long rom_file_size = ftell(rom_file);
    fseek(rom_file, 0L, SEEK_SET);

    if (rom_file_size < 0) {
        fclose(rom_file);
        error("Cannot determine ROM file size");
    }
    if (rom_file_size > ROM_SIZE) {
        fclose(rom_file);
        error("ROM file is larger than available ROM space");
    }

    size_t bytes_read = fread(&memory[ROM_START], 1, rom_file_size, rom_file);
    if (bytes_read != rom_file_size) {
        fclose(rom_file);
        error("Failed to read entire ROM file");
    }
    fclose(rom_file);
    printf("ROM loaded successfully. Size: %zu bytes\n", bytes_read);
}
