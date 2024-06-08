#define SDL_MAIN_HANDLED
#include "window.h"

int main(int argc, char const *argv[])
{
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "Usage: %s <path-to-rom_file.ch8> [-s/-xo]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *rom_file = argv[1];
    const char *mod = (argc == 3) ? argv[2] : "CHIP8";

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    chip8_t chip8 = {0};
    sdl_t sdl = {0};

    system_init(&chip8, mod);
    load_rom(&chip8, rom_file);
    window_init(&sdl);
    window_clear(&sdl);
    audio_init(&sdl);

    srand(time(NULL));

    while (chip8.state != QUIT)
    {
        do
        {
            keyboard(&chip8, mod, rom_file);
        } while (chip8.state == PAUSED);

        size_t start_perf = SDL_GetPerformanceFrequency();

        for (uint8_t i = 0; i < 700 / 60; i++)
        {
            instruction_execution(&chip8);
            // db_instruction_execution(&chip8);
        }

        size_t end_perf = SDL_GetPerformanceFrequency();

        const double elapsed_time = (double)((end_perf - start_perf) * 1000) / SDL_GetPerformanceFrequency();

        SDL_Delay(16.666f > elapsed_time ? 16.666f - elapsed_time : 0);
        
        if (chip8.draw_flag)
        {
            window_print(&sdl, &chip8);
            chip8.draw_flag = false;
        }

        update_timer(&chip8, &sdl);
    }
    
    SDL_Quit();
    return 0;
}
