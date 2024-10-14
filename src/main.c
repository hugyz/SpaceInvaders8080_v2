#include <SDL.h>
#include "utils.h" 
#include "cpu.h"
#include "input.h"
#include "output.h"

#define SCREEN_HEIGHT 256
#define SCREEN_WIDTH 224
#define SCREEN_SIZE_BYTES (SCREEN_WIDTH * SCREEN_HEIGHT / 8)  // 7,168 bytes

#define CPU_CLOCK 2000000 //2MHz
#define FRAMES_PER_SECOND 60 

// Screen memory (1-bit per pixel)
uint8_t screenMemory[SCREEN_SIZE_BYTES] = {0};

// Function to update the texture from screen memory
void updateTexture(SDL_Texture* texture) {
    uint32_t* pixels;
    int pitch;

    // Lock the texture to get access to the pixel buffer
    if (SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch) != 0) {
        printf("Failed to lock texture: %s\n", SDL_GetError());
        return;
    }

    // Iterate over each pixel and convert the 1-bit data to 32-bit RGBA
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            int index = (y * SCREEN_WIDTH + x) / 8;  // Find the byte in the memory array
            int bit = 7 - (x % 8);  // Find the bit within the byte (7 is the left-most bit)

            // Set the pixel color to white (0xFFFFFFFF) if the bit is 1, otherwise black (0xFF000000)
            uint32_t color = (screenMemory[index] & (1 << bit)) ? 0xFFFFFFFF : 0xFF000000;

            // Set the pixel in the texture's pixel buffer
            pixels[y * (pitch / 4) + x] = color;  // pitch / 4 gives the number of pixels per row
        }
    }

    // Unlock the texture after updating the pixel buffer
    SDL_UnlockTexture(texture);
}

// Toggle pixel color at (x, y)
void togglePixel(int x, int y) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;  // Ignore clicks outside of the screen bounds
    }

    int index = (y * SCREEN_WIDTH + x) / 8;  // Find the byte in screenMemory[]
    int bit = 7 - (x % 8);  // Find the bit within the byte

    // Toggle the bit (turn the pixel on/off)
    screenMemory[index] ^= (1 << bit);
}

void sync_to_real_time() {
    static Uint32 last_frame_time = 0;
    Uint32 current_time = SDL_GetTicks();
    Uint32 frame_duration = 1000 / 60;  // 60 frames per second

    if (current_time - last_frame_time < frame_duration) {
        SDL_Delay(frame_duration - (current_time - last_frame_time));
    }

    last_frame_time = SDL_GetTicks();
}


int main(int argc, char* argv[]) {

    CPU *cpu = cpu_init();

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(0);
    }

    // Create an SDL window
    SDL_Window* window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(0);
    }

    // Create an SDL renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(0);
    }

    // Create an SDL texture that matches the screen size
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    

    int running = 1;

    // Game loop
    while (running) {
        SDL_Event event;

        void emulate_8080(cpu);

        while (SDL_PollEvent(&event)) {         //if there is an event on the screen
            if (event.type == SDL_QUIT) {
                running = 0;
                break;
            }

            const Uint8 *state = SDL_GetKeyboardState(NULL);

            printf("__________");
            input_update(state);
            
printf("__________");



            // Handle mouse click event
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                togglePixel(mouseX, mouseY);  // Toggle the pixel at the click position
            }
            
                reset_ports();
        }

        // Update the texture with the screen memory
        updateTexture(texture);

        // Clear and render the updated texture
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);  // Simulate ~60 FPS
    }

    // Clean up
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void emulate_8080(CPU *cpu) {
    int cycles_per_frame = CPU_CLOCK / FRAMES_PER_SECOND;  // CPU_CLOCK in Hz, FRAMES_PER_SECOND is 60
    int current_cycles = 0;

    while (1) {
        // Step through CPU instructions, enough to simulate a full frame's worth of cycles
        while (current_cycles < cycles_per_frame) {
            // Fetch, decode, and execute the next instruction
            int cycles = execute_instruction(cpu);
            current_cycles += cycles;  // Keep track of how many cycles have been executed
        }

        // Handle user inputs (e.g., movement, shooting)
        handle_input(cpu);

        // Update the display to reflect any changes in the game
        render_display(cpu);

        // Optionally: Handle sound
        // sound_emulation(cpu);

        // Sync to real-time to maintain 60Hz frame rate
        sync_to_frame_rate();

        // Reset cycle count for the next frame
        current_cycles = 0;
    }
}