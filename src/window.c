#include "window.h"

void update_timer(chip8_t *chip8, sdl_t *sdl)
{
    if (chip8->delay_timer > 0)
    {
        chip8->delay_timer--;
        // printf("Delay Timer: %d\n", chip8->delay_timer);     //For debug purpose
    }

    if (chip8->sound_timer > 0)
    {
        chip8->sound_timer--;
        SDL_PauseAudioDevice(sdl->device, 0);
        // printf("Sound Timer: %d\n", chip8->sound_timer);     //For debug purpose
    }
    else
    {
        SDL_PauseAudioDevice(sdl->device, 1);
    }
}

void callback(void *userdata, uint8_t *stream, int len)
{
    (void)userdata;

    uint8_t *audio_data = stream;
    uint16_t sample_index = 0;
    const uint8_t wave_period = 44100 / 440;
    const uint8_t half_period = wave_period / 2;

    for (uint16_t i = 0; i < len / 2; i++)
    {
        audio_data[i] = ((sample_index++ / half_period) % 2) ? 3000 : -3000;
    }
}

void audio_init(sdl_t *sdl)
{
    sdl->desired = (SDL_AudioSpec){
        .freq = 44100,
        .format = AUDIO_U8,
        .channels = 1,
        .samples = 4096,
        .callback = callback,
        .userdata = NULL   
    };

    sdl->device = SDL_OpenAudioDevice(NULL, 0, &sdl->desired,
                                      &sdl->obtained, 0);
    if (sdl->device == 0)
    {
        fprintf(stderr, "Failed open audio device: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
}

void window_init(sdl_t *sdl)
{
    sdl->window = SDL_CreateWindow("CHIP8 Emulator",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (sdl->window == NULL)
    {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    
    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (sdl->renderer == NULL)
    {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
}

void window_print(sdl_t *sdl, chip8_t *chip8)
{
    uint8_t scale = (chip8->mod.SUPERCHIP && chip8->hr.HiRes) ? SCALE_S : SCALE;
    uint8_t screen_width = (chip8->mod.SUPERCHIP && chip8->hr.HiRes) ? SCREEN_WIDTH_S : SCREEN_WIDTH;
    uint8_t screen_height = (chip8->mod.SUPERCHIP && chip8->hr.HiRes) ? SCREEN_HEIGHT_S : SCREEN_HEIGHT;

    SDL_Rect rect = {.w = scale, .h = scale};
    
    for (uint8_t y = 0; y < screen_height; y++)
    {
        for (uint8_t x = 0; x < screen_width; x++)
        {
            rect.x = x * scale;
            rect.y = y * scale;
            
            uint32_t i = y * screen_width + x;
            
            if (chip8->gfx[i])
            {
                SDL_SetRenderDrawColor(sdl->renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(sdl->renderer, &rect);

                //Draw pixel outline
                SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(sdl->renderer, &rect);
            }
            else
            {
                SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(sdl->renderer, &rect);
            }
        }
    }
    
    SDL_RenderPresent(sdl->renderer);
}

void window_clear(sdl_t *sdl)
{    
    SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl->renderer);
}