Memory Map

8080's 16-bit address bus

    Total addressable memory: 64 KB (0x0000 - 0xFFFF)


        ROM: 0x0000 - 0x1FFF (8 KB)

            0x0000 - 0x07FF: invaders.h
            0x0800 - 0x0FFF: invaders.g
            0x1000 - 0x17FF: invaders.f
            0x1800 - 0x1FFF: invaders.e

        RAM: 0x2000 - 0x3FFF (8 KB)

            0x2000 - 0x23FF: Work RAM (1 KB)
            0x2400 - 0x3FFF: Video RAM (7 KB)

        RAM Mirror: 0x4000 and above

            0x4000 - 0xFFFF (48 KB)
