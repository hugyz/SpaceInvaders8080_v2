#include <SDL.h>
#include "utils.h"
#include "cpu.h"
#include "input.h"
#include "output.h"
#include "memory.h"
#include <stdio.h>

#define SCREEN_HEIGHT 256
#define SCREEN_WIDTH 224
#define SCREEN_MEMORY_START 0x2400
#define SCREEN_MEMORY_END 0x3FFF
#define SCREEN_SIZE_BYTES (SCREEN_MEMORY_END - SCREEN_MEMORY_START + 1)  // 7168 bytes

#define CPU_CLOCK 2000000  // 2MHz
#define FRAMES_PER_SECOND 60 
#define CYCLES_PER_FRAME (CPU_CLOCK / FRAMES_PER_SECOND)

void update_texture(SDL_Texture* texture, CPU *cpu);
void sync_to_real_time();
void emulate_8080(CPU *c, int cycles);
void handle_interrupts(CPU *cpu, int interrupt_type);
void handle_input(CPU *cpu, SDL_Event *event);

int main(int argc, char* argv[]) {
    CPU *cpu = cpu_init();
    
    memory_init();
    load_rom_into_mem();
    audio_init();
    printf("Finished initializations\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    int cycles_per_frame = CPU_CLOCK / FRAMES_PER_SECOND;
    uint32_t current_cycles = 0;


    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            
        }

        // Update input state
        input_update(SDL_GetKeyboardState(NULL));

        // Emulate CPU
        while (current_cycles < cycles_per_frame) {
            uint16_t instruction_cycles = cpu_execute_instruction(cpu);
            current_cycles += instruction_cycles;

            // Check for mid-frame interrupt
            if (current_cycles >= cycles_per_frame / 2 && current_cycles < (cycles_per_frame / 2 + instruction_cycles)) {
                handle_interrupts(cpu, 0x08);  // Mid-frame interrupt
            }
        }

        // VBlank interrupt
        handle_interrupts(cpu, 0x10);

        // Reset cycle count for the next frame
        current_cycles -= cycles_per_frame;

        // Update display
        update_texture(texture, cpu);
        SDL_RenderClear(renderer);


        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Set to green
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);


        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Sync to maintain 60 FPS
        sync_to_real_time();
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    cpu_free(cpu);
    memory_free();
    audio_free();
    return 0;
}

void handle_interrupts(CPU *cpu, int interrupt_type) {
    if (cpu->interrupts_enabled) {
        write_memory(cpu->SP - 1, (cpu->PC >> 8) & 0xFF);
        write_memory(cpu->SP - 2, cpu->PC & 0xFF);
        cpu->SP -= 2;  // Update the stack pointer

        // Set the PC to the interrupt address (interrupt_type)
        cpu->PC = interrupt_type;
        // Disable interrupts after handling
        cpu->interrupts_enabled = 0;
    }
}

void update_texture(SDL_Texture* texture, CPU *cpu) {
    uint32_t* pixels;
    int pitch;

    if (SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch) != 0) {
        printf("Failed to lock texture: %s\n", SDL_GetError());
        return;
    }

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int byte_index = SCREEN_MEMORY_START + (y * 32) + (x / 8);
            int bit_index = 7 - (x % 8);
            uint8_t byte = read_memory(byte_index);
            uint32_t color = (byte & (1 << bit_index)) ? 0xFFFFFFFF : 0xFF000000;
            pixels[y * (pitch / 4) + x] = color;
        }
    }

    SDL_UnlockTexture(texture);

    // Debug: Print the first few bytes of video memory
    printf("First 16 bytes of video memory: ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", read_memory(SCREEN_MEMORY_START + i));
    }
    printf("\n");
}

void sync_to_real_time() {
    static uint32_t last_time = 0;
    uint32_t current_time = SDL_GetTicks();
    uint32_t frame_time = 1000 / FRAMES_PER_SECOND;

    if (current_time - last_time < frame_time) {
        SDL_Delay(frame_time - (current_time - last_time));
    }

    last_time = SDL_GetTicks();
}