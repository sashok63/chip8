#include "chip8.h"

void instruction_execution(chip8_t *chip8)
{
    bool carry_flag;

    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8->ram[chip8->PC + 1];
    chip8->PC += 2;

    switch (chip8->inst.opcode & 0xF000)
    {
        case 0x0000:
            switch (chip8->inst.opcode & 0x00FF)
            {
                //Opcode 00E0: Clear screen
                case 0x00E0:
                    memset(&chip8->gfx[0], false, sizeof(chip8->gfx));
                    chip8->draw_flag = true;
                    break; 

                //Opcode 00EE: Return from subroutine
                case 0x00EE:
                    chip8->PC = *--chip8->SP;
                    break;

                default:
                    fprintf(stderr, "Error instruction: 0x%X\n", chip8->inst.opcode);
                    break;
            }
        break;

        //Opcode 1NNN: Jump to address NNN
        case 0x1000:
            chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;

            chip8->PC = chip8->inst.NNN;
            break;

        //Opcode 2NNN: Calls subroutin at NNN
        case 0x2000:
            chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;

            *chip8->SP++ = chip8->PC;
            chip8->PC = chip8->inst.NNN;
            break;

        //Opcode 3XNN: Skip next instruction if VX == NN
        case 0x3000:
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

            if (chip8->V[chip8->inst.X] == chip8->inst.NN)
            {
                chip8->PC += 2;
            }
            break;

        //Opcode 4XNN: Skip next instruction if VX != NN
        case 0x4000:
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

            if (chip8->V[chip8->inst.X] != chip8->inst.NN)
            {
                chip8->PC += 2;
            }
            break;

        //Opcode 5XY0: Skip next instruction if VX == VY
        case 0x5000:
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

            if (chip8->V[chip8->inst.X] == chip8->V[chip8->inst.Y])
            {
                chip8->PC += 2;
            }
            break;

        //Opcode 6XNN: Set VX to NN    
        case 0x6000:
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

            chip8->V[chip8->inst.X] = chip8->inst.NN;
            break;

        //Opcode 7XNN: Adds VX to NN (carry flag not changed)
        case 0x7000:
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

            chip8->V[chip8->inst.X] += chip8->inst.NN;
            break;

        //Opcodes 8000
        case 0x8000:
            switch (chip8->inst.opcode & 0x000F)
            {
                //Opcode 8XY0: Sets VX to value of VY
                case 0x0000:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                        
                    chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y];
                    break;

                //Opcode 8XY1: Sets VX to VX | VY
                case 0x0001:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                        
                    chip8->V[chip8->inst.X] |= chip8->V[chip8->inst.Y]; 
                    chip8->V[0xF] = 0;
                    break;

                //Opcode 8XY2: Sets VX to VX & VY
                case 0x0002:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                        
                    chip8->V[chip8->inst.X] &= chip8->V[chip8->inst.Y]; 
                    chip8->V[0xF] = 0;
                    break;

                //Opcode 8XY3: Sets VX to VX ^ VY
                case 0x0003:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                        
                    chip8->V[chip8->inst.X] ^= chip8->V[chip8->inst.Y]; 
                    chip8->V[0xF] = 0;
                    break;

                //Opcode 8XY4: Adds VY to VX, VF is set to 1 when there's an overflow
                //             and to 0 when there is not
                case 0x0004:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    carry_flag = ((uint16_t)(chip8->V[chip8->inst.X] + chip8->V[chip8->inst.Y]) > 255);
                        
                    chip8->V[chip8->inst.X] += chip8->V[chip8->inst.Y];
                    chip8->V[0xF] = carry_flag;
                    break;

                //Opcode 8XY5: VY is subtracted from VX. VF is set to 0 when there's an
                //             underflow, and 1 when there is not
                case 0x0005:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    carry_flag = (chip8->V[chip8->inst.Y] <= chip8->V[chip8->inst.X]);
                        
                    chip8->V[chip8->inst.X] -= chip8->V[chip8->inst.Y];
                    chip8->V[0xF] = carry_flag;
                    break;

                //Opcode 8XY6: Stores the least significant bit of VX in VF
                //             and then shifts VX to the right by 1
                case 0x0006:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    carry_flag = chip8->V[chip8->inst.Y] & 1;

                    chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] >> 1;
                    chip8->V[0xF] = carry_flag;
                    break;

                //Opcode 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's an underflow
                //             and 1 when there is not
                case 0x0007:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    carry_flag = (chip8->V[chip8->inst.X] <= chip8->V[chip8->inst.Y]);

                    chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] - chip8->V[chip8->inst.X];
                    chip8->V[0xF] = carry_flag;
                    break;

                //Opcode 8XYE: Stores the most significant bit of VX in VF
                //             and then shifts VX to the left by 1
                case 0x000E:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    carry_flag = (chip8->V[chip8->inst.Y] & 0x80) >> 7;
                    
                    chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] << 1;
                    chip8->V[0xF] = carry_flag;
                    break;

                default:
                    fprintf(stderr, "Error instruction: 0x%X\n", chip8->inst.opcode);
                    break;
            }
            break;

        //Opcode 9XY0: Skips the next instruction if VX != VY
        case 0x9000:
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

            if (chip8->V[chip8->inst.X] != chip8->V[chip8->inst.Y])
            {
                chip8->PC += 2;
            }
            break;

        //Opcode ANNN: Sets I to the address NNN
        case 0xA000:
            chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
            
            chip8->I = chip8->inst.NNN;
            break;

        //Opcode BNNN: Jumps to the address NNN + V0
        case 0xB000:
            chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;

            chip8->PC = chip8->V[0] + chip8->inst.NNN;
            break;

        //Opcode CXNN: Sets VX to the result of a bitwise and
        //             operation on a random number (Typically: 0 to 255) and NN
        case 0xC000:
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

            chip8->V[chip8->inst.X] = (rand() % 256) & chip8->inst.NN;
            break;

        //Opcode DXYN: Draws a sprite at coordinate (VX, VY),
        //             VF is set to 1 if any screen pixels are flipped
        case 0xD000:
            chip8->inst.N = chip8->inst.opcode & 0x0F;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
            uint8_t x_coord = chip8->V[chip8->inst.X] % SCREEN_WIDTH;
            uint8_t y_coord = chip8->V[chip8->inst.Y] % SCREEN_HEIGHT;
            const uint8_t original_x = x_coord;
            chip8->V[0xF] = 0;

            for (uint8_t y = 0; y < chip8->inst.N; y++) {
                const uint8_t pixel_data = chip8->ram[chip8->I + y];
                x_coord = original_x;

                for (int8_t x = 7; x >= 0; x--) {
                    bool *pixel = &chip8->gfx[y_coord * SCREEN_WIDTH + x_coord];
                    const bool sprite_bit = (pixel_data & (1 << x));    

                    if (sprite_bit && *pixel) {
                        chip8->V[0xF] = 1;
                    }

                    *pixel ^= sprite_bit;

                    if (++x_coord >= SCREEN_WIDTH) {
                        break;
                    }
                }

                if (++y_coord >= SCREEN_HEIGHT) {
                    break;
                }
            } 
            chip8->draw_flag = true;
            break;


        case 0xE000:
            switch (chip8->inst.opcode & 0xF0FF)
            {
                //Opcode EX9E: Skips the next instruction if the key stored in VX is pressed
                case 0xE09E:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    if (chip8->keyboard[chip8->V[chip8->inst.X]])
                    {
                        chip8->PC += 2;
                    }
                    break;

                //Opcode EXA1: Skips the next instruction if the key stored in VX is not pressed
                case 0xE0A1:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    
                    if (!chip8->keyboard[chip8->V[chip8->inst.X]])
                    {
                        chip8->PC += 2;
                    }
                    break;

                default:
                    fprintf(stderr, "Error instruction: 0x%X\n", chip8->inst.opcode);
                    break;
            }
            break;

        case 0xF000:
            switch (chip8->inst.opcode & 0xF0FF)
            {
                //Opcode FX07: Sets VX to the value of the delay timer
                case 0xF007:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->V[chip8->inst.X] = chip8->delay_timer;
                    break;
            
                //Opcde FX0A: A key press is awaited, and then stored in VX
                case 0xF00A:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->key_pressed = false;
                    
                    for (uint8_t i = 0; i < NUM_KEYS; i++)
                    {
                        if (chip8->keyboard[i])
                        {
                            chip8->V[chip8->inst.X] = i;
                            chip8->key_pressed = true;
                            break;
                        }
                    }

                    if (!chip8->key_pressed)
                    {
                        chip8->PC -= 2;
                    }
                    break;

                //Opcode FX15: Sets the delay timer to VX
                case 0xF015:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->delay_timer = chip8->V[chip8->inst.X];
                    break;

                //Opcode FX18: Sets the sound timer to VX
                case 0xF018:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->sound_timer = chip8->V[chip8->inst.X];
                    break;

                //Opcode FX1E: Adds VX to I
                case 0xF01E:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->I += chip8->V[chip8->inst.X];
                    break;

                //Opcode FX29: Sets I to the location of the sprite for the character in VX
                //             Characters 0-F are represented by a 4x5 font
                case 0xF029:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->I = chip8->V[chip8->inst.X] * 0x5;
                    break;

                //Opcode FX33: Stores the binary-coded decimal representation of VX,
                //             with the hundreds digit in memory at location in I,
                //             the tens digit at location I+1, and the ones digit at location I+2
                case 0xF033:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->ram[chip8->I] = chip8->V[chip8->inst.X] / 100;
                    chip8->ram[chip8->I + 1] = (chip8->V[chip8->inst.X] / 10) % 10;
                    chip8->ram[chip8->I + 2] = (chip8->V[chip8->inst.X] % 100) % 10;
                    break;

                case 0xF055:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    for (uint8_t i = 0; i <= chip8->inst.X; i++)
                    {
                        chip8->ram[chip8->I++] = chip8->V[i];
                    }
                    break;

                case 0xF065:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    for (uint8_t i = 0; i <= chip8->inst.X; i++)
                    {
                        chip8->V[i] = chip8->ram[chip8->I++];
                    }
                    break;
                    
                default:
                    fprintf(stderr, "Error instruction: 0x%X\n", chip8->inst.opcode);
                    break;
            }
            break;

        default:
            fprintf(stderr, "Error instruction: 0x%X\n", chip8->inst.opcode);
            break;
    }
}