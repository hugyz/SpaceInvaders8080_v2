#include "utils.h"
#include "cpu.h"
#include "input.h"
#include "output.h"
#include "memory.h"

#include "sound.h"
#include "video.h"

#include <SDL.h>
#include <stdio.h>

#define CPU_CLOCK 2000000  // CPU clock speed in Hz (2 MHz)
#define FRAMES_PER_SECOND 60  // The frame rate (60 FPS)

#define CYCLES_PER_FRAME (CPU_CLOCK / FRAMES_PER_SECOND)

int main(int argc, char* argv[]) {

    CPU *cpu = cpu_init();
    memory_init();
    load_rom_into_mem();
    audio_init();      // Initialize audio for sound effects
    reset_ports();     // Reset input/output ports

    printf("Finished initializations\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0) {
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
    uint32_t current_cycles = 0;

    while (running) {
        // Handle events (e.g., SDL_QUIT)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Update input state from keyboard
        input_update(SDL_GetKeyboardState(NULL));

        // Emulate CPU
        while (current_cycles < CYCLES_PER_FRAME) {
            uint16_t instruction_cycles = cpu_execute_instruction(cpu);
            current_cycles += instruction_cycles;

            // Check for mid-frame interrupt
            if (current_cycles >= CYCLES_PER_FRAME / 2 && current_cycles < (CYCLES_PER_FRAME / 2 + instruction_cycles)) {
                if (cpu->interrupts_enabled)
                    generate_interrupt(cpu, 1);  // Mid-frame interrupt
            }
        }

        // VBlank interrupt
        if (cpu->interrupts_enabled)
            generate_interrupt(cpu, 2);

        // Reset cycle count for the next frame
        current_cycles -= CYCLES_PER_FRAME;

        // Update display
        update_texture(texture, cpu);

        // Clear and present the renderer
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Handle output effects based on CPU state or ports
        for (int port = 0; port < NUM_OUTPUT_PORTS; port++) {
            uint8_t port_value = output_read(port);  // Read from each output port
            machine_out(cpu, port, port_value);      // Process output based on port value
        }

        SDL_Delay(10000);

        // Sync to maintain 60 FPS
        //sync_to_real_time();
    }

    // Cleanup resources
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    cpu_free(cpu);
    memory_free();
    audio_free();

    return 0;
}
