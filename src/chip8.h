/*
 * Copyright (c) 2025 Yoga. All rights reserved.
 *
 * This work is licensed under the terms of the MIT license.  
 * For a copy, see <https://opensource.org/licenses/MIT>.
*/

#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_STACK_SIZE 16
#define CHIP8_KEY_STACK_SIZE 32
#define CHIP8_VRAM_SIZE 256
#define CHIP8_FLAG_REGISTER 15
#define CHIP8_NUMBER_REGISTERS 16
#define CHIP8_START_LOCATION 512
#define CHIP8_MAX_ROM_SIZE CHIP8_MEMORY_SIZE - CHIP8_START_LOCATION

typedef unsigned char CHIP8_register_address;
typedef unsigned int CHIP8_address;
typedef unsigned char CHIP8_vram_address;
typedef unsigned char CHIP8_word;
typedef int CHIP8_dword;
typedef unsigned long int CHIP8_size;

typedef enum CHIP8_key {
  CHIP8_key_none = 0xFF,
  CHIP8_key_0 = 0x0,
  CHIP8_key_1 = 0x1,
  CHIP8_key_2 = 0x2,
  CHIP8_key_3 = 0x3,
  CHIP8_key_4 = 0x4,
  CHIP8_key_5 = 0x5,
  CHIP8_key_6 = 0x6,
  CHIP8_key_7 = 0x7,
  CHIP8_key_8 = 0x8,
  CHIP8_key_9 = 0x9,
  CHIP8_key_A = 0xA,
  CHIP8_key_B = 0xB,
  CHIP8_key_C = 0xC,
  CHIP8_key_D = 0xD,
  CHIP8_key_E = 0xE,
  CHIP8_key_F = 0xF,
} CHIP8_key;

typedef enum CHIP8_errors {
  CHIP8_Ok,
  CHIP8_FileNotFound,
  CHIP8_ROMTooBig,
  CHIP8_MemoryAllocationFailed,
  CHIP8_MemoryAddressNotFound,
  CHIP8_RegisterNotFound,
  CHIP8_InvalidArguments,
  CHIP8_StackOverflow,
  CHIP8_NoCall,
  Chip8_InvalidOpcode
} CHIP8_errors;

typedef struct CHIP8_chip8 {
  CHIP8_word* memory;

  // NOTE:
  // VRAM is a data structure that contains pixels in the display stored in a 1-dimensional array (kinda).
  // Example :
  // Index:             0                        1
  // VRAM:    0  0  0  0  0  0  0  0   0  0  0  0  0  0  0  0
  //          |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |
  // Nth-bit: 7  6  5  4  3  2  1  0  15 14 13 12 11 10  9  8
  // If you want to access pixels in x and y. Then the calculation is required:
  // I (Index): (y * 8) + (x / 8)
  // N (Nth bit): x % 7
  // Why? Because I want to try to make a small and efficient screen buffer.
  CHIP8_word* vram;
  CHIP8_word dt, st;
  CHIP8_address pc;
  CHIP8_dword i, sp, kp;
  CHIP8_word r[CHIP8_NUMBER_REGISTERS];
  
  CHIP8_address stack[CHIP8_STACK_SIZE];
  CHIP8_word wait;
  CHIP8_key keys[CHIP8_KEY_STACK_SIZE];
} CHIP8_chip8;

void CHIP8_init(CHIP8_chip8* chip8);
CHIP8_key CHIP8_kpeek(CHIP8_chip8* chip8);
CHIP8_key CHIP8_kpop(CHIP8_chip8* chip8);
void CHIP8_kpush(CHIP8_chip8* chip8, CHIP8_key key);
void CHIP8_cls(CHIP8_chip8* chip8);
CHIP8_errors CHIP8_jmp(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_call(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_ret(CHIP8_chip8* chip8);
CHIP8_errors CHIP8_eq1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word b);
CHIP8_errors CHIP8_eq2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_neq1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word b);
CHIP8_errors CHIP8_neq2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_addr1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word b);
CHIP8_errors CHIP8_addr2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_setr1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word byte);
CHIP8_errors CHIP8_setr2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_or(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_and(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_xor(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_sub(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_subn(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_shr(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_shl(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_seti(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_chseti(CHIP8_chip8* chip8, CHIP8_word ch);
CHIP8_errors CHIP8_rjmp(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_rand(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word b);
CHIP8_errors CHIP8_draw(CHIP8_chip8* chip8, CHIP8_register_address rx, CHIP8_register_address ry, CHIP8_word n);
CHIP8_errors CHIP8_waitkey(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_skp(CHIP8_chip8* chip8, CHIP8_register_address key);
CHIP8_errors CHIP8_sknp(CHIP8_chip8* chip8, CHIP8_register_address key);
CHIP8_errors CHIP8_setdt(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_setst(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_addi(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_bcd(CHIP8_chip8* chip8, CHIP8_word regAddr);
CHIP8_errors CHIP8_memtor(CHIP8_chip8* chip8, CHIP8_word r);
CHIP8_errors CHIP8_rtomem(CHIP8_chip8* chip8, CHIP8_word r);
CHIP8_dword CHIP8_getop(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_cycle(CHIP8_chip8* chip8);
CHIP8_errors CHIP8_loadrom(CHIP8_chip8* chip8, CHIP8_word* rom, CHIP8_size n);
CHIP8_errors CHIP8_loadfile(CHIP8_chip8* chip8, const char* path);
