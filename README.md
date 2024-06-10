# CHIP8
* CHIP8, SUPERCHIP implementation in C with SDL2 library

## Still in work
* Mod for XOCHIP

## Building 
```bash
make
```

## Usage
```bash
./chip8 <rom.ch8> [-s/-xo] - on Linux
.\chip8 <rom.ch8> [-s/-xo] - on Windows

-s for SUPERCHIP
-xo for XOCHIP
```

SPACE - Pause/Resume

LALT - Reload rom

ESC - Exit

## Keyboard
| Original keyboard | Equivalent |
| -------------   | ------------- |
|  1	2	3	C       | 1 2 3 4      |  
|  4	5	6	D       |  Q W E R      |
|  7	8	9	E       |  A S D F      |
|  A	0	B	F       |  Z X C V       |

## Dependencies
* SDL2 library (https://www.libsdl.org)
