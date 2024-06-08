// #define SDL_MAIN_HANDLED
#include "window.h"

size_t get_time_in_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return spec.tv_sec * 1000 + spec.tv_nsec / 1e6;
}

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

    size_t last_timer_update = get_time_in_ms();
    size_t last_instruction_update = get_time_in_ms();
    size_t timer_interval = 1000 / 60;                  //60 Hz for timer
    size_t instruction_interval = 1000 / 1000;          //700 Hz for instructions

    while (chip8.state != QUIT)
    {
        do
        {
            keyboard(&chip8, mod, rom_file);
        } while (chip8.state == PAUSED);

        size_t current_time = get_time_in_ms();

        if (current_time - last_instruction_update >= instruction_interval)
        {
            instruction_execution(&chip8);
            // db_instruction_execution(&chip8);
            last_instruction_update = current_time;
        }

        if (current_time - last_timer_update >= timer_interval)
        {
            update_timer(&chip8, &sdl);
            last_timer_update = current_time;
        }

        if (chip8.draw_flag)
        {
            window_print(&sdl, &chip8);
            chip8.draw_flag = false;
        }

        SDL_Delay(1);        
    }
    
    SDL_Quit();
    return 0;
}
