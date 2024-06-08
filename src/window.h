#include "chip8.h"

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

bool calculate_pixel_value_for_hires(chip8_t *chip8, uint8_t x, uint8_t y);
bool calculate_pixel_value_for_regular(chip8_t *chip8, uint8_t x, uint8_t y);