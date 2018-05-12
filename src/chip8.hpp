#pragma once

#include <cstdint>

class Chip8
{
    private:
        uint8_t memory[4096]; // 4K
        
        // Registers
        struct
        {
            uint8_t v[16]; // 16 V registers, V0-VF
            uint16_t i;
        } regs;

        struct
        {
            uint8_t delay_timer;
            uint8_t sound_timer;
        } timers;

        // Stack and stack pointer
        uint16_t stack[16];
        uint16_t sp;

        uint16_t pc; // Program counter
        uint16_t opcode; // Current opcode

        void init();

    public:
        Chip8(){}
        ~Chip8(){}

        bool should_draw;
        bool display[64][32]; // Pixel values
        uint8_t key[16]; // Keypad

        void load_rom(const char* file_name);
        void run_opcode();
        void decrement_timers();
};