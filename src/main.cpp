#include <iostream>

#include <SFML/Graphics.hpp>

#include "chip8.hpp"

// Keymap
sf::Keyboard::Key keymap[16] = 
{
    sf::Keyboard::X,
    sf::Keyboard::Num1,
    sf::Keyboard::Num2,
    sf::Keyboard::Num3,
    sf::Keyboard::Q,
    sf::Keyboard::W,
    sf::Keyboard::E,
    sf::Keyboard::A,
    sf::Keyboard::S,
    sf::Keyboard::D,
    sf::Keyboard::X,
    sf::Keyboard::C,
    sf::Keyboard::Num4,
    sf::Keyboard::R,
    sf::Keyboard::F,
    sf::Keyboard::V
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: chip8 <ROM file>" << std::endl;
        return 1;
    }

    // Window width and height
    int w = 640;
    int h = 360; 

    // Initialize SFML window
    sf::RenderWindow window(sf::VideoMode(w, h), "Chip8 Emulator");

    // Create pixel template
    sf::RectangleShape pixel({10.f, 10.f});
    pixel.setFillColor(sf::Color::White);

    // Proper timing
    float timer_period = 1e6f/60.f;
    sf::Clock timer;
    timer.restart();

    Chip8 chip8 = Chip8();

    load:
        // Attempt to load ROM
        chip8.load_rom(argv[1]);

    window.clear(sf::Color::Black);

    // Emulation loop
    while (window.isOpen())
    {
        // Process SFML events
        sf::Event event;
        while(window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // Process SFML input
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            return 0;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F1))
        {
            goto load;
        }
        for (int i = 0; i < 16; ++i)
        {
            if (sf::Keyboard::isKeyPressed(keymap[i]))
            {
                chip8.key[i] = 1;
            }
            else
            {
                chip8.key[i] = 0;
            }
        }

        if (timer.getElapsedTime().asMicroseconds() > timer_period)
        {
            chip8.decrement_timers();
            timer.restart();
        }

        chip8.run_opcode();

        // If draw occured, redraw screen
        if (chip8.should_draw)
        {
            chip8.should_draw = false;
            window.clear(sf::Color::Black);
            for (int i = 0; i < 64; ++i)
            {
                for (int j = 0; j < 32; ++j)
                {
                    if(chip8.display[i][j])
                    {
                        pixel.setFillColor(sf::Color::White);
                    }
                    else
                    {
                        pixel.setFillColor(sf::Color::Black);
                    }
                    pixel.setPosition(i * 10.f, j * 10.f);
                    window.draw(pixel);
                }
            }
        }
        window.display();

        // Sleep to slow down emulation speed
        sf::sleep(sf::microseconds(2000));
    }
}