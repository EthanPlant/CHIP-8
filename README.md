# CHIP-8

This is a [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) emulator written in C++ with SFML

## Features

This is a fairly complete implementation of the CHIP-8 Interpreter with all 35 opcodes implemented (however at the moment there is no sound). Currently there's two "quirks" in the program.

- Opcodes FX55 (LD [I], Vx) and FX65 (LD Vx, [I]) increment the I register according to the CHIP-8 spec however some programs assume they don't.
- Opcodes 8XY6 (SHR Vx {, Vy}) and 8XYE (SHL Vx {, Vy}) shift the Vy register and store it in Vx however some programs incorrectly assume that Vx is to be shifted and Vy is left unchanged.

At the moment I'm choosing not to implement a fix for these quirks because they aren't bugs in the emulator but rather a programmer not following the CHIP-8 spec.

The emulator should work with any ROM for the CHIP-8. Some ROMs won't work correctly due to the quirks I've listed above.

The original CHIP-8 keypad has been mapped to a PC keyboard as below

    |1|2|3|C| => |1|2|3|4|
    |4|5|6|D| => |Q|W|E|R|
    |7|8|9|E| => |A|S|D|F|
    |A|0|B|F| => |Z|X|C|D|
    
## Compiling & Running

Requires cmake and SFML:

    $ sudo apt install cmake libsfml-dev

Clone and compile:

    $ git clone https://github.com/OfficialExedo/Chip8.git
    $ cd Chip8
    $ cmake . && make
    
Run:

    $ ./chip8 <ROM file>
    
23 public domain ROMS can be found in the `roms` directory.

[Rom compaitibility list](https://docs.google.com/spreadsheets/d/1-yieuiIF_zLWZBJa7zim8vuR8wYVeWO3ZUuTOS-TD0s/edit?usp=sharing)

## Resources

http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

http://www.emulator101.com/introduction-to-chip-8.html

https://en.wikipedia.org/wiki/CHIP-8

http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3xkk
