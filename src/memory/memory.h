#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

//further specifications about memory located in memory_map.md
#define MEMORY_START        0x0000
#define MEMORY_END          0xFFFF
#define MEMORY_SIZE         (MEMORY_END - MEMORY_START + 1)

#define ROM_START           0x0000
#define ROM_END             0x1FFF
#define ROM_SIZE            (ROM_END - ROM_START + 1)

#define RAM_START           0x2000
#define RAM_END             0x3FFF
#define RAM_SIZE            (RAM_END - RAM_START + 1)

#define WORK_RAM_START      0x2000
#define WORK_RAM_END        0x23FF
#define WORK_RAM_SIZE       (WORK_RAM_END - WORK_RAM_START + 1)

#define VIDEO_RAM_START     0x2400
#define VIDEO_RAM_END       0x3FFF
#define VIDEO_RAM_SIZE      (VIDEO_RAM_END - VIDEO_RAM_START + 1)

#define RAM_MIRROR_START    0x4000
#define RAM_MIRROR_END      0xFFFF

void memory_init(void);
void load_rom_into_mem(void);
uint8_t read_memory(uint16_t address);
void write_memory(uint16_t address, uint8_t value);
void memory_free();

#endif

/***
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void initMemory();
void loadROM(const char* filename);
uint8_t readByte(uint16_t address);
void writeByte(uint16_t address, uint8_t value);
uint16_t readWord(uint16_t address);
void writeWord(uint16_t address, uint16_t value);
uint8_t readIO(uint8_t port);
void writeIO(uint8_t port, uint8_t value);

extern uint8_t memory[MEMORY_SIZE];

#endif

***/