# Compiler and flags
CC = gcc
CFLAGS = -w -I./src -I./src/cpu -I./src/memory -I./src/io -I./src/utils -I./src/video -I./src/sound

# Object files
OBJ = src/cpu/cpu.o \
      src/cpu/update_flags.o \
      src/memory/memory.o \
      src/io/input.o \
      src/io/output.o \
      src/utils/utils.o \
      src/video/video.o \
      src/sound/sound.o

# Target executable placed into the 'bin' folder
TARGET = bin/space_invaders_emulator.exe

# Build the emulator
$(TARGET): $(OBJ) src/main.c
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) src/main.c

# Individual object file compilation rules
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

src/video/video.o: src/video/video.c src/video/video.h
	$(CC) $(CFLAGS) -c src/video/video.c -o src/video/video.o

src/sound/sound.o: src/sound/sound.c src/sound/sound.h
	$(CC) $(CFLAGS) -c src/sound/sound.c -o src/sound/sound.o

# Clean object files and the executable
clean:
	# Clean object files and the executable
clean:
	del /Q src\cpu\cpu.o src\cpu\update_flags.o src\memory\memory.o src\io\input.o src\io\output.o src\utils\utils.o src\video\video.o src\sound\sound.o
	