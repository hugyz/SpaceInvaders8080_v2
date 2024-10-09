10/4/24
Still working on cpu.h, cpu.c
Need a better understanding of the whole 8080 processor picture

Space Invaders 8080 Processor Emulator

<---- Points to where Hugyz is currently work on<br/>
+++++ Finished rationally, but may need to work on more<br/>
XXXXX Completed<br/>


Calloc instead of memset?

Assign array extern, static
static exclusive to the class
extern avaliable to all files that include
arr[] goes to stack
*arr goes to heap

STACK more for constants<br/>
HEAP for dynamically allocated values, longer lasting vars<br/>
Threading in computer science is a technique that allows multiple programs to run simultaneously on a single processor<br/>


1) Main program initialization:

    - The main() function in src/main.c is the entry point.
    - It would set up basic program structures and parse any command-line arguments.

2) Memory: +++++

    - create memory
    - Intialize memory 
    - Functions in src/memory/memory.c would be called to set up the emulated memory space.
    - read/write bytes
    - maybe still something to edit, havent touch RAM, VRAM , WRAM yet ????

3) ROM loading: XXXXX

    - Still part of memory initialization, but specifically loading ROM files.
    - The ROM files (invaders.h, invaders.g, invaders.f, invaders.e) from the roms/ directory would be loaded into the appropriate memory locations (0x0000, 0x0800, 0x1000, 0x1800 respectively).

4) CPU initialization: <----

    - Functions in src/cpu/cpu.c would initialize the CPU state.
    This includes setting up registers, flags, and the program counter.

5) I/O system setup:

    - src/io/input.c and src/io/output.c would initialize input/output ports and any necessary hardware emulation.

6) Video system initialization:

    - src/video/video.c would set up the video rendering system, including any necessary buffers or display windows.

7) Sound system initialization:

    - src/sound/sound.c would initialize the sound emulation components.

8) Debugger setup (if included):

    Any debugging tools or interfaces would be initialized last, before entering the main emulation loop.

    
    
    After these initialization steps, the program would likely enter its main emulation loop, where it begins executing instructions from the loaded ROM.
<<<<<<< HEAD











Steps to Build the Emulator
Understand the Intel 8080 Architecture:

The Intel 8080 processor has 8-bit registers and a 16-bit address bus, and the instruction set has about 256 instructions.
You will need to implement the instruction set of the 8080, which includes arithmetic, logic, branching, and data transfer instructions.
Key registers include: A (Accumulator), B, C, D, E, H, L, and the Program Counter (PC) and Stack Pointer (SP).
Set Up Memory and I/O:

Space Invaders uses 16 KB of ROM for its code and 1 KB of RAM for scratch memory.
Map the ROM and RAM in your emulator. ROM contains the game code, while RAM is for storing temporary game data.
The I/O system includes ports for input devices (like joysticks) and output for sound and video.
Emulate CPU Instructions:

Write functions to emulate each instruction of the Intel 8080 processor. For example, you’ll need to handle operations like MOV, ADD, JMP, etc.
Implement flags: Zero flag, Sign flag, Parity flag, Carry flag, and Auxiliary Carry flag.
You’ll also need to properly handle interrupts, which the original Space Invaders game used.
Display and Sound Emulation:

Space Invaders uses a monochrome display with a resolution of 224x256 pixels. You can use a pixel buffer to represent the screen and update it as the game runs.
Sound is handled via I/O ports, so you need to emulate how these ports work and create sound accordingly.
Input Handling:

Map the input keys to the 8080’s I/O ports. Space Invaders uses specific input like movement keys and the shoot button. Your emulator needs to capture these inputs and feed them into the appropriate I/O ports.
ROM Loading:

Space Invaders comes with multiple ROM files, typically divided into several parts. You’ll need to load these into the proper memory locations.
Ensure you handle memory mapping correctly, as the ROM code expects to be loaded at a specific address.
General Components of the Emulator:
CPU Emulator: This will mimic the 8080's behavior, executing instructions and managing registers, memory, and I/O.
Memory Management: A way to load the Space Invaders ROM into memory and emulate RAM behavior.
Display and Sound: Functions to render graphics and produce sounds.
Input Handling: Functions to capture user input and pass them to the emulated CPU.
=======
>>>>>>> 87c6bad7679a497667bae3526aa8ee4afabf2a0b
