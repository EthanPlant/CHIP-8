#include <cstdio>
#include <cstdlib>

#include "chip8.hpp"

uint8_t fontset[80] = 
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

void Chip8::init()
{
    // Clear screen
    for (int i = 0; i < 64; ++i)
    {
        for (int j = 0; j < 32; ++j)
        {
            display[i][j] = 0;
            should_draw = true;
        }
    }
    // Reset values
    sp = 0;
    pc = 0x200; // Starting point for Chip8 programs
    opcode = 0;
    regs.i = 0x200;

    // Clear stack, key, and V registers
    for (int i = 0; i < 16; ++i)
    {
        regs.v[i] = 0;
        key[i] = 0;
        stack[i] = 0;
    }

    // Clear memory
    for (int i = 0; i < 4096; ++i)
    {
        memory[i] = 0;
    }

    // Load font into memory
    for (int i = 0; i < 80; ++i)
    {
        memory[i] = fontset[i];
    }

    // Reset timers
    timers.delay_timer = 0;
    timers.sound_timer = 0;
}

void Chip8::load_rom(const char* file_name)
{
    init();

    printf("Opening ROM: %s\n", file_name);

    FILE* rom = fopen(file_name, "rb");
    if (rom == NULL)
    {
        printf("Error opening ROM: %s\n", file_name);
        exit(1);
    }

    // Get file size
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);

    // Allocate memory to store ROM
    char* buffer = (char*) malloc(sizeof(char) * rom_size);
    if (buffer == NULL)
    {
        printf("Failed to allocate memory for ROM\n");
        exit(1);
    }

    // Copy ROM into buffer
    size_t result = fread(buffer, sizeof(char), (size_t) rom_size, rom);
    if (result != rom_size)
    {
        printf("Failed to read ROM\n");
        exit(1);
    }


    if ((4096 - 0x200) > rom_size)
    {
        for (int i = 0; i < rom_size; ++i)
        {
            memory[i + 0x200] = (uint8_t) buffer[i]; // Load into memory startin at 0x200
        }
    }
    else
    {
        printf("ROM too large to fit in memory\n");
        exit(1);
    }

    // Clean up
    fclose(rom);
    free(buffer);

    printf("Loaded ROM Succesfully\n");
}

void Chip8::decrement_timers()
{
    if (timers.delay_timer > 0) --timers.delay_timer;
    if (timers.sound_timer > 0) --timers.sound_timer; 
}

void Chip8::run_opcode()
{
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];
    
    printf("PC: %03x, OP: %04x DT: %04x ST: %04x\n", pc, opcode, timers.delay_timer, timers.sound_timer);
    for (int i = 0; i <= 0xF; ++i)
    {
        printf("V%02d: %01x\n", i, regs.v[i]);
    }

    pc += 2; // Increment pc to next instruction

    switch (opcode & 0xF000) // Grab first nibble of opcode
    {
        case 0x0000:
            switch (opcode & 0x000F)
            {
                case 0x0000:
                    // 00E0, CLR
                    for (int i = 0; i < 64; ++i)
                    {
                        for (int j = 0; j < 32; ++j)
                        {
                            display[i][j] = 0;
                            should_draw = true;
                        }
                    }
                    break;
                case 0x000E:
                    // 00EE, RET
                    --sp;
                    pc = stack[sp];
                    break;
                default:
                    printf("Unsupported operation\n");
                    break;
            }
            break;
        case 0x1000:
            // 1NNN, JMP addr
            pc = opcode & 0x0FFF;
            break;
        case 0x2000:
            // 2NNN, CALL addr
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;
        case 0x3000:
            // 3XNN, SE Vx, byte
            if (regs.v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 2;
            }
            break;
        case 0x4000:
            // 4XNN, SNE Vx, byte
            if (regs.v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 2;
            }
            break;
        case 0x5000:
            // 5XY0, SE Vx, Vy
            if (regs.v[(opcode & 0x0F00) >> 8] == regs.v[(opcode & 0x00F0) >> 4]) {
                pc += 2;
            }
            break;
        case 0x6000:
            // 6XNN, LD Vx, byte
            regs.v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        case 0x7000:
            // 7XNN, ADD Vx, byte
            regs.v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;
        case 0x8000:
            switch(opcode & 0x000F)
            {
                case 0x0000:
                    // 8XY0, LD Vx, Vy
                    regs.v[(opcode & 0x0F00) >> 8] = regs.v[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0001:
                    // 8XY1, OR Vx, Vy
                    regs.v[(opcode & 0x0F00) >> 8] |= regs.v[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0002:
                    // 8XY2, AND Vx, Vy
                    regs.v[(opcode & 0x0F00) >> 8] &= regs.v[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0003:
                    // 8XY3, XOR Vx, Vy
                    regs.v[(opcode & 0x0F00) >> 8] ^= regs.v[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0004:
                    // 8XY4, ADD Vx, Vy
                    if (regs.v[(opcode & 0x00F0) >> 4] > (0xFF - regs.v[(opcode & 0x0F00) >> 8]))
                    {
                        regs.v[0xF] = 1;
                    } else
                    {
                        regs.v[0xF] = 0;
                    }
                    regs.v[(opcode & 0x0F00) >> 8] += regs.v[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0005:
                    // 8XY5, SUB Vx, Vy
                    if (regs.v[(opcode & 0x00F0) >> 4] > regs.v[(opcode & 0x0F00) >> 8])
                    {
                        regs.v[0xF] = 0;
                    }
                    else
                    {
                        regs.v[0xF] = 1;
                    }
                    regs.v[(opcode & 0x0F00) >> 8] -= regs.v[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0006:
                    // 8XY6, SHR Vx, Vy
                    regs.v[0xF] = (regs.v[(opcode & 0x00F0) >> 4] & 1);
                    regs.v[(opcode & 0x0F00) >> 8] = (regs.v[(opcode & 0x00F0) >> 4] >> 1);
                    break;
                case 0x0007:
                    // 8XY7, SUBN Vx, Vy
                    if (regs.v[(opcode & 0x0F00) >> 8] > regs.v[(opcode & 0x00F0) >> 4])
                    {
                        regs.v[0xF] = 0;
                    }
                    else
                    {
                        regs.v[0xF] = 1;
                    }
                    regs.v[(opcode & 0x0F00) >> 8] = regs.v[(opcode & 0x00F0) >> 8] - regs.v[(opcode & 0x0F00) >> 8];
                    break;
                case 0x000E:
                    // 8XYE, SHL Vx, Vy
                    regs.v[0xF] = regs.v[(opcode & 0x00F0) >> 4] >> 7;
                    regs.v[(opcode & 0x0F00) >> 8] = regs.v[(opcode & 0x00F0) >> 4] << 4;
                    break;
                default:
                    printf("Unsupported operation\n");
                    break;
            }
            break;
        case 0x9000:
            // 9XY0, SNE Vx, Vy
            if (regs.v[(opcode & 0x0F00) >> 8] != regs.v[(opcode & 0x00F0) >> 4])
            {
                pc += 2;
            }
            break;
        case 0xA000:
            // ANNN, LD I, addr
            regs.i = opcode & 0x0FFF;
            break;
        case 0xB000:
            // BNNN, JMP addr, V0
            pc = opcode & 0x0FFF + regs.v[0];
            break;
        case 0xC000:
            // CXNN, RND Vx, byte
            regs.v[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
            break;
        case 0xD000:
            // DXYN, DRW Vx, Vy, nibble
            {
                uint8_t x = regs.v[(opcode & 0x0F00) >> 8];
                uint8_t y = regs.v[(opcode & 0x00F0) >> 4];
                uint8_t height = opcode & 0x000F;
                uint8_t row = 0;

                regs.v[0xF] = 0;

                while (row < height)
                {
                    uint8_t crow = memory[regs.i + row];
                    uint8_t pixel_offset = 0;
                    while (pixel_offset < 8)
                    {
                        bool& pixel = display[x + pixel_offset][y + row];
                        pixel_offset++;
                        if (y + row >= 32 || x + pixel_offset - 1 >= 64)
                        {
                            continue;
                        }
                        uint8_t mask = 1 << (8 - pixel_offset);
                        bool curr_pixel = (crow & mask) >> (8 - pixel_offset);
                        pixel ^= curr_pixel;
                        if (curr_pixel && !pixel && !regs.v[0xF])
                        {
                            regs.v[0xF] = 1;
                        }
                    }
                    row++;
                }
                should_draw = true;
            }
            break;
        case 0xE000:
            switch(opcode & 0x00FF)
            {
                case 0x009E:
                    // EX9E, SKP Vx
                    if (key[regs.v[(opcode & 0x0F00) >> 8]] != 0)
                    {
                        pc += 2;
                    }
                    break;
                case 0x00A1:
                    // EXA1 SKNP Vx
                    if (key[regs.v[(opcode & 0x0F00) >> 8]] == 0)
                    {
                        pc += 2;
                    }
                    break;
                default:
                    printf("Unsupported operation\n");
                    break;
            }
            break;
        case 0xF000:
            switch(opcode & 0x00FF)
            {
                case 0x0007:
                    // FX07, LD Vx, DT
                    regs.v[(opcode & 0x0F00) >> 8] = timers.delay_timer;
                    break;
                case 0x000A:
                    // FX0A, LD Vx, K
                    {
                        bool key_pressed = false;

                        for (int i = 0; i < 16; ++i)
                        {
                            if (key[i] != 0)
                            {
                                regs.v[(opcode & 0x0F00) >> 8] = key[i];
                                key_pressed = true;
                            }
                        }

                        if (!key_pressed)
                        {
                            pc -= 2;
                        }
                    }
                    break;
                case 0x0015:
                    // FX15, LD DT, Vx
                    timers.delay_timer = regs.v[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0018:
                    // FX18 LD ST, Vx
                    timers.sound_timer = regs.v[(opcode & 0x0F00) >> 8];
                    break;
                case 0x001E:
                    // FX1E, ADD I, Vx
                    regs.i += regs.v[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0029:
                    // FX29, LD F, Vx
                    regs.i = regs.v[(opcode & 0x0F00) >> 8] * 0x5;
                    break;
                case 0x0033:
                    // FX33, LD B, Vx
                    memory[regs.i] = regs.v[(opcode & 0x0F00) >> 8] / 100;
                    memory[regs.i + 1] = (regs.v[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[regs.i + 2] = (regs.v[(opcode & 0x0F00) >> 8] / 100) % 10;
                    break;
                case 0x0055:
                    // FX55, DMP I ,Vx
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                    {
                        memory[regs.i + i] = regs.v[i];
                    }

                    regs.i += ((opcode & 0x0F00) >> 8) + 1;
                    break;
                case 0x0065:
                    //  FX65, LOAD Vx, I
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                    {
                        regs.v[i] = memory[regs.i + i];
                    }

                    regs.i += ((opcode & 0x0F00) >> 8) + 1;
                    break;
                default:
                    printf("Unsupported operation\n");
                    break;
            }
            break;
        default:
            printf("Unsupported operation\n");
            break;
    }
}