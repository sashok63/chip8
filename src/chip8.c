#include "chip8.h"

void load_rom(chip8_t *chip8, const char *rom_name)
{
    FILE *rom = fopen(rom_name, "rb");
    if (rom == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", rom_name);
        exit(EXIT_FAILURE);
    }

    fseek(rom, 0, SEEK_END);
    uint16_t rom_size = ftell(rom);
    if (rom_size > RAM_SIZE - START_ADDRESS)
    {
        fprintf(stderr, "Error %s too big, available size up to 3584 bytes\n", rom_name);
        fclose(rom);
        exit(EXIT_FAILURE);
    }
    rewind(rom);

    if (fread(&chip8->ram[START_ADDRESS], rom_size, 1, rom) != 1)
    {
        fprintf(stderr, "Error reading rom: %s, size: %d\n", rom_name, rom_size);
        fclose(rom);
        exit(EXIT_FAILURE);
    }

    fclose(rom);
}

void system_init(chip8_t *chip8, const char *mod)    
{
    const uint8_t font[] = {
        //Standard 8x5 font
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80,  // F

        //Hi-res 8x10 font (S-CHIP)
        0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C,  // 0
        0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C,  // 1
        0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF,  // 2
        0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C,  // 3
        0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06,  // 4
        0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C,  // 5
        0x3E, 0x7C, 0xC0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C,  // 6
        0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60,  // 7
        0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C,  // 8
        0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C   // 9
    };

    memset(chip8, 0, sizeof(chip8_t));
    memcpy(&chip8->ram[FONT_START], font, sizeof(font));
    chip8->PC = START_ADDRESS;
    chip8->state = RUNNING;

    if (strcmp(mod, "-s") == 0)
    {
        chip8->mod.SUPERCHIP = true;
    }
    else if (strcmp(mod, "-xo") == 0)
    {
        chip8->mod.XOCHIP = true;
    }
    else if (strcmp(mod, "CHIP8") == 0)
    {
        chip8->mod.CHIP = true;
    }
    else
    {
        fprintf(stderr, "Mod error: %s\n", mod);
        printf("You have modes like: ");
        printf("[-s] for SUPERCHIP, ");
        printf("[-xo] for XOCHIP\n");
        printf("If you don't wanna use them, don't set any flag\n");
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
}

void keyboard(chip8_t *chip8, const char *mod, const char *rom_file)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                chip8->state = QUIT;
                break;

            case SDL_QUIT:
                chip8->state = QUIT;
                break;
            
            case SDLK_SPACE:
                if (chip8->state == PAUSED)
                {
                    chip8->state = RUNNING;
                }
                else
                {
                    chip8->state = PAUSED;
                }
                break;

            case SDLK_LALT:
                system_init(chip8, mod);
                load_rom(chip8, rom_file);
                break;

            default:
                break;
            }
        }

        for (uint8_t i = 0; i < NUM_KEYS; i++)
        {
            if (event.key.keysym.sym == keyboard_map[i])
            {
                chip8->keyboard[i] = true;
            }
        }

        if (event.type == SDL_KEYUP)
        {
            for (uint8_t i = 0; i < NUM_KEYS; i++)
            {
                if (event.key.keysym.sym == keyboard_map[i])
                {
                    chip8->keyboard[i] = false;
                }
            }
        }
    }
}