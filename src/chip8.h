#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <SDL2/SDL.h>

#define TIMER_MAX 255
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define NUM_REGS 16
#define STACK_SIZE 12
#define RAM_SIZE 4096
#define START_ADDRESS 512
#define NUM_KEYS 16
#define SCALE 20

typedef enum {
    QUIT,
    RUNNING,
    PAUSED
} chip8_state_t;

typedef enum {
    CHIP,
    SUPERCHIP,
    XOCHIP
} chip8_mods_t;

typedef struct {
    uint16_t opcode;
    uint16_t NNN;
    uint8_t NN;
    uint8_t N;
    uint8_t X;
    uint8_t Y;
} instruction_t;

typedef struct {
    chip8_state_t state;
    chip8_mods_t mod;
    instruction_t inst;
    uint8_t ram[RAM_SIZE];
    uint16_t stack[STACK_SIZE];
    uint8_t V[NUM_REGS];          //(0 - 14), carry flag (15)
    uint16_t I;             //Index register
    uint16_t PC;            //Program counter
    uint16_t *SP;            //Stack pointer
    bool gfx[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint32_t gfx_color[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool keyboard[NUM_KEYS];
    bool key_pressed;
    bool draw_flag;
} chip8_t;

static const uint8_t keyboard_map[NUM_KEYS] = {
    SDLK_x, // 0
    SDLK_1, // 1
    SDLK_2, // 2
    SDLK_3, // 3
    SDLK_q, // 4
    SDLK_w, // 5
    SDLK_e, // 6
    SDLK_a, // 7
    SDLK_s, // 8
    SDLK_d, // 9
    SDLK_z, // A
    SDLK_c, // B
    SDLK_4, // C
    SDLK_r, // D
    SDLK_f, // E
    SDLK_v  // F
};

void load_rom(chip8_t *chip8, const char *rom_name);
void system_init(chip8_t *chip8, const char *mod);
void keyboard(chip8_t *chip8, const char *mod, const char *rom_file);
void instruction_execution(chip8_t *chip8);