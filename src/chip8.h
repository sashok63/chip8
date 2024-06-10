#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <SDL2/SDL.h>

#define TIMER_MAX 255
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 640
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH_S 128
#define SCREEN_HEIGHT_S 64
#define SCALE 20
#define SCALE_S 10
#define NUM_REGS 16
#define NUM_RPL 8
#define STACK_SIZE 12
#define RAM_SIZE 4096
#define START_ADDRESS 512
#define FONT_START 80
#define EXTENDED_FONT_START 160
#define NUM_KEYS 16

typedef enum {
    QUIT,
    RUNNING,
    PAUSED
} chip8_state_t;

typedef struct {
    bool CHIP;
    bool SUPERCHIP;
    bool XOCHIP;
} chip8_mods_t;

typedef struct {
    bool HiRes;
} chip8_hires_t;

typedef struct {
    uint16_t opcode;
    uint16_t NNN;
    uint8_t NN;
    uint8_t N;
    uint8_t X;
    uint8_t Y;
} instruction_t;

typedef struct chip8_t {
    chip8_state_t state;
    chip8_mods_t mod;
    instruction_t inst;
    chip8_hires_t hr;
    uint8_t ram[RAM_SIZE];
    uint16_t stack[STACK_SIZE];
    uint8_t V[NUM_REGS];            //(0 - 14), carry flag (15)
    uint8_t RPL[NUM_RPL];           //Additional general-purpose registers similar to the V
    uint16_t I;                   
    uint16_t PC;                    
    uint8_t SP;                 
    uint8_t gfx[SCREEN_WIDTH_S * SCREEN_HEIGHT_S];
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool keyboard[NUM_KEYS];
    bool key_pressed;
    bool wait_to_key;
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
void db_instruction_execution(chip8_t *chip8);
void handle_undef_inst(chip8_t *chip8);