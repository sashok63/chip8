#include "chip8.h"

void instruction_execution(chip8_t *chip8)
{
    bool carry_flag = false;
    uint8_t screen_height = 0;
    uint8_t screen_width = 0;

    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8->ram[chip8->PC + 1];
    chip8->PC += 2;

    switch (chip8->inst.opcode & 0xF000)
    {
        case 0x0000:
            switch (chip8->inst.opcode & 0x00FF)
            {
                //Opcode 00E0: Clear screen
                case 0x00E0:
                    memset(&chip8->gfx, 0, sizeof(chip8->gfx));
                    chip8->draw_flag = true;
                    break; 

                //Opcode 00EE: Return from subroutine
                case 0x00EE:
                    assert(chip8->SP > 0);
                    chip8->PC = chip8->stack[chip8->SP--];
                    break;

                //Opcode 00FF: Enable 128x64 high-resolution graphics mode
                case 0x00FF:
                    chip8->hr.HiRes = true;
                    chip8->draw_flag = true;
                    break;

                //Opcode 00FE: Disable high resolution graphics mode and return to 64x32
                case 0x00FE:
                    chip8->hr.HiRes = false;
                    chip8->draw_flag = true;
                    break;

                //Opcode 00FB: Scroll the display right by 4 pixels
                case 0x00FB:
                    chip8->inst.N = chip8->inst.opcode & 0x0F;

                    screen_height = chip8->hr.HiRes ? SCREEN_HEIGHT_S : SCREEN_HEIGHT;
                    screen_width = chip8->hr.HiRes ? SCREEN_WIDTH_S : SCREEN_WIDTH;

                    for (uint8_t y = 0; y < screen_height; y++)
                    {
                        for (int8_t x = screen_width - 1; x >= 4; x--)
                        {
                            chip8->gfx[y * screen_width + x] = chip8->gfx[y * screen_width + (x - 4)];
                        }

                        for (int x = 0; x < 4; x++)
                        {
                            chip8->gfx[y * screen_width + x] = 0;
                        }
                    }

                    chip8->draw_flag = true;
                    break;

                //Opcode 00FC: Scroll the display left by 4 pixels
                case 0x00FC:
                    chip8->inst.N = chip8->inst.opcode & 0x0F;

                    screen_height = chip8->hr.HiRes ? SCREEN_HEIGHT_S : SCREEN_HEIGHT;
                    screen_width = chip8->hr.HiRes ? SCREEN_WIDTH_S : SCREEN_WIDTH;

                    for (uint8_t y = 0; y < screen_height; y++)
                    {
                        for (int8_t x = 0; x < screen_width - 4; x++)
                        {
                            chip8->gfx[y * screen_width + x] = chip8->gfx[y * screen_width + (x + 4)];
                        }

                        for (int x = screen_width - 4; x < screen_width; x++)
                        {
                            chip8->gfx[y * screen_width + x] = 0;
                        }
                    }

                    chip8->draw_flag = true;
                    break;

                //Opcode 00CN: Scroll the display down by 0 to 15 pixels
                case 0x00C0:
                case 0x00C1:
                case 0x00C2:
                case 0x00C3:
                case 0x00C4:
                case 0x00C5:
                case 0x00C6:
                case 0x00C7:
                case 0x00C8:
                case 0x00C9:
                case 0x00CA:
                case 0x00CB:
                case 0x00CC:
                case 0x00CD:
                case 0x00CE:
                case 0x00CF:
                    chip8->inst.N = chip8->inst.opcode & 0x0F;

                    screen_height = chip8->hr.HiRes ? SCREEN_HEIGHT_S : SCREEN_HEIGHT;
                    screen_width = chip8->hr.HiRes ? SCREEN_WIDTH_S : SCREEN_WIDTH;

                    for (int y = screen_height - 1; y >= chip8->inst.N; y--)
                    {
                        for (int x = 0; x < screen_width; x++)
                        {
                            chip8->gfx[y * screen_width + x] = chip8->gfx[(y - chip8->inst.N) * screen_width + x];
                        }
                    }
                    for (int y = 0; y < chip8->inst.N; y++)
                    {
                        for (int x = 0; x < screen_width; x++)
                        {
                            chip8->gfx[y * screen_width + x] = 0;
                        }
                    }

                    chip8->draw_flag = true;
                    break;

                //Opcode 00FD: Exit the interpreter (halt the program)
                case 0x00FD:
                    fprintf(stdout, "Program halted by 00FD instruction.\n");
                    SDL_Quit();
                    break;

                default:
                    fprintf(stderr, "Error: Undefined instruction encountered.\n");
                    fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
                    fprintf(stderr, "Binary: ");
                    for (int8_t i = 15; i >= 0; i--)
                    {
                        fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
                    }
                    fprintf(stderr, "\n");
                    fprintf(stderr, "PC (Program Counter): 0x%X\n", chip8->PC);
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

            assert(chip8->SP < 15);
            chip8->stack[++chip8->SP] = chip8->PC;
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
                    
                    if (chip8->mod.CHIP == true)
                    {
                        chip8->V[0xF] = 0;
                    }
                    break;

                //Opcode 8XY2: Sets VX to VX & VY
                case 0x0002:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                        
                    chip8->V[chip8->inst.X] &= chip8->V[chip8->inst.Y]; 
                    
                    if (chip8->mod.CHIP == true)
                    {
                        chip8->V[0xF] = 0;
                    }
                    break;

                //Opcode 8XY3: Sets VX to VX ^ VY
                case 0x0003:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                        
                    chip8->V[chip8->inst.X] ^= chip8->V[chip8->inst.Y];

                    if (chip8->mod.CHIP == true)
                    {
                        chip8->V[0xF] = 0;
                    }
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

                    if (chip8->mod.CHIP == true)
                    {
                        carry_flag = chip8->V[chip8->inst.Y] & 1;
                        chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] >> 1;    
                    }
                    else if (chip8->mod.SUPERCHIP == true)
                    {
                        carry_flag = chip8->V[chip8->inst.X] & 1;
                        chip8->V[chip8->inst.X] >>= 1;   
                    }
                    
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
                    
                    if (chip8->mod.CHIP == true)
                    {
                        carry_flag = (chip8->V[chip8->inst.Y] & 0x80) >> 7;
                        chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] << 1;    
                    }
                    else if (chip8->mod.SUPERCHIP == true)
                    {
                        carry_flag = (chip8->V[chip8->inst.X] & 0x80) >> 7;
                        chip8->V[chip8->inst.X] <<= 1;    
                    }
                    
                    chip8->V[0xF] = carry_flag;
                    break;

                default:
                    fprintf(stderr, "Error: Undefined instruction encountered.\n");
                    fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
                    fprintf(stderr, "Binary: ");
                    for (int8_t i = 15; i >= 0; i--)
                    {
                        fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
                    }
                    fprintf(stderr, "\n");
                    fprintf(stderr, "PC (Program Counter): 0x%X\n", chip8->PC);
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
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

            if (chip8->mod.CHIP == true)
            {
                chip8->PC = chip8->V[0] + chip8->inst.NNN;
            }
            else if (chip8->mod.SUPERCHIP == true)
            {
                chip8->PC = chip8->V[chip8->inst.X] + chip8->inst.NNN;   
            }
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
            chip8->inst.N = chip8->inst.opcode & 0x000F;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
            chip8->V[0xF] = 0;
            
            if (chip8->mod.CHIP == true)
            {
                uint8_t x_coord = chip8->V[chip8->inst.X] % SCREEN_WIDTH;
                uint8_t y_coord = chip8->V[chip8->inst.Y] % SCREEN_HEIGHT;
                const uint8_t original_x = x_coord;

                for (uint8_t y = 0; y < chip8->inst.N; y++)
                {
                    const uint8_t pixel_data = chip8->ram[chip8->I + y];
                    x_coord = original_x;

                    for (int8_t x = 7; x >= 0; x--)
                    {
                        uint8_t *pixel = &chip8->gfx[y_coord * SCREEN_WIDTH + x_coord];
                        const uint8_t sprite_bit = (pixel_data & (1 << x)) != 0;    

                        if (sprite_bit && *pixel)
                        {
                            chip8->V[0xF] = 1;
                        }

                        *pixel ^= sprite_bit;

                        if (++x_coord >= SCREEN_WIDTH)
                        {
                            break;
                        }
                    }

                    if (++y_coord >= SCREEN_HEIGHT)
                    {
                        break;
                    }
                }

                chip8->draw_flag = true;
            }
            else if (chip8->mod.SUPERCHIP == true)
            {
                for (uint8_t byte = 0; byte < chip8->inst.N; byte++)
                {
                    const uint8_t sprite_data = chip8->ram[chip8->I + byte];
                    uint8_t x_coord = chip8->V[chip8->inst.X];
                    uint8_t y_coord = chip8->V[chip8->inst.Y] + byte;
                    
                    if (chip8->hr.HiRes == true)
                    {
                        for (uint8_t bit = 0; bit < 8; bit++)
                        {
                            uint8_t x = (x_coord + bit) % SCREEN_WIDTH_S;
                            uint8_t y = y_coord % SCREEN_HEIGHT_S;

                            uint8_t sprite_pixel = sprite_data & (0x80 >> bit);

                            if (sprite_pixel)
                            {
                                if (chip8->gfx[x + y * SCREEN_WIDTH_S])
                                {
                                    chip8->V[0xF] = 1;
                                }

                                chip8->gfx[x + y * SCREEN_WIDTH_S] ^= 1;
                            }
                        }
                    }
                    else if (chip8->hr.HiRes == false)
                    {
                        for (uint8_t bit = 0; bit < 8; bit++)
                        {
                            uint8_t x = (x_coord + bit) % SCREEN_WIDTH;
                            uint8_t y = y_coord % SCREEN_HEIGHT;

                            uint8_t sprite_pixel = sprite_data & (0x80 >> bit);
            
                            if (sprite_pixel)
                            {
                                if (chip8->gfx[x + y * SCREEN_WIDTH])
                                {
                                    chip8->V[0xF] = 1;
                                }
            
                                chip8->gfx[x + y * SCREEN_WIDTH] ^= 1;
                            }
                        }
                    }
                    else
                    {
                        fprintf(stderr, "0xDXYN insruction error 1\n");
                    }
                }
                chip8->draw_flag = true;
            }
            else
            {
                fprintf(stderr, "0xDXYN insruction error 2\n");
            }
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
                    fprintf(stderr, "Error: Undefined instruction encountered.\n");
                    fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
                    fprintf(stderr, "Binary: ");
                    for (int8_t i = 15; i >= 0; i--)
                    {
                        fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
                    }
                    fprintf(stderr, "\n");
                    fprintf(stderr, "PC (Program Counter): 0x%X\n", chip8->PC);
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

                    if (chip8->mod.CHIP == true)
                    {
                        chip8->V[0xF] = chip8->I > 0xFFF;
                    }
                    break;

                //Opcode FX29: Sets I to the location of the sprite for the character in VX
                //             Characters 0-F are represented by a 4x5 font
                case 0xF029:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->I = FONT_START + chip8->V[chip8->inst.X] * 5;
                    break;

                //Opcode FX30: Sets I to the location of the extended sprite for the character in VX
                case 0xF030:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->I = EXTENDED_FONT_START + chip8->V[chip8->inst.X] * 10;
                    break;

                //Opcode FX33: Stores the binary-coded decimal representation of VX,
                //             with the hundreds digit in memory at location in I,
                //             the tens digit at location I+1, and the ones digit at location I+2
                case 0xF033:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    chip8->ram[chip8->I + 0] = (chip8->V[chip8->inst.X] / 100) % 10;
                    chip8->ram[chip8->I + 1] = (chip8->V[chip8->inst.X] / 10) % 10;
                    chip8->ram[chip8->I + 2] = (chip8->V[chip8->inst.X] / 1) % 10;
                    break;

                //Opcode FX55: Stores from V0 to VX (including VX) in memory,
                //             starting at address I. The offset from I is increased
                //             by 1 for each value written, but I itself is left unmodified.
                case 0xF055:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    if (chip8->mod.CHIP == true)
                    {
                        for (uint8_t i = 0; i <= chip8->inst.X; ++i)
                        {
                            chip8->ram[chip8->I++] = chip8->V[i];
                        }

                        chip8->I += chip8->inst.X + 1;
                    }
                    else if (chip8->mod.SUPERCHIP == true)
                    {
                        for (uint8_t i = 0; i <= chip8->inst.X; ++i)
                        {
                            chip8->ram[chip8->I + i] = chip8->V[i];
                        }

                        chip8->I += chip8->inst.X;
                    }
                    break;

                //Opcode FX65: Fills from V0 to VX (including VX) with values from memory,
                //             starting at address I. The offset from I is increased by 1
                //             for each value read, but I itself is left unmodified.
                case 0xF065:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;

                    if (chip8->mod.CHIP == true)
                    {
                        for (uint8_t i = 0; i <= chip8->inst.X; ++i)
                        {
                            chip8->V[i] = chip8->ram[chip8->I++];
                        }

                        chip8->I += chip8->inst.X + 1;
                    }
                    else if (chip8->mod.SUPERCHIP == true)
                    {
                        for (uint8_t i = 0; i <= chip8->inst.X; ++i)
                        {
                            chip8->V[i] = chip8->ram[chip8->I + i];
                        }

                        chip8->I += chip8->inst.X;
                    }
                    break;

                //Opcode FX75: Stores V0 to VX (including VX) in RPL user flags (X <= 7)
                case 0xF075:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    assert(chip8->inst.X <= 7);
                    memcpy(chip8->RPL, chip8->V, chip8->inst.X + 1);
                    break;

                //Opcode FX85: Fills V0 to VX (including VX) with values from RPL user flags (X <= 7)
                case 0xF085:
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;   
                    assert(chip8->inst.X <= 7);
                    memcpy(chip8->V, chip8->RPL, chip8->inst.X + 1);
                    break;
                    
                default:
                    fprintf(stderr, "Error: Undefined instruction encountered.\n");
                    fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
                    fprintf(stderr, "Binary: ");
                    for (int8_t i = 15; i >= 0; i--)
                    {
                        fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
                    }
                    fprintf(stderr, "\n");
                    fprintf(stderr, "PC (Program Counter): 0x%X\n", chip8->PC);
                    break;
            }
            break;

        default:
            fprintf(stderr, "Error: Undefined instruction encountered.\n");
            fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
            fprintf(stderr, "Binary: ");
            for (int8_t i = 15; i >= 0; i--)
            {
                fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
            }
            fprintf(stderr, "\n");
            fprintf(stderr, "PC (Program Counter): 0x%X\n", chip8->PC);
            break;
    }
}



void db_instruction_execution(chip8_t *chip8)
{
    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8->ram[chip8->PC + 1];
    printf("Executing instruction: 0x%X at PC: 0x%X\n", chip8->inst.opcode, chip8->PC);

    printf("PC: %04X, I: %04X\n", chip8->PC, chip8->I);

    switch (chip8->inst.opcode & 0xF000)
    {
        case 0x0000:
            switch (chip8->inst.opcode & 0x00FF)
            {
                case 0x00E0: //Clear screen
                    printf("Opcode 00E0: Clear screen\n");
                    break; 

                case 0x00EE: //Return from subroutine
                    printf("Opcode 00EE: Return from subroutine\n");
                    break;

                case 0x00FF: //Enable 128x64 high-resolution graphics mode
                    printf("Opcode 00FF: Enable 128x64 high-resolution graphics mode\n");
                    break;

                case 0x00FE: //Disable high resolution graphics mode and return to 64x32
                    printf("Opcode 00FE: Disable high resolution graphics mode\n");
                    break;

                case 0x00FD: //Turn off SCHIP mode
                    printf("Opcode 00FD: Turn off SCHIP mode\n");
                    break;

                case 0x00C0:
                case 0x00C1:
                case 0x00C2:
                case 0x00C3:
                case 0x00C4:
                case 0x00C5:
                case 0x00C6:
                case 0x00C7:
                case 0x00C8:
                case 0x00C9:
                case 0x00CA:
                case 0x00CB:
                case 0x00CC:
                case 0x00CD:
                case 0x00CE:
                case 0x00CF: //Scroll the display down by 0 to 15 pixels
                    chip8->inst.N = chip8->inst.opcode & 0x0F;
                    printf("Opcode 00CN: Scroll the display down by %d pixels\n", chip8->inst.N);
                    break;

                case 0x00FB: //Scroll the display right by 4 pixels
                    printf("Opcode 00FB: Scroll the display right by 4 pixels\n");
                    break;

                case 0x00FC: //Scroll the display left by 4 pixels
                    printf("Opcode 00FC: Scroll the display left by 4 pixels\n");
                    chip8->draw_flag = true;
                    break;

                default:
                    fprintf(stderr, "Error: Undefined instruction encountered.\n");
                    fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
                    for (int8_t i = 15; i >= 0; i--)
                    {
                        fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
                    }
                    fprintf(stderr, "\nPC (Program Counter): 0x%X\n", chip8->PC);
                    break;
            }
            break;

        case 0x1000: //Jump to address NNN
            chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
            printf("Opcode 1000: Jump to address 0x%X\n", chip8->inst.NNN);
            break;

        case 0x2000: //Calls subroutine at NNN
            chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
            printf("Opcode 2000: Call subroutine at 0x%X\n", chip8->inst.NNN);
            break;

        case 0x3000: //Skip next instruction if VX == NN
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            printf("Opcode 3000: Skip next instruction if V[%d] == 0x%X\n", chip8->inst.X, chip8->inst.NN);
            break;

        case 0x4000: //Skip next instruction if VX != NN
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            printf("V[0]: %02X\n", chip8->V[0]);
            printf("Opcode 4000: Skip next instruction if V[%d] != 0x%X\n", chip8->inst.X, chip8->inst.NN);
            break;

        case 0x5000: //Skip next instruction if VX == VY
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
            printf("Opcode 5000: Skip next instruction if V[%d] == V[%d]\n", chip8->inst.X, chip8->inst.Y);
            break;

        case 0x6000: //Set VX to NN    
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            printf("Opcode 6000: Set V[%d] to 0x%X\n", chip8->inst.X, chip8->inst.NN);
            break;

        case 0x7000: //Add NN to VX (carry flag not changed)
            chip8->inst.NN = chip8->inst.opcode & 0x0FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            printf("Opcode 7000: Add 0x%X to V[%d]\n", chip8->inst.NN, chip8->inst.X);
            break;

        case 0x8000:
            switch (chip8->inst.opcode & 0x000F)
            {
                case 0x0000: //Set VX to VY
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    printf("Opcode 8000: Set V[%d] to V[%d]\n", chip8->inst.X, chip8->inst.Y);
                    break;

                case 0x0001: //Set VX to VX or VY
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    printf("Opcode 8001: Set V[%d] to V[%d] or V[%d]\n", chip8->inst.X, chip8->inst.X, chip8->inst.Y);
                    break;

                case 0x0002: //Set VX to VX and VY
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    printf("Opcode 8002: Set V[%d] to V[%d] and V[%d]\n", chip8->inst.X, chip8->inst.X, chip8->inst.Y);
                    break;

                case 0x0003: //Set VX to VX xor VY
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    printf("Opcode 8003: Set V[%d] to V[%d] xor V[%d]\n", chip8->inst.X, chip8->inst.X, chip8->inst.Y);
                    break;

                case 0x0004: //Add VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    printf("Opcode 8004: Add V[%d] to V[%d]\n", chip8->inst.Y, chip8->inst.X);
                    break;

                case 0x0005: //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    printf("Opcode 8005: Subtract V[%d] from V[%d]\n", chip8->inst.Y, chip8->inst.X);
                    break;

                case 0x0006: //Store the least significant bit of VX in VF and then shifts VX to the right by 1
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode 8006: Shift V[%d] right by 1\n", chip8->inst.X);
                    break;

                case 0x0007: //Set VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
                    printf("Opcode 8007: Set V[%d] to V[%d] - V[%d]\n", chip8->inst.X, chip8->inst.Y, chip8->inst.X);
                    break;

                case 0x000E: //Store the most significant bit of VX in VF and then shifts VX to the left by 1
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode 800E: Shift V[%d] left by 1\n", chip8->inst.X);
                    break;

                default:
                    fprintf(stderr, "Error: Undefined instruction encountered.\n");
                    fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
                    for (int8_t i = 15; i >= 0; i--)
                    {
                        fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
                    }
                    fprintf(stderr, "\nPC (Program Counter): 0x%X\n", chip8->PC);
                    break;
            }
            break;

        case 0x9000: //Skip next instruction if VX != VY
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
            printf("Opcode 9000: Skip next instruction if V[%d] != V[%d]\n", chip8->inst.X, chip8->inst.Y);
            break;

        case 0xA000: //Set I to the address NNN
            chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
            printf("Opcode A000: Set I to the address 0x%X\n", chip8->inst.NNN);
            break;

        case 0xB000: //Jump to address NNN plus V0
            chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
            printf("Opcode B000: Jump to address 0x%X + V[0]\n", chip8->inst.NNN);
            break;

        case 0xC000: //Set VX to a random number and NN
            chip8->inst.NN = chip8->inst.opcode & 0x00FF;
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            printf("Opcode C000: Set V[%d] to random number & 0x%X\n", chip8->inst.X, chip8->inst.NN);
            break;

        case 0xD000: //Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I
            chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
            chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;
            chip8->inst.N = (chip8->inst.opcode & 0x000F);
            printf("Opcode D000: Draw sprite at V[%d], V[%d] with %d bytes of sprite data\n", 
                    chip8->inst.X, chip8->inst.Y, chip8->inst.N);
            break;

        case 0xE000:
            switch (chip8->inst.opcode & 0x00FF)
            {
                case 0x009E: //Skip next instruction if key with the value of VX is pressed
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode E09E: Skip next instruction if key V[%d] (%d) is pressed\n", 
                            chip8->inst.X, chip8->V[chip8->inst.X]);
                    break;

                case 0x00A1: //Skip next instruction if key with the value of VX is not pressed
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode E0A1: Skip next instruction if key V[%d] (%d) is not pressed\n", 
                            chip8->inst.X, chip8->V[chip8->inst.X]);
                    break;

                default:
                    fprintf(stderr, "Error: Undefined instruction encountered.\n");
                    fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
                    for (int8_t i = 15; i >= 0; i--)
                    {
                        fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
                    }
                    fprintf(stderr, "\nPC (Program Counter): 0x%X\n", chip8->PC);
                    break;
            }
            break;

        case 0xF000:
            switch (chip8->inst.opcode & 0x00FF)
            {
                case 0x0007: //Set VX to the value of the delay timer
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Set V[%d] to delay timer value: %d\n", chip8->inst.X, chip8->delay_timer);
                    break;

                case 0x000A: //Wait for a key press, store the value of the key in VX
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F00A: Wait for a key press and store it in V[%d]\n", chip8->inst.X);
                    break;

                case 0x0015: //Set the delay timer to VX
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Set Delay Timer to V[%X]: %d\n", chip8->inst.X, chip8->V[chip8->inst.X]);
                    break;

                case 0x0018: //Set the sound timer to VX
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F018: Set sound timer to V[%d]\n", chip8->inst.X);
                    break;

                case 0x001E: //Add VX to I. VF is not affected
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F01E: Add V[%d] (%d) to I, resulting I: 0x%03X\n", 
                           chip8->inst.X, chip8->V[chip8->inst.X], chip8->I);
                    break;

                case 0x0029: //Set I to the location of the sprite for the character in VX.
                             //Characters 0-F (in hexadecimal) are represented by a 4x5 font
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F029: Set I to sprite location for character in V[%d]\n", chip8->inst.X);
                    break;
                
                case 0x0030: //Set I to the location of the sprite for the character in VX.
                             //Characters 0-F (in hexadecimal) are represented by a 4x5 font
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F030: Set I to extended sprite location for character in V[%d]\n", chip8->inst.X);
                    break;

                case 0x0033: //Store the Binary-coded decimal representation of VX at the addresses I, I+1, and I+2
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F033: Store BCD representation of V[%d] at addresses I, I+1, and I+2\n", chip8->inst.X);
                    break;

                case 0x0055: //Store V0 to VX in memory starting at address I
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F055: Store V[0] to V[%d] in memory starting at address I\n", chip8->inst.X);
                    break;

                case 0x0065: //Fill V0 to VX with values from memory starting at address I
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    for (int i = 0; i <= chip8->inst.X; i++)
                    {
                        printf("Memory[%04X]: %02X\n", chip8->I + i, chip8->ram[chip8->I + i]);
                    }
                    printf("Opcode F065: Fill V[0] to V[%d] with values from memory starting at address I\n", chip8->inst.X);
                    break;

                case 0x0075: //Stores V0 to VX (including VX) in RPL user flags (X <= 7)
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F075: Fill V[0] to V[%d] (including VX) in RPL user flags (X <= 7)\n", chip8->inst.X);
                    break;

                
                case 0x0085: //Fills V0 to VX (including VX) with values from RPL user flags (X <= 7)
                    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
                    printf("Opcode F075: Fill V[0] to V[%d] (including VX) with values from RPL user flags (X <= 7)\n", chip8->inst.X);
                    break;

                default:
                    fprintf(stderr, "Error: Undefined instruction encountered.\n");
                    fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
                    for (int i = 15; i >= 0; i--)
                    {
                        fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
                    }
                    fprintf(stderr, "\nPC (Program Counter): 0x%X\n", chip8->PC);
                    break;
            }
            break;

        default:
            fprintf(stderr, "Error: Undefined instruction encountered.\n");
            fprintf(stderr, "Opcode: 0x%X\n", chip8->inst.opcode);
            for (int8_t i = 15; i >= 0; i--)
            {
                fprintf(stderr, "%d", (chip8->inst.opcode >> i) & 1);
            }
            fprintf(stderr, "\nPC (Program Counter): 0x%X\n", chip8->PC);
            break;
    }
}
