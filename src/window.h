#include "chip8.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 640

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

typedef struct {
    SDL_Window *window; 
    SDL_Renderer *renderer;
    SDL_AudioDeviceID device;
    SDL_AudioSpec desired, obtained;
} sdl_t;

void update_timer(chip8_t *chip8, sdl_t *sdl);
void audio_init(sdl_t *sdl);
void window_init(sdl_t *sdl);
void window_print(sdl_t *sdl, chip8_t *chip8);
void window_clear(sdl_t *sdl);