# Compiler and flags
CC = gcc
CFLAGS = -w -I./src -I./src/cpu -I./src/memory -I./src/io -I./src/utils -I"C:/SDL2/include" -I"C:/SDL2_MIXER/include"

# SDL2 and SDL2_mixer paths (for 32-bit MinGW)
SDL2_LIB = -L"C:/SDL2/lib/x86" -lSDL2main -lSDL2
SDL2_MIXER_LIB = -L"C:/SDL2_MIXER/lib/x86" -lSDL2_mixer

# Object files
OBJ = src/cpu/cpu.o \
      src/cpu/update_flags.o \
      src/memory/memory.o \
      src/io/input.o \
      src/io/output.o \
      src/utils/utils.o

# Target executable placed into the 'bin' folder
TARGET = bin/space_invaders_emulator.exe

# Build the emulator
$(TARGET): $(OBJ) src/main.c
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) src/main.c $(SDL2_LIB) $(SDL2_MIXER_LIB)

# Compilation rules
src/cpu/cpu.o: src/cpu/cpu.c src/cpu/cpu.h
	$(CC) $(CFLAGS) -c src/cpu/cpu.c -o src/cpu/cpu.o

src/cpu/update_flags.o: src/cpu/update_flags.c src/cpu/update_flags.h
	$(CC) $(CFLAGS) -c src/cpu/update_flags.c -o src/cpu/update_flags.o

src/memory/memory.o: src/memory/memory.c src/memory/memory.h
	$(CC) $(CFLAGS) -c src/memory/memory.c -o src/memory/memory.o

src/io/input.o: src/io/input.c src/io/input.h
	$(CC) $(CFLAGS) -c src/io/input.c -o src/io/input.o

src/io/output.o: src/io/output.c src/io/output.h
	$(CC) $(CFLAGS) -c src/io/output.c -o src/io/output.o

src/utils/utils.o: src/utils/utils.c src/utils/utils.h
	$(CC) $(CFLAGS) -c src/utils/utils.c -o src/utils/utils.o

# Clean object files and the executable
clean:
	del /Q bin\space_invaders_emulator.exe src\cpu\cpu.o src\cpu\update_flags.o src\memory\memory.o src\io\input.o src\io\output.o src\utils\utils.o
