10/4/24
Still working on cpu.h, cpu.c
Need a better understanding of the whole 8080 processor picture

Space Invaders 8080 Processor Emulator

<---- Points to where Hugyz is currently work on<br/>
+++++ Finished rationally, but may need to work on more<br/>
XXXXX Completed<br/>




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
